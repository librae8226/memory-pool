#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mempool.h"


#define ALLOC_NODE_NUM	16
#define NEW_NODE_NUM	16
#define PUBLIC_SHM_KEY	1234

int main(int argc, char *argv[])
{
	int *h_mem = NULL;
	int shmid = -1;
	int cnt = 0;
	struct mp_node_t *pp_node_array[NEW_NODE_NUM];

	shmid = mp_shm_alloc(&h_mem, PUBLIC_SHM_KEY, ALLOC_NODE_NUM);
	if (shmid == -1) {
		printf("alloc shm failed!\n");
		return -1;
	}
	printf("shmid: %d, h_mem: %p\n", shmid, h_mem);
	mp_shm_init(h_mem, ALLOC_NODE_NUM);
//	for (cnt = 0; cnt < NEW_NODE_NUM; cnt++) {
//		pp_node_array[cnt] = mp_new_node_of(h_mem);
//		if (pp_node_array[cnt] == NULL) {
//			mp_shm_clean(shmid, &h_mem);
//			printf("pp_node_array[%d] is NULL!\n", cnt);
//			return -1;
//		}
//	}

#if 0
	/* container_of test */
	pp_node_array[3]->node_id = 3;
	pp_node_array[3]->data.a = 5;
	struct item_t *p_item = NULL;
	struct test_t *p_test = &(pp_node_array[3]->data.t);
	p_item = container_of(p_test, struct item_t, t);
	printf(">>> data.a: %d\n", p_item->a);

	struct mp_node_t *p_node_test = NULL;
	p_node_test = container_of(p_item, struct mp_node_t, data);
	printf(">>> node_id: %d\n", p_node_test->node_id);
#endif
#if 0
	/* random node test */
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt += 3) {
		mp_del_node_of(h_mem, pp_node_array[cnt]);
	}
	mp_del_node_of(h_mem, pp_node_array[NEW_NODE_NUM - 1]);
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt += 7) {
		pp_node_array[cnt] = mp_new_node_of(h_mem);
	}
#endif
	mp_dump_pool(h_mem);
	sleep(60);
	mp_dump_pool(h_mem);
	mp_shm_clean(shmid, &h_mem);
	return 0;
}
