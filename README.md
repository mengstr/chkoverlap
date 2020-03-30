# chkoverlap
Check for overlaps in data/code areas in PDP8 .rim/.bin files

This program analyzes (and optionally dumps) a BIN or RIM file
for overlapping areas. It was created to analyze the files from
palbart that doesn't warn when data and/or code areas run into
eachother.

It can also print memory usage maps, either in compressed (two words per character) or full (one character per word).
