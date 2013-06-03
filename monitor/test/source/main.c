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

#define MODULE_NAME	"test_monitor_"
#define MODULE_TIMEOUT	5000
#define MODULE_TYPE	0

#if 0
static HRESULT callback_function(VOID)
{
	return S_OK;
}
#endif

int main(int argc, char *argv[])
{
	INT32 id;
	char name[20] = {0};
	int interval = 0;
	int cnt = 0;

	if (argc != 4) {
		printf("USAGE: test_monitor NAME INTERVAL CNT\n");
		return -1;
	}

	interval = atoi(argv[2]);
	strcpy(name, MODULE_NAME);
	strcat(name, argv[1]);
	struct M_DAEMON_ModAttr_t attr = {
		MODULE_NAME,
		MODULE_TIMEOUT,
		MODULE_TYPE
	};

	id = M_DAEMON_RegisterModule(&attr, NULL);
	if (strcmp(argv[1], "doom") == 0)
		M_DAEMON_NotifyCriticalError(id, NULL);

	cnt = atoi(argv[3]);
	while (cnt--) {
		M_OSAL_Task_Sleep(interval + 1000);
		M_DAEMON_UpdateStatus(id, NULL);
	}
	M_DAEMON_UnregisterModule(id);
	return 0;
}
