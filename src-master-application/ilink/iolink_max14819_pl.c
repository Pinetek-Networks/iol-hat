/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2019 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <iolink_dl.h>
#include <errno.h>

#include "iolink_max14819_pl.h"
#include "iolink_pl_hw_drv.h"
#include "osal_log.h"
#include "osal_spi.h"
#include "osal.h"

#include <stdio.h>

#ifdef __rtk__
#include <gpio.h>
#include "spi/spi.h"

#define SPI_TRANSFER(fd, rbuf, wbuf, size)                                     \
   spi_select (fd);                                                            \
   spi_bidirectionally_transfer (fd, rbuf, wbuf, size);                        \
   spi_unselect (fd);
#elif defined(__linux__)
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include "iolink_max14819.h"



//#define SPI_TRANSFER(fd, rbuf, wbuf, size) spi_transfer (fd, rbuf, wbuf, size);
#endif

/**
 * @file
 * @brief Driver for the MAX14819 IO-Link chip
 *
 * The iolink_max14819 driver is used for communication with a MAX14819
 * IO-Link chip. The chip contains two IO-Link master transceivers with logic.
 *
 */

#define MAX14819_CH_MIN 0
#define MAX14819_CH_MAX (MAX14819_NUM_CHANNELS - 1)

#define MAX14819_COMMAND_WRITE   0x00
#define MAX14819_COMMAND_READ    0x80
#define MAX14819_RRDYA           0x01
#define MAX14819_RERRA           0x02
#define MAX14819_RRDYB           0x04
#define MAX14819_RERRB           0x08
#define MAX14819_IRQ             0x10
#define MAX14819_ADDR_OFFSET     5
#define MAX14819_REGISTER_OFFSET 0

#define REG_TxRxDataA   0x00
#define REG_TxRxDataB   0x01
#define REG_Interrupt   0x02
#define REG_InterruptEn 0x03
#define REG_RxFIFOLvlA  0x04
#define REG_RxFIFOLvlB  0x05
#define REG_CQCtrlA     0x06
#define REG_CQCtrlB     0x07
#define REG_CQErrA      0x08
#define REG_CQErrB      0x09
#define REG_MsgCtrlA    0x0A
#define REG_MsgCtrlB    0x0B
#define REG_ChanStatA   0x0C
#define REG_ChanStatB   0x0D
#define REG_LEDCtrl     0x0E
#define REG_Trigger     0x0F
#define REG_CQCfgA      0x10
#define REG_CQCfgB      0x11
#define REG_CyclTmrA    0x12
#define REG_CyclTmrB    0x13
#define REG_DeviceDlyA  0x14
#define REG_DeviceDlyB  0x15
#define REG_TrigAssgnA  0x16
#define REG_TrigAssgnB  0x17
#define REG_LPCnfgA     0x18
#define REG_LPCnfgB     0x19
#define REG_IOStCfgA    0x1A
#define REG_IOStCfgB    0x1B
#define REG_DrvrCurrLim 0x1C
#define REG_Clock       0x1D
#define REG_Status      0x1E
#define REG_RevID       0x1F

#define MAX14819_SPI_INBAND_RRDYA BIT (0)
#define MAX14819_SPI_INBAND_RERRA BIT (1)
#define MAX14819_SPI_INBAND_RRDYB BIT (2)
#define MAX14819_SPI_INBAND_RERRB BIT (3)
#define MAX14819_SPI_INBAND_IRQ   BIT (4)

#define MAX14819_CQCTRL_CQ_SEND     BIT (0)
#define MAX14819_CQCTRL_CYC_TMR_EN  BIT (1)
#define MAX14819_CQCTRL_RX_FIFO_RST BIT (2)
#define MAX14819_CQCTRL_TX_FIFO_RST BIT (3)
#define MAX14819_CQCTRL_WU_PULS     BIT (4)
#define MAX14819_CQCTRL_EST_COM     BIT (5)

#define MAX14819_INTERRUPT_RX_DATA_RDY_A BIT (0)
#define MAX14819_INTERRUPT_RX_DATA_RDY_B BIT (1)
#define MAX14819_INTERRUPT_RX_ERR_A      BIT (2)
#define MAX14819_INTERRUPT_RX_ERR_B      BIT (3)
#define MAX14819_INTERRUPT_TX_ERR_A      BIT (4)
#define MAX14819_INTERRUPT_TX_ERR_B      BIT (5)
#define MAX14819_INTERRUPT_WURQ          BIT (6)
#define MAX14819_INTERRUPT_STATUS        BIT (7)

#define MAX14819_INTERRUPTEN_RX_DATA_RDY_A MAX14819_INTERRUPT_RX_DATA_RDY_A
#define MAX14819_INTERRUPTEN_RX_DATA_RDY_B MAX14819_INTERRUPT_RX_DATA_RDY_B
#define MAX14819_INTERRUPTEN_RX_ERR_A      MAX14819_INTERRUPT_RX_ERR_A
#define MAX14819_INTERRUPTEN_RX_ERR_B      MAX14819_INTERRUPT_RX_ERR_B
#define MAX14819_INTERRUPTEN_TX_ERR_A      MAX14819_INTERRUPT_TX_ERR_A
#define MAX14819_INTERRUPTEN_TX_ERR_B      MAX14819_INTERRUPT_TX_ERR_B
#define MAX14819_INTERRUPTEN_WURQ          MAX14819_INTERRUPT_WURQ
#define MAX14819_INTERRUPTEN_STATUS        MAX14819_INTERRUPT_STATUS

#define MAX14819_REVID_MAX14819          0x0A
#define MAX14819_REVID_MAX14819A         0x0E

typedef enum
{
   IOLINK_14819_CHANNEL_INVALID = -1,
   IOLINK_14819_CHANNEL_A       = 0,
   IOLINK_14819_CHANNEL_B       = 1,
} iolink_14819_channel_t;

typedef union
{
   struct
   {
      uint8_t cq_conf_val;
   } DO;
   struct
   {
      uint8_t cq_conf_val;
   } DI;
   struct
   {
      uint8_t IntE;
      uint8_t cq_ctrl_val;
      uint8_t msg_ctrl_val;
      uint8_t chan_stat_val;
      uint8_t cycl_tmr_val;
      uint8_t dev_del_val;
      uint8_t trig_assg_val;
   } SDCI;
} iolink_14819_cq_cfg_t;

static void iolink_pl_max14819_pl_handler (iolink_hw_drv_t * iolink_hw, void * arg);

static void iolink_14819_write_register (
   iolink_14819_drv_t * iolink,
   uint8_t reg,
   uint8_t value)
{
   uint8_t wbuf[2];
   uint8_t rbuf[2];

   CC_ASSERT (iolink->fd_spi >= 0);

   wbuf[0] = MAX14819_COMMAND_WRITE |
             (iolink->chip_address << MAX14819_ADDR_OFFSET) |
             (reg << MAX14819_REGISTER_OFFSET);
   wbuf[1] = value;
   _iolink_pl_hw_spi_transfer ( iolink->fd_spi, rbuf, wbuf, sizeof (wbuf));
}

static uint8_t iolink_14819_read_register (iolink_14819_drv_t * iolink, uint8_t reg)
{
   uint8_t wbuf[2];
   uint8_t rbuf[2];

   CC_ASSERT (iolink->fd_spi >= 0);

   wbuf[0] = MAX14819_COMMAND_READ |
             (iolink->chip_address << MAX14819_ADDR_OFFSET) |
             (reg << MAX14819_REGISTER_OFFSET);
   wbuf[1] = 0;
   _iolink_pl_hw_spi_transfer (iolink->fd_spi, rbuf, wbuf, sizeof (wbuf));
   return rbuf[1];
}

static void iolink_14819_burst_write_tx (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   uint8_t * data,
   uint8_t size)
{
   uint8_t rxtxreg = REG_TxRxDataA + ch;
   uint8_t idx;
   uint8_t data_tx[IOLINK_RXTX_BUFFER_SIZE + 1];
   uint8_t data_rx[IOLINK_RXTX_BUFFER_SIZE + 1] = {0};

   data_tx[0] = MAX14819_COMMAND_WRITE |
                (iolink->chip_address << MAX14819_ADDR_OFFSET) |
                (rxtxreg << MAX14819_REGISTER_OFFSET);
   memcpy (&data_tx[1], data, size);
   _iolink_pl_hw_spi_transfer (iolink->fd_spi, data_rx, data_tx, size + 1);

   for (idx = 0; idx < size; idx++)
   {
      if (data_tx[idx] != data_rx[idx + 1])
      {
         // TODO: Fix better id of iolink. Only channel is not good enough
         LOG_ERROR (
            IOLINK_PL_LOG,
            "IOLINK: CH %d. Data transfer error byte %d\n",
            ch,
            idx);
      }
   }
}

static uint8_t iolink_14819_burst_read_rx (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   uint8_t * data,
   uint8_t rxbytes)
{
   uint8_t rxtxreg                             = REG_TxRxDataA + ch;
   uint8_t txdata[IOLINK_RXTX_BUFFER_SIZE + 2] = {0};
   uint8_t rxdata[IOLINK_RXTX_BUFFER_SIZE + 2] = {0};

   txdata[0] = MAX14819_COMMAND_READ |
               (iolink->chip_address << MAX14819_ADDR_OFFSET) |
               (rxtxreg << MAX14819_REGISTER_OFFSET);
   _iolink_pl_hw_spi_transfer (iolink->fd_spi, rxdata, txdata, rxbytes + 2);
   memcpy (data, &rxdata[1], rxbytes);

   return rxdata[0];
}

static void iolink_14819_set_DO (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   iolink_14819_cq_cfg_t * cfg)
{
   uint8_t regval;
   os_mutex_lock (iolink->exclusive);
   //printf("A>%d\n", ch);	 
   regval = iolink_14819_read_register (iolink, REG_InterruptEn);
   iolink_14819_write_register (iolink, REG_InterruptEn, regval & ~(0x05 << ch));
   regval = iolink_14819_read_register (iolink, REG_IOStCfgA + ch);
   iolink_14819_write_register (iolink, REG_IOStCfgA + ch, regval | MAX14819_IOSTCFG_TXEN);
   os_mutex_unlock (iolink->exclusive);
	 //printf("A<%d\n", ch);
   iolink_14819_write_register (iolink, REG_CQCtrlA + ch, 0x0C);
   iolink_14819_write_register (iolink, REG_MsgCtrlA + ch, 0x01);
   iolink_14819_write_register (iolink, REG_CQCfgA + ch, cfg->DO.cq_conf_val & 0x0C);
   iolink_14819_write_register (iolink, REG_TrigAssgnA + ch, 0x00);
   iolink->is_iolink[ch]    = false;
   iolink->wurq_request[ch] = false;
}

static void iolink_14819_set_DI (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   iolink_14819_cq_cfg_t * cfg)
{
   uint8_t regval;

   os_mutex_lock (iolink->exclusive);
	 //printf("B>%d\n", ch);	
   regval = iolink_14819_read_register (iolink, REG_InterruptEn);
   iolink_14819_write_register (iolink, REG_InterruptEn, regval & ~(0x05 << ch));
   os_mutex_unlock (iolink->exclusive);
	 //printf("B<%d\n", ch);	
   iolink_14819_write_register (iolink, REG_CQCtrlA + ch, 0x0C);
   iolink_14819_write_register (iolink, REG_MsgCtrlA + ch, 0x01);
   iolink_14819_write_register (
      iolink,
      REG_CQCfgA + ch,
      (cfg->DI.cq_conf_val & 0xF1) | 0x02);
   iolink_14819_write_register (iolink, REG_TrigAssgnA + ch, 0x00);
   iolink->is_iolink[ch]    = false;
   iolink->wurq_request[ch] = false;
	 //printf("B1<%d\n", ch);	
}

static void iolink_14819_set_SDCI (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   iolink_14819_cq_cfg_t * cfg)
{
   uint8_t regval;

   os_mutex_lock (iolink->exclusive);
	 //printf("C>%d\n", ch);	
   // Disable interrupts
   regval = iolink_14819_read_register (iolink, REG_InterruptEn);
   iolink_14819_write_register (iolink, REG_InterruptEn, regval & ~(0x05 << ch));
   // Set registers according to config
   iolink_14819_write_register (iolink, REG_CQCtrlA + ch, cfg->SDCI.cq_ctrl_val);
   iolink_14819_write_register (iolink, REG_MsgCtrlA + ch, cfg->SDCI.msg_ctrl_val);
   iolink_14819_write_register (iolink, REG_ChanStatA + ch, cfg->SDCI.chan_stat_val);
   iolink_14819_write_register (iolink, REG_CyclTmrA + ch, cfg->SDCI.cycl_tmr_val);
   iolink_14819_write_register (iolink, REG_DeviceDlyA + ch, cfg->SDCI.dev_del_val);
   iolink_14819_write_register (iolink, REG_TrigAssgnA + ch,  cfg->SDCI.trig_assg_val);
   iolink_14819_write_register (iolink, REG_CQCfgA + ch, 0x34);
   // Enable interrupts
   regval = iolink_14819_read_register (iolink, REG_InterruptEn);
   iolink_14819_write_register (
      iolink,
      REG_InterruptEn,
      regval | (cfg->SDCI.IntE & (0x05 << ch)));
   iolink->is_iolink[ch] = true;
   os_mutex_unlock (iolink->exclusive);
	 //printf("C<%d\n", ch);	
}

static void iolink_14819_send_master_message (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch)
{
   uint8_t reg_val;
   uint8_t reg = REG_CQCtrlA + ch;
	 
	 //printf("J>%d\n", ch);	

   reg_val = iolink_14819_read_register (iolink, reg);
   reg_val |= MAX14819_CQCTRL_CQ_SEND;
   iolink_14819_write_register (iolink, reg, reg_val);
	 //printf("J<%d\n", ch);	
	 

}

static void iolink_14819_delete_master_message (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch)
{
   uint8_t regCQCtrl, reg_val;

   regCQCtrl = REG_CQCtrlA + ch;
	 
	 //printf("K>%d\n", ch);	

   reg_val = iolink_14819_read_register (iolink, regCQCtrl);
   reg_val |= MAX14819_CQCTRL_TX_FIFO_RST;
   iolink_14819_write_register (iolink, regCQCtrl, reg_val);
	 
	 //printf("K<%d\n", ch);
}

static void iolink_14819_set_master_message (
   iolink_14819_drv_t * iolink,
   iolink_14819_channel_t ch,
   uint8_t * data,
   uint8_t txlen,
   uint8_t rxlen,
   bool keepmessage)
{
   uint8_t regMC, reg_val;
   uint8_t buffer[100];

   CC_ASSERT (txlen != 0);
   CC_ASSERT (txlen <= IOLINK14819_TX_MAX);
   CC_ASSERT (rxlen != 0);

   regMC = REG_MsgCtrlA + ch;

   buffer[0] = rxlen;
   buffer[1] = txlen;
   memcpy (&buffer[2], data, txlen);
	 
	 //printf("L>%d\n", ch);
   iolink_14819_burst_write_tx (iolink, ch, buffer, txlen + 2);

   reg_val = iolink_14819_read_register (iolink, regMC);
   if (keepmessage)
   {
      reg_val |= BIT (3);
   }
   else
   {
      reg_val &= ~BIT (3);
   }
   iolink_14819_write_register (iolink, regMC, reg_val);
	 //printf("L<%d\n", ch);
}

static iolink_baudrate_t iolink_pl_max14819_get_baudrate (
   iolink_hw_drv_t * iolink_hw,
   void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg_val;
   uint8_t reg = REG_CQCtrlA + ch;

   reg_val = iolink_14819_read_register (iolink, reg);

   switch (reg_val >> 6)
   {
   case 1:
      return IOLINK_BAUDRATE_COM1;
   case 2:
      return IOLINK_BAUDRATE_COM2;
   case 3:
      return IOLINK_BAUDRATE_COM3;
   default:
      return IOLINK_BAUDRATE_NONE;
   }
   return IOLINK_BAUDRATE_NONE;
}

static uint8_t iolink_pl_max14819_get_cycletime (
   iolink_hw_drv_t * iolink_hw,
   void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg_val;
   uint8_t reg = REG_CyclTmrA + ch;

   reg_val = iolink_14819_read_register (iolink, reg);

   return reg_val;
}

static void iolink_pl_max14819_set_cycletime (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   uint8_t cycbyte)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg = REG_CyclTmrA + ch;

   iolink_14819_write_register (iolink, reg, cycbyte);
}

static bool iolink_pl_max14819_set_mode (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   iolink_pl_mode_t mode)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;
   iolink_14819_cq_cfg_t iol_cfg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);


   os_mutex_lock (iolink->exclusive);
	 //printf("D>%d\n", ch);	
   switch (mode)
   {
   case iolink_mode_DO:
      iol_cfg.DO.cq_conf_val = 0x04;
      iolink_14819_set_DO (iolink, ch, &iol_cfg);
      break;
   case iolink_mode_INACTIVE:
   case iolink_mode_DI:
      iol_cfg.DI.cq_conf_val = 0xA2;
      iolink_14819_set_DI (iolink, ch, &iol_cfg);
      break;
   case iolink_mode_SDCI:
      iolink_14819_write_register (iolink, REG_CQCtrlA + ch, 0x0C);
      iolink_14819_write_register (iolink, REG_MsgCtrlA + ch, 0);
      iolink_14819_write_register (iolink, REG_ChanStatA + ch, 0);
      iolink_14819_write_register (iolink, REG_CQCfgA + ch, 0);
      iolink_14819_write_register (iolink, REG_CyclTmrA + ch, 0);
      iolink_14819_write_register (iolink, REG_DeviceDlyA + ch, 0);
      iolink_14819_write_register (iolink, REG_TrigAssgnA + ch, 0);
      iolink_14819_read_register (iolink, REG_CQErrA + ch);
      iolink_14819_read_register (iolink, REG_DeviceDlyA + ch);

      iolink->wurq_request[ch] = false; /* Since we clear CQCTRL_EST_COM */

      iol_cfg.SDCI.IntE         = 0xFF;
      iol_cfg.SDCI.msg_ctrl_val = 0x26;  /* InsChks = 1, RChksEn = 1,
                                            RMessageRdyEn = 1, TSizeEn = 1 */
      iol_cfg.SDCI.chan_stat_val = 0x40; /* FramerEn */
      iol_cfg.SDCI.cycl_tmr_val  = 0x00;
			#warning check RpsnsTmrEn
      iol_cfg.SDCI.dev_del_val   = 0x4F; //0x43; /* BDelay=2, DDelay=1, RpsnsTmrEn=1 */
      iol_cfg.SDCI.trig_assg_val = 0x00;
      iol_cfg.SDCI.cq_ctrl_val   = 0x00; /* TxFifoRst and RxFifoRst to 0*/
      iolink_14819_set_SDCI (iolink, ch, &iol_cfg);
      break;
   }
   os_mutex_unlock (iolink->exclusive);
	 //printf("D<%d\n", ch);	

   return true;
}

static void iolink_pl_max14819_configure_event (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   os_event_t * event,
   uint32_t flag)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   iolink->dl_event[ch] = event;
   iolink->pl_flag      = flag;
}

static void iolink_pl_max14819_enable_cycle_timer (
   iolink_hw_drv_t * iolink_hw,
   void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg_val;
   uint8_t reg = REG_CQCtrlA + ch;

   reg_val = iolink_14819_read_register (iolink, reg);
	 //printf("E>%d\n", ch);	
   reg_val |= MAX14819_CQCTRL_CYC_TMR_EN;
   iolink_14819_write_register (iolink, reg, reg_val);
	 //printf("E<%d\n", ch);	
}

static void iolink_pl_max14819_disable_cycle_timer (
   iolink_hw_drv_t * iolink_hw,
   void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg_val;
   uint8_t reg = REG_CQCtrlA + ch;

   reg_val = iolink_14819_read_register (iolink, reg);
   reg_val &= ~MAX14819_CQCTRL_CYC_TMR_EN;
   iolink_14819_write_register (iolink, reg, reg_val);
}

static void iolink_pl_max14819_get_error (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   uint8_t * cqerr,
   uint8_t * devdly)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   os_mutex_lock (iolink->exclusive);
	 //printf("F>%d\n", ch);	
   *cqerr  = iolink_14819_read_register (iolink, REG_CQErrA + ch);
   *devdly = iolink_14819_read_register (iolink, REG_DeviceDlyA + ch);
   os_mutex_unlock (iolink->exclusive);
	 //printf("F<%d\n", ch);	
}

static bool iolink_pl_max14819_get_data (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   uint8_t * rxdata,
   uint8_t len)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;
   uint8_t reg                 = REG_TxRxDataA + ch;
   uint8_t lvlreg              = REG_RxFIFOLvlA + ch;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   os_mutex_lock (iolink->exclusive);
	 //printf("G>%d\n", ch);	
   uint8_t RxBytesAct = iolink_14819_read_register (iolink, reg);
   uint8_t rxbytes    = iolink_14819_read_register (iolink, lvlreg);

	#warning check error handling

   if (RxBytesAct != rxbytes)
   {
      LOG_WARNING (
         IOLINK_PL_LOG,
         "%s [fs: %d, ch: %d]: RxFIFOLvl (%d) != TxRxData (%d), len = %d\n",
         __func__,
         iolink->fd_spi,
         ch,
         rxbytes,
         RxBytesAct,
         len);
      uint8_t cqctrl = iolink_14819_read_register (iolink, REG_CQCtrlA + ch);
      cqctrl |= MAX14819_CQCTRL_RX_FIFO_RST;
      iolink_14819_write_register (iolink, REG_CQCtrlA + ch, cqctrl);
      iolink->data_ready[ch] = false;
      os_mutex_unlock (iolink->exclusive);
      return false;
   }


   if (len < rxbytes)
   {
      LOG_ERROR (
         IOLINK_PL_LOG,
         "%s [fd: %d, ch: %d]: Read buffer too small. Needed: %d. Actual: %d",
         __func__,
         iolink->fd_spi,
         ch,
         rxbytes,
         len);
      rxbytes = len;
   }

   uint32_t inband = 0;
   if (rxbytes > 0)
   {
      inband = iolink_14819_burst_read_rx (iolink, ch, rxdata, rxbytes);
      if ((inband & MAX14819_SPI_INBAND_IRQ) != 0)
      {
         os_event_set (iolink->dl_event[ch], iolink->pl_flag);
      }
   }

   iolink->data_ready[ch] = false;
   os_mutex_unlock (iolink->exclusive);
	 //printf("G<%d\n", ch);	

   return (rxbytes > 0);
}

static void iolink_pl_max14819_send_msg (iolink_hw_drv_t * iolink_hw, void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   iolink_14819_send_master_message (iolink, ch);
}

static void iolink_pl_max14819_dl_msg (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   uint8_t rxbytes,
   uint8_t txbytes,
   uint8_t * data)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   iolink_14819_delete_master_message (iolink, ch);
   iolink_14819_set_master_message (iolink, ch, data, txbytes, rxbytes, true);
}

static void iolink_pl_max14819_transfer_req (
   iolink_hw_drv_t * iolink_hw,
   void * arg,
   uint8_t rxbytes,
   uint8_t txbytes,
   uint8_t * data)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   iolink_14819_delete_master_message (iolink, ch);
   iolink_14819_set_master_message (iolink, ch, data, txbytes, rxbytes, true);
   iolink_14819_send_master_message (iolink, ch);
}

static bool iolink_pl_max14819_init_sdci (iolink_hw_drv_t * iolink_hw, void * arg)
{
   iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t ch   = (iolink_14819_channel_t)arg;

   CC_ASSERT (ch >= MAX14819_CH_MIN);
   CC_ASSERT (ch <= MAX14819_CH_MAX);

   uint8_t reg_val;
   uint8_t reg_cqctrl = REG_CQCtrlA + ch;

   os_mutex_lock (iolink->exclusive);
	 //printf("H>%d\n", ch);	
   if (iolink->wurq_request[ch])
   {
      os_mutex_unlock (iolink->exclusive);
			//printf("Ha<%d\n", ch);	
      return false;
   }
   reg_val = iolink_14819_read_register (iolink, reg_cqctrl);
   reg_val &= ~MAX14819_CQCTRL_EST_COM;
   reg_val &= ~MAX14819_CQCTRL_CYC_TMR_EN;
   reg_val &= ~MAX14819_CQCTRL_CQ_SEND;
   reg_val |= MAX14819_CQCTRL_RX_FIFO_RST | MAX14819_CQCTRL_TX_FIFO_RST;
   iolink_14819_write_register (iolink, reg_cqctrl, reg_val);

   reg_val = iolink_14819_read_register (iolink, REG_InterruptEn);
   reg_val |= MAX14819_INTERRUPTEN_WURQ;
   iolink_14819_write_register (iolink, REG_InterruptEn, reg_val);

   reg_val = MAX14819_CQCTRL_EST_COM;
   iolink_14819_write_register (iolink, reg_cqctrl, reg_val);

   iolink->wurq_request[ch] = true;
   os_mutex_unlock (iolink->exclusive);
	 //printf("H<%d\n", ch);	

   return true;
}

static void iolink_pl_max14819_pl_handler (iolink_hw_drv_t * iolink_hw, void * arg)
{
   iolink_14819_drv_t * iolink    = (iolink_14819_drv_t *)iolink_hw;
   iolink_14819_channel_t channel = (iolink_14819_channel_t)arg;

   uint8_t reg;
   uint8_t ch;

   CC_ASSERT (channel >= MAX14819_CH_MIN);
   CC_ASSERT (channel <= MAX14819_CH_MAX);

   if (!iolink->is_iolink[channel])
   {
      return;
   }

   os_mutex_lock (iolink->exclusive);
	 //printf("I>%d\n", ch);	
   // Check if this chip has interrupted
   // Read interrupt register
   reg = iolink_14819_read_register (iolink, REG_Interrupt);

   // Trigger correct event(s)
   if (reg & MAX14819_INTERRUPT_STATUS)
   {
      // TODO: Status error. Get error(s)
      LOG_ERROR (IOLINK_PL_LOG, "PL: Got status error\n");
   }

   if (reg & MAX14819_INTERRUPT_WURQ)
   {
      bool completed_wurq = false;

      for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
      {
         if (iolink->wurq_request[ch])
         {
            uint8_t reg_val;
            uint8_t reg_cqctrl = REG_CQCtrlA + ch;

            reg_val = iolink_14819_read_register (iolink, reg_cqctrl);

            if (!(reg_val & MAX14819_CQCTRL_EST_COM))
            {
               iolink->wurq_request[ch] = false;
               os_event_set (iolink->dl_event[ch], IOLINK_PL_EVENT_WURQ);
               completed_wurq = true;
            }
         }
      }

      if (!completed_wurq)
      {
         // TODO: WURQ-request interrupt, but no WURQ sent?
         LOG_ERROR (IOLINK_PL_LOG, "PL: Got WURQ, but no channel was handled\n");
      }
   }

   // Check channel specific flags
   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
      if (reg & (MAX14819_INTERRUPT_TX_ERR_A << ch))
      {
         os_event_set (iolink->dl_event[ch], IOLINK_PL_EVENT_TXERR);
      }
      if (reg & (MAX14819_INTERRUPT_RX_ERR_A << ch))
      {
         os_event_set (iolink->dl_event[ch], IOLINK_PL_EVENT_RXERR);
      }
      if (reg & (MAX14819_INTERRUPT_RX_DATA_RDY_A << ch))
      {
         if (!iolink->data_ready[ch])
         {
            iolink->data_ready[ch] = true;
          os_event_set (iolink->dl_event[ch], IOLINK_PL_EVENT_RXRDY);
         }
      }
   }
   os_mutex_unlock (iolink->exclusive);
	 //printf("I<%d\n", ch);	
}


   static const iolink_hw_ops_t iolink_hw_ops = {
      .get_baudrate        = iolink_pl_max14819_get_baudrate,
      .get_cycletime       = iolink_pl_max14819_get_cycletime,
      .set_cycletime       = iolink_pl_max14819_set_cycletime,
      .set_mode            = iolink_pl_max14819_set_mode,
      .enable_cycle_timer  = iolink_pl_max14819_enable_cycle_timer,
      .disable_cycle_timer = iolink_pl_max14819_disable_cycle_timer,
      .get_error           = iolink_pl_max14819_get_error,
      .get_data            = iolink_pl_max14819_get_data,
      .send_msg            = iolink_pl_max14819_send_msg,
      .dl_msg              = iolink_pl_max14819_dl_msg,
      .transfer_req        = iolink_pl_max14819_transfer_req,
      .init_sdci           = iolink_pl_max14819_init_sdci,
      .configure_event     = iolink_pl_max14819_configure_event,
      .pl_handler          = iolink_pl_max14819_pl_handler,
			.set_power           = iolink_pl_max14819_set_power,
			.set_led             = iolink_pl_max14819_set_led,
			.get_status          = iolink_pl_max14819_get_status,
   };
	 
	 
iolink_hw_drv_t * iolink_14819_init (const iolink_14819_cfg_t * cfg)
{
	LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_init >>>\n");


   iolink_14819_drv_t * iolink;
   uint8_t ch;
	 uint8_t rev;
   /* Allocate driver structure */
	 
   iolink = calloc (1, sizeof (iolink_14819_drv_t));
   if (iolink == NULL)
   {
		 LOG_ERROR (IOLINK_PL_LOG, "Cannot allocate driver structure\n");
      return NULL;
   }

   /* Initialise driver structure */
   iolink->drv.ops      = &iolink_hw_ops;
   iolink->chip_address = cfg->chip_address;
   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
      iolink->wurq_request[ch] = false;
   }	 
   iolink->fd_spi = _iolink_pl_hw_spi_init(cfg->spi_slave_name);
	 
   if (iolink->fd_spi <= 0)
   {
      LOG_ERROR (IOLINK_PL_LOG, "PL: Unable to open spi device: %s\n", cfg->spi_slave_name);
      free (iolink);
      return NULL;
   }
	 
   iolink->exclusive = os_mutex_create();
   iolink->drv.mtx = iolink->exclusive;
	 
   /* Verify chip is supported */
   rev = iolink_14819_read_register (iolink, REG_RevID);
   if (!(rev == MAX14819_REVID_MAX14819 ||
         rev == MAX14819_REVID_MAX14819A))
   {
      LOG_ERROR (IOLINK_PL_LOG, "PL: Unsupported chip revision: 0x%02x\n", rev);
      os_mutex_destroy(iolink->exclusive);
      _iolink_pl_hw_spi_close(iolink->fd_spi);
      free (iolink);
      return NULL;
   }

		
		LOG_DEBUG (IOLINK_PL_LOG, "SPI mode set\n");

   iolink->exclusive = os_mutex_create();

		//LOG_DEBUG(IOLINK_PL_LOG, "Init MAX registers\n");
   // Reset all registers
   // Disable interrupts
   iolink_14819_write_register (iolink, REG_InterruptEn, 0x00);
	 
   // Stop cycle timer (if activated)
   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
		 
      iolink_14819_write_register (iolink, REG_CQCtrlA + ch, 0x00);
   }

   iolink_14819_write_register (iolink, REG_Trigger, 0x00);

   // Reset interrupt register
   iolink_14819_read_register (iolink, REG_Interrupt);

   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
      iolink_14819_write_register (iolink, REG_ChanStatA + ch, 0x80);
   }

   // Set registers as requested in cfg
   iolink_14819_write_register (iolink, REG_LEDCtrl, cfg->LEDCtrl);
   iolink_14819_write_register (iolink, REG_DrvrCurrLim, cfg->DrvCurrLim);
   iolink_14819_write_register (iolink, REG_Clock, cfg->Clock);

   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
      iolink_14819_write_register (iolink, REG_ChanStatA + ch, 0x00);
   }
	 
	 
   iolink_14819_write_register (iolink, REG_CQCfgA, cfg->CQCfgA);
	 LOG_DEBUG (IOLINK_PL_LOG, "*1\n");
   iolink_14819_write_register (iolink, REG_LPCnfgA, cfg->LPCnfgA);
	 LOG_DEBUG (IOLINK_PL_LOG, "*2\n");
   iolink_14819_write_register (iolink, REG_IOStCfgA, cfg->IOStCfgA);
   iolink_14819_write_register (iolink, REG_CQCtrlA, cfg->CQCtrlA);
   iolink_14819_write_register (iolink, REG_CQCfgB, cfg->CQCfgB);
   iolink_14819_write_register (iolink, REG_LPCnfgB, cfg->LPCnfgB);
   iolink_14819_write_register (iolink, REG_IOStCfgB, cfg->IOStCfgB);
   iolink_14819_write_register (iolink, REG_CQCtrlB, cfg->CQCtrlB);

   // Reset interrupt register
   iolink_14819_read_register (iolink, REG_Interrupt);


   // Reset L+ false errors
   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
      iolink_14819_read_register (iolink, REG_ChanStatA + ch);

   }

   if (cfg->register_read_reg_fn != NULL)
   {
      cfg->register_read_reg_fn (iolink_14819_read_register);
   }
	 
   return &iolink->drv;
}


void iolink_14819_isr (void * arg)
{
//	LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>1\n");
//	LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>ptr=%d\n", arg);
   iolink_14819_drv_t * iolink;
//	 LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>2\n");
   iolink = (iolink_14819_drv_t *)arg;
//	 LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>3\n");
   uint8_t ch;
//LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>5\n");
//LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>ptr=%d\n", iolink);
   // Set main event to trigger iolink_main
   for (ch = 0; ch < MAX14819_NUM_CHANNELS; ch++)
   {
		 

		 //LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>5-%d\n", ch);
      if (iolink->dl_event[ch] != NULL)
      {
				//LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>5a-%d\n", ch);
         os_event_set (iolink->dl_event[ch], iolink->pl_flag);
				 //LOG_DEBUG (IOLINK_PL_LOG, "iolink_14819_isr:>>5b-%d\n", ch);
      }
   }
   if ((iolink->dl_event[0] == NULL) && (iolink->dl_event[1] == NULL))
   {
      // DEBUG
      iolink->pl_flag = 0xBADBAD;
   }
}

void iolink_pl_max14819_set_power(iolink_hw_drv_t * iolink_hw, uint8_t _port, bool _power)
{
	if (_port > 1)
		return;
	
	iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;
	
	os_mutex_lock (iolink->exclusive);	 
			 
	uint8_t myLPCnfgAValue = 0x00;
		
	if (_power == true)
		myLPCnfgAValue = 0x01;
		
	iolink_14819_write_register (iolink, REG_LPCnfgA+_port, myLPCnfgAValue);	
	
	os_mutex_unlock (iolink->exclusive);	 
}



void iolink_pl_max14819_set_led(iolink_hw_drv_t * iolink_hw, uint8_t _port, bool _ledR, bool _ledG)
{
	if (_port > 1)
		return;
		
	iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;

	os_mutex_lock (iolink->exclusive);	 
	
	uint8_t myLEDCtrlValue = iolink_14819_read_register (iolink, REG_LEDCtrl);
	
	uint8_t myCurrentLedValue = 0;
	
	if (_ledR)
		myCurrentLedValue |= 0x02;
		
	if (_ledG)
		myCurrentLedValue |= 0x08;
	
		
	if (_port == 0)
	{
		myLEDCtrlValue &= 0xF5;
		myLEDCtrlValue |= myCurrentLedValue;
	}
	
	else
	{
		myLEDCtrlValue &= 0x5F;
		myLEDCtrlValue |= (myCurrentLedValue << 4);
	}
		
	iolink_14819_write_register (iolink, REG_LEDCtrl, myLEDCtrlValue);	
	
	os_mutex_unlock (iolink->exclusive);
}

void iolink_pl_max14819_get_status(iolink_hw_drv_t * iolink_hw, uint8_t _port, bool *_power, uint8_t *_baudrate)
{
	if (_port > 1)
		return;
		
	iolink_14819_drv_t * iolink = (iolink_14819_drv_t *)iolink_hw;	
	os_mutex_lock (iolink->exclusive);
	
	uint8_t myStatusRegister = iolink_14819_read_register (iolink, REG_LPCnfgA+_port);
	uint8_t myBaudrateRegister = iolink_14819_read_register (iolink, REG_CQCtrlA+_port);

	*_baudrate = (myBaudrateRegister>>6) & 0x03;
	
	if (myStatusRegister & 0x01)
	{
		*_power = 1;
	}
	
	else 
	{
		*_power = 0;
	}
	os_mutex_unlock (iolink->exclusive);
}


