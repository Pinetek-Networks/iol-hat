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
#include "iolink.h"
#include "iolink_max14819.h"
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
#define VERSION "0.1"

 #define DEFAULT_TCP_12	12010
 #define DEFAULT_TCP_34	12011


// GPIO new

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

struct timespec ts = { 5, 0 };
struct gpiod_line_event event;
struct gpiod_chip *chip;
struct gpiod_line *line;

// GPIO old
#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF        64

#define IOLINK_MASTER_THREAD_STACK_SIZE (10 * 1024)
#define IOLINK_MASTER_THREAD_PRIO       6
#define IOLINK_DL_THREAD_STACK_SIZE     5000
#define IOLINK_DL_THREAD_PRIO           (IOLINK_MASTER_THREAD_PRIO + 1)





typedef void (*isr_func_t) (void *);

typedef struct
{
	int spi_fd;
	isr_func_t isr_func;
	
	int line_num;
	struct timespec ts;
  struct gpiod_line_event event;
	struct gpiod_chip *chip;
	struct gpiod_line *line;	 
	
} irq_thread_t;


void * irq_thread (void * arg)
{
	int ret;
	
	irq_thread_t * irq = (irq_thread_t *)arg;
	
	while (1)
	{
		ret = gpiod_line_event_wait(irq->line, &ts);
		if (ret < 0) 
		{
			perror("Wait event notification failed\n");
			gpiod_line_release(irq->line);
			gpiod_chip_close(irq->chip);			
			return NULL;			
		} 
		
		else if (ret == 0) {
			printf("Wait event notification on line #%u timeout\n", irq->line_num);
			continue;
		}

		ret = gpiod_line_event_read(line, &irq->event);
		//printf("Get event notification on line #%u\n", irq->line_num);
		if (ret < 0) {
			perror("Read last event notification failed\n");
			gpiod_line_release(irq->line);
			gpiod_chip_close(irq->chip);		
			return NULL;
		}
		
		irq->isr_func (&irq->spi_fd);
	}

	gpiod_line_release(irq->line);
	gpiod_chip_close(irq->chip);			
	
	
	/*
   irq_thread_t * irq = (irq_thread_t *)arg;
   struct pollfd p_fd;
   char buf[MAX_BUF];

   while (true)
   {
      memset (&p_fd, 0, sizeof (p_fd));

      p_fd.fd     = irq->irq_fd;
      p_fd.events = POLLPRI;

      int rc = poll (&p_fd, 1, 5000);

      if (rc < 0)
      {
         printf ("\npoll() failed!\n");
      }
      else if (rc == 0)
      {
         printf (".");
      }
      else if (p_fd.revents & POLLPRI)
      {
         lseek (p_fd.fd, 0, SEEK_SET);
         if (read (p_fd.fd, buf, MAX_BUF) > 0)
         {
            irq->isr_func (&irq->spi_fd);
         }
      }

      fflush (stdout);
   }
   close (irq->irq_fd);
	  * */
}

static pthread_t thread;


uint8_t getRpiType()
{
	 FILE *fp;
    char model[256];

    fp = fopen("/proc/device-tree/model", "r");
    if (fp == NULL) {
        perror("Error opening /proc/device-tree/model");
        return 0;
    }

    if (fgets(model, sizeof(model), fp) == NULL) {
        perror("Error reading model info");
        fclose(fp);
        return 0;
    }

    fclose(fp);

    if (strstr(model, "Raspberry Pi 4") != NULL) {
        printf("Running on Raspberry Pi 4\n");
				return 4;
    } else if (strstr(model, "Raspberry Pi 5") != NULL) {
        printf("Running on Raspberry Pi 5\n");
				return 5;
    } else {
        printf("Running on an unknown model: %s\n", model);
				
    }

    return 0;
}

pthread_t * setup_int (unsigned int gpio_pin, isr_func_t isr_func, int spi_fd)
{
   pthread_attr_t attr;
   static irq_thread_t irqarg;

/*
   // Add gpio_pin to exported pins
   int fd = open (SYSFS_GPIO_DIR "/export", O_WRONLY);

   if (fd < 0)
   {
      perror ("gpio/export");
      return NULL;
   }

   int n = write (fd, buf, snprintf (buf, sizeof (buf), "%d", gpio_pin));
   close (fd);


   if (n < 0)
   {
      perror ("setup_int(): Failed to write gpio_pin to gpio/export");
   }
*/

	//-- GPIO NEW
	
	// Raspberry Pi 5 uses gpiochip4

	if (getRpiType() == 5)
	{
		chip = gpiod_chip_open("/dev/gpiochip4");
	}
	
	else
	{
		chip = gpiod_chip_open("/dev/gpiochip0");
	}
		
	if (!chip)
   {
      perror ("gpiod_chip_open");
      return NULL;
   }
	 
	line = gpiod_chip_get_line(chip, gpio_pin);
	if (!line) {
		perror("Get line failed\n");
		gpiod_chip_close(chip);
		return NULL;
	}
	
	int ret;	
	ret = gpiod_line_request_falling_edge_events(line, CONSUMER);
	if (ret < 0) {
		perror("Request event notification failed\n");
		gpiod_line_release(line);
		gpiod_chip_close(chip);
		return NULL;
	}	


		//--OLD
		/*
   // Set direction of pin to input
   snprintf (buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/direction", gpio_pin);
   int fd = open (buf, O_WRONLY);

   if (fd < 0)
   {
      perror ("gpio/direction");
      return NULL;
   }

   int n = write (fd, "in", sizeof ("in") + 1);
   close (fd);

   if (n < 0)
   {
      perror ("write direction");
      return NULL;
   }

   // Set edge detection to falling
   snprintf (buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/edge", gpio_pin);
   fd = open (buf, O_WRONLY);

   if (fd < 0)
   {
      perror ("gpio/edge");
      return NULL;
   }

   n = write (fd, "falling", sizeof ("falling") + 1);
   close (fd);

   if (n < 0)
   {
      perror ("write edge");
      return NULL;
   }
	  
   // Open file and return file descriptor
   snprintf (buf, sizeof (buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio_pin);
   fd = open (buf, O_RDONLY | O_NONBLOCK);

   if (fd < 0)
   {
      perror ("gpio/value");
   }
	  * */
	 

   // Create isr service thread
		irqarg.isr_func = isr_func;
		irqarg.spi_fd   = spi_fd;
		irqarg.chip   = chip;
		irqarg.line_num = gpio_pin;
		irqarg.ts = ts;
		irqarg.event = event;
		irqarg.chip = chip;
		irqarg.line = line;

   pthread_attr_init (&attr);
   pthread_create (&thread, &attr, irq_thread, &irqarg);

   return &thread;
}


static pthread_t tcpThread;

pthread_t * setup_tcp (int _fd, uint16_t _tcpPort)
{
   pthread_attr_t attr;
	static socket_thread_t arg;
	
	arg.spi_fd = _fd;
	arg.tcpPort = _tcpPort;

   pthread_attr_init (&attr);
   pthread_create (&tcpThread, &attr, runServer, &arg);

   return &thread;
}

static pthread_t statusThread;

pthread_t * setup_status ()
{
   pthread_attr_t attr;	

   pthread_attr_init (&attr);
   pthread_create (&statusThread, &attr, runStatus, 0);

   return &thread;
}


void sigterm_handler(int signo) {
    printf("Received SIGTERM\n");
		sleep(2);
}


iolink_pl_mode_t mode_ch_0 = iolink_mode_SDCI; 
iolink_pl_mode_t mode_ch_1 = iolink_mode_SDCI; 




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

	LOG_INFO (IOLINK_PL_LOG, "IOL port = %d\n", myIolPort );
	LOG_INFO (IOLINK_PL_LOG, "TCP port = %d\n", myTcpPort );
	
	struct sigaction action;
	/*
	 * Setup a signal handler for the SIGALRM signal
	 * */
	
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_flags = 0;
	action.sa_handler = sigterm_handler;
	sigaction(SIGTERM, &action, NULL);
	
	uint8_t myCsChannel = 0;
	
	if (myIolPort == 12)
	{
		myCsChannel = 0;
	}
	
	else
	{
		myCsChannel = 1;
	}	

	iolink_14819_cfg_t iol_14819_cfg;
 
	memset (&iol_14819_cfg, 0x00, sizeof (iol_14819_cfg));


	iol_14819_cfg.chip_address   = 0x00;
	if (myIolPort == 12)
	{
		iol_14819_cfg.spi_slave_name = "/dev/spidev0.0"; // 0.0 or 0.1
	}

	else
	{
		iol_14819_cfg.spi_slave_name = "/dev/spidev0.1"; // 0.0 or 0.1
	}
	
	iol_14819_cfg.CQCfgA         = 0x22; // 0x20: 2mA Source/Sink, 0x02: Disable CQ DRV
	iol_14819_cfg.LPCnfgA        = 0x00; // !0x01 -> Initial state = Ox00 (off)
	iol_14819_cfg.IOStCfgA       = 0x25; // DiCSinkA 0x01, DiFilterEnA 0x04, CQEnA 0x20

	
	iol_14819_cfg.CQCfgB         = 0x22; // 0x20: 2mA Source/Sink, 0x02: Disable CQ DRV
	iol_14819_cfg.LPCnfgB        = 0x00; // !0x01 -> Initial state = Ox00 (off)
	iol_14819_cfg.IOStCfgB       = 0x25; // DiCSinkA 0x01, DiFilterEnA 0x04, CQEnA 0x20
	
	iol_14819_cfg.DrvCurrLim     = 0xC0; // CQ 500mA current limit

	if (myExtClock != 1)
	{
		iol_14819_cfg.Clock          = 0x11; // XtalEn 0x01, ClkOEn 0x04
	}
	else
	{
		iol_14819_cfg.Clock          = 0x02; // ExtClkEn
	}

	iol_14819_cfg.pin[0] = 0;
	iol_14819_cfg.pin[1] = 0;
	iol_14819_cfg.LEDCtrl				= 0x00;
	iol_14819_cfg.cs_channel     = myCsChannel;
	iol_14819_cfg.register_read_reg_fn = NULL;


	iolink_port_cfg_t port_cfgs[2];
	if (myIolPort == 12)
	{
		port_cfgs[0].name = "/dev/spidev0.0/0";
		port_cfgs[0].mode = &mode_ch_0;
		
		port_cfgs[1].name = "/dev/spidev0.0/1";
		port_cfgs[1].mode = &mode_ch_1;
	}
	
	else
	{
		port_cfgs[0].name = "/dev/spidev0.1/0";
		port_cfgs[0].mode = &mode_ch_0;
		
		port_cfgs[1].name = "/dev/spidev0.1/1";
		port_cfgs[1].mode = &mode_ch_1;

	}

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

	LOG_DEBUG (IOLINK_PL_LOG, "Init IOL ...\n");

	int fd = iolink_14819_init ("iolink", &iol_14819_cfg);
	 
	LOG_DEBUG (IOLINK_PL_LOG, "fd=%d\n", fd);
	 
	 
	if (fd < 0)
	{
		LOG_ERROR (IOLINK_PL_LOG, "Failed to open SPI\n");
	}
	
	LOG_DEBUG (IOLINK_PL_LOG, "SPI init done\n");
	 

	if (myIolPort == 12)
	{
		if (setup_int (24, iolink_14819_isr, fd) == NULL) 
		{
			printf ("Failed to setup interrupt\n");
			return -1;
		}	 
	}

	else
	{
		if (setup_int (25, iolink_14819_isr, fd) == NULL) 
		{
			printf ("Failed to setup interrupt\n");
			return -1;
		}
	}	 
	 
	usleep (200 * 1000);
	 
	setup_tcp(fd, myTcpPort);
	setup_status();
		 
	iolink_handler(iolink_cfg);
	
	return 0;		

}
	 
