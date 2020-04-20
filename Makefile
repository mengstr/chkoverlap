.PHONY:	clean check docker

all: chkoverlap

chkoverlap: chkoverlap.c
	$(CC) -Wall -Wextra -Werror -O2 -o $@ $<

check: chkoverlap
	./chkoverlap -m tests/CHEKMO.BN | tee check.log
	@if [ "$$(grep -ao 'Overlap in area' check.log | wc -l)" -ne "8" ]; then false; fi

# Compile the code inside a docker container using this Makefile again
docker:	
	docker run --rm -v "$(PWD)":/usr/src/myapp -w /usr/src/myapp gcc:latest make

clean:
	rm -f chkoverlap *.log
