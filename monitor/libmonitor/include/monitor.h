#ifndef __GALOIS_MONITOR_H__
#define __GALOIS_MONITOR_H__


#define LOG_ALL		0
#define LOG_DEBUG	1
#define LOG_INFO	2
#define LOG_WARN	3
#define LOG_ERROR	4
#define LOG_LEVEL	LOG_INFO

#define M_DAEMON_DEBUG_PRINT	1
#ifdef M_DAEMON_DEBUG_PRINT
#ifdef ANDROID
#include <cutils/log.h>
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG	"monitord"
#define M_DAEMON_PRINT(level, msg, args...) \
	do { \
		if (level >= LOG_LEVEL) { \
			if (level == LOG_ERROR) \
				LOGE("[%s] "msg, __FUNCTION__, ##args); \
			else \
				LOGI("[%s] "msg, __FUNCTION__, ##args); \
		} \
	} while (0)

#define M_DAEMON_ASSERT(x) \
{ \
	if ((x) == 0) { \
		LOGE("---ASSERT--- at line %d in %s!\n", __LINE__, __FILE__); \
		while (1) { \
			M_OSAL_Task_Sleep(100 * 1000); \
		} \
	} \
}
#else /* ANDROID */
#define M_DAEMON_PRINT(level, msg, args...) \
	do { \
		if (level > LOG_LEVEL) { \
			printf("[%s] "msg, __FUNCTION__, ##args); \
		} \
	} while (0)

#define M_DAEMON_ASSERT(x) \
{ \
	if ((x) == 0) { \
		printf("---ASSERT--- at line %d in %s!\n", __LINE__, __FILE__); \
		while (1) { \
			M_OSAL_Task_Sleep(100 * 1000); \
		} \
	} \
}
#endif /* ANDROID */
#else /* M_DAEMON_DEBUG_PRINT */
#define M_DAEMON_PRINT(level, msg, args...) \
	do { \
	} while (0)
#define M_DAEMON_ASSERT(x) \
	do { \
	} while (0)
#endif /* M_DAEMON_DEBUG_PRINT */

/* this callback should not take too much time */
typedef HRESULT (*M_DAEMON_Callback)(VOID);

struct M_DAEMON_ModAttr_t {
	CHAR name[16];
	INT32 timeout;
	INT32 type;
};

struct module_control_block_t {
	struct M_DAEMON_ModAttr_t attr;
	INT32 mid;
	UINT64 time;
	INT32 ready;
	INT32 critical_error;
	M_DAEMON_Callback callback_fn;
};

HRESULT monitor_init(VOID);
HRESULT monitor_exit(VOID);
HRESULT monitor_continue(VOID);
HRESULT monitor_stop(VOID);
INT32 M_DAEMON_RegisterModule(struct M_DAEMON_ModAttr_t *p_attr,
		M_DAEMON_Callback callback_fn);
HRESULT M_DAEMON_UnregisterModule(INT32 mid);
HRESULT M_DAEMON_UpdateStatus(INT32 mid, VOID *arg);
HRESULT M_DAEMON_NotifyCriticalError(INT32 mid, CHAR *log);
HRESULT M_DAEMON_SaveLog(INT32 mid, CHAR *log);


#endif /* __GALOIS_MONITOR_H__ */
