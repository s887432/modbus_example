#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus/modbus.h>

#include "modbus_ex.h"

int writeLed(modbus_t *ctx, POS_ADDR addr, BIT_VALUE val)
{
	int rc = 0;
	uint8_t read_data;
	
	rc = modbus_write_bit(ctx, addr, val);
    if (rc != 1) {
        printf("FAILED\n");
        return -1;
    }

    rc = modbus_read_bits(ctx, addr, 1, &read_data);
    if (rc != 1) {
        printf("FAILED (nb points %d)\n", rc);
        return -1;
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    modbus_t *ctx;
    int rc;
    int i;

	if( argc != 2 )
	{
		printf("USAGE: simple_server_03 SERVER_IP\r\n");
		return -1;
	}
	
    ctx = modbus_new_tcp(argv[1], 1502);
    //modbus_set_debug(ctx, TRUE);
    modbus_set_error_recovery(ctx,
                              MODBUS_ERROR_RECOVERY_LINK |
                              MODBUS_ERROR_RECOVERY_PROTOCOL);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    printf("\nTEST WRITE/READ:\n");

	for(i=0; i<10; i++)
	{
		printf("*******************************************************************\r\n");
		printf("Turn on LED_RED\r\n");
		rc = writeLed(ctx, POS_LED_RED, ON);
		if( rc < 0 )
		{
			printf("Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		sleep(1);

		printf("*******************************************************************\r\n");
		printf("Turn off LED_RED\r\n");
		rc = writeLed(ctx, POS_LED_RED, OFF);
		if( rc < 0 )
		{
			printf("Write LED error (%d)\r\n", POS_LED_RED);
			goto close;
		}
		
		sleep(1);
	}

close:

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}

// end of file
