#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include "osal_spi.h"
#include "osal_log.h"
#include "options.h"
#include "fcntl.h"
#include <stdio.h>
#include "unistd.h"
#include <string.h>

int _iolink_pl_hw_spi_init (const char * spi_slave_name)
{
   int fd = -1;
   fd     = open (spi_slave_name, O_RDWR);
   if (fd == -1)
   {
      return -1;
   }
   return fd;
}

void _iolink_pl_hw_spi_close (int fd)
{
   close (fd);
}

void _iolink_pl_hw_spi_transfer (
   int fd,
   void * data_read,
   const void * data_written,
   size_t n_bytes_to_transfer)
{
   int spi_fd = fd;
   
   int delay = 10;
   int speed = 10*1000*1000;
   int bits  = 8;

   struct spi_ioc_transfer tr;
		memset(&tr, 0, sizeof (tr));
		
		tr.tx_buf        = (unsigned long)data_written;
		tr.rx_buf        = (unsigned long)data_read;
		tr.len           = n_bytes_to_transfer;
		tr.delay_usecs   = delay;
		tr.speed_hz      = speed;
		tr.bits_per_word = bits;

	
   if (ioctl (spi_fd, SPI_IOC_MESSAGE (1), &tr) < 1)
   {
      LOG_ERROR (IOLINK_PL_LOG, "%s: failed to send SPI message\n", __func__);
   }
	 

	 /*
	 char txBuf[255] = "tx=";
	 char rxBuf[255] = "rx="; 
	 
	 for (int i=0; i<n_bytes_to_transfer; i++)
	 {
		 sprintf(txBuf, "%s %02X", txBuf, data_written[i]);
		 sprintf(rxBuf, "%s %02X", rxBuf, data_read[i]);		 
		 
	 }
	 
	 printf("%s\n", txBuf);
	 printf("%s\n", rxBuf);
	*/
}
