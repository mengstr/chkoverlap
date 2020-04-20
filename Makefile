.PHONY:	clean check

all: chkoverlap

chkoverlap: chkoverlap.c
	$(CC) -Wall -Werror -O2 -o $@ $<

check: chkoverlap
	./chkoverlap -m tests/CHEKMO.BN | tee check.log
	@if [ "$$(grep -ao 'Overlap in area' check.log | wc -l)" -ne "8" ]; then false; fi

clean:
	rm -f chkoverlap *.log
