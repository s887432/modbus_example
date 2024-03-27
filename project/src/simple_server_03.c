#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <linux/input.h>

#include <modbus/modbus.h>

#define MODBUS_PORT	1503
#define MODBUS_CONNECTIONS	5

#define KEY_DEV				"/dev/input/event0"
static int gKeyfd = -1;

#define LED_RED_DEV			"/sys/class/leds/red/brightness"
#define LED_GREEN_DEV		"/sys/class/leds/green/brightness"
#define LED_BLUE_DEV		"/sys/class/leds/blue/brightness"

static int gLedRedfd = -1;
static int gLedGreenfd = -1;
static int gLedBluefd = -1;

#define MODBUS_DATA_POSITION_LED_RED	0
#define MODBUS_DATA_POSITION_LED_GREEN	1
#define MODBUS_DATA_POSITION_LED_BLUE	2
#define MODBUS_DATA_POSITION_KEY		3

typedef enum __LED_ID__
{
	LED_RED = 1,
	LED_GREEN,
	LED_BLUE
}LED_ID;

int ledInit(void)
{
	if( (gLedRedfd = open(LED_RED_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_RED_DEV);
		return -1;
	}
	
	if( (gLedGreenfd = open(LED_GREEN_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_GREEN_DEV);
		return -1;
	}
	
	if( (gLedBluefd = open(LED_BLUE_DEV, O_WRONLY)) < 0 )
	{
		printf("Fail to open %s device\r\n", LED_BLUE_DEV);
		return -1;
	}
	
	return 0;
}

void ledClose(void)
{
	if( gLedRedfd >= 0 )
		close(gLedRedfd);
	if( gLedGreenfd >= 0 )
		close(gLedGreenfd);
	if( gLedBluefd >= 0 )
		close(gLedBluefd);
}

int ledSet(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "255", 3);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "255", 3);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "255", 3);
			break;
		default:
			return -1;
	}
	
	return 0;
}

int ledClean(LED_ID id)
{
	switch( id )
	{
		case LED_RED:
			if( gLedRedfd < 0 )
				return -1;
			write(gLedRedfd, "0", 1);
			break;
		case LED_GREEN:
			if( gLedGreenfd < 0 )
				return -1;
			write(gLedGreenfd, "0", 1);
			break;
		case LED_BLUE:
			if( gLedBluefd < 0 )
				return -1;
			write(gLedBluefd, "0", 1);
			break;
		default:
			return -1;
	}
	
	return 0;
}

int keyInit(void)
{
	if( (gKeyfd = open(KEY_DEV, O_RDONLY)) <= 0 )
	{
		printf("open event error\r\n");
		return -1;
	}
	
	return 0;
}

void keyClose(void)
{
	close(gKeyfd);
}

void thread_proc(void *param)
{
	int keys_fd;
	struct input_event t;
	modbus_mapping_t *mb_mapping = (modbus_mapping_t *)param;
	
	while(1)
	{
		if( read(keys_fd, &t, sizeof(t)) == sizeof(t))
		{
			if( t.type == EV_KEY )
			{
				if( t.value ==0 || t.value == 1 )
				{
					printf("key %d %s (%d)\n", t.code, (t.value) ? "Pressed" : "Released", t.value);
					
					if( t.value == 0 )
						mb_mapping->tab_registers[MODBUS_DATA_POSITION_KEY] = 1;
						
					if( t.code == KEY_ESC )
					{
						break;
					}
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	modbus_t *ctx = NULL;
	int server_socket;
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	modbus_mapping_t *mb_mapping;
	int rc;
	int i=0;
	pthread_t threadID;
	
	if( ledInit() < 0 )
	{
		printf("Initial LED error......\r\n");
	}
	
	if( keyInit() < 0 )
	{
		printf("Initial KEY error......\r\n");		
	}
	
	memset(query, 0, MODBUS_TCP_MAX_ADU_LENGTH);
	ctx = modbus_new_tcp(NULL, MODBUS_PORT);
	
	mb_mapping = modbus_mapping_new(0, 0, 500, 500);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	if( (server_socket = modbus_tcp_listen(ctx, MODBUS_CONNECTIONS)) == -1 )
	{
		printf("Unable to listen TCP connection\r\n");
		modbus_free(ctx);
		return -1;
	}
	
	if( pthread_create(&threadID, NULL, (void *) thread_proc, mb_mapping->tab_registers) != 0 )
	{
        printf ("Create key handler pthread error!n");
    }
	
	while(1)
	{
		modbus_tcp_accept(ctx, &server_socket);
		
		while(1)
		{
			printf("%d---", i++);
			rc = modbus_receive(ctx, query);
			printf("SLAVE: regs[] =\t");
			for(int i = 1; i < 10; i++) { // looks like 1..n index
				printf("%d ", mb_mapping->tab_registers[i]);
			}
			printf("\r\n");
			
			if (rc > 0) {
				/* rc is the query size */
				modbus_reply(ctx, query, rc, mb_mapping);
			}
			else if (rc == -1) {
				/* Connection closed by the client or error */
				break;
			}
			
			mb_mapping->tab_registers[MODBUS_DATA_POSITION_LED_RED] == 0 ? ledClean(LED_RED) : ledSet(LED_RED);
			mb_mapping->tab_registers[MODBUS_DATA_POSITION_LED_GREEN] == 0 ? ledClean(LED_GREEN) : ledSet(LED_GREEN);
			mb_mapping->tab_registers[MODBUS_DATA_POSITION_LED_BLUE] == 0 ? ledClean(LED_BLUE) : ledSet(LED_BLUE);
		}
		
		printf("Quit the loop: %s\n", modbus_strerror(errno));
	}
  
	if (server_socket != -1) {
		close(server_socket);
	}
	modbus_mapping_free(mb_mapping);
	modbus_close(ctx);
	modbus_free(ctx);
  	
  	ledClose();
  	keyClose();
  	
	return 0;
}

// end of file

