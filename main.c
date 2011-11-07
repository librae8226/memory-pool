#include <stdio.h>
#include <stdlib.h>
#include "mempool.h"


#define ALLOC_NODE_NUM	16
#define NEW_NODE_NUM	16

int main(int argc, char *argv[])
{
	int *h_mem = NULL;
	int cnt = 0;
	struct mp_node_t *pp_node_array[NEW_NODE_NUM];

	mp_alloc(&h_mem, ALLOC_NODE_NUM);
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt++) {
		pp_node_array[cnt] = mp_new_node(h_mem);
		if (pp_node_array[cnt] == NULL) {
			mp_clean(h_mem);
			return -1;
		}
	}

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

	for (cnt = 0; cnt < NEW_NODE_NUM; cnt += 3) {
		mp_del_node(h_mem, pp_node_array[cnt]);
	}
	mp_del_node(h_mem, pp_node_array[NEW_NODE_NUM - 1]);
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt += 7) {
		pp_node_array[cnt] = mp_new_node(h_mem);
	}
	mp_dump_pool(h_mem);
	mp_clean(h_mem);
	return 0;
}
