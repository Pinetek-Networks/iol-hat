#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <string.h>

#include "gpiod.h"

#include "osal_irq.h"
#include "osal_log.h"
#include "options.h"

#define MAX_BUF        64

#ifndef	CONSUMER
#define	CONSUMER	"Consumer"
#endif

#define SYSFS_GPIO_DIR "/sys/class/gpio"
#define MAX_BUF        64

struct timespec ts = { 5, 0 };
struct gpiod_line_event event;
struct gpiod_chip *chip;
struct gpiod_line *line;


typedef struct
{
	void* irq_arg;
	isr_func_t isr_func;
	
	int line_num;
	struct timespec ts;
  struct gpiod_line_event event;
	struct gpiod_chip *chip;
	struct gpiod_line *line;	 
	
} irq_thread_t;

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
        LOG_INFO(IOLINK_PL_LOG, "Running on Raspberry Pi 4\n");
				return 4;
    } else if (strstr(model, "Raspberry Pi 5") != NULL) {
        LOG_INFO(IOLINK_PL_LOG, "Running on Raspberry Pi 5\n");
				return 5;
    } else {
        LOG_INFO(IOLINK_PL_LOG,"Running on an unknown model: %s\n", model);
				
    }

    return 0;
}

static void * irq_thread (void * arg)
{
	irq_thread_t * irq = (irq_thread_t *)arg;
	
	LOG_DEBUG (IOLINK_PL_LOG, "irq args:p=%p\n", irq);
	LOG_DEBUG (IOLINK_PL_LOG, "irq args->iolink_14819_drv_t:p=%p\n", irq->irq_arg);
	
   int myPreviousValue = 1;  // Start assuming HIGH
   int myValue;
   
	while (1)
	{
      usleep(50);  // 50µs polling - faster than your original 100µs
      
      myValue = gpiod_line_get_value(irq->line);
      
      // Detect falling edge: HIGH → LOW transition
      if ((myValue == 0) && (myPreviousValue == 1))
		{
         // Call ISR only once per falling edge
         irq->isr_func(irq->irq_arg);
		} 
		
      myPreviousValue = myValue;
		}

			gpiod_line_release(irq->line);
			gpiod_chip_close(irq->chip);		

	 return 0;
	}


static pthread_t thread;

int _iolink_setup_int (int gpio_pin, isr_func_t isr_func, void* irq_arg)
{
	
	//LOG_DEBUG (IOLINK_PL_LOG, "_iolink_setup_int:>ptr=%p\n", irq_arg);
  pthread_attr_t attr;
   static irq_thread_t irqarg;
	
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
      return -1;
   }
	 
	line = gpiod_chip_get_line(chip, gpio_pin);
	if (!line) {
		perror("Get line failed\n");
		gpiod_chip_close(chip);
		return -1;
	}
	
	int ret = gpiod_line_request_falling_edge_events(line, CONSUMER);

	if (ret < 0) {
		perror("Request event notification failed\n");
		gpiod_line_release(line);
		gpiod_chip_close(chip);
		return -1;
	}	
	

   // Create isr service thread
		irqarg.isr_func = isr_func;
		irqarg.chip   = chip;
		irqarg.line_num = gpio_pin;
		irqarg.ts = ts;
		irqarg.event = event;
		irqarg.chip = chip;
		irqarg.line = line;
		irqarg.irq_arg = irq_arg;

   pthread_attr_init (&attr);
   pthread_create (&thread, &attr, irq_thread, &irqarg);

   return 0;
}
