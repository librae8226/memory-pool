/*
 * @brief System Monitor for apps to kickoff watchdog periodically.
 *        If critical error occures, reset request message will be sent to sm.
 *        Provide the interface to register or unregister for different modules.
 * @usage only need a startup app to call monitor_init() to start monitor daemon_thread
 *        then other apis are available for monitoring modules
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include "com_type.h"
#include "ErrorCode.h"
#include "OSAL_api.h"
#include "SM_common.h"
#include "SM_Agent.h"
#include "monitor.h"
#include "mempool.h"


#define DEFAULT_TIMEOUT		10000
#define MODULE_NUMBER_MAX	16
#define MSG_NUMBER_MAX		16
#define MSG_CMD_EXIT		0xFF
#define PUBLIC_SHM_KEY		1122
#define PUBLIC_SEM_KEY		2345


static M_OSAL_HANDLE_TASK_t h_daemon = NULL;
static UINT32 cnt_init = 0;
static INT32 shmid = -1;
static INT32 semid = -1;
static INT32 *h_mem = NULL;

static void dump_module_info(struct module_control_block_t *p_mcb);
static void notify_watchdog(INT32 msg);
static UINT64 get_clock();
static void daemon_thread(void);

/*
 * @brief    register module to be monitored by daemon
 *           this api should not be called before monitor_init()
 * @param    attr -i- module attribute for registration
 *           callback_fn -i- callback function (optional)
 * @return   module id (a positive number) if success
 * @node     the id returned has nothing to do with its position in the
 *           logical linked list, but the position in the memory pool
 */
INT32 M_DAEMON_RegisterModule(struct M_DAEMON_ModAttr_t *p_attr,
		M_DAEMON_Callback callback_fn)
{
	HRESULT ret = E_FAIL;
	struct mp_node_t *p_node = NULL;

	M_DAEMON_PRINT(LOG_WARN, "enter, name: %s, timeout: %d\n",
			p_attr->name, p_attr->timeout);
#ifndef __BIONIC__
	if (strcmp(p_attr->name, "monitord") != 0) {
		shmid = mp_shm_alloc(&h_mem, PUBLIC_SHM_KEY, MODULE_NUMBER_MAX);
		M_DAEMON_ASSERT(h_mem != NULL);
		M_DAEMON_PRINT(LOG_INFO, "shmid: %d, h_mem: %p\n", shmid, h_mem);
		semid = mp_sem_create(PUBLIC_SEM_KEY);
		M_DAEMON_ASSERT(semid != -1);
		M_DAEMON_PRINT(LOG_INFO, "semid: %d\n", semid);
	}
#endif
	p_node = mp_new_node_of(h_mem);
	if (p_node == NULL)
		return 0;
#ifndef __BIONIC__
	ret = mp_sem_acquire(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	strcpy(p_node->data.attr.name, p_attr->name);
	if (callback_fn == NULL)
		p_node->data.attr.timeout = p_attr->timeout;
	else
		p_node->data.attr.timeout = DEFAULT_TIMEOUT;
	p_node->data.attr.type = p_attr->type;
	p_node->data.mid = p_node->node_id;
	p_node->data.callback_fn = callback_fn;
	p_node->data.time = get_clock();
	p_node->data.ready = 1;
#ifndef __BIONIC__
	ret = mp_sem_release(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	dump_module_info(&p_node->data);

	mp_update_pool(h_mem);
	mp_dump_pool(h_mem);
	M_DAEMON_PRINT(LOG_WARN, "leave, this time = %llu\n", p_node->data.time);
	return p_node->data.mid;
}

/*
 * @brief    unregister module which is monitored by daemon
 * @param    mid -i- module id
 * @return   status
 */
HRESULT M_DAEMON_UnregisterModule(INT32 mid)
{
	HRESULT ret = 0;
	struct mp_mem_head_t *p_mem_head = NULL;
	struct mp_node_t *p_node = NULL;

	M_DAEMON_PRINT(LOG_DEBUG, "enter, mid: %d\n", mid);

	p_mem_head = (struct mp_mem_head_t *)h_mem;
	if (p_mem_head == NULL || mid <= 0 ||
			((UINT32)mid > p_mem_head->max_node_num))
		return E_FAIL;

	p_node = mp_get_node(h_mem, mid);
#ifndef __BIONIC__
	ret = mp_sem_acquire(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	ret = mp_del_node_of(h_mem, p_node);
	if (ret != S_OK)
		return E_FAIL;
#ifndef __BIONIC__
	ret = mp_shm_detach(shmid, h_mem);
	if (ret != S_OK)
		return E_FAIL;
#endif
#ifndef __BIONIC__
	ret = mp_sem_release(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	M_DAEMON_PRINT(LOG_DEBUG, "leave\n");
	return S_OK;
}

/*
 * @brief    update module status to indicate that it is alive
 * @param    mid -i- module id
 *           arg -i- reserved
 * @return   status
 */
HRESULT M_DAEMON_UpdateStatus(INT32 mid, VOID *arg)
{
	HRESULT ret = 0;
	struct mp_mem_head_t *p_mem_head = NULL;
	struct mp_node_t *p_node = NULL;

	M_DAEMON_PRINT(LOG_DEBUG, "enter, mid: %d, arg: %p, h_mem: %p\n",
			mid, arg, h_mem);

	p_mem_head = (struct mp_mem_head_t *)h_mem;
	if (p_mem_head == NULL || mid <= 0 ||
			((UINT32)mid > p_mem_head->max_node_num))
		return E_FAIL;

	p_node = mp_get_node(h_mem, mid);
	if (p_node == NULL)
		return E_FAIL;
#ifndef __BIONIC__
	ret = mp_sem_acquire(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	p_node->data.time = get_clock();
#ifndef __BIONIC__
	ret = mp_sem_release(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	M_DAEMON_PRINT(LOG_DEBUG, "leave, this time = %llu\n", p_node->data.time);
	return S_OK;
}

/*
 * @brief    critical error process routine, called directly by apps
 * @param    mid -i- module id
 *           log -i- record error log into specific memory address
 * @return   status
 */
HRESULT M_DAEMON_NotifyCriticalError(INT32 mid, CHAR *log)
{
	HRESULT ret = E_FAIL;
	struct mp_mem_head_t *p_mem_head = NULL;
	struct mp_node_t *p_node = NULL;

	M_DAEMON_PRINT(LOG_DEBUG, "enter, mid: %d, log: %p, h_mem: %p\n",
			mid, log, h_mem);

	p_mem_head = (struct mp_mem_head_t *)h_mem;
	if (p_mem_head == NULL || mid <= 0 ||
			((UINT32)mid > p_mem_head->max_node_num))
		return E_FAIL;

	p_node = mp_get_node(h_mem, mid);
#ifndef __BIONIC__
	ret = mp_sem_acquire(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	p_node->data.critical_error = 1;
#ifndef __BIONIC__
	ret = mp_sem_release(semid);
	M_DAEMON_ASSERT(ret == 0);
#endif
	return S_OK;
}

/*
 * @brief    interface to save log
 * @param    mid -i- module id
 *           log -i- record error log into specific memory address
 * @return   status
 */
HRESULT M_DAEMON_SaveLog(INT32 mid, CHAR *log)
{
	M_DAEMON_PRINT(LOG_DEBUG, "enter, id: %d, log: %p\n", mid, log);

	M_DAEMON_PRINT(LOG_DEBUG, "leave\n");
	return S_OK;
}

/*
 * @brief    initialize system monitor
 * @param    none
 * @return   status
 * @note     only monitor daemon can call this function
 */
HRESULT monitor_init(VOID)
{
	HRESULT ret = E_FAIL;
	struct mp_mem_head_t *p_mem_head = NULL;

	M_DAEMON_PRINT(LOG_DEBUG, "enter\n");
	p_mem_head = (struct mp_mem_head_t *)h_mem;

	cnt_init++;
	if (cnt_init != 1) {
		M_DAEMON_PRINT(LOG_INFO,
			"warning: daemon has already been initialized\n");
		return S_OK;
	}
#ifndef __BIONIC__
	shmid = mp_shm_alloc(&h_mem, PUBLIC_SHM_KEY, MODULE_NUMBER_MAX);
	if (h_mem == NULL)
		return E_FAIL;
	mp_shm_init(h_mem, MODULE_NUMBER_MAX);
	M_DAEMON_PRINT(LOG_WARN, "shmid: %d, h_mem: %p\n", shmid, h_mem);

	semid = mp_sem_create(PUBLIC_SEM_KEY);
	M_DAEMON_PRINT(LOG_WARN, "semid: %d\n", semid);
	M_DAEMON_ASSERT(semid != -1);
#else
        mp_alloc(&h_mem, MODULE_NUMBER_MAX);
#endif
	ret = M_OSAL_Init();
	M_DAEMON_ASSERT(ret == S_OK);

	ret = M_OSAL_Task_Create_Advanced(&h_daemon, daemon_thread, NULL,
			M_OSAL_TASK_PRIORITY_DEFAULT, MSG_NUMBER_MAX);
	M_DAEMON_ASSERT(ret == S_OK);

	M_DAEMON_PRINT(LOG_DEBUG, "leave, shmid: %d, h_mem: %p\n",
			shmid, h_mem);
	return ret;
}

/*
 * @brief    exit system monitor
 * @param    none
 * @return   status
 * @note     this function can be external, but intentionally set to be private
 *           only for monitor daemon to call this function
 */
HRESULT monitor_exit(VOID)
{
	M_CC_MSG_t msg = {0, 0, 0};
	HRESULT ret = E_FAIL;
	struct mp_mem_head_t *p_mem_head = NULL;

	M_DAEMON_PRINT(LOG_DEBUG, "enter\n");
	p_mem_head = (struct mp_mem_head_t *)h_mem;

	if (cnt_init == 0) {
		M_DAEMON_PRINT(LOG_WARN, "warning: daemon is not running\n");
		return S_OK;
	} else
		cnt_init = 0;

	M_DAEMON_ASSERT(h_mem != NULL);
	msg.m_MsgID = (UINT32)MSG_CMD_EXIT;
	msg.m_Param1 = 0;
	msg.m_Param2 = 0;
	ret = M_OSAL_Task_MsgQ_Post(&h_daemon, &msg);
	M_DAEMON_ASSERT(ret == S_OK);

	ret = M_OSAL_Task_Join(&h_daemon);
	M_DAEMON_ASSERT(ret == S_OK);

#if 0
	/* do not need to exit OSAL */
	ret = M_OSAL_Exit();
	M_DAEMON_ASSERT(ret == S_OK);
#endif
#ifndef __BIONIC__
	ret = mp_sem_delete(semid);
	M_DAEMON_ASSERT(ret == 0)
#endif
#ifndef __BIONIC__
	mp_shm_clean(shmid, &h_mem);
#else
        mp_clean(&h_mem);
#endif
	M_DAEMON_PRINT(LOG_DEBUG, "leave\n");
	return S_OK;
}

/*
 * @brief    tell sm that monitor thread will go on to kick off watchdog
 * @param    none
 * @return   status
 * @note     this function can be external, but intentionally set to be private
 *           only for monitor daemon to call this function
 */
HRESULT monitor_continue(VOID)
{
	notify_watchdog(M_SM_WD_APP_CONTINUE);
	return S_OK;
}

/*
 * @brief    tell sm that monitor thread will stop to kick off watchdog,
 *           sm should take charge
 * @param    none
 * @return   status
 * @note     this function can be external, but intentionally set to be private
 *           only for monitor daemon to call this function
 */
HRESULT monitor_stop(VOID)
{
	notify_watchdog(M_SM_WD_APP_EXIT);
	return S_OK;
}

static void dump_module_info(struct module_control_block_t *p_mcb)
{
	M_DAEMON_ASSERT(p_mcb != NULL);
	M_DAEMON_PRINT(LOG_INFO, "+---- module info ----+\n");
	M_DAEMON_PRINT(LOG_INFO, "attr.name: %s\n", p_mcb->attr.name);
	M_DAEMON_PRINT(LOG_INFO, "attr.timeout: %d\n", p_mcb->attr.timeout);
	M_DAEMON_PRINT(LOG_INFO, "attr.type: %d\n", p_mcb->attr.type);
	M_DAEMON_PRINT(LOG_INFO, "mid: %d\n", p_mcb->mid);
	M_DAEMON_PRINT(LOG_INFO, "time: %llu\n", p_mcb->time);
	M_DAEMON_PRINT(LOG_INFO, "critical_error: %d\n", p_mcb->critical_error);
	M_DAEMON_PRINT(LOG_INFO, "callback_fn: %p\n", p_mcb->callback_fn);
	M_DAEMON_PRINT(LOG_INFO, "+---------------------+\n");
}

static void notify_watchdog(INT32 msg)
{
	M_SM_Message sm_msg_send;
	INT32 *p_msg_tx = NULL;

	memset(&sm_msg_send, 0, sizeof(M_SM_Message));

	sm_msg_send.m_iModuleID = M_SM_ID_WD;

	/* open sm device */
	M_SM_Agent_Init();

	/* send message */
	p_msg_tx = (INT32 *)sm_msg_send.m_pucMsgBody;
	*p_msg_tx = 1;
	*(p_msg_tx + 1) = msg;
	sm_msg_send.m_iMsgLen = sizeof(INT32) * 2;
#if 1
	/* comment out for debug */
	M_SM_Agent_Write_Msg(sm_msg_send.m_iModuleID,
			sm_msg_send.m_pucMsgBody, sm_msg_send.m_iMsgLen);
#endif
	/* close sm device */
	M_SM_Agent_Close();
}

static UINT64 get_clock()
{
	UINT64 ms = 0;
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	ms = (UINT64)(ts.tv_sec*1000) + (UINT64)(ts.tv_nsec/1000000ULL);
	return ms;
}

static void daemon_thread(void)
{
	static INT32 thread_run = 1;
	struct mp_node_t *p_node_it = NULL;
	struct mp_mem_head_t *p_mem_head = NULL;
	M_CC_MSG_t msg = {0, 0, 0};
	UINT64 curr_time = 0;
	HRESULT ret = E_FAIL;
	INT32 timeout_occured = 0;

	p_mem_head = (struct mp_mem_head_t *)h_mem;
	notify_watchdog(M_SM_WD_APP_START);

	while (thread_run) {
		if (M_OSAL_Task_MsgQ_GetTry(&h_daemon, &msg) == S_OK) {
			if (msg.m_MsgID == MSG_CMD_EXIT) {
				M_DAEMON_PRINT(LOG_INFO,
						"daemon_thread is to exit.\n");
				thread_run = 0;
			}
			else
				M_DAEMON_PRINT(LOG_INFO,
						"msg id undefined: 0x%X\n",
						msg.m_MsgID);
		}
#ifndef __BIONIC__
		ret = mp_sem_acquire(semid);
		M_DAEMON_ASSERT(ret == 0);
#endif
		mp_update_pool(h_mem);

		/* iterate each module node */
		curr_time = get_clock();
		timeout_occured = 0;
		/*
		 * FIXME
		 * We do need a lock here to avoid some maybe
		 * concurrency issue. In RegisterModule, before the
		 * whole register flow finish, daemon should not access
		 * the data of the module which is being registered.
		 */
		for (p_node_it = p_mem_head->p_node_first;
				(p_node_it != NULL) && (p_node_it->data.ready == 1);
				p_node_it = p_node_it->p_next) {
			if (p_node_it->data.callback_fn != NULL) {
				ret = p_node_it->data.callback_fn();
				if (ret == S_OK)
					p_node_it->data.time = curr_time;
				else
					M_DAEMON_PRINT(LOG_WARN,
							"#%d %s return fail!\n",
							p_node_it->data.mid,
							p_node_it->data.attr.name);
			}
			if (curr_time - p_node_it->data.time >
					(UINT64)p_node_it->data.attr.timeout) {
				M_DAEMON_PRINT(LOG_ERROR, "#%d %s timeout (%llu)\n",
						p_node_it->data.mid,
						p_node_it->data.attr.name,
						curr_time - p_node_it->data.time);
				timeout_occured = 1;
			}
			if (p_node_it->data.critical_error == 1) {
				M_DAEMON_PRINT(LOG_ERROR, "#%d %s critical error occured\n",
						p_node_it->data.mid,
						p_node_it->data.attr.name);
				timeout_occured = 1;
				break;
			}
			M_DAEMON_PRINT(LOG_DEBUG, "#%d last: %llu -> curr: %llu\n",
					p_node_it->data.mid, p_node_it->data.time, curr_time);
		}
#ifndef __BIONIC__
		ret = mp_sem_release(semid);
		M_DAEMON_ASSERT(ret == 0);
#endif
		if (timeout_occured == 0) {
			M_DAEMON_PRINT(LOG_DEBUG, "kick off watchdog\n");
			notify_watchdog(M_SM_WD_Kickoff);
		} else {
			M_DAEMON_PRINT(LOG_ERROR, "daemon_thread exit\n");
			thread_run = 0;
		}
		M_OSAL_Task_Sleep(500);
	}
	monitor_exit();
	return;
}
