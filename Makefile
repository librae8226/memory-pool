.DEFAULT_GOAL := sketch

sketch:
#	gcc -Wall -Werror mempool.c main.c -o mempool
	gcc -Wall mempool.c main.c -o mempool

clean:
	rm -f mempool

distclean:
	rm -f mempool tag* *cscope*
