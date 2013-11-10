//
//  cmd.h
//  satellite
//
//  Created by Yujiro Miyabayashi on 13/09/02.
//  Copyright (c) 2013年 Yujiro Miyabayashi. All rights reserved.
//

#ifndef satellite_cmd_h
#define satellite_cmd_h

#define DEVICE_MASK		(0xF0)
//#define DEVICE_ID		(0x20) 	// DEVICE 1	// 3bit
#define DEVICE_ID		(0x40) 	// DEVICE 2
//#define DEVICE_ID		(0x60) 	// DEVICE 3
// #define DEVICE_ID	(0x80) 	// DEVICE 4
// #define DEVICE_ID	(0xA0) 	// DEVICE 5
// #define DEVICE_ID	(0xC0) 	// DEVIVE 6
// #define DEVICE_ID	(0xD0) 	// DEVIVE 7

#define TLM_PARAM_SIZE		((uint8)182)
#define CMD_PARAM_SIZE		((uint8)185)


//CMD
#define CMD_RUN_CMD	    ((uint8)0x01)

//ファイル転送
#define CMD_GET_FILE	((uint8)0x02)
#define CMD_PUT_FILE	((uint8)0x03)

//SYSTEM
#define CMD_SYS_REBOOT	((uint8)0x08)

// DATA種別
#define HK_DATA 	 ((uint8)0x01)		// S band
#define MISSION_DATA ((uint8)0x02)		// X band

// CMD STATUS
#define CMD_STATUS_NONE             ((uint8)0x00)
#define CMD_STATUS_RECEIVED         ((uint8)0x01)
#define CMD_STATUS_IN_PROGRESS      ((uint8)0x02)
#define CMD_STATUS_COMPLETED        ((uint8)0x03)
#define CMD_STATUS_CMD_CONTENT_ERR  ((uint8)0xFE)
#define CMD_STATUS_CMD_EXE_ERR      ((uint8)0xFF)

// CMD ERROR STATUS
#define CMD_ERR_STS_NORMAL          ((uint8)0x00)
#define CMD_ERR_STS_UNDEFINED       ((uint8)0xFF)
#define CMD_ERR_STS_PARAM_ERR       ((uint8)0xFC)
#define CMD_ERR_STS_CRC_ERR         ((uint8)0xFE)
#define CMD_ERR_STS_LENGTH_ERR      ((uint8)0xFD)
#define CMD_ERR_STS_CONTEXT_ERR     ((uint8)0xFB)
#define CMD_ERR_STS_CONFLICT        ((uint8)0xFF)


//TLM
#define TLM_CMD_DISCRIMINATION 3
#define TLM_ID 4
#define TLM_COUNT 5
#define TLM_TIME1 6
#define TLM_TIME2 7
#define TLM_TIME3 8
#define TLM_TIME4 9
#define TLM_CMD_CODE 10
#define TLM_CMD_STS 11
#define TLM_CMD_ERROR_STS 12
#define TLM_CMD_COUNT 13
#define TLM_PACKET_ID 14
#define TLM_FROM_ID 15
#define TLM_FROM_ID_SUB 16
#define TLM_TO_ID 17
#define TLM_PARAM 18

#define TLM_DEFAULT_SIZE 15


#define STX ((uint8)0x80)
#define ETX ((uint8)0x81)

// APP STATE
#define APP_ST_NONE     (0)
#define APP_ST_HEADER   (1)
#define APP_ST_LEN      (2)
#define APP_ST_CMD      (3)
#define APP_ST_CRC1      (4)
#define APP_ST_CRC2      (5)
#define APP_ST_FOOTER1   (6)
#define APP_ST_FOOTER2   (7)



// ===================
//  COMMAND DATA
// ===================
typedef struct {
    
    uint8 length;
	uint8 tlm_cmd_discrimination; // fix
	uint8 from_id;   //of 0xff
	uint8 to_id;     // fix
	uint8 cmd_exe_typeid;	// 0x00 reealtime 0x01 timeline
	uint32 time;   // 0x00000000 - 0xffffffff
	uint8 u8_time[4];   // 0x00000000 - 0xffffffff
	uint8 cmd_type_id;   // command status 0x00 or 0x10
 	uint8 cmd_count;            // 0 - 255 Count up per command, includes all realtime and back-orbit command.
	uint8 cmd_id;           // 0x81 - 0x84 User Define コマンドが複数ある場合、コマンドごとにIDを振る
	uint8 to_id_sub;	// fix
	uint8 cmd_param[185];	// max 185byte
	uint8 crc1;
	uint8 crc2;
    
} CMD_DATA;


// ===================
//  TELEMETORY DATA
// ===================
typedef struct {
    
	uint8 length;				// Index4～16のデータ長を記載
	uint8 tlm_cmd_discrimination;	// 0xff:TLM
	uint8 tlm_id;
	uint8 tlm_count;	// 0x00 - 0xff
	uint32 time;
	uint8 cmd_code;
	uint8 cmd_sts;
	uint8 cmd_error_sts;
	uint8 cmd_count;
	uint8 tlm_packet_id;	//0x01 HK data 0x02 Mission Data
	uint8 from_id;
	uint8 from_id_sub;
	uint8 to_id;	//0x00 HK data 0x02 Mission Data
	uint8 tlm_param[182];   // 0-182bytes
	uint8 crc1;
	uint8 crc2;
    
} TLM_DATA;



// ===================
//  FUNCTION
// ===================
void cmd_main();
void cmd_init();


#endif
