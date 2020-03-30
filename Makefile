.PHONY:	clean

all: chkoverlap

chkoverlap: chkoverlap.c
	$(CC) -Wall -Werror -O2 -o $@ $<

clean:
	rm -f chkoverlap 
