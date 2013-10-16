/*
 *  CRC.c
 *  UOBC_Tester
 *
 *  Created by 木村 真一 on  09/03/22.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

//*******************************************************
//	CRC
//*******************************************************
#include "common.h"

#define CRCPOLY1  (uint16)0x1021
//#define UCHAR_MAX	255
#define CHAR_BIT (uint16)8

unsigned short crc(unsigned char *c, int n)
{
	uint16	i,j,r;
	
//    r = 0xFFFFU;
    r = (uint16)0x0000;
    for (i = 0; i < n; i++) {

    	r ^= (uint16)c[i] << (16 - CHAR_BIT);
		for(j = 0; j < CHAR_BIT; j++)
			if(r & (uint16)0x8000){
				r = (r << 1) ^ CRCPOLY1;
			}
			else{
				r <<= 1;
			}
//			printf("%04x ",r);
	}
//	return ~r & 0xFFFFU;
	return r;
}

