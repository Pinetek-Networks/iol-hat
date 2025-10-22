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

#include <fstream>

#include <iostream>
#include <termios.h>
#include <unistd.h>

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
#define VERSION "1.4"


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

uint8_t CQCfgA_DI[] = 
{
	0x00,
	0x00
};

uint8_t CQCfgA_DO[] = 
{
	0x00,
	0x00
};


iolink_pl_mode_t mode_ch[] =
{
   iolink_mode_SDCI,
   iolink_mode_SDCI,
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

char getChar() {
    struct termios old_tio, new_tio;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    
    char ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return ch;
}

bool saveWarningFlag(const std::string& filename, bool flag) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(&flag), sizeof(flag));
    return true;
}

bool loadWarningFlag(const std::string& filename, bool defaultValue = false) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return defaultValue;
    
    bool flag;
    file.read(reinterpret_cast<char*>(&flag), sizeof(flag));
    return file.good() ? flag : defaultValue;
}



int main (int argc, char ** argv)
{	
	LOG_INFO (IOLINK_PL_LOG, "IOL HAT master application\n");
	
	argparse::ArgumentParser program("iol-hat", VERSION);
	
	int myMode[] = {-1,-1};
	program.add_argument("-m0", "--mode0").store_into(myMode[0])
	.help("Operating Mode Port 0 (X1), Possible values 0: IOL, 1: DI, 2: DO, 3: OFF");
	
	
	program.add_argument("-m1", "--mode1").store_into(myMode[1])
	.help("Operating Mode Port 1 (X2), Possible values 0: IOL, 1: DI, 2: DO, 3: OFF");
	
	#if 0
	int myDIMode[] = {-1,-1};
	program.add_argument("-di0", "--di-mode0").store_into(myDIMode[0])
	.help("DI Configuration Port 0 (X1)");
			
	program.add_argument("-di1", "--di-mode1").store_into(myDIMode[1])
	.help("DI Configuration Port 1 (X2)");
	
	
	int myDOMode[]= {-1,-1};
	program.add_argument("-do0", "--do-mode0").store_into(myDOMode[0])
	.help("DO Configuration Port 0 (X1)");
			
	
	program.add_argument("-do1", "--do-mode1").store_into(myDOMode[1])
	.help("DO Configuration Port 1 (X2)");				
	#endif
		
	int myExtClock = -1;
	program.add_argument("-e", "--extclock")
	.help("Use clock for MAX14819 from ext source")
	.default_value(false)
	.implicit_value(true);

		
	int myDelayCurrentLimit = -1;
	program.add_argument("-d", "--delaycurrentlimit").store_into(myDelayCurrentLimit)
	.help("Delay time in ms for current limit enabling after port enable, range [1..1500]. Please note that using this you need to limit the drawn current to 1,5A. ");
	
	
	int myBlankingTime  = -1;
	program.add_argument("-b", "--blankingtime").store_into(myBlankingTime)
	.help("Blanking time setting 0=short...3=long for capacitive loads");

	
	int myRealtime = -1;
	program.add_argument("-r", "--realtime").store_into(myRealtime)
		.help("Run realtime on given kernel (requires root rights, see manual)");
	
	int myIolPort = -1;
	program.add_argument("-i", "--iolport").store_into(myIolPort)
		.help("Specify the IOL port, possible values 12 and 34)");


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
	
	
	// Operating mode
	for (int i=0; i<2; i++)
	{

		
		LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> myMode=%d\n", i, myMode[i]);				
		
		switch (myMode[i])
		{
			//
			case 0: 
			case -1:
				LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> iolink_mode_SDCI\n", i);
				mode_ch[i] = iolink_mode_SDCI; break;
			break;
			
			case 1:
				mode_ch[i] = iolink_mode_DI;
				
				LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> iolink_mode_DI\n",i);
				#if 0
				LOG_DEBUG  (IOLINK_PL_LOG, "myDIMode=%d\n", myDIMode[i]);				
				switch (myDIMode[i])
				{					
					case 0: 
					case -1: // Sink 5mA --> DEFAULT
						CQCfgA_DI[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_5MA) | MAX14819_CQCFG_DRVDIS	;
						break; 
					case 1: CQCfgA_DI[i]=	MAX14819_CQCFG_IEC3TH | MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_2MA) | MAX14819_CQCFG_DRVDIS;	break; // Sink 2mA
					case 2: CQCfgA_DI[i]=	MAX14819_CQCFG_IEC3TH | MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_150UA) | MAX14819_CQCFG_DRVDIS;	break; // Sink 150uA
					case 3: CQCfgA_DI[i]=	MAX14819_CQCFG_IEC3TH | MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_2MA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SOURCESINK;	break; // Source 2mA
					case 4: CQCfgA_DI[i]=	MAX14819_CQCFG_IEC3TH | MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_150UA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SOURCESINK;	break; // Source 150uA
					case 5: CQCfgA_DI[i]=	MAX14819_CQCFG_IEC3TH | MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_150UA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SOURCESINK;	break; // Spurce 150uA

					default :
						std::cerr << "Invalid argument -di"<<i<<"; --di-mode"<<i<<". Mode must be in [0: Sink 5mA, 1: Sink 2mA, 2: Source 150µADI, 3: Source 5mA, 4: Source 2mA, 5: Sink 150µA,]. Given value: "<<myMode[0] << std::endl;
						exit (-1);
						break;
				}
				#else
				CQCfgA_DI[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_5MA) | MAX14819_CQCFG_DRVDIS	;
				#endif
				
				
				LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> CQCfgA_DI=%02x\n",i,CQCfgA_DI[i]);
			break;
			case 2: 
				mode_ch[i] = iolink_mode_DO;
				
				LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> iolink_mode_DO\n", i);
				#if 0
				LOG_DEBUG  (IOLINK_PL_LOG, "myDOMode=%d\n", myDOMode[i]);				
				switch (myDOMode[i])
				{
					case 0:
					case -1:
					// Push/Pull
						CQCfgA_DO[i]=	 MAX14819_CQCFG_PUSHPUL; // Default Push-Pull 500mA
						break; 
						
						case 1: CQCfgA_DO[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_5MA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SOURCESINK;	break; // Sink 5mA, PNP
						case 2: CQCfgA_DO[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_2MA) | MAX14819_CQCFG_DRVDIS;	break; // Sink 2mA, PNP
						case 3: CQCfgA_DO[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_150UA) | MAX14819_CQCFG_DRVDIS;	break; // Sink 150uA, PNP
						case 4: CQCfgA_DO[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_5MA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_NPN;	break; // Source 2mA, NPN
						case 5: CQCfgA_DO[i]=	 MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_2MA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_NPN;	break; // Source 150uA, NPN
						case 6: CQCfgA_DO[i]= MAX14819_CQCFG_SINKSEL(MAX14819_CQCFG_SINK_SRC_150UA) | MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_NPN;	break; // Spurce 150uA, NPN
	
						default :
							std::cerr << "Invalid argument -di"<<myMode[i]<<"; --di-mode"<<myMode[i]<<". Mode must be in [0: Push-pull 500mA, 1: 5mA PNP, 2: 2mA PNP, 3: 150µ PNP, 4: 5mA NPN, 5: 2mA NPN, 6: k 150µA NPN]. Given value: "<<myMode[0] << std::endl;
							exit (-1);
							break;
						
					}
					#else
					CQCfgA_DO[i]=	 MAX14819_CQCFG_PUSHPUL; // Default Push-Pull 500mA
					#endif 
					
					LOG_DEBUG  (IOLINK_PL_LOG, "Channel %d --> CQCfgA_DO=%02x\n",i,CQCfgA_DO[i]);
					
				break;
					
					
					
					
			case 3: mode_ch[i] = iolink_mode_INACTIVE; break;
			
			default:
				std::cerr << "Invalid argument -m"<<i<<"; --mode"<<i<<". Mode must be in [0: IOL, 1: DI, 2: DO, 3: OFF]. Given value: "<<myMode[0] << std::endl;
				std::exit(1);
			break;
		}
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
	
	delay_current_limit = 0;
	
	if (myDelayCurrentLimit != -1) 
	{
		if ((myDelayCurrentLimit <1) || (myDelayCurrentLimit > 1500))
		{
			std::cerr << "Invalid argument -d; --delaycurrentlimit. Current limit delay time allowed in range [1..1500]ms. Given value: "<<myDelayCurrentLimit << std::endl;
			std::exit(1);
		}
		
		delay_current_limit = myDelayCurrentLimit;
		LOG_INFO (IOLINK_PL_LOG, "Delay for L+ current limit enabled, delay time = %dms\n", myDelayCurrentLimit);
		LOG_INFO (IOLINK_PL_LOG, "Please note that the L+ line will not be protected in the delay time.\n");
		LOG_INFO (IOLINK_PL_LOG, "The user needs to ensure that the current drawn on L+ does not exceed 1,5A for the provided timespan\n");
		LOG_INFO (IOLINK_PL_LOG, "This feature is at user's own risk. Damages caused by overcurrent on the L+ line are not covered by the IOL HAT's warranty\n");
		
		
		bool myWarningHideFlag = loadWarningFlag("currentLimitAck", false);
		
		if (myWarningHideFlag == false)
		{
			LOG_INFO (IOLINK_PL_LOG, "Please acknowledge that you read the above statement (y/n): ");
			char ch = getChar();
			std::cout << ch << std::endl;
			
			if (ch == 'y' || ch == 'Y') 
			{
				LOG_INFO (IOLINK_PL_LOG, "Shall this warning be hidden on the next program start (y/n): ");
				char ch = getChar();
				std::cout << ch << std::endl;
					if (ch == 'y' || ch == 'Y') saveWarningFlag("currentLimitAck", true);
					else saveWarningFlag("currentLimitAck", false);
				
			}
			else exit (-1);
		}
		
		else
		{
			LOG_INFO (IOLINK_PL_LOG, "The user has set to not show the acknowledgment for this agreement on each startup\n");
		}
	}

	else 
	{
		// Do not delay
		myDelayCurrentLimit = 0;
	}	
	
		
	if (myBlankingTime != -1) 
	{
		if ((myBlankingTime <0) || (myBlankingTime > 3))
		{
			std::cerr << "Invalid argument -b; --blankingtime. Blanking time is allowed in the range [0..3] with 0 being short and 3 being long blanking time. Given value: "<<myBlankingTime << std::endl;
			std::exit(1);
		}
		
		std::string myBlankingtimeString = "n/a";
		
		switch (myBlankingTime)
		{
			case 0: myBlankingtimeString="5.5ms"; break;
			case 1: myBlankingtimeString="16.5ms"; break;
			case 2: myBlankingtimeString="55ms"; break;
			case 3: myBlankingtimeString="165ms"; break;
		}
		
		LOG_INFO (IOLINK_PL_LOG, "Blanking time for capacitive loads enabled, blanking time = %s\n", myBlankingtimeString.c_str());
	}
	
	else 
	{
		// Default settting
		myBlankingTime = 0;
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
	iolink_hw_drv_t * hw;

	uint8_t myBlankingTimeUInt8 = (uint8_t) myBlankingTime;
	myBlankingTimeUInt8 = myBlankingTimeUInt8 << 3;

	static iolink_14819_cfg_t iol_14819_0_cfg;

	iol_14819_0_cfg.chip_address   = 0,
	
	iol_14819_0_cfg.CQCfgA         = 0x02; //MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SINKSEL(0x2),
  iol_14819_0_cfg.LPCnfgA        = 0x02 | myBlankingTimeUInt8; //MAX14819_LPCNFG_LPEN,
  iol_14819_0_cfg.IOStCfgA       = 0x25; //MAX14819_IOSTCFG_DICSINK | MAX14819_IOSTCFG_DIEC3TH,
	
	iol_14819_0_cfg.CQCfgB         = 0x02; //MAX14819_CQCFG_DRVDIS | MAX14819_CQCFG_SINKSEL(0x2),
  iol_14819_0_cfg.LPCnfgB        = 0x00 | myBlankingTimeUInt8; //MAX14819_LPCNFG_LPEN,
  iol_14819_0_cfg.IOStCfgB       = 0x25; //MAX14819_IOSTCFG_DICSINK | MAX14819_IOSTCFG_DIEC3TH,	
	
	iol_14819_0_cfg.DrvCurrLim     = 0xC0;
  iol_14819_0_cfg.cs_channel     = myCsChannel;
	iol_14819_0_cfg.delay_current_limit = myDelayCurrentLimit;
	 
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
		iol_14819_0_cfg.Clock          = MAX14819_CLOCK_XTALEN | MAX14819_CLOCK_TXTXENDIS | MAX14819_CLOCK_CLKOEN; 
	}
	else
	{
		iol_14819_0_cfg.Clock          = 0x0E |  MAX14819_CLOCK_CLKOEN; // ExtClkEn
	}

	LOG_DEBUG (IOLINK_PL_LOG, "iol_14819_1_cfg.Clock  = %02x\n", iol_14819_0_cfg.Clock );	

	if (myIolPort == 12)
	{
		hw = main_14819_init("/iolink0", &iol_14819_0_cfg, 24);
	}

	else
	{
		hw = main_14819_init("/iolink0", &iol_14819_0_cfg, 25);
	}
	
   CC_ASSERT (hw != NULL);


   iolink_port_cfg_t port_cfgs[] = {

      {
         .name = "/iolink0/0",
         .mode = &mode_ch[0],
         .drv  = hw,
         .arg  = (void*)0,
      },
      {
         .name = "/iolink0/1",
         .mode = &mode_ch[1],
         .drv  = hw,
         .arg  = (void*)1,
      },
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
	 
	
	setup_tcp(hw, myTcpPort);
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
