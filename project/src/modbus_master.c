#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>

#include <modbus/modbus.h>
#include "modbus_ex.h"

typedef struct MODBUS_TASK_PARAM__
{
	char slave_addr[128];
	int slave_port;
}MODBUS_TASK_PARAM, *pMODBUS_TASK_PARAM;

pthread_mutex_t gMutex;

int writeLed(modbus_t *ctx, POS_ADDR addr, BIT_VALUE val)
{
	int rc = 0;
	uint8_t read_data;
	
	pthread_mutex_lock(&gMutex);
	rc = modbus_write_bit(ctx, addr, val);
    if (rc != 1) {
        printf("[writeLed] FAILED\n");
        return -1;
    }

    rc = modbus_read_bits(ctx, addr, 1, &read_data);
    pthread_mutex_unlock(&gMutex);
    
    if (rc != 1) {
        printf("[writeLed] FAILED (nb points %d)\n", rc);
        return -1;
    }
    
    return 0;
}

int readKey(modbus_t *ctx, POS_ADDR addr, BIT_VALUE *val)
{
	int rc = 0;
	pthread_mutex_lock(&gMutex);
	rc = modbus_read_input_bits(ctx, addr, 1, (uint8_t*)val);
	pthread_mutex_unlock(&gMutex);
	
	if (rc != 1) {
        printf("[readKey] FAILED (nb points %s)\n", modbus_strerror(errno));
        return -1;
    }
    
    return 0;
}

void monitorSlaveStatus(void *param)
{
	pMODBUS_TASK_PARAM pkeyTaskParam = (pMODBUS_TASK_PARAM)param;
	modbus_t *ctx;
	int rc;
	BIT_VALUE keyValue;
	
	printf("[monitorSlaveStatus] thread created\r\n");
	
	ctx = modbus_new_tcp(pkeyTaskParam->slave_addr, pkeyTaskParam->slave_port);
	
	if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "monitorSlaveStatus] Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }
	
	while(1)
	{
		rc = readKey(ctx, POS_KEY_RED, &keyValue);
		if( rc < 0 )
		{
			printf("[monitorSlaveStatus] Read key error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		if( keyValue != 0 )
		{
			printf("[monitorSlaveStatus] Key pressed\r\n");
		}
		
		usleep(100*1000);
	}
	
close:
	modbus_close(ctx);
    modbus_free(ctx);
}

int main(int argc, char *argv[])
{
    modbus_t *ctx;
    int rc;
    int i;
    MODBUS_TASK_PARAM keyTaskParam;
    pthread_t keyMonitorId;
    pthread_mutex_init(&gMutex, NULL);

	if( argc != 2 )
	{
		printf("USAGE: simple_server_03 SERVER_IP\r\n");
		return -1;
	}

#if 1
	// ****************************************************	
	// key check task
	// ****************************************************	
	strcpy(keyTaskParam.slave_addr, argv[1]);
	keyTaskParam.slave_port = 1502;
	
	if( pthread_create(&keyMonitorId, NULL, (void*)monitorSlaveStatus, &keyTaskParam) != 0 )
	{
		printf("Create slave status monitor thread fail\r\n");
	}
	pthread_detach(keyMonitorId);
	pthread_join(keyMonitorId, NULL);
#endif	
	// ****************************************************	
	// end of task process
	// ****************************************************	
	
	// ****************************************************	
	// master connection process
	// ****************************************************	
    ctx = modbus_new_tcp(argv[1], 1502);
    //modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "[main] Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

#if 1
	for(i=0; i<10; i++)
	{
		printf("[main] write LED_R ON\r\n");
		rc = writeLed(ctx, POS_LED_RED, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		usleep(300*1000);
		
		printf("[main] write LED_R OFF\r\n");
		rc = writeLed(ctx, POS_LED_RED, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_GREEN);
			goto close;
		}
		
		usleep(300*1000);
		
		printf("[main] write LED_G ON\r\n");
		rc = writeLed(ctx, POS_LED_GREEN, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_BLUE);
			goto close;
		}
		
		usleep(300*1000);
	
		printf("[main] write LED_G OFF\r\n");
		rc = writeLed(ctx, POS_LED_GREEN, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		usleep(300*1000);
		
		printf("[main] write LED_B ON\r\n");
		rc = writeLed(ctx, POS_LED_BLUE, ON);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_GREEN);
			goto close;
		}
		
		usleep(300*1000);
		
		printf("[main] write LED_B OFF\r\n");
		rc = writeLed(ctx, POS_LED_BLUE, OFF);
		if( rc < 0 )
		{
			printf("[main] Write LED error (%d)\r\n", POS_LED_BLUE);
			goto close;
		}
		
		usleep(300*1000);
	}
#endif

close:

	pthread_mutex_destroy(&gMutex);
	
    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}
