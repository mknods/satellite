//
//  cmd.c
//  satellite
//
//  Created by Yujiro Miyabayashi on 13/09/02.
//  Copyright (c) 2013 Yujiro Miyabayashi. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "common.h"
#include "cmd.h"
#include "crc.h"

//#define BAUDRATE B115200
#define BAUDRATE B921600

//UART
#define BUF_SIZE    (1024)
#define PACKET_SIZE (255)

uint8 sci_buf[BUF_SIZE];
uint8 pos = 0;
uint8 pos_r = 0;

//COMMAND
uint8 cmd_buf[PACKET_SIZE];
CMD_DATA cmd_dt[10];
uint8 cmd_str[256];
uint8 cmd_len = 0;
uint8 dt_cnt = 0;


//TLM
uint8 tlm_buf[10][PACKET_SIZE+1];
uint8 tlm_pos = 0;
uint8 tlm_cnt = 0;
uint8 tlm_cmd_count = 0;

uint8 app_st = APP_ST_NONE;

uint16 crc_val = 0;

char tlm_dt[182+8+15+1] = {0};


FILE *fp;
int fsize;


void makeCmdData(uint8 *cmd, CMD_DATA *dt, int len);
void selectCommnd( CMD_DATA* dt );
void run_cmd(CMD_DATA* dt);
void cmd_err( uint8 cmd_sts, uint8 cmd_err_sts );
void send_file( CMD_DATA* dt);
int put_file( CMD_DATA* dt);

long read_dev(int fd, CMD_DATA* cmd){
    
    uint8 rx_buf[256] = {0};

    long len = read(fd, (void*)rx_buf, 255);	 //Filestream, buffer to store in, number of bytes to read (max)
    
    if (len < 0){
    }else if (len == 0){
    }else{
        
        rx_buf[len] = '\0';
        PRINTF("%ld bytes read : %s\n", len, rx_buf);
    }
    return len;
}

int fd_sci0 = -1;

void cmd_main(){

    //==============
    //	OPEN
    //==============
#ifdef BBB
    fd_sci0 = open("/dev/ttyO4", O_RDWR | O_NOCTTY| O_NDELAY); // | O_NDELAY|O_NONBLOCK);
#else
    fd_sci0 = open("/dev/ttyAMA0", O_RDWR | O_NOCTTY| O_NDELAY); // | O_NDELAY|O_NONBLOCK);
#endif
    if (fd_sci0 == -1)
    {
        perror("open error");
        return;
    }

    struct termios options;
	tcgetattr(fd_sci0, &options);
	options.c_cflag = CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR | ICRNL;
	options.c_oflag = 0;
	options.c_lflag = 0;
	cfmakeraw(&options);

	cfsetispeed(&options, BAUDRATE);
	cfsetospeed(&options, BAUDRATE);

	tcflush(fd_sci0, TCIFLUSH);
	tcsetattr(fd_sci0, TCSANOW, &options);
    
    PRINTF("UART opened\n");
    
    int cmd_len = 0;
    uint8 rx_buf[256] = {0};

//    // LOOPBACK TEST
//    while(1){
//        memset( rx_buf, 0, 256) ;
//        cmd_len=0;
//        long len = read(fd_sci0, rx_buf, 255);
//        if(len>0){
//			for(int i=0; i<len; i++){
//				PRINTF("0x%02x ", rx_buf[i]);
//			}
//			PRINTF("\n");
//        }
////        write(fd_sci0, rx_buf, len);
//    }


    while(1){

    	memset( rx_buf, 0, 256);

    	// === READ DEVICE ===
        long len = read(fd_sci0, (void*)rx_buf, 255);
        if (len > 0){
            
            for( int i=0; i < len; i++){
                
                sci_buf[pos] = rx_buf[i];

                switch (app_st){
                        
                    case APP_ST_NONE:
                    	dt_cnt = 0;
                        if( sci_buf[pos] == STX ){
                            PRINTF("STX [%u] = %02x\n", pos,sci_buf[pos]);
                            app_st = APP_ST_HEADER;
                        }else{
                            PRINTF("[%u] = %02x\n", pos,sci_buf[pos]);
                        }
                        break;
                        
                    case APP_ST_HEADER:
                    	if( sci_buf[pos] == STX ){
                            PRINTF("STX [%u] = %02x\n", pos,sci_buf[pos]);
                            app_st = APP_ST_LEN;
                        }else{
                            app_st = APP_ST_NONE;
                        }
                        break;
                        
                    case APP_ST_LEN:

                    	//コマンド受信ごとにカウントアップ
                    	tlm_cmd_count++;

                    	PRINTF("LEN [%u] = %02x\n", pos,sci_buf[pos]);
                        app_st = APP_ST_CMD;
                        cmd_len = sci_buf[pos];

                        //copy cmd
                        dt_cnt = 0;
                        break;
                        
                    case APP_ST_CMD:
                        PRINTF("CMD [%u] = %02x\n", pos,sci_buf[pos]);

                        cmd_buf[dt_cnt] = sci_buf[pos];
                        dt_cnt++;
                        if(dt_cnt >= cmd_len){
                            app_st = APP_ST_CRC1;
                        }
                        break;

                    case APP_ST_CRC1:

                        PRINTF("CRC1 [%u] = %02x\n", pos,sci_buf[pos]);
                        crc_val = 0;
                        crc_val = (uint16)sci_buf[pos] << 8;
                        app_st = APP_ST_CRC2;
                        break;

                    case APP_ST_CRC2:
                    	 PRINTF("CRC2 [%u] = %02x", pos,sci_buf[pos]);

                        crc_val |= (uint16)sci_buf[pos];

                        uint16 tmp = crc(cmd_buf, cmd_len);
                        PRINTF("  CALC CRC 0x%x  RECV 0x%x len=%d cmd_buf[0]=%02x, cmd_buf[%d]=%02x\n",
                        		tmp,
                        		crc_val,
                        		cmd_len,
                        		cmd_buf[0],
                        		cmd_len-1,
                        		cmd_buf[cmd_len-1]);

                        if( crc_val == tmp ){
                        	PRINTF("  [OK] CRC OK\n");
                            app_st = APP_ST_FOOTER1;
                        }else{
                            app_st = APP_ST_NONE;
                        	perror("  [ERR] BAD CRC\n");
                        	cmd_err( CMD_STATUS_CMD_CONTENT_ERR, CMD_ERR_STS_CRC_ERR);
                        }
                        break;

                    case APP_ST_FOOTER1:

                        if( sci_buf[pos] == ETX ){

                        	PRINTF("FOOTER1 [%u] = %02x\n", pos,sci_buf[pos]);
                            app_st = APP_ST_FOOTER2;
                        }else{
                            app_st = APP_ST_NONE;   //ERROR
                        }
                        break;
                        
                    case APP_ST_FOOTER2:

                        if( sci_buf[pos] == ETX ){
                        	PRINTF("FOOTER2 [%u] = %02x\n", pos,sci_buf[pos]);
                            
                            //VALID COMMAND
                            CMD_DATA dt = {0};
                            makeCmdData(cmd_buf, &dt, cmd_len);
                            checkCmd(&dt);
                        }
                        app_st = APP_ST_NONE;       // END
                        break;
                    default:
                        app_st = APP_ST_NONE;
                        break;
                }
                
                // rx_ring buf position
                pos++;
                if(pos > BUF_SIZE){
                    pos = 0;
                }
            }
        }
    }
    
    //==============
    //	CLOSE
    //==============
    if (-1 == close(fd_sci0)) {
        perror("close error");
    }
    
    PRINTF("uart0 opened\n");
    
}

int checkCmd(CMD_DATA* cmd){

	int ret = 0;
	do{
		//0x02以外は受信しない
		if( cmd->to_id != (uint8)0x02){
			PRINTF("NOT MY DATA TO_ID=0x%x\n", cmd->to_id);
			ret = -1;
			break;
		}

		//0x82以外は受信しない
		if( cmd->to_id_sub != (uint8)0x82){
			PRINTF("NOT MY DATA TO_SUB_ID=0x%x\n", cmd->to_id_sub);
			ret = -1;
			break;
		}

		//CHECK CMD_ID (DEVICE ID)
		uint8 id_val = cmd->cmd_id & DEVICE_MASK;

		if( id_val == DEVICE_ID ){
			PRINTF("MY DATA DEVICE_ID=0x%x\n", id_val);
			selectCommnd(cmd);
		}else{
			PRINTF("NOT MY DATA DEVICE_ID=0x%x\n", id_val);
			ret = -1;
		}
	} while(0);

	return ret;
}



void makeCmdData(uint8 *cmd, CMD_DATA* dt, int len){
     
    dt->length = len;
    dt->tlm_cmd_discrimination = cmd[0];
    dt->from_id = cmd[1];
    dt->to_id = cmd[2];
    dt->cmd_exe_typeid = cmd[3];
    
    uint32 time = ( ((uint32)cmd[4] << 24)|
                    ((uint32)cmd[5] << 16)|
                    ((uint32)cmd[6] << 8)|
                     cmd[7]);
    
    dt->time = time;
    dt->u8_time[0] = cmd[4];
    dt->u8_time[1] = cmd[5];
    dt->u8_time[2] = cmd[6];
    dt->u8_time[3] = cmd[7];

    dt->cmd_type_id = cmd[8];
    dt->cmd_count = cmd[9];
    dt->cmd_id = cmd[10];
    dt->to_id_sub = cmd[11];

    int param_size = dt->length - 12;

    for( int i=0; i<param_size; i++ ){
        dt->cmd_param[i] = cmd[12+i];
    }
}


void selectCommnd( CMD_DATA* dt ){

	switch( dt->cmd_id & ~(DEVICE_MASK) ){
		case CMD_RUN_CMD:
			PRINTF("=== CMD_RUN_CMD ===\n");
			run_cmd_popen(dt);
			break;

		case CMD_GET_FILE:
			PRINTF("=== CMD_GET_FILE ===\n");
			send_file(dt);
			break;

		case CMD_PUT_FILE:
			PRINTF("=== CMD_PUT_FILE ===\n");
			put_file(dt);
			break;

		default:
			//未定義のコマンド
			cmd_err(CMD_STATUS_CMD_CONTENT_ERR, CMD_ERR_STS_UNDEFINED);
			break;
	}
}



void send(uint8* tx_data, int len){

    long z = write(fd_sci0, tx_data, len);
    if( z < 0 ){
        PRINTF("write error\n");
    }
    usleep(10000);
}



// ==========================
//  UPLOAD FILE
//  filename (8byte:FIX) + len(0x00-0xff 1byte:FIX) + data(176byte) = 185byte
// ==========================
// CMD [FILENAME]        //<<
// /sat/put/(FILENAME)   //<< create or append
 //
// system(cat > dt1|dt2 > /sat/sh/script.sh & chmod 777 /sat/sh/script.sh);
//
int put_file( CMD_DATA* dt){

	int len = 0;

	uint8 tmp_file[9] = {0};
	uint8 save_file[32] = {0};
	strcpy(tmp_file, dt->cmd_param);	//nullどめ用(8byteの場合対策)

	sprintf(save_file, "/sat/put/%s", tmp_file);

	// ファイルの有無確認
	struct stat st;
	int fp;

	int ret = stat(save_file,&st);
	if( 0 == ret){
		// EXIST -> APPEND MODE
		fp = open( save_file, O_APPEND | O_NOCTTY | O_RDWR,
				S_IRWXU|S_IRWXG|S_IRWXO);
	}else{
		// NOT EXIST -> CREATE
		fp = open( save_file, O_CREAT | O_NOCTTY | O_RDWR,
				S_IRWXU|S_IRWXG|S_IRWXO);
	}

	if( fp < 0 ){
		PRINTF("[put_file] open error");
		cmd_err(CMD_STATUS_CMD_CONTENT_ERR, CMD_ERR_STS_PARAM_ERR);
		return -2;
	}
	PRINTF("[put_file] open file %s\n", save_file);

	len = dt->cmd_param[8];
	PRINTF("len=%d\n",len);

	for(int i=0; i<len; i++){
		PRINTF("%02x ", dt->cmd_param[9+i] );
	}
	PRINTF("\n");

    long z = write(fp, &dt->cmd_param[9], len);
    if( z < 0 ){
        PRINTF("[put_file] write error\n");
        close(fp);
        return -2;
    }
    close(fp);

    return 0;
}




void send_file( CMD_DATA* dt){

	int len = 0;
	int tx_size = 0;
	uint8 tx_buf[256] = {0};
	uint8 file_dt[TLM_PARAM_SIZE] = {0};

	int fsize;

	//file open
	PRINTF("[send_file] open file\n");
	int fp = open( dt->cmd_param, O_RDONLY | O_NOCTTY);
	if( fp < 0 ){
		PRINTF("[send_file] file is not founded.");
		// file is not founded.
		cmd_err(CMD_STATUS_CMD_CONTENT_ERR, CMD_ERR_STS_PARAM_ERR);
		return;
	}

	struct stat stat_buf;
	 if (stat(dt->cmd_param, &stat_buf) == 0){
		fsize = stat_buf.st_size;
	 }else{
			perror("error");
	 }

	PRINTF("[send_file] size=%d  \n", fsize);

	do{
		memset( tx_buf,  (uint8)0x00, (uint8)256);
		memset( file_dt, (uint8)0x00, TLM_PARAM_SIZE);

		//file
		len = read( fp, file_dt, TLM_PARAM_SIZE );
		if(len <= 0){
			PRINTF("[send_file] end of file\n");
			break;
		}
		tx_size = TLM_DEFAULT_SIZE + len;

		tx_buf[0] = STX;
		tx_buf[1] = STX;
		tx_buf[2] = tx_size; //length
		tx_buf[TLM_CMD_DISCRIMINATION] = (uint8)0xff;
		tx_buf[TLM_ID] = (uint8)0x09;
		tx_buf[TLM_COUNT] = tlm_cnt;
		tlm_cnt++;
		tx_buf[TLM_TIME1] = (uint8)(dt->u8_time[0]);
		tx_buf[TLM_TIME2] = (uint8)(dt->u8_time[1]);
		tx_buf[TLM_TIME3] = (uint8)(dt->u8_time[2]);
		tx_buf[TLM_TIME4] = (uint8)(dt->u8_time[3]);
		tx_buf[TLM_CMD_CODE] = dt->cmd_id;

		fsize -= len;
		if(tlm_cnt == 1){
			//if(len < TLM_PARAM_SIZE){
			if(fsize <= 0){
				tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED;
			}else{
				tx_buf[TLM_CMD_STS] = CMD_STATUS_RECEIVED;
			}
		}else{
			if(fsize <= 0){
				tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED;
			}else{
				tx_buf[TLM_CMD_STS] = CMD_STATUS_IN_PROGRESS;
			}
		}
		tx_buf[TLM_CMD_ERROR_STS] = CMD_ERR_STS_NORMAL;
		tx_buf[TLM_CMD_COUNT] = tlm_cmd_count;
		tx_buf[TLM_PACKET_ID] = MISSION_DATA;
		tx_buf[TLM_FROM_ID] = 0x02;		//mission data
		tx_buf[TLM_FROM_ID_SUB] = 0x82;
		tx_buf[TLM_TO_ID] = 0x02;		//mission data

		int idx = 3 + TLM_DEFAULT_SIZE; // + len;

		for (int i = 0; i < len; i++) {
			tx_buf[idx+i] = file_dt[i];
			PRINTF("0x%02x ", tx_buf[idx+i]);
		}
		PRINTF("\n");
		idx += len;

		uint16 crc_val = crc( &tx_buf[TLM_CMD_DISCRIMINATION], tx_size);

		tx_buf[idx] = (uint8)((crc_val & (uint16)0xff00) >> 8);	idx++;	//crc
		tx_buf[idx] = (uint8)(crc_val & (uint16)0x00ff);		idx++;
		tx_buf[idx] = ETX;	idx++;
		tx_buf[idx] = ETX;	idx++;

		send( tx_buf ,idx);

	} while( len > 0);

	close(fp);
}


/*
・LENGTHエラー パケット長を意図的に短くしてエラー検出を確認
	CMD_STS                 0xFE（コマンドエラー）
	CMD_ERROR_STS           0xfd
・CRCエラー　　　意図的にCRCに誤りを入れてエラー検出を確認
    CMD_STS                 0xFE（コマンドエラー）
    CMD_ERROR_STS           0xfe
・未定義コマンド　意図的に未定義のコマンドコードを送信してエラー検出を確認
    CMD_STS                 0xFE（コマンドエラー）
    CMD_ERROR_STS           0xff
・PARAMETERエラー　意図的にコマンドパラメータに誤りを混入してエラー検出を確認
    CMD_STS                 0xFE（コマンドエラー）
    CMD_ERROR_STS           0xfc
*/
void cmd_err( uint8 cmd_sts, uint8 cmd_err_sts ) {

	uint8 tx_buf[256] = {0};
	int len = TLM_DEFAULT_SIZE;

	tx_buf[0] = STX;
	tx_buf[1] = STX;

	tx_buf[2] = len; //length

	tx_buf[TLM_CMD_DISCRIMINATION] = 0xff;
	tx_buf[TLM_ID] = (uint8)0x09;
	tx_buf[TLM_COUNT] = (uint8)0x00;
	tx_buf[TLM_TIME1] = (uint8) 0x00;
	tx_buf[TLM_TIME2] = (uint8)0x00;
	tx_buf[TLM_TIME3] = (uint8)0x00;
	tx_buf[TLM_TIME4] = (uint8)0x00;
	tx_buf[TLM_CMD_CODE] = (uint8)0x00;
	tx_buf[TLM_CMD_STS] = cmd_sts;
	tx_buf[TLM_CMD_ERROR_STS] = cmd_err_sts;
	tx_buf[TLM_CMD_COUNT] = tlm_cmd_count;
	tx_buf[TLM_PACKET_ID] = HK_DATA;
	tx_buf[TLM_FROM_ID] = 0x02;
	tx_buf[TLM_FROM_ID_SUB] = 0x82;
	tx_buf[TLM_TO_ID] = 0x00; //HK_DATA

	int idx = 3 + len;

	uint16 crc_val = crc( &tx_buf[TLM_CMD_DISCRIMINATION], len);
	tx_buf[idx] = (uint8)((crc_val & (uint16)0xff00) >> 8);	idx++;	//crc
	tx_buf[idx] = (uint8)(crc_val & (uint16)0x00ff);		idx++;
	tx_buf[idx] = ETX;	idx++;
	tx_buf[idx] = ETX;	idx++;

	send( tx_buf ,idx);
}

void run_cmd(CMD_DATA* dt){

	uint8 tx_buf[256] = {0};

	PRINTF("%s (length=%d)\n", dt->cmd_param, strlen(dt->cmd_param));
	system(dt->cmd_param);

	int len = TLM_DEFAULT_SIZE;

	tx_buf[0] = STX;
	tx_buf[1] = STX;

	tx_buf[2] = len; //length

	tx_buf[TLM_CMD_DISCRIMINATION] = 0xff;
	tx_buf[TLM_ID] = 0x09;
	tx_buf[TLM_COUNT] = tlm_cnt;
	tlm_cnt++;
	tx_buf[TLM_TIME1] = dt->u8_time[0];
	tx_buf[TLM_TIME2] = dt->u8_time[1];
	tx_buf[TLM_TIME3] = dt->u8_time[2];
	tx_buf[TLM_TIME4] = dt->u8_time[3];
	tx_buf[TLM_CMD_CODE] = dt->cmd_id;
	tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED; //CMD_STATUS_NONE;
	tx_buf[TLM_CMD_ERROR_STS] = CMD_ERR_STS_NORMAL;
	tx_buf[TLM_CMD_COUNT] = tlm_cmd_count;
	tx_buf[TLM_PACKET_ID] = HK_DATA;
	tx_buf[TLM_FROM_ID] = 0x02;
	tx_buf[TLM_FROM_ID_SUB] = 0x82;
	tx_buf[TLM_TO_ID] = 0x00;	//HK_DATA

	int idx = 3 + len;

	uint16 crc_val = crc( &tx_buf[TLM_CMD_DISCRIMINATION], len);

	tx_buf[idx] = (uint8)((crc_val & (uint16)0xff00) >> 8);	idx++;	//crc
	tx_buf[idx] = (uint8)(crc_val & (uint16)0x00ff);		idx++;
	tx_buf[idx] = ETX;	idx++;
	tx_buf[idx] = ETX;	idx++;

	send( tx_buf ,idx);
}

void run_cmd_popen( CMD_DATA* dt){

	int len = 0;
	int tx_size = 0;
	uint8 tx_buf[256] = {0};
//	uint8 file_dt[TLM_PARAM_SIZE] = {0};

	int fsize;

	FILE *fp;
	char buf[255];
//	char *cmdline = "cd /sat/img/ && split -b 1000000 IMG_10.jpg dt. && ls -l";
//	char *cmdline = "cd /sat/img/ && split -b 1000000 IMG_10.jpg dt. && ls -l";

	PRINTF("%s (length=%d)\n", dt->cmd_param, strlen(dt->cmd_param));

//	fp = popen(cmdline,"r");
	fp = popen(dt->cmd_param,"r");
	if( fp == NULL){
//		PRINTF("popen err %s\n", cmdline);
		PRINTF("popen err %s\n", dt->cmd_param);
		return;
	}

	while(fgets(buf, 185, fp) != NULL) {

		int len = strlen( buf );
		PRINTF("%s", buf);

		tx_size = TLM_DEFAULT_SIZE + len;

		tx_buf[0] = STX;
		tx_buf[1] = STX;
		tx_buf[2] = tx_size; //length
		tx_buf[TLM_CMD_DISCRIMINATION] = (uint8)0xff;
		tx_buf[TLM_ID] = (uint8)0x09;
		tx_buf[TLM_COUNT] = tlm_cnt;
		tlm_cnt++;
		tx_buf[TLM_TIME1] = (uint8)(dt->u8_time[0]);
		tx_buf[TLM_TIME2] = (uint8)(dt->u8_time[1]);
		tx_buf[TLM_TIME3] = (uint8)(dt->u8_time[2]);
		tx_buf[TLM_TIME4] = (uint8)(dt->u8_time[3]);
		tx_buf[TLM_CMD_CODE] = dt->cmd_id;

		fsize -= len;
		if(tlm_cnt == 1){
			if(len < TLM_PARAM_SIZE){
				tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED;
			}else{
				tx_buf[TLM_CMD_STS] = CMD_STATUS_RECEIVED;
			}
		}else{
			if(len < TLM_PARAM_SIZE){
				tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED;
			}else{
				if(fsize <= 0){
					tx_buf[TLM_CMD_STS] = CMD_STATUS_COMPLETED;
				}else{
					tx_buf[TLM_CMD_STS] = CMD_STATUS_IN_PROGRESS;
				}
			}
		}
		tx_buf[TLM_CMD_ERROR_STS] = CMD_ERR_STS_NORMAL;
		tx_buf[TLM_CMD_COUNT] = tlm_cmd_count;
		tx_buf[TLM_PACKET_ID] = HK_DATA;	//s-band
		tx_buf[TLM_FROM_ID] = 0x02;
		tx_buf[TLM_FROM_ID_SUB] = 0x82;
		tx_buf[TLM_TO_ID] = 0x00;		//s-band

		int idx = 3 + TLM_DEFAULT_SIZE; // + len;
		for (int i = 0; i < len; i++) {
			tx_buf[idx+i] = buf[i];
//			PRINTF("0x%02x ", tx_buf[idx+i]);
		}
		idx += len;

		uint16 crc_val = crc( &tx_buf[TLM_CMD_DISCRIMINATION], tx_size);

		tx_buf[idx] = (uint8)((crc_val & (uint16)0xff00) >> 8);	idx++;	//crc
		tx_buf[idx] = (uint8)(crc_val & (uint16)0x00ff);		idx++;
		tx_buf[idx] = ETX;	idx++;
		tx_buf[idx] = ETX;	idx++;

		send( tx_buf ,idx);

		memset(buf, 0, 185);
	}

	pclose(fp);
}

