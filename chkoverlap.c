/*
 *	chkoverlap.c
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
 * As palbart also lacks the memory usage map that other assemblers
 * can generate this is added as an option here. The compressed variant
 * prints an X if any of two consecutive words  are used, this so it
 * fits on a 80 character wide screen. The full version prints an X
 * on every location in use, so it will fit on a 132 column device.
 * The X is replaced with an O if there's an overlap in that location(s)
 *
 * 2020-03-30 v1.00 MEM     First Release
 * 2020-03-30 v1.01 MEM     Added memory maps
 * 2020-03-31 v1.02 MEM     Added page numbers to memory maps
 * 2020-04-29 v1.03 MEM     Fixed compilation warnings in Github CI
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *progname = "chkoverlap";
char *version = "1.03";

void usage() {
    fprintf(stderr, "\nUsage:  %s [options] filename\n\n", progname);
    fprintf(stderr, "where [options] are optional parameters chosen from:\n");
    fprintf(stderr, "    -b             process a .BIN file\n");
    fprintf(stderr, "    -r             process a .RIM file\n");
    fprintf(stderr, "    -m             display (compressed) memory usage map\n");
    fprintf(stderr, "    -M             display (full) memory usage map\n");
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
    uint8_t isBin = 0, isRim = 0, isVerbose = 0, isSilent = 0, isMapC = 0, isMapF = 0;

    int opt;
    while ((opt = getopt(argc, argv, "rbvsmMV")) != -1) {
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
        case 'm':
            isMapC++;
            break;
        case 'M':
            isMapF++;
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
        size_t len;
        len=fread(&b1, 1, 1, f);
        if (len!=1) exit(1);
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
            len=fread(&b2, 1, 1, f);
            if (len!=1) exit(1);
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
            len=fread(&b2, 1, 1, f);
            if (len>1) exit(1); // Keep gcc/lint happy
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
    uint16_t start=0;
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

    if (isMapC) {
        if (overlaps > 0 && !isSilent) printf("\n");
        for (uint16_t i = 0; i < 4096; i += 2) {
            if (i % 128 == 0) printf("%04o ", i);
            char s = '.';
            if (inuse[i] > 0 || inuse[i + 1] > 0) s = 'X';
            if (inuse[i] > 1 || inuse[i + 1] > 1) s = 'O';
            printf("%c", s);
            if (i % 128 == 126) printf("\n");
        }
    }
    if (isMapF) {
        if (overlaps > 0 && !isSilent) printf("\n");
        for (uint16_t i = 0; i < 4096; i++) {
            if (i % 128 == 0) printf("%02o ", i / 64);
            char s = '.';
            if (inuse[i] > 0) s = 'X';
            if (inuse[i] > 1) s = 'O';
            printf("%c", s);
            if (i % 128 == 127) printf("\n");
        }
    }

    return overlaps;
}