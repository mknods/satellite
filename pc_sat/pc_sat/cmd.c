#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include "common.h"
#include "crc.h"
#include "cmd.h"

#define B921600 921600

int fd;
uint8 tx_buf[256];
uint8 rx_buf[1024];

uint8 header[] = { 0x80,0x80 };

//uint8 hk_cmd[]      = { 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,DEVICE_ID|CMD_RUN_CMD, 0x81};
//uint8 get_file_cmd[] = { 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,DEVICE_ID|CMD_GET_FILE,0x81};
uint8 hk_cmd[]      = { 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,CMD_RUN_CMD, 0x82};
uint8 get_file_cmd[] = { 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,CMD_GET_FILE,0x82};

// halt -p
// rm -f /sat/mov/* && rm -f /sat/out/*
// raspivid -n -o /sat/mov/out.h264 -t 1000 && MP4Box -fps 30 -add /sat/mov/out.h264 /sat/out/out.mp4 &0000 && MP4Box -fps 30 -add /sat/mov/out.h264 /sat/out/out.mp4 &
/*
uint8 param[][185] = {
		"raspivid -o /sat/mov/out.h264 -t 30000 && MP4Box -fps 30 -add /sat/mov/out.h264 /sat/out/out.mp4 &",
		"/sat/sh/tl.sh 10 1920x1080 5 5 /dev/video0 65 15&",
		"/sat/sh/gpio1.sh",
		"/sat/sh/gpio0.sh",
		"/sat/sh/temp.sh",
		"/sat/sh/lux.sh",
		"python /sat/sh/irs.py",
		"/sat/sh/record.sh",
		//"ls -l /sat/img/*",
		"cd /sat/out/ && split -b 1m img.tgz dt. && ls -l",
//		"/sat/img/IMG_10.jpg",	// ファイル取得
//		"/sat/out/dat",	// ファイル取得
		"/sat/out/dt.af",	// ファイル取得
//		"/sat/out/out.mp4",	// ファイル取得
//		"/sat/out/img.tgz",	// ファイル取得
};
*/
//ファイル転送
uint8 put_file_cmd[] =
	{ 0x00,0x00,0x02,0x00,0x12,0x34,0x56,0x78,0x00,0x00,CMD_PUT_FILE, 0x82};
	//   header(12bytes)
	// + filename (8bytes)
	// + len(1bytes)
	// + data(data bytes)

int fd_out = -1;
uint8 out_file[] = "/Users/jiropost/out"; // LOCAL DL FILE

uint8 footer[] = { 0x81,0x81 };

//UART
#define BUF_SIZE    ((uint32)8192)
#define PACKET_SIZE (255)

uint8 sci_buf[BUF_SIZE];
uint32 pos = 0;
uint32 pos_r = 0;

//COMMAND
uint8 cmd_buf[PACKET_SIZE];
CMD_DATA cmd_dt[10];
uint8 cmd_str[256];
uint8 cmd_len = 0;
uint8 dt_cnt = 0;
uint8 app_st = APP_ST_NONE;

uint16 crc_val = 0;

long rcv_length=0;

int fd_sci0;

uint8 ofilename[64];


void end(){

	close(fd_sci0);
	close(fd_out);
}

void run_cmd(uint8* p_param, uint8 dev_id){

	int len;
	int param_len = strlen( p_param );
	memcpy( &tx_buf[0], header, 2 );

	hk_cmd[10] &= 0x0f;
	hk_cmd[10] |= dev_id;

//	hk_cmd[10] &= ~(0xE0);
//	hk_cmd[10] |= dev_id;
	len = sizeof(hk_cmd);
	memcpy( &tx_buf[3], hk_cmd, len);

	tx_buf[2] = (uint8)len + param_len;

	memcpy( &tx_buf[3+len], p_param, param_len );

	len += param_len;

	uint16 u16_crc = crc( &tx_buf[3], len );
	tx_buf[3+len] = (uint8)((u16_crc & 0xff00) >> 8);
	tx_buf[3+len+1] = (uint8)((u16_crc & 0x00ff));
	memcpy( &tx_buf[3+len+2], footer, 2 );

	for(int i=0; i< len+7; i++){
		printf("%02x",tx_buf[i]);
	}
	printf("\n");

	long z = write(fd_sci0, tx_buf, len+7 );
	if( z < 0 ){
		printf("write error\n");
	}
}


void get_file( uint8* p_param , uint8 dev_id){

	int len;
	int param_len = strlen( p_param );
	memcpy( &tx_buf[0], header, 2 );

	strcpy(ofilename, p_param);

	//FILE作成
	uint8 out[64] = {0};
	sprintf(out, "%s%s",out_file, ofilename);
	printf("%s\n",out);

	if(fd_out >= 0){
		close(fd_out);
	}
	fd_out = open(
			out,
			O_NOCTTY | O_NONBLOCK |O_CREAT| O_RDWR,
			S_IRWXU|S_IRWXG|S_IRWXO);
	if(fd_out<0){
		perror("[get_file] can't create file.\n");
		return;
	}

	get_file_cmd[10] &= 0x0f;
	get_file_cmd[10] |= dev_id;

	len = sizeof(get_file_cmd);
	memcpy( &tx_buf[3], get_file_cmd, len);

	tx_buf[2] = (uint8)len + param_len;

	memcpy( &tx_buf[3+len], p_param, param_len );

	len += param_len;

	uint16 u16_crc = crc( &tx_buf[3], len );
	tx_buf[3+len] = (uint8)((u16_crc & 0xff00) >> 8);
	tx_buf[3+len+1] = (uint8)((u16_crc & 0x00ff));
	memcpy( &tx_buf[3+len+2], footer, 2 );

//	for(int i=0; i< len+7; i++){
//		printf("%02x",tx_buf[i]);
//	}
//	printf("\n");

	long z = write(fd_sci0, tx_buf, len+7 );
	if( z < 0 ){
		printf("write error\n");
	}
}



void put_file(uint8* file_name, uint8 dev_id){

	uint8 file[64] = {0};
	uint8 buf[255] = {0};
	sprintf(file,"/Users/jiropost/out/put/%s", file_name);

	int fp = open( file, O_RDONLY | O_NOCTTY);
	if( fp < 0 ){
		printf("[put_file] file is not founded.");
		return;
	}

	uint8 len;
	int param_len = strlen( file_name );
	memcpy( &tx_buf[0], header, 2 );

	put_file_cmd[10] &= 0x0f;
	put_file_cmd[10] |= dev_id;
	memcpy( &tx_buf[3], put_file_cmd, sizeof(put_file_cmd));

	strcpy( &tx_buf[15], file_name);

	while(1){

		len = read( fp, buf, CMD_PARAM_SIZE-9 );
		if(len <= 0){
			printf("[put_file] end of file\n");
			break;
		}
		//LEN
		tx_buf[2] = 12+9+len;

		// clear
		for(int i=23; i< 255; i++){
			tx_buf[i]=0x00;
		}

		//set new data
		tx_buf[23] = len; //len
		for(int i=0; i< len; i++){
			tx_buf[24+i] = buf[i];
			printf("%02x ",tx_buf[i]);
		}
		printf("\n");

		//CRC
		uint16 u16_crc = crc( &tx_buf[3], tx_buf[2] );
		tx_buf[3+tx_buf[2]] = (uint8)((u16_crc & 0xff00) >> 8);
		tx_buf[3+tx_buf[2]+1] = (uint8)((u16_crc & 0x00ff));

		//ETX
		memcpy( &tx_buf[3+tx_buf[2]+2], footer, 2 );

		// SEND BY UART
		long z = write(fd_sci0, tx_buf, tx_buf[2]+7 );
		if( z < 0 ){
			printf("write error\n");
		}
		usleep(100000);
	}

	return;
}



void drv_rcv(){

    printf("drv_rcv\n");

    //==============
    //	OPEN
    //==============

//    fd_sci0 = open("/dev/tty.usbserial-FTGQL42V", O_RDWR | O_NOCTTY | O_NDELAY);	//USB->SERIAL
//    fd_sci0 = open("/dev/tty.usbserial-FTGCVJGW", O_RDWR | O_NOCTTY | O_NDELAY);	//USB->TTL
    fd_sci0 = open("/dev/tty.usbserial-FTGCYCXZ", O_RDWR | O_NOCTTY | O_NDELAY);	//USB->TTL (ls -l /dev/tty.*)
    
    if(fd_sci0 == -1){
        perror("open error");
        return;
    }

    struct termios options;
	tcgetattr(fd_sci0, &options);

	cfsetispeed(&options, B921600);
	cfsetospeed(&options, B921600);
	options.c_cflag = CS8 | CLOCAL | CREAD;				// <Set baud rate
	options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfmakeraw(&options);
	tcflush(fd_sci0, TCIFLUSH);
	if (tcsetattr(fd_sci0, TCSANOW, &options) < 0){
		printf("unable to set comm port");
		return;
	}


//	sprintf(ofilename, "%s%s",out_file, param[NUM_GET_FILE]);
//	printf("%s\n",ofilename);
//
//	fd_out = open(
//			ofilename,
//			O_CREAT| O_RDWR,
//			S_IRWXU|S_IRWXG|S_IRWXO);
//	if (fd_out < 0) {
//		perror("open error");
//		return;
//	}

//	while(1){
//
//    	memset( rx_buf, (uint8)0x00, 255 );
//
//		int len = read(fd_sci0, rx_buf, 255);
//		if(len>0){
//			write(fd_out, rx_buf, len);
//		}
//    }
//
#if 1

	int len = 0;
    while(1){

    	memset( rx_buf, (uint8)0x00, 255 );

		len = read(fd_sci0, rx_buf, 255);
		for( int i=0; i < len; i++){

			sci_buf[pos] = rx_buf[i];

			switch (app_st){
			case APP_ST_NONE:
				dt_cnt = 0;
				if( sci_buf[pos] == STX ){
//					printf("STX [%u] = %x\n", pos,sci_buf[pos]);
					app_st = APP_ST_HEADER;
				}
				break;
			case APP_ST_HEADER:
				if( sci_buf[pos] == STX ){
//					printf("STX [%u] = %x\n", pos,sci_buf[pos]);
					app_st = APP_ST_LEN;
				}else{
					app_st = APP_ST_NONE;
				}
				break;

			case APP_ST_LEN:
//				printf("LEN [%u] = %x\n", pos,sci_buf[pos]);
				app_st = APP_ST_CMD;
				cmd_len = sci_buf[pos];

				memset(cmd_buf, (uint8)0, PACKET_SIZE);
				//copy cmd
				dt_cnt = 0;
				break;

			case APP_ST_CMD:
/*
				if(dt_cnt == 0){
					printf("TLM_CMD_DISCRIMINATION [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 1){
					printf("TLM_ID [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 2){
					printf("TLM_COUNT [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 3){
					printf("TIME [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 4){
					printf("TIME [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 5){
					printf("TIME [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 6){
					printf("TIME [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 7){
					printf("CMD_CODE [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 8){
					printf("CMD_STS [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 9){
					printf("CMD_ERROR_STS [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 10){
					printf("CMD_COUNT [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 11){
					printf("TLM packet ID [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 12){
					printf("FROM_ID [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 13){
					printf("From_ID_Sub [%u] = %x\n", pos,sci_buf[pos]);
				}else if(dt_cnt == 14){
					printf("TO_ID [%u] = %x\n", pos,sci_buf[pos]);
				}else{
					printf("PARAM [%u] = %x\n", pos,sci_buf[pos]);
				}
*/
//				if(dt_cnt >= 15){
//					printf("%02x", sci_buf[pos]);
//				}
//					if(dt_cnt == 2){
//						printf("TLM_COUNT [%u] = %x\n", pos,sci_buf[pos]);
//					}


				cmd_buf[dt_cnt] = sci_buf[pos];
				dt_cnt++;
				if(dt_cnt >= cmd_len){
					app_st = APP_ST_CRC1;
				}
				break;

			case APP_ST_CRC1:

//				printf("CRC1 [%u] = %x\n", pos,sci_buf[pos]);
				crc_val = 0;
				crc_val = (uint16)sci_buf[pos] << 8;
				app_st = APP_ST_CRC2;
				break;

			case APP_ST_CRC2:

//				 printf("CRC2 [%u] = %x", pos,sci_buf[pos]);
				crc_val |= (uint16)sci_buf[pos];

				uint16 tmp = crc(cmd_buf, cmd_len);
//				printf(" CALC CRC 0x%02x  RECV 0x%x len=%d ",tmp, crc_val, cmd_len);

				if( crc_val == tmp ){
//					printf("\n");
//					printf(" [OK] CRC OK\n");
					app_st = APP_ST_FOOTER1;
				}else{
					printf(" [ERR] BAD CRC\n");
					app_st = APP_ST_NONE;
				}
				break;

			case APP_ST_FOOTER1:
				if( sci_buf[pos] == ETX ){
//					printf("FOOTER1 [%u] = %x\n", pos,sci_buf[pos]);
					app_st = APP_ST_FOOTER2;
				}else{
					app_st = APP_ST_NONE;   //ERROR
				}
				break;

			case APP_ST_FOOTER2:
				if( sci_buf[pos] == ETX ){
//					printf("FOOTER2 [%u] = %x\n", pos,sci_buf[pos]);

					int cmd_sts = TLM_CMD_STS-3;
					int param_offset = TLM_PARAM-3;

					int param_len = cmd_len - TLM_DEFAULT_SIZE;

					//VALID COMMAND
					if( cmd_buf[TLM_PACKET_ID] == MISSION_DATA ){

//						printf("CMD_STATUS = %x\n", cmd_buf[cmd_offset]);

						if( cmd_buf[cmd_sts] == CMD_STATUS_RECEIVED ){

//							if(fd_out < 0){
//
//								uint8 out[64] = {0};
//								sprintf(out, "%s%s",out_file, ofilename);
//								printf("%s\n",out);
//
//								fd_out = open(
//										out,
//										O_NOCTTY | O_NONBLOCK |O_CREAT| O_RDWR,
//										S_IRWXU|S_IRWXG|S_IRWXO);
//							}
//							if (fd_out < 0) {
//								perror("open error");
//								return;
//							}else{
								write(fd_out, &cmd_buf[param_offset], param_len);
//							}

						}else if( cmd_buf[cmd_sts] == CMD_STATUS_COMPLETED ){

//							if(fd_out < 0){
//								uint8 out[64] = {0};
//								sprintf(out, "%s%s",out_file, ofilename);
//								printf("%s\n",out);
//
//								fd_out = open(
//										out,
//										O_RDWR | O_NOCTTY | O_NONBLOCK |O_CREAT| O_RDWR,
//										S_IRWXU|S_IRWXG|S_IRWXO);
//							}
//							if(fd_out >= 0){
								if( write(fd_out, &cmd_buf[param_offset], param_len) < 0 ){
									perror("write error");
									close(fd_out);
									return;
								}

								close(fd_out);
								break;
//							}else{
//								perror("CMD_STATUS_COMPLETED else\n");
//								break;
//							}
						}else{
//							if( fd_out >= 0 ){
								if( write(fd_out, &cmd_buf[param_offset], param_len) < 0 ){
									perror("write error\n");
									close(fd_out);
									return;
								}
//							}else{
//								close(fd_out);
//								perror("inprogress else\n");
//							}
						}
					}
					app_st = APP_ST_NONE;       // END
					break;
				default:
					app_st = APP_ST_NONE;
					break;
				}

				pos++;
				if(pos > BUF_SIZE){
					pos = 0;
				}
			}
		}
    }
#endif
}


