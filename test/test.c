#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../mempool.h"


#define ALLOC_NODE_NUM	16
#define PUBLIC_SHM_KEY	1234

int main(int argc, char *argv[])
{
	int *h_mem = NULL;
	int shmid = -1;
	struct mp_node_t *p_node = NULL;
	unsigned int time = 0;

	shmid = mp_shm_alloc(&h_mem, PUBLIC_SHM_KEY, ALLOC_NODE_NUM);
	if (shmid == -1) {
		printf("alloc shm failed!\n");
		return -1;
	}
	p_node = mp_new_node_of(h_mem);
	if (p_node == NULL) {
		printf("create new node failed!\n");
		return -1;
	}
	/* FIXME call mp_dump_pool() could cause a segmentation fault */
	/*mp_dump_pool(h_mem);*/
	if (argc == 2) {
		time = atoi(argv[1]);
		sleep(time);
	}
	else
		sleep(5);
	mp_del_node_of(h_mem, p_node);
	mp_shm_detach(shmid, h_mem);
	return 0;
}
