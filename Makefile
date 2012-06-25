all:
	${CC} -O3 -Wall -Wextra -pedantic -std=c99 -lshout -ltag_c upstream.c -o upstream

clean:
	rm -f upstream
