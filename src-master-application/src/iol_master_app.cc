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
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdlib.h>

#include <iostream>


#include <linux/spi/spidev.h>
#include <sys/ioctl.h>


#include "osal.h"
#include "osal_log.h"
#include "osal_irq.h"
#include "iolink.h"
#include "iolink_max14819.h"
#include "iolink_max14819_pl.h"
#include "iolink_handler.h"

#include "realtime.h"

// GPIO with interrupt handler 
#include "gpiod.h"
#include <sys/time.h>

//TCP Server	
#include "server.h"


#include <signal.h>
#include "common.h"

#include "status.h"


// Args parser
#include "argparse.h"

// Defaults
#define VERSION "1.1"

#define DEFAULT_TCP_12	12010
#define DEFAULT_TCP_34	12011


// GPIO old
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF        64

#define IOLINK_MASTER_THREAD_STACK_SIZE (4 * 1024)
#define IOLINK_MASTER_THREAD_PRIO       6
#define IOLINK_DL_THREAD_STACK_SIZE     1500
#define IOLINK_DL_THREAD_PRIO           (IOLINK_MASTER_THREAD_PRIO + 1)
#define IOLINK_HANDLER_THREAD_STACK_SIZE (2048)
#define IOLINK_HANDLER_THREAD_PRIO       6


#ifndef IOLINK_APP_CHIP0_ADDRESS
#define IOLINK_APP_CHIP0_ADDRESS 0x0
#endif

#ifndef IOLINK_APP_CHIP1_ADDRESS
#define IOLINK_APP_CHIP1_ADDRESS 0x0
#endif


#define IOLINK_APP_CHIP0_SPI      "/dev/spidev0.0"


#ifndef IOLINK_APP_CHIP0_ADDRESS
#define IOLINK_APP_CHIP0_ADDRESS 0x0
#endif

#ifndef IOLINK_APP_CHIP1_ADDRESS
#define IOLINK_APP_CHIP1_ADDRESS 0x0
#endif


static iolink_pl_mode_t mode_ch[] =
{
#ifdef IOLINK_APP_CHIP0_SPI
   iolink_mode_SDCI,
   iolink_mode_SDCI,
#endif
#ifdef IOLINK_APP_CHIP1_SPI
   iolink_mode_SDCI,
   iolink_mode_INACTIVE,
#endif
};


static pthread_t tcpThread;

int setup_tcp (iolink_hw_drv_t *_iolink_hw, uint16_t _tcpPort)
{
   pthread_attr_t attr;
	static socket_thread_t arg;
	
	arg.iolink_hw = _iolink_hw;
	arg.tcpPort = _tcpPort;

   pthread_attr_init (&attr);
   pthread_create (&tcpThread, &attr, runServer, &arg);

   return 0;
}

static pthread_t statusThread;

int setup_status ()
{
   pthread_attr_t attr;	

   pthread_attr_init (&attr);
   pthread_create (&statusThread, &attr, runStatus, 0);

   return 0;
}


void sigterm_handler(int signo) {
    printf("Received SIGTERM\n");
		sleep(2);
}




void main_handler_thread (void * ctx)
{
   const iolink_m_cfg_t * cfg = (const iolink_m_cfg_t *)ctx;
   iolink_handler (*cfg);
}


static iolink_hw_drv_t * main_14819_init(const char* name, const iolink_14819_cfg_t * cfg, int irq)
{
   iolink_hw_drv_t  * drv;
   drv = iolink_14819_init (cfg);
   if (drv == NULL)
   {
      LOG_ERROR (IOLINK_PL_LOG, "APP: Failed to open SPI %s\n", name);
      return NULL;
   }

   if (_iolink_setup_int (irq, iolink_14819_isr, drv) < 0)
   {
      LOG_ERROR (IOLINK_PL_LOG, "APP: Failed to setup interrupt %s\n", name);
      return NULL;
   }
   return drv;
}

iolink_pl_mode_t mode_ch_0 = iolink_mode_SDCI; 
iolink_pl_mode_t mode_ch_1 = iolink_mode_INACTIVE; 

int main (int argc, char ** argv)
{
	
	LOG_INFO (IOLINK_PL_LOG, "IOL HAT master application\n");
	
	argparse::ArgumentParser program("iol-hat", VERSION);
	
	int myExtClock = -1;
	program.add_argument("-e", "--extclock")
	.help("Use clock for MAX14819 from ext source")
	.default_value(false)
	.implicit_value(true);
	
	
	int myRealtime = -1;
	program.add_argument("-r", "--realtime").store_into(myRealtime)
		.help("Run realtime on given kernel (requires root rights, see manual)");
	
	int myIolPort = -1;
	program.add_argument("-i", "--iolport").store_into(myIolPort)
		.help("Specify the IOL port (12/34)");


	int myTcpPort = -1;	
	program.add_argument("-t", "--tcpport").store_into(myTcpPort)
		.help("Specify the TCP port");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		std::cerr << program;
		std::exit(1);
	}
	
	if (program["-e"] == true) {
		myExtClock = 1;
	}
	
	if (program["--extclock"] == true) {
		myExtClock = 1;
	}
	
	if (myExtClock == 1)
	{
		LOG_INFO (IOLINK_PL_LOG, "Use external clock for MAX14819\n");
	}
	
		
	if (myRealtime != -1) 
	{
		if ((myRealtime > 3) || (myRealtime < 0))
		{
			std::cerr << "Invalid argument -r; --realtime. Kernel must be in range [0..3]. Given value: "<<myRealtime << std::endl;
			std::exit(1);
		}
		
		LOG_INFO (IOLINK_PL_LOG, "realtime enabled, run on kernel %d\n", myRealtime);
		Realtime::setup(myRealtime);
	}
		
	// IOL port was given
	if (myIolPort!= -1)
	{
		if ((myIolPort != 12) && (myIolPort !=34))
		{
			std::cerr << "Invalid argument -i; --iolport. Must be either 12 or 34. Given value: "<<myIolPort << std::endl;
			std::exit(1);
		}		
	}
	
	// Default IOL Port
	else
	{
		myIolPort = 12;
	}
	
	// Set default TCP

	// IOL port was given
	if (myTcpPort!= -1)
	{
	}	
	
	else
	{
		if (myIolPort == 12)
		{
			myTcpPort = DEFAULT_TCP_12;
		}

		else
		{
			myTcpPort = DEFAULT_TCP_34;
		}
	}
	
	// Set the chip select channel
	uint8_t myCsChannel = 0;
	
	if (myIolPort == 12)
	{
		myCsChannel = 0;
	}
	
	else
	{
		myCsChannel = 1;
	}	

	LOG_INFO (IOLINK_PL_LOG, "IOL port = %d\n", myIolPort );
	LOG_INFO (IOLINK_PL_LOG, "TCP port = %d\n", myTcpPort );
	

	 
	os_thread_t * iolink_handler_thread;
	iolink_hw_drv_t * hw[2];


#ifdef IOLINK_APP_CHIP0_SPI
   static iolink_14819_cfg_t iol_14819_0_cfg;

	iol_14819_0_cfg.chip_address   = IOLINK_APP_CHIP0_ADDRESS,
	
	iol_14819_0_cfg.CQCfgA         = 0x02; //MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SINKSEL(0x2),
  iol_14819_0_cfg.LPCnfgA        = 0x00; //MAX14819_LPCNFG_LPEN,
  iol_14819_0_cfg.IOStCfgA       = 0x25; //MAX14819_IOSTCFG_DICSINK | MAX14819_IOSTCFG_DIEC3TH,
	
	iol_14819_0_cfg.CQCfgB         = 0x02; //MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SINKSEL(0x2),
  iol_14819_0_cfg.LPCnfgB        = 0x00; //MAX14819_LPCNFG_LPEN,
  iol_14819_0_cfg.IOStCfgB       = 0x25; //MAX14819_IOSTCFG_DICSINK | MAX14819_IOSTCFG_DIEC3TH,	
	
	iol_14819_0_cfg.DrvCurrLim     = 0xC0;
  iol_14819_0_cfg.cs_channel     = myCsChannel;
	 
	if (myIolPort == 12)
	{
		iol_14819_0_cfg.spi_slave_name = "/dev/spidev0.0"; // 0.0 or 0.1
	}

	else
	{
		iol_14819_0_cfg.spi_slave_name = "/dev/spidev0.1"; // 0.0 or 0.1
	}
	
	if (myExtClock != 1)
	{
		iol_14819_0_cfg.Clock          = 0x11; // XtalEn 0x01, ClkOEn 0x04
	}
	else
	{
		iol_14819_0_cfg.Clock          = 0x02; // ExtClkEn
	}	
	 
#endif

#ifdef IOLINK_APP_CHIP1_SPI
   static const iolink_14819_cfg_t iol_14819_1_cfg = {
      .chip_address   = IOLINK_APP_CHIP1_ADDRESS,
      .spi_slave_name = IOLINK_APP_CHIP1_SPI,
      .CQCfgA         = MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SINKSEL(0x2),
      .LPCnfgA        = MAX14819_LPCNFG_LPEN,
      .IOStCfgA       = MAX14819_IOSTCFG_DICSINK | MAX14819_IOSTCFG_DIEC3TH,
      .DrvCurrLim     = 0x00,
      .Clock          = MAX14819_CLOCK_XTALEN | MAX14819_CLOCK_TXTXENDIS,
			.cs_channel     = myCsChannel,
   };
#endif

#ifdef IOLINK_APP_CHIP0_SPI

	if (myIolPort == 12)
	{
		hw[0] = main_14819_init("/iolink0", &iol_14819_0_cfg, 24);
	}

	else
	{
		hw[0] = main_14819_init("/iolink0", &iol_14819_0_cfg, 25);
	}	 
	   
   CC_ASSERT (hw[0] != NULL);
#endif

#ifdef IOLINK_APP_CHIP1_SPI
   hw[1] = main_14819_init("/iolink1", &iol_14819_1_cfg, IOLINK_APP_CHIP1_IRQ);
   CC_ASSERT (hw[1] != NULL);
#endif

   iolink_port_cfg_t port_cfgs[] = {
#ifdef IOLINK_APP_CHIP0_SPI
      {
         .name = "/iolink0/0",
         .mode = &mode_ch[0],
         .drv  = hw[0],
         .arg  = (void*)0,
      },
      {
         .name = "/iolink0/1",
         .mode = &mode_ch[1],
         .drv  = hw[0],
         .arg  = (void*)1,
      },
#endif
#ifdef IOLINK_APP_CHIP1_SPI
      {
         .name = "/iolink1/0",
         .mode = &mode_ch[2],
         .drv  = hw[1],
         .arg  = (void*)0,
      },
      {
         .name = "/iolink1/1",
         .mode = &mode_ch[3],
         .drv  = hw[1],
         .arg  = (void*)1,
      },
#endif
   };

   iolink_m_cfg_t iolink_cfg = {
      .cb_arg                   = NULL,
      .cb_smi                   = NULL,
      .cb_pd                    = NULL,
      .port_cnt                 = NELEMENTS (port_cfgs),
      .port_cfgs                = port_cfgs,
      .master_thread_prio       = IOLINK_MASTER_THREAD_PRIO,
      .master_thread_stack_size = IOLINK_MASTER_THREAD_STACK_SIZE,
      .dl_thread_prio           = IOLINK_DL_THREAD_PRIO,
      .dl_thread_stack_size     = IOLINK_DL_THREAD_STACK_SIZE,
   };

   os_usleep (200 * 1000);
	 
	#warning Check 2xMAX14819
	setup_tcp(hw[0], myTcpPort);
	setup_status();

   iolink_handler_thread = os_thread_create (
      "iolink_handler_thread",
      IOLINK_HANDLER_THREAD_PRIO,
      IOLINK_HANDLER_THREAD_STACK_SIZE,
      main_handler_thread,
      (void*)&iolink_cfg);
   CC_ASSERT (iolink_handler_thread != NULL);

   for (;;)
   {
      os_usleep (1000 * 1000);
   }

   return 0;
}
