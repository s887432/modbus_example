#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#include <modbus/modbus.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "modbus_ex.h"

#define NB_CONNECTION    5

#define LED_RED_DEV			"/sys/class/leds/red/brightness"
#define LED_GREEN_DEV		"/sys/class/leds/green/brightness"
#define LED_BLUE_DEV		"/sys/class/leds/blue/brightness"

static int gLedRedfd = -1;
static int gLedGreenfd = -1;
static int gLedBluefd = -1;

modbus_t *ctx = NULL;
int server_socket = -1;
modbus_mapping_t *mb_mapping;

int gKeyStatus = 0;

void key_proc(void *param)
{
	int keys_fd;
	struct input_event t;
	
	//pthread_obj obj = (pthread_obj)param;
	
	if( (keys_fd = open("/dev/input/event0", O_RDONLY)) <= 0 )
	{
		printf("open event error\r\n");
		return;
	}
	
	while(1)
	{
		if( read(keys_fd, &t, sizeof(t)) == sizeof(t))
		{
			if( t.type == EV_KEY )
			{
				if( t.value ==0 || t.value == 1 )
				{
					printf("key %d %s\n", t.code, (t.value) ? "Pressed" : "Released");
				}
				
				if( t.value )
				{
					gKeyStatus = 1;
				}
			}
		}
		
		usleep(100*1000);
	}
	
	close(keys_fd);
}

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

static void close_sigint(int dummy)
{
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main(void)
{
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int master_socket;
    int rc;
    fd_set refset;
    fd_set rdset;
    /* Maximum file descriptor number */
    int fdmax;
    
    int header_length;
    uint8_t *ptrPackage;
    SingleBit_t SingleBitValue;

    pthread_t threadKeyID;
    
    ledInit();

	// ********************************************************************************
	// key event monitor
	// ********************************************************************************    
	if( pthread_create(&threadKeyID, NULL, (void*)key_proc, NULL) != 0 )
	{
		printf("Create key process thread fail\r\n");
	}
	
	pthread_detach(threadKeyID);
	pthread_join(threadKeyID, NULL);
	// ********************************************************************************
	
    ctx = modbus_new_tcp(NULL, 1502);
    header_length = modbus_get_header_length(ctx);

    mb_mapping = modbus_mapping_new(10, 10, 0, 0);
    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);

    signal(SIGINT, close_sigint);

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for (;;) {
        rdset = refset;
        if (select(fdmax+1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Server select() failure.");
            close_sigint(1);
        }

        /* Run through the existing connections looking for data to be
         * read */
        for (master_socket = 0; master_socket <= fdmax; master_socket++) {

            if (!FD_ISSET(master_socket, &rdset)) {
                continue;
            }

            if (master_socket == server_socket) {
                /* A client is asking a new connection */
                socklen_t addrlen;
                struct sockaddr_in clientaddr;
                int newfd;

                /* Handle new connections */
                addrlen = sizeof(clientaddr);
                memset(&clientaddr, 0, sizeof(clientaddr));
                newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
                if (newfd == -1) {
                    perror("Server accept() error");
                } else {
                    FD_SET(newfd, &refset);

                    if (newfd > fdmax) {
                        /* Keep track of the maximum */
                        fdmax = newfd;
                    }
                    printf("New connection from %s:%d on socket %d\n",
                           inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
                }
            } else {
                modbus_set_socket(ctx, master_socket);
                rc = modbus_receive(ctx, query);
                if (rc > 0) {
                
                	ptrPackage = query+header_length;
                	switch( *ptrPackage )
					{
						case MODBUS_CMD_WRITE_SIGNLE_COIL: // 5
							// write single coil
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							printf("Write single bit: %d, %s\r\n", SingleBitValue.address, SingleBitValue.value==0xFF00?"ON":"OFF");
							
							if( SingleBitValue.address == POS_LED_RED ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_RED) : ledClean(LED_RED);
							} else if ( SingleBitValue.address == POS_LED_GREEN ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_GREEN) : ledClean(LED_GREEN);
							} else if( SingleBitValue.address == POS_LED_BLUE ) {
								SingleBitValue.value==0xFF00 ? ledSet(LED_BLUE) : ledClean(LED_BLUE);
							}
							break;
							
						case MODBUS_CMD_READ_DESCRETE_INPUTS: // 2
							// read bits
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							printf("Read descrete inputs bit: %d, %d\r\n", SingleBitValue.address, SingleBitValue.value);
							
							if( SingleBitValue.address == POS_KEY_RED )
							{
								printf("Read single bit: %d\r\n", SingleBitValue.address);

								if( gKeyStatus == 1 )
								{
									printf("Key pressed\r\n");
									mb_mapping->tab_input_bits[POS_KEY_RED] = 1;
								}
							}
							break;
					}
					
                    modbus_reply(ctx, query, rc, mb_mapping);
                    
                    switch( *ptrPackage )
					{
						case MODBUS_CMD_READ_DESCRETE_INPUTS: // 2
							// read bits
							SingleBitValue.address = *(ptrPackage+1) << 8 | *(ptrPackage+2);
							SingleBitValue.value = *(ptrPackage+3) << 8 | *(ptrPackage+4);
							
							if( SingleBitValue.address == POS_KEY_RED )
							{
								if( gKeyStatus == 1 )
								{
									gKeyStatus = 0;
									mb_mapping->tab_input_bits[POS_KEY_RED] = 0;
								}
							}
												
							break;
					}
			
                } else if (rc == -1) {
                    /* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax) {
                        fdmax--;
                    }
                }
            }
        }
    }

    return 0;
}
