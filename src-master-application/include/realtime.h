//	Realtime.h - setup for optimal realtime response
/*
	Copyright 2018 by Tom Roberts. All rights reserved.
	This code may be redistributed in accordance with the 
	Gnu Public License: https://www.gnu.org/licenses/gpl-3.0.en.html
*/

/*
 * Modifications for IOL HAT by Pinetek Networks
 */

/*
	This class sets up a Raspbeerry Pi system for optimal realtime
	response. On multi-core systems it requires this parameter in
	/boot/cmdline.txt:
		isolcpus=3

	Any program using this class must be run as root (e.g. via sudo).

	Unfortunately, the RPi hardware does not seem to be capable of
	turning interrupts off on just a single core (i.e. the kernel routine
	local_irq_disable() disables interrupts on all cores). There is
	still a lot to be gained, and system latencies under 30 microseconds
	are achievable on a RPi 3B+ dedicated to a single real-time process.

	This works well with a program that uses this basic structure:
	  Realtime::setup()
	  loop forever {
	      wait until time for next sample, using Realtime::delay() or a
	          short loop calling gettimeofday(), or a short loop
		  waiting for an edge on some GPIO, etc.
	      perform a sample using GPIO, SPI, I2C, ..., that needs good
	          realtime response
	      write result to a file or socket
	  }
	The kernel must run occasionally on every cpu, so the sample rate
	should be slow enough so that the wait is usually longer than 100
	microseconds (occasionally less is OK) -- Realtime::delay() will
	call usleep(). Alternatively, if the sampling does a BLOCKING read
	or write (e.g. SPI or I2C), then no usleep is needed, and the wait
	can be a CPU loop with dleays less tham 100 usec.

	setup() does the following:
	 1. disables turbo mode. This makes the cpu clock be fixed at its
	    lowest value. If it varies, timing in micros() gets screwed
	    up, and the SPI clock varies (destroying any timing using it).
	 2. sets the current process to real-time scheduling with priority
	    99. NOTE: the process must not use 100% of the CPU, or it will
	    starve the kernel. Reading or writing any device or file is
	    usually enough, or calling usleep() or sleep(); this includes
	    calling Realtime::delay(us) with us >= 100.
	 3. on multi-core systems sets CPU affinity to CPU 3 (the isolated 
	    one).

	On a Raspberry Pi 3B+, the latency program shows this achieves a 
	maximum of 41 us latency on an otherwise idle system. That can include 
	several idle ssh sessions, with one running top. With four copies of
	the following script running via ssh in 4 windows, latencies remain 
	< 120 us:
		while true; do date; done &
	With four copies of this script running, latencies remain < 41 us:
		while true; do true; done &
	So the increased latencies for the first script are primarily due to
	ssh data transfers.

	A "make -j 3" increases latencies to 45-50 us.

	#define TEST_REALTIME to create a test program in Realtime.cc.
	Or build the latency program.
*/

#ifndef REALTIME_H
#define REALTIME_H

#include <stdint.h>

class Realtime {
	static volatile uint32_t *systReg;
	static int fdMem;
	static uint32_t phys;
public:
	/// Sets up everything for optimum realtime response.
	/// If error, prints a message to stderr and exits the program 
	/// (usually means not running as root).
	/// NOTE: if you are calling gpioInitialise(), call it BEFORE setup().
	static void setup(uint8_t kernel);

	/// Returns the system clock in microseconds. That is basically the 
	/// time since booting, except it wraps every 71 minutes or so.
	/// NOTE: initMicros() (or setup()) must already have been called.
	static uint32_t micros() { return systReg[1]; }

	/// Delays for us microseconds. Or longer:
	/// This routine gives latencies < 25 us on an otherwise idle RPi 3B+;
	/// pigpio's gpioDelay() gives 40-50 us latencies.
	static void delay(int us);

	// The following routines are not normally called directly --
	// most are called by setup().

	/// Initializes micros(). Called by setup().
	/// Maps the system clock, so no external library is needed.
	static void initMicros();

	/// Disables turbo mode on a Raspberry Pi. Called by setup().
	static void disableTurbo();

	/// Enables turbo mode on a Raspberry Pi.
	/// This is rarely, if ever, used (the system boots up with it enabled).
	static void enableTurbo();

	/// Sets the current process's scheduling to SCHED_FIFO, with
	/// maximum priority. Linux only. Called by setup().
	/// NOTE: the program MUST yield the CPU reasonably frequently,
	/// or it will starve the kernel. Reading/writing a linux device
	/// or file will do, or calling usleep() or sleep(). That includes
	/// Realtime::delay() with a sleep >= 100 us.
	static void realTimeSched();

	/// Sets the current process to use cpu 3 (does nothing if fewer
	/// than 4 cores). Called by setup().
	static void cpu(uint8_t cpuNumber);

};

#endif // REALTIME_H