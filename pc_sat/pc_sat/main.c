//
//  main.c
//  pc_sat
//
//  Created by Yujiro Miyabayashi on 11/29/13.
//  Copyright (c) 2013 mknod. All rights reserved.
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


static int cmd_state = 0;

int main(int argc, const char * argv[])
{
	int pos_start = 0;
	int pos = 0;
	pthread_t thread1;
	int cmd_pos = 0;
    
	pthread_create(&thread1,
                   NULL,
                   (void *)drv_rcv,
                   (void *)0);
    
	while(1){
        
		uint8 u8_input = getchar();
		if(u8_input == EOF){
			break;
		}
        
		input_buf[pos] = u8_input;
		pos++;
		if(pos >= INPUT_BUF_SIZE){
			pos = 0;
		}
        
		if(u8_input == '\n'){
            
			printf(">");
            
			memset( cmd_buf, (uint8)0x00, INPUT_BUF_SIZE);
            
			do{
				if(input_buf[pos_start] == '\n'){
					cmd_buf[cmd_pos] = 0x00;
				}else{
					cmd_buf[cmd_pos] = input_buf[pos_start];
				}
				cmd_pos++;
				pos_start++;
                
				if(pos_start >= INPUT_BUF_SIZE){
					pos_start = 0;
				}
			}while( pos_start != pos );
            
			cmd_pos = 0;
            
			if(cmd_state==0){
                
				if( strcmp( "run1", &cmd_buf[0]) == 0 ){
					cmd_state = 1;
				}else if( strcmp( "get1", &cmd_buf[0]) == 0 ){
					cmd_state = 2;
				}else if( strcmp( "put1", &cmd_buf[0]) == 0 ){
					cmd_state = 3;
				}else if( strcmp( "run2", &cmd_buf[0]) == 0 ){
					cmd_state = 4;
				}else if( strcmp( "get2", &cmd_buf[0]) == 0 ){
					cmd_state = 5;
				}else if( strcmp( "put2", &cmd_buf[0]) == 0 ){
					cmd_state = 6;
				}else{
					cmd_state = 0;
				}
			}else{
				if(cmd_state == 1){
					run_cmd(&cmd_buf[0], DEVICE_ID1);
				}else if(cmd_state == 2){
					get_file(&cmd_buf[0], DEVICE_ID1);
				}else if(cmd_state == 3){
					put_file(&cmd_buf[0], DEVICE_ID1);
				}else if(cmd_state == 4){
					run_cmd(&cmd_buf[0], DEVICE_ID2);
				}else if(cmd_state == 5){
					get_file(&cmd_buf[0], DEVICE_ID2);
				}else if(cmd_state == 6){
					put_file(&cmd_buf[0], DEVICE_ID2);
				}else{
				}
				cmd_state = 0;
			}
		}
	}
    
	pthread_join(thread1, NULL);
	return 0;
}





