//
//  main.c
//  satellite
//
//  Created by Yujiro Miyabayashi on 13/08/31.
//  Copyright (c) 2013年 Yujiro Miyabayashi. All rights reserved.
//

#include <stdio.h>

#include "common.h"
#include "cmd.h"

//void test_popen(){
//
//	FILE *fp;
//	char buf[255];
////	char *cmdline = "/bin/ls -l /sat/";
//	char *cmdline = "cd /sat/img/ && split -b 1000000 IMG_10.jpg dt. && ls -l";
//
//	fp = popen(cmdline,"r");
//	if( fp == NULL){
//		printf("popen err %s\n", cmdline);
//		return;
//	}
//
//	while(fgets(buf, 255, fp) != NULL) {
//		(void) fputs(buf, stdout);
//	}
//
//	pclose(fp);
//}

int main(int argc, const char * argv[])
{
    printf("BOOT_UP REC-EARTH_PI\n");

//    test_popen();
    cmd_main();
    
    return 0;
}



