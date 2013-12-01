//
//  main.cpp
//  putFile
//
//  Created by Yujiro Miyabayashi on 12/2/13.
//  Copyright (c) 2013 mknod. All rights reserved.
//

#include <iostream>
#include <string>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "common.h"
#include "crc.h"
#include "cmd.h"


int init_serial();



int main(int argc, const char * argv[])
{

    string in_file = "Users/jiropost/out/icon_1024.png";

    int fP_in;
    int fp_sci;
    
    fp_sci = init_serial()

    
    
    
    
    
    
    close(fp_sci);
    
    return 0;
}


int init_serial(){

    int fd_sci0 = open("/dev/tty.usbserial-FTGCYCXZ", O_RDWR | O_NOCTTY | O_NDELAY);	//USB->TTL
    if(fd_sci0 == -1){
        perror("open error");
        return -1;
    }

    struct termios options;
	tcgetattr(fd_sci0, &options);
    
	cfsetispeed(&options, B921600);
	cfsetospeed(&options, B921600);
    //	cfsetispeed(&options, B115200);
    //	cfsetospeed(&options, B115200);
	options.c_cflag = CS8 | CLOCAL | CREAD;				// <Set baud rate
	options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfmakeraw(&options);
	tcflush(fd_sci0, TCIFLUSH);
	if (tcsetattr(fd_sci0, TCSANOW, &options) < 0){
		printf("unable to set comm port");
		return -1å;
	}

    retunr fd_sci0;
    
}

