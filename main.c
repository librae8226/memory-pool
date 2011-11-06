#include <stdio.h>
#include <stdlib.h>
#include "mempool.h"


#define ALLOC_NODE_NUM	16
#define NEW_NODE_NUM	16

int main(int argc, char *argv[])
{
	int *h_mem = NULL;
	int cnt = 0;
	struct mp_mem_head_t *p_mem_head = NULL;
	struct mp_node_t *pp_node_array[NEW_NODE_NUM];
	struct mp_node_t *p_node = NULL;

	p_mem_head = mp_alloc(&h_mem, ALLOC_NODE_NUM);
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt++) {
		pp_node_array[cnt] = mp_new_node(h_mem);
		if (pp_node_array[cnt] == NULL) {
			mp_clean(h_mem);
			return -1;
		}
	}
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt += 3) {
		mp_del_node(h_mem, pp_node_array[cnt]);
	}
	mp_del_node(h_mem, pp_node_array[NEW_NODE_NUM - 1]);
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt++) {
		pp_node_array[cnt] = mp_new_node(h_mem);
	}
//	pp_node_array[3] = mp_new_node(h_mem);
//	pp_node_array[6] = mp_new_node(h_mem);
//	pp_node_array[9] = mp_new_node(h_mem);
//	pp_node_array[12] = mp_new_node(h_mem);
//	pp_node_array[15] = mp_new_node(h_mem);
	p_node = (struct mp_node_t *)(p_mem_head + 1);
	for (cnt = 0; cnt < ALLOC_NODE_NUM; cnt++, p_node++) {
		printf("node: %p\tprev: %p\tnext: %p\tused: %d\n",
				p_node, p_node->p_prev, p_node->p_next, p_node->used);
	}
#if 0
	for (cnt = 0; cnt < NEW_NODE_NUM; cnt++) {
		if (pp_node_array[cnt] != NULL) {
			printf("p_node: %p, p_node prev: %p, p_node next: %p\n",
					pp_node_array[cnt],
					pp_node_array[cnt]->p_prev,
					pp_node_array[cnt]->p_next);
		}
	}
#endif
	printf("------------- memory pool info ------------\n");
	printf("first %p, tail %p, used: %d\n",
			p_mem_head->p_node_first,
			p_mem_head->p_node_tail,
			p_mem_head->used_node_num);
	mp_clean(h_mem);
	return 0;
}
