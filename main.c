/*
 * run as a daemon for 5 minitues
 * other process can get resources from this shared memory
 * during this period
 * print memory pool info every 10 seconds
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mempool.h"


#define ALLOC_NODE_NUM	16
#define PUBLIC_SHM_KEY	1234

int main(int argc, char *argv[])
{
	int *h_mem = NULL;
	int shmid = -1;
	int cnt = 0;

	shmid = mp_shm_alloc(&h_mem, PUBLIC_SHM_KEY, ALLOC_NODE_NUM);
	if (shmid == -1) {
		printf("alloc shm failed!\n");
		return -1;
	}
	printf("shmid: %d, h_mem: %p\n", shmid, h_mem);
	mp_shm_init(h_mem, ALLOC_NODE_NUM);
	for (cnt = 0; cnt < 60; cnt++) {
		mp_dump_pool(h_mem);
		sleep(10);
	}
	mp_shm_clean(shmid, &h_mem);
	return 0;
}
