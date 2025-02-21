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

#ifndef IOLINK_PORT_H
#define IOLINK_PORT_H

#include "iolink_handler.h"

struct pd_in_struct {
  char data_in[256];
};

#ifdef HISTORY


#include <deque>
extern std::deque <pd_in_struct> pd_in_queue0;
extern std::deque <pd_in_struct> pd_in_queue1;

#endif


void generic_setup0 (iolink_app_port_ctx_t * app_port);
void generic_setup1 (iolink_app_port_ctx_t * app_port);


#endif // IOLINK_PORT_H
