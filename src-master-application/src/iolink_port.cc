/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include "osal.h"
#include "osal_log.h"
#include "iolink.h"
#include "iolink_handler.h"
#include "iolink_smi.h"

#include "server.h"
#include "status.h"


uint8_t counter;
//uint8_t debugOut[14] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};

//uint8_t debugIn[2];

std::mutex generic_run_mutex0;
std::mutex generic_run_mutex1;



#include <iostream>
#include <thread>

// #define MSG_DEBUG

static void generic_run0(iolink_app_port_ctx_t *app_port) 
{
	
	std::lock_guard<std::mutex> guard(generic_run_mutex0);
	os_mutex_lock (mtx_cmd0);
  bool pdin_valid = false;
	
	//LOG_DEBUG (LOG_STATE_ON, "c%d p%d\n", cmd.command, app_port->portnumber);

  if (((cmd0.port+1) != app_port->portnumber) && (cmd0.command != CMD_EMPTY)) {
		
		LOG_ERROR(LOG_STATE_ON, "Wrong portumber!\n");
		os_mutex_unlock (mtx_cmd0);
    return;
  }	
		
	#if 0
	uint8_t myDataOut[255] =  {0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	uint8_t myDataIn[255];
	bool myValid;
	
	
	do_smi_pdout(app_port, true, 14, myDataOut);
	do_smi_pdin(app_port, &myValid, myDataIn);
	do_smi_pdinout (app_port);
	
	printf("%d\n",myDataIn[0]);
	#else
	//printf ("#0\n");
	pdCount0++;
	#endif
			
	#if MSG_DEBUG
	char myBuf1[1024] = "";
	
	
	for (int i = 0; i < cmd0.lenOut; i++) 
	{
	 char tempBuffer[10];
	 sprintf(tempBuffer, "%02X ", cmd0.dataOut[i]);
	 strcat(myBuf1, tempBuffer);
	}
				
	printf("tx[%d]=%s\n", cmd0.lenOut, myBuf1);
	#endif
	
	status[cmd0.port].pdOutLength = cmd0.lenOut;
			
	if (cmd0.lenOut > 0)
	{
		do_smi_pdout(app_port, true, cmd0.lenOut, cmd0.dataOut);
		status[cmd0.port].pdOutValid = 1;
	}
	
	else 
	{
		status[cmd0.port].pdOutValid = 0;
	}
				
	status[cmd0.port].pdInLength = cmd0.lenIn;
	
	if (cmd0.lenIn > 0)
	{
		int8_t readLen = do_smi_pdin(app_port, &pdin_valid, cmd0.dataIn);
		
		if (readLen != cmd0.lenIn)
		{ 
			LOG_WARNING(LOG_STATE_ON, "%s: Failed to get PDIn from port %u, len requested=%d, len act=%d\n", __func__, app_port->portnumber, cmd0.lenIn, readLen);
			status[cmd0.port].pdInValid  = 0;
		} 
	}
				
	if (do_smi_pdinout (app_port) != IOLINK_ERROR_NONE)
	{
		 LOG_WARNING (LOG_STATE_ON, "%s: PDInOut failed on port %u\n", __func__, app_port->portnumber);
		 status[cmd0.port].pdInValid  = 0;
	}				
	
	else 
	{
		if (!pdin_valid) 
		{
			LOG_WARNING(LOG_STATE_ON, "%s: PDIn data is invalid for port %u\n", __func__, app_port->portnumber);
			status[cmd0.port].pdInValid  = 0;
		} 
		else 
		{
			status[cmd0.port].pdInValid  = 1;
			
			#if MSG_DEBUG
			char myBuf2[1024] = "";
			
			for (int i = 0; i < cmd0.lenIn; i++) 
			{
			 char tempBuffer[10];
				sprintf(tempBuffer, "%02X ", cmd0.dataIn[i]);
				strcat(myBuf2, tempBuffer);
			}
			 
			printf("r%d rx[%d]=%s\n",cmd0.port, cmd0.lenIn, myBuf2);
			#endif
		}
	}


	os_mutex_unlock (mtx_cmd0);

	
}


static void generic_run1(iolink_app_port_ctx_t *app_port) 
{
	
	std::lock_guard<std::mutex> guard(generic_run_mutex1);
	os_mutex_lock (mtx_cmd1);
  bool pdin_valid = false;
	
	//LOG_DEBUG (LOG_STATE_ON, "c%d p%d\n", cmd.command, app_port->portnumber);

  if (((cmd1.port+1) != app_port->portnumber) && (cmd1.command != CMD_EMPTY)){
		
		LOG_ERROR(LOG_STATE_ON, "Wrong portumber!\n");
		os_mutex_unlock (mtx_cmd1);
    return;
  }	
		
	#if 0
	uint8_t myDataOut[255] =  {0,0,0,0,0,0,0,0,0,0,0,0,0,1};
	uint8_t myDataIn[255];
	bool myValid;
	
	
	do_smi_pdout(app_port, true, 14, myDataOut);
	do_smi_pdin(app_port, &myValid, myDataIn);
	do_smi_pdinout (app_port);
	
	printf("%d\n",myDataIn[0]);
	#else
	//printf ("#1\n");
	pdCount1++;
	#endif
	
	#if MSG_DEBUG
	char myBuf1[1024] = "";
	
	
	for (int i = 0; i < cmd1.lenOut; i++) 
	{
	 char tempBuffer[10];
	 sprintf(tempBuffer, "%02X ", cmd1.dataOut[i]);
	 strcat(myBuf1, tempBuffer);
	}
				
	printf("tx[%d]=%s\n", cmd1.lenOut, myBuf1);
	#endif
		
	status[cmd1.port].pdOutLength = cmd1.lenOut;
		
	if (cmd1.lenOut > 0)
	{
		do_smi_pdout(app_port, true, cmd1.lenOut, cmd1.dataOut);
		status[cmd1.port].pdOutValid = 1;
	}
		
	else 
	{
		status[cmd1.port].pdOutValid = 0;
	}
					
	status[cmd1.port].pdInLength = cmd1.lenIn;
		
	if (cmd1.lenIn > 0)
	{
		int8_t readLen = do_smi_pdin(app_port, &pdin_valid, cmd1.dataIn);
		
		if (readLen != cmd1.lenIn)
		{ 
			LOG_WARNING(LOG_STATE_ON, "%s: Failed to get PDIn from port %u, len requested=%d, len act=%d\n", __func__, app_port->portnumber, cmd1.lenIn, readLen);
			status[cmd1.port].pdInValid  = 0;
		} 
	}
			
	if (do_smi_pdinout (app_port) != IOLINK_ERROR_NONE)
	{
		 LOG_WARNING (LOG_STATE_ON, "%s: PDInOut failed on port %u\n", __func__, app_port->portnumber);
		 status[cmd1.port].pdInValid  = 0;
	}				
	
	else 
	{
		if (!pdin_valid) 
		{
			LOG_WARNING(LOG_STATE_ON, "%s: PDIn data is invalid for port %u\n", __func__, app_port->portnumber);
			status[cmd1.port].pdInValid  = 0;
		} 
		else 
		{
			status[cmd1.port].pdInValid  = 1;
			
			#if MSG_DEBUG
			char myBuf2[1024] = "";
			
			for (int i = 0; i < cmd1.lenIn; i++) 
			{
			 char tempBuffer[10];
				sprintf(tempBuffer, "%02X ", cmd1.dataIn[i]);
				strcat(myBuf2, tempBuffer);
			}
			 
			printf("r%d rx[%d]=%s\n",cmd1.port, cmd1.lenIn, myBuf2);
			#endif
		}
	}

	os_mutex_unlock (mtx_cmd1);
	
}


void generic_setup0 (iolink_app_port_ctx_t * app_port)
{
	LOG_INFO (LOG_STATE_ON, "generic_setup0\n");
	 do_smi_pdout (app_port, true, 0, NULL);
	 
   app_port->type = GENERIC;

   app_port->run_function = generic_run0; 

   app_port->app_port_state = IOL_STATE_RUNNING;
}

void generic_setup1 (iolink_app_port_ctx_t * app_port)
{
	LOG_INFO (LOG_STATE_ON, "generic_setup1\n");
	 do_smi_pdout (app_port, true, 0, NULL);
	 

   app_port->type = GENERIC;

   app_port->run_function = generic_run1; 

   app_port->app_port_state = IOL_STATE_RUNNING;
}