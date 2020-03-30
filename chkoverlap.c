/*
 *	chkover.c
 *
 *  https://github.com/SmallRoomLabs/chkoverlap
 *
 *	(C) Copyright 2020 by Mats Engstrom
 *  Released under the MIT license.
 *
 * This program analyzes (and optionally dumps) a BIN or RIM file
 * for overlapping areas. It was created to analyze the files from
 * palbart that doesn't warn when data and/or code areas run into
 * eachother.
 *
 * 2020-03-30 v1.00 MEM     First Release
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *progname = "chkoverlap";
char *version = "1.00";

void usage() {
    fprintf(stderr, "\nUsage:  %s [options] filename\n\n", progname);
    fprintf(stderr, "where [options] are optional parameters chosen from:\n");
    fprintf(stderr, "    -b             process a .BIN file\n");
    fprintf(stderr, "    -r             process a .RIM file\n");
    fprintf(stderr, "    -s             silent operation\n");
    fprintf(stderr, "    -v             verbose operation\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Checks for overlapping addresses in PDP8 .RIM or .BIN files.\n");
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    uint8_t oflag = 0;
    uint16_t address = 0;
    int cnt = 0;
    int inuse[4096];
    uint8_t isBin = 0, isRim = 0, isVerbose = 0, isSilent = 0;

    int opt;
    while ((opt = getopt(argc, argv, "rbvsV")) != -1) {
        switch (opt) {
        case 'r':
            isRim++;
            break;
        case 'b':
            isBin++;
            break;
        case 'v':
            isVerbose++;
            break;
        case 's':
            isSilent++;
            break;
        case 'V':
            printf("%s version %s\n", progname, version);
            break;
        default:
            usage();
        }
    }
    if (optind >= argc) usage();

    FILE *f;
    f = fopen(argv[optind++], "rb");

    memset(inuse, 0, sizeof(inuse));

    // Read the file byte by byte and process them according to the
    // command type (the two highest bits)
    while (!feof(f)) {
        uint8_t b1, b2;
        fread(&b1, 1, 1, f);
        uint8_t cmd = b1 & 0xC0;

        // As long as it's a header just tally them up
        if (b1 == 0x80) {
            cnt++;
            continue;
        }

        // When going from header to something else then print the count
        if (b1 != 0x80 && cnt > 0) {
            if (isVerbose) printf("Leader * %d\n", cnt);
            cnt = 0;
        }

        // Just print setting of fields
        if (cmd == 0xC0) {
            if (isVerbose) printf("Field %d\n", (b1 >> 3) & 7);
            continue;
        }

        // This command should not be in the files
        if (cmd == 0x80) {
            if (isVerbose) printf("Invalid record type\n");
            continue;
        }

        // The address command is two bytes, so read one additional
        // byte from the file and update the address variable.
        // The oflag is set to signal that we have a newly changed address
        if (cmd == 0x40) {
            fread(&b2, 1, 1, f);
            address = ((b1 & 0x3f) << 6) + (b2 & 0x3f);
            oflag = 1;
            continue;
        }

        // The data command is two bytes, so read one additional
        // byte from the file. Then print the data if verbose mode.
        // Print with a leading * if the address was just set.
        // Also count the number of times this address has been used here
        // for the later overlap check.
        if (cmd == 0x00) {
            fread(&b2, 1, 1, f);
            uint16_t data = ((b1 & 0x3f) << 6) + (b2 & 0x3f);
            if (isVerbose) {
                if (oflag)
                    printf("* %04o : %04o\n", address, data);
                else
                    printf("  %04o : %04o\n", address, data);
            }
            oflag = 0;
            inuse[address]++;
            address++;
            continue;
        }
    }
    if (cnt > 0 && isVerbose) printf("Leader * %d\n", cnt);

    // Remove false positive caused by the .BIN checksum at the end of file
    if (isBin && address > 0) inuse[address - 1]--;

    // Iterate over the inuse file searching for areas with a usage count
    // larger than one. If not silent then print a message for each separate
    // area
    cnt = 0;
    uint16_t start;
    uint16_t overlaps = 0;
    for (uint16_t i = 0; i < 4096; i++) {
        if (inuse[i] > 1) {
            if (cnt == 0) start = i;
            cnt++;
        }
        if (inuse[i] < 2 && cnt > 0) {
            if (!isSilent) printf("Overlap in area %04o to %04o\n", start, i - 1);
            overlaps++;
            cnt = 0;
        }
    }
    if (cnt > 0 && !isSilent) {
        printf("Overlap in area %04o to %04o\n", start, 4095);
        overlaps++;
    }

    return overlaps;
}