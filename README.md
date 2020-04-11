# chkoverlap
Check for overlaps in data/code areas in PDP8 .rim/.bin files

This program analyzes (and optionally dumps) a BIN or RIM file
for overlapping areas. It was created to analyze the files from
palbart that doesn't warn when data and/or code areas run into
eachother.

It can also print memory usage maps, either in compressed (two words per character) or full (one character per word).

Return value from the program is the number of overlapping areas found so it can be directly used in a Makefile to abort processing if there are overlaps.

### Example Makefile
This first assembles the source using palbart, followed by a check of overlaps and finally it writes a map-file.

```
PAL:=../tools/palbart
CHK:=../tools/chkoverlap

all: tb8.bin

tb8.bin: tb8.pal $(PAL)
        $(PAL) -a -p -n -d -x -t 8 $<
        $(CHK) -b $@
        $(CHK) -b -M -s $@ > tb8.map
```



### Usage instructions
```
Usage:  chkoverlap [options] filename

where [options] are optional parameters chosen from:
    -b             process a .BIN file
    -r             process a .RIM file
    -m             display (compressed) memory usage map
    -M             display (full) memory usage map
    -s             silent operation
    -v             verbose operation

Checks for overlapping addresses in PDP8 .RIM or .BIN files.
```

### Examples of memory maps

#### Compressed (-m)

```
0000 XXXXXXXXXXXXXXXXXXXX............................................
0200 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
0400 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
0600 XXXXXXXXXXXXXXXXXXXX...........................................X
1000 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX...
1200 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.............X
1400 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
1600 XXXXXXXXXXXXXXXXXXXXX...........................................
2000 ................................................................
2200 ................................................................
<snip>
7000 ................................................................
7200 ................................................................
7400 ................................................................
7600 ................................................................
```

#### Full (-M)
(Shown snipped both horizontally and vertically)
```
00 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX....<snip>..........................
02 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX<snip>XXXXXXXXXXXXXXXXXXXXXXXXXX
04 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX<snip>XXXXXXXXXXXXXXXXXXXXXXXXXX
06 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.....<snip>.........................X
10 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX<snip>XXXXXXXXXXXXXXXXXXXX......
12 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX<snip>........................XX
14 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX<snip>XXXXXXXXXXXXXXXXXXXXXXXXXX
16 XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX..<snip>..........................
20 .....................................<snip>..........................
22 .....................................<snip>..........................
<snip>
70 .....................................<snip>..........................
72 .....................................<snip>..........................
74 .....................................<snip>..........................
76 .....................................<snip>..........................
```
