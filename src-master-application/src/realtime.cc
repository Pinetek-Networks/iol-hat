//	Realtime.cc
/*
	Copyright 2018 by Tom Roberts. All rights reserved.
	This code may be redistributed in accordance with the 
	Gnu Public License: https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>

#include "realtime.h"

volatile uint32_t *Realtime::systReg = 0;
int Realtime::fdMem = -1;
uint32_t Realtime::phys = 0;

void Realtime::setup(uint8_t kernel)
{
	initMicros();
	disableTurbo();
	realTimeSched();
	cpu(kernel);
}

void Realtime::initMicros()
{
	// based on pigpio source; simplified and re-arranged
	fdMem = open("/dev/mem",O_RDWR|O_SYNC);
	if(fdMem < 0) {
		fprintf(stderr,"Cannot map memory (need sudo?)\n");
		exit(1);
	}
	// figure out the address
	phys = 0x3F000000;
	/*
	FILE *f = fopen("/proc/cpuinfo","r");
	char buf[1024];
	fgets(buf,sizeof(buf),f); // skip first line
	fgets(buf,sizeof(buf),f); // model name
	if(strstr(buf,"ARMv6")) {
		phys = 0x20000000;
	} else if(strstr(buf,"ARMv7")) {
		phys = 0x3F000000;
	} else if(strstr(buf,"ARMv8")) {
		phys = 0x3F000000;
	} else {
		fprintf(stderr,"Unknown CPU type\n");
		exit(1);
	}
	
	fclose(f);
	*/
	systReg = (uint32_t *)mmap(0,0x1000,PROT_READ|PROT_WRITE,
				MAP_SHARED|MAP_LOCKED,fdMem,phys+0x3000);
}

void Realtime::delay(int us)
{
	// The final microsecond can be short; don't let the delay be short.
	++us;

	// usleep() on its own gives latencies 20-40 us; this combination
	// gives < 25 us.
	uint32_t start = micros();
	if(us >= 100)
		usleep(us - 50);
	while(micros()-start < ((uint32_t) us))
		;
}

void Realtime::disableTurbo()
{
	// This fixes the CPU clock to its minimum value, so timing is
	// not screwed up by changing it; also keep the SPI clock correct.
	system("sudo cp /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq "
		"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
}

void Realtime::enableTurbo()
{
	system("sudo cp /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq "
		"/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
}

void Realtime::realTimeSched()
{
	int prio = sched_get_priority_max(SCHED_FIFO);
	struct sched_param param;
	param.sched_priority = prio;
	sched_setscheduler(0,SCHED_FIFO,&param);
	// This permits realtime processes to use 100% of a CPU, but on a
	// RPi that starves the kernel. Without this there are latencies
	// up to 50 MILLISECONDS.
	system("echo -1 >/proc/sys/kernel/sched_rt_runtime_us");
}

void Realtime::cpu(uint8_t cpuNumber)
{
	if (cpuNumber > 3)
		return;
	
	// this does nothing if there are fewer than 4 CPUs
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(cpuNumber,&cpuset);
	sched_setaffinity(0,sizeof(cpu_set_t),&cpuset);
}