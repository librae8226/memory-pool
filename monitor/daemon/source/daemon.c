#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "com_type.h"
#include "ErrorCode.h"
#include "OSAL_api.h"
#include "monitor.h"

#define MODULE_NAME	"monitord"
#define MODULE_TIMEOUT	5000
#define MODULE_TYPE	1

#if 0
static HRESULT callback_function(VOID)
{
	return S_OK;
}
#endif

int main(int argc, char *argv[])
{
	HRESULT ret = E_FAIL;
	INT32 id = 0;

	struct M_DAEMON_ModAttr_t attr = {
		MODULE_NAME,
		MODULE_TIMEOUT,
		MODULE_TYPE
	};

	if (argc != 1) {
		if (argc != 2) {
			printf("Usage: %s continue/stop\n", argv[0]);
			exit(0);
		} else {
			if (strcmp(argv[1], "continue") == 0)
				ret = monitor_continue();
			else if (strcmp(argv[1], "stop") == 0)
				ret = monitor_stop();
			else
				;
			exit(0);
		}
	}

	ret = monitor_init();
	if (ret != S_OK) {
		printf("%s - init fail\n", MODULE_NAME);
		return E_FAIL;
	}
	id = M_DAEMON_RegisterModule(&attr, NULL);

	while (1) {
		M_OSAL_Task_Sleep(1000);
		ret = M_DAEMON_UpdateStatus(id, NULL);
		if (ret != S_OK)
			printf("%s - update status fail, ret = 0x%X\n",
					MODULE_NAME, ret);
	}
	return 0;
}
