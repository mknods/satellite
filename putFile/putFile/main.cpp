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
#include "time.h"
#include <stdlib.h>

#include "common.h"
#include "crc.h"
#include "cmd.h"



int init_serial();


#define B921600 921600

using namespace std;


uint8 header[] =
{ 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,CMD_PUT_FILE, 0x82};

uint8 stx[] = { 0x80,0x80 };
uint8 etx[] = { 0x81,0x81 };


int main(int argc, const char * argv[])
{

    string in_file =  "/Users/jiropost/out/Icon_1024.png";
    string out_file = "/Users/jiropost/out/put.data";
    string filename = "icon4";
    
    int fp_in;
    int fp_sci;
    int fp_out;
    int dev_id = 0x20;
    int len;
    
    uint8 out_buf[255];
    uint8 in_buf[255];
    int out_size = 0;
    
    
    fp_sci = init_serial();
    if(fp_sci < 0){

        printf("sci open err\n");
        return -1;
    }
    
    fp_in = open( in_file.c_str(), O_RDONLY );
    if(fp_in < 0){
        
        printf("file open err\n");
        return -1;
    }

    fp_out = open( out_file.c_str(),
                    O_RDWR | O_CREAT,
                    S_IREAD | S_IWRITE	);
    if(fp_out < 0){
        
        printf("file open err\n");
        return -1;
    }
    
    //時刻
    time_t t;
    time(&t);
    out_buf[4] = (uint8)((0xff000000ul & (uint32)t) >> 24);
    out_buf[5] = (uint8)((0x00ff0000ul & (uint32)t) >> 16);
    out_buf[6] = (uint8)((0x0000ff00ul & (uint32)t) >> 8);
    out_buf[7] = (uint8)(0x000000fful & (uint32)t);
    
    header[10] |= dev_id;
    
    do{
        memset( out_buf, 0, 255);
        memset( in_buf, 0, 255);
        out_size = 0;
        
        len = read(fp_in, in_buf, 185-9);
        
        //STX
        memcpy( &out_buf[0], stx, sizeof(stx));
        
        //LEN
        out_buf[2] = 12 + 9 + len;
        
        //CMD
        memcpy( &out_buf[3], header, sizeof(header));
        
        //DATA FILENAME(fix:8byte)
        memcpy( &out_buf[15], filename.c_str(), sizeof(filename));
        
        //DATA LEN(fix:1byte)
        out_buf[23] = len;
        
        //DATA
        memcpy( &out_buf[24], in_buf, len );
        
        out_size = 3 + out_buf[2];
        //CRC
		uint16 u16_crc = crc( &out_buf[3], out_buf[2] );
		out_buf[out_size] = (uint8)((u16_crc & 0xff00) >> 8);
		out_buf[out_size+ 1] = (uint8)((u16_crc & 0x00ff));
        
        //ETX
        memcpy( &out_buf[out_size+2], etx, sizeof(stx));

        out_size += 7;  //STX LEN CRC ETX

        int ret = write(fp_sci, out_buf, out_size);
        if(ret < 0 ){
            printf("serial write err\n");
            break;
        }
        ret = write(fp_out, out_buf, out_size);
        if(ret < 0 ){
            printf("file write err\n");
            break;
        }
        
        
        usleep(10000);
        
    } while(len > 0);

    close(fp_in);
    close(fp_out);
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
		return -1;
	}

    return fd_sci0;
    
}

