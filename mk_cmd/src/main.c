//
//  main.c
//  satellite
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

#include "common.h"
#include "cmd.h"

#define INPUT_BUF_SIZE 1024

uint8 hk_cmd[]      = { 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,0x40|CMD_RUN_CMD, 0x81};

int fd;

uint8 input_buf[INPUT_BUF_SIZE];
uint8 cmd_buf[INPUT_BUF_SIZE];


int main(int argc, const char * argv[])
{

	uint8 file[64] = {0};
	uint8 buf[255] = {0};
	sprintf(file,"./%s", argv[1]);

	int fp = open( file, O_RDWR | O_NOCTTY| O_CREAT, S_IREAD | S_IWRITE	);
	if( fp < 0 ){
		printf("file is not founded.");
		return -1;
	}

	uint8 cmd_len = sizeof(hk_cmd);
	uint8 len = cmd_len + strlen(argv[2]);

	buf[0] = 0x80;
	buf[1] = 0x80;
	buf[2] = len;
	memcpy(&buf[3],hk_cmd,sizeof(hk_cmd));
	memcpy(&buf[3+cmd_len],argv[2],strlen(argv[2]));

	//CRC
	uint16 u16_crc = crc( &buf[3], buf[2] );
	buf[3+buf[2]] = (uint8)((u16_crc & 0xff00) >> 8);
	buf[3+buf[2]+1] = (uint8)((u16_crc & 0x00ff));
	buf[3+buf[2]+2] =0x81;
	buf[3+buf[2]+3] =0x81;


	write(fp, buf, len+7); // STX LEN CRC ETX

	close(fp);
	return 0;
}





