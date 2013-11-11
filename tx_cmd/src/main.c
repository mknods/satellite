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


int fd;

uint8 input_buf[INPUT_BUF_SIZE];
uint8 cmd_buf[INPUT_BUF_SIZE];


int main(int argc, const char * argv[])
{
    int fd_sci0 = open("/dev/tty.usbserial-FTGCVJGW", O_RDWR | O_NOCTTY | O_NDELAY);	//USB->TTL
    if(fd_sci0 == -1){
        perror("open error");
		return -1;
    }

    struct termios options;
	tcgetattr(fd_sci0, &options);

	cfsetispeed(&options, 921600);
	cfsetospeed(&options, 921600);
	options.c_cflag = CS8 | CLOCAL | CREAD;				// <Set baud rate
	options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfmakeraw(&options);
	tcflush(fd_sci0, TCIFLUSH);
	if (tcsetattr(fd_sci0, TCSANOW, &options) < 0){
		printf("unable to set comm port");
		return -1;
	}

	//転送コマンドファイル(バイナリ)
	uint8 file[64] = {0};
	uint8 buf[255] = {0};
	sprintf(file,"%s", argv[1]);

	int fp = open( file, O_RDWR | O_NOCTTY| O_CREAT, S_IREAD | S_IWRITE	);
	if( fp < 0 ){
		printf("file is not founded.");
		return -1;
	}

	int len = read(fp, buf, 255);
	printf("len=%d \n", len);
	int i;
	for(i=0; i< len; i++ ){
		printf("%x ",buf[i]);
	}
	printf("\n");

	//OUT SERAIL
	int ret = write(fd_sci0, buf, len); // STX LEN CRC ETX
	if(ret < 0){
		printf("err\n");
	}

	close(fd_sci0);
	close(fp);
	return 0;
}





