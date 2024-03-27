#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <modbus/modbus.h>

#define MODBUS_PORT	1503
#define MODBUS_CONNECTIONS	5

int main(int argc, char **argv)
{
	modbus_t *ctx = NULL;
	int server_socket;
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	modbus_mapping_t *mb_mapping;
	int rc;
	int i=0;
	
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
		}
		
		printf("Quit the loop: %s\n", modbus_strerror(errno));
	}
  
	if (server_socket != -1) {
		close(server_socket);
	}
	modbus_mapping_free(mb_mapping);
	modbus_close(ctx);
	modbus_free(ctx);
  	
	return 0;
}

// end of file

