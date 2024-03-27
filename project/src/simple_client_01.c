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

#if 1
int main(int argc, char **argv)
{
	int i, j;
	modbus_t *ctx;
	int rc;
	int addr = 1;
	int16_t regs[10] = {101, 102 };
	int16_t regs2[10];
	int nb = sizeof(regs)/sizeof(int16_t);

	// connect to server
	ctx = modbus_new_tcp("127.0.0.1", MODBUS_PORT); 
	if (modbus_connect(ctx) == -1) {
		fprintf(stderr,"Connection failed: %s\n",modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	modbus_set_debug(ctx, FALSE);

	// write regs
	for(j = 0; j != nb; j++)
	{
		regs[j] = rand() % 100;
	}
	rc = modbus_write_registers(ctx, 1, nb, regs);
	if (rc != nb) 
	{
		printf("ERROR modbus_write_registers (%d)\n", rc);
		printf("Address = %d, nb = %d\n", 1, nb);
		return -1;
	} 

	rc = modbus_read_registers(ctx, 1, nb, regs2);
	if (rc != nb)
	{
		printf("ERROR modbus_read_registers (%d)\n", rc);
		printf("Address = %d, nb = %d\n", addr, nb);
		return -1;
	} 

	// print and check the response
	printf("regs2[] =\t");
	for(j = 0; j != nb; j++) 
	{
		if(regs[j] != regs2[j]) 
		{
			printf("FAILED to read back what we wrote at %d\n", j);
			exit(103);
		}
		printf("%d ", regs2[j] );
	}
	printf("\n");

	return 0;
}
#else
int main(int argc, char **argv)
{
	modbus_t *ctx;
	int rc;
	int value;
	uint16_t regs[10];
	
	if( (ctx = modbus_new_tcp("127.0.0.1", MODBUS_PORT)) == NULL )
	{
		printf("Initial fail\r\n");
		return -1;
	}
	
	if( modbus_connect(ctx) == -1 )
	{
		printf("Connection fail\r\n");
		modbus_free(ctx);
		return -1;
	}
	
	while( 1 )
	{
		if( (rc = modbus_write_register(ctx, 0, 10)) == -1 )
		{
			printf("Write error: %s\r\n", modbus_strerror(errno));
			break;
		}
		
		if( (rc = modbus_read_registers(ctx, 0, 1, regs)) == -1 )
		{
			printf("Read error: %s\r\n", modbus_strerror(errno));
			break;
		}
		
		printf("Read: %d\r\n", regs[0]);
		
		break;
	}
	
	modbus_free(ctx);
	
	return 0;
}
#endif
// end of file

