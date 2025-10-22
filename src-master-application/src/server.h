#pragma once

#include "osal.h"

typedef struct
{
	iolink_hw_drv_t *iolink_hw;
	uint8_t port;
	uint16_t tcpPort;
} socket_thread_t;

#include <condition_variable>

extern std::condition_variable cv0;
extern std::mutex cv_m0;

extern std::condition_variable cv1;
extern std::mutex cv_m1;

extern os_mutex_t *mtx_cmd0;
extern os_mutex_t *mtx_cmd1;

extern uint16_t  delay_current_limit;


#define CMD_EMPTY 			0
#define CMD_PWR 				1
#define CMD_LED 				2
#define CMD_PD  				3
#define CMD_READ 				4
#define CMD_WRITE 			5
#define CMD_STATUS 			6
#ifdef HISTORY
#define CMD_PD_HISTORY	7
#endif
#define CMD_STATUS2 		8


typedef struct {
	uint8_t port;
	uint8_t command;
	uint8_t lenIn;
	uint8_t lenOut;
	uint8_t dataIn[256];
	uint8_t dataOut[256];
	uint16_t index;
	uint8_t subindex;
	iolink_smi_errortypes_t error;
} iolCmd;


#define COM_FAILURE 0x00;
#define COM1 0x01;
#define COM2 0x02;
#define COM3 0x03;


typedef struct {
	
	uint8_t pdInValid;
	uint8_t pdOutValid;
	
	uint8_t transmissionRate;
	uint8_t masterCycleTime;

	uint8_t pdInLength;
	uint8_t pdOutLength;
	
	uint16_t vendorId;
	uint32_t deviceId;
	uint8_t power;
	
	uint8_t error;
} __attribute__((__packed__)) iolStatus;


extern iolCmd cmd0, cmd1;
extern iolStatus status[2];

void *runServer (void *_arg);