/*
 * Copyright (c) 2011-2012 LeafGrass <leafgrass.g@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 */

/*
 * @file    mempool.h
 * @brief   memory pool simple implementation
 * @log     2011.11 first revision
 */

#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

/* 
 * customization area
 */
//#define DEBUG_PRINT_ON	0
#define ENABLE_SHM	0
#define ENABLE_SEM	0


#define mp_malloc(x)	malloc(x)
#define mp_free(x)	free(x)

typedef int		mp_i32;
typedef char		mp_i8;
typedef unsigned int	mp_u32;
typedef unsigned char	mp_u8;

struct test_t {
	int z;
};

struct item_t {
	int a;
	int b;
	int c;
	struct test_t t;
};

typedef struct item_t	elem_t;
typedef int		status_t;

/* 
 * memory pool definition
 * note: on 64-bit machine, a pointer is 8 bytes (64 bit)
 *       memory will be aligned by 8-byte
 */
#if 0
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({ \
		const typeof( ((type *)0)->member ) *__mptr = (ptr); \
		(type *)( (char *)__mptr - offsetof(type,member) );})
#endif

struct mp_mem_head_t {
	struct mp_node_t *p_node_first;
	struct mp_node_t *p_node_tail;
	mp_u32 node_first_offset;
	mp_u32 node_tail_offset;
	mp_u32 size;
	mp_u32 used_node_num;
	mp_u32 max_node_num;
};

struct mp_node_t {
	struct mp_node_t *p_prev;
	mp_u32 prev_offset;
	mp_u32 used;
	mp_u32 node_id;
	elem_t data;
	mp_u32 next_offset;
	struct mp_node_t *p_next;
};

struct mp_mem_head_t *mp_alloc(mp_i32 **pp_mem_base, mp_u32 max_node_num);
status_t mp_clean(mp_i32 **pp_mem_base);
struct mp_node_t *mp_new_node(mp_i32 *p_mem_base);
struct mp_node_t *mp_new_node_of(mp_i32 *p_mem_base);
status_t mp_del_node(mp_i32 *p_mem_base, struct mp_node_t *p_node);
status_t mp_del_node_of(mp_i32 *p_mem_base, struct mp_node_t *p_node);
void mp_dump_pool(mp_i32 *p_mem_base);
struct mp_node_t *mp_get_node(mp_i32 *p_mem_base, mp_u32 node_id);
#ifdef ENABLE_SHM
mp_i32 mp_shm_alloc(mp_i32 **pp_mem_base, mp_i32 key, mp_u32 max_node_num);
status_t mp_shm_clean(mp_i32 shmid, mp_i32 **pp_mem_base);
status_t mp_shm_detach(mp_i32 shmid, mp_i32 *p_mem_base);
status_t mp_shm_init(mp_i32 *p_mem_base, mp_u32 max_node_num);
struct mp_node_t *mp_shm_new_node(mp_i32 *p_mem_base);
status_t mp_shm_del_node(mp_i32 *p_mem_base, struct mp_node_t *p_node);
#endif
#ifdef ENABLE_SEM
mp_i32 mp_sem_create(mp_i32 key);
status_t mp_sem_delete(mp_i32 semid);
status_t mp_sem_acquire(mp_i32 semid);
status_t mp_sem_release(mp_i32 semid);
#endif


#endif /* __MEMPOOL_H__ */
