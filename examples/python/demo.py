# 
# This file is part of the iol-hat distribution (https://github.com/Pinetek-Networks/iol-hat/).
# Copyright (c) 2024 Pinetek Networks.
# 
# This program is free software: you can redistribute it and/or modify  
# it under the terms of the GNU General Public License as published by  
# the Free Software Foundation, version 3.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#


#!/usr/bin/env python
import socket
import struct
import iolhat
import sys

import time


# Shut down
def closePorts(setLeds = False):
	print ("Close Ports - Set Power+LEDs...")

	try:
		iolhat.power (0,0)
		if (setLeds):
			iolhat.led(0,iolhat.LED_OFF)
	except:
		iolhat.led(0,iolhat.LED_RED)

	try:
		iolhat.power (1,0)
		if (setLeds):
			iolhat.led(1,iolhat.LED_OFF)
	except:
		iolhat.led(1,iolhat.LED_RED)
	sys.exit()

# This example assumes the following connections
# Port 0:
# IFM HMI E30391: https://www.ifm.com/de/en/product/E30391
# 1 octets PD in input
# 14 octets PD out output
# PD layout see here: https://www.ifm.com/download/files/ifm-E30391_AB-20160613-IODD11-en-fix/$file/ifm-E30391_AB-20160613-IODD11-en-fix.pdf
#
# Port 1:
# IFM 6124 https://www.ifm.com/de/de/product/IF6124
# 2 octets PD out output
# 0 octeds PD in input
# PD layout see here: https://www.ifm.com/download/files/ifm-IF6123-20170707-IODD11-en/$file/ifm-IF6123-20170707-IODD11-en.pdf


def main():
	print ("Demo program for IOL HAT")
	print ("This program expects the following devices:")
	print ("	Port 0: ifm HMI E30391")
	print ("	Port 0: ifm distance sensor IF6124")
	# Set POWER ON with the power command
	# Set the LEDs on with the led command
	print ("Set Power+LEDs...")

	try:
		iolhat.power (0,1)
		iolhat.led(0,iolhat.LED_GREEN)
	except:
		iolhat.led(0,iolhat.LED_RED)

	try:
		iolhat.power (1,1)
		iolhat.led(1,iolhat.LED_GREEN)
	except:
		iolhat.led(1,iolhat.LED_RED)

	# CMD_STATUS: port-level status without error field
	print("\n--- CMD_STATUS (port 0) ---")
	iol_status = iolhat.readStatus(0)
	iol_status.print_status()

	print("\n--- CMD_STATUS (port 1) ---")
	iol_status = iolhat.readStatus(1)
	iol_status.print_status()

	# CMD_STATUS2: port-level status including REG_ChanStatA/B error field
	print("\n--- CMD_STATUS2 (port 0) ---")
	iol_status2 = iolhat.readStatus2(0)
	iol_status2.print_status()

	print("\n--- CMD_STATUS2 (port 1) ---")
	iol_status2 = iolhat.readStatus2(1)
	iol_status2.print_status()

	# CMD_STATUS3: chip-level REG_Status (VCC and thermal faults)
	# chip=0 covers ports 0+1, chip=1 covers ports 2+3
	print("\n--- CMD_STATUS3 (chip 0, ports 0+1) ---")
	reg_status = iolhat.readStatus3(0)
	print(f"reg_status = 0x{reg_status:02X}")
	if reg_status & 0x11:
		print("  WARNING: VCCWarn — VCC supply voltage <18V (live={}, latched={})".format(
			bool(reg_status & 0x01), bool(reg_status & 0x10)))
	if reg_status & 0x22:
		print("  WARNING: VCCUV — VCC undervoltage <9V (live={}, latched={})".format(
			bool(reg_status & 0x02), bool(reg_status & 0x20)))
	if reg_status & 0x44:
		print("  WARNING: ThWarn — die temperature >135°C (live={}, latched={})".format(
			bool(reg_status & 0x04), bool(reg_status & 0x40)))
	if reg_status & 0x88:
		print("  CRITICAL: ThShdn — thermal shutdown >150°C (live={}, latched={})".format(
			bool(reg_status & 0x08), bool(reg_status & 0x80)))
	if reg_status == 0:
		print("  OK: no faults")

	# Wait for Power
	time.sleep(1)

	# i is counter
	i = 0
	# pos is the position value read from the distance sensor
	pos_value = 0
	# button_value is the value of the HMI input buttons
	button_value = 0

	# The following section demonstrates how to read and write parameters
	# Read the application specific tag (index 24, subindex 0, length 32 byte)
	print ("Demo: Read port 1, index 24")
	try:
		myReadData=iolhat.read(1,24,0,32)
		print ("Read raw data:", myReadData)

	except Exception as e:
		print("read exception: ",e)
		closePorts()
		iolhat.led(1,iolhat.LED_RED)
		sys.exit()

	# Write the application specific tag (index 24, subindex 0, length 32 byte)
	# The first write might fail due to the state of the IOL stack and/or the device, therefore repeat twice
	for i in range(2):
	
		print ("Demo: Write port 1, index 24")
		try:
			myWriteData = bytes("PinetekNetworks", 'utf-8')
			#length is length of raw data
			print ("Demo: Data to write:", myWriteData)
			data=iolhat.write(1,24,0,len(myWriteData), myWriteData)

		except Exception as e:
			print("write exception: ",e)
			closePorts()
			iolhat.led(0,iolhat.LED_RED)
			sys.exit()
		time.sleep (1)

	# Start loop to aquire input data and send output data
	print ("Run loop, press 'q' to exit")
	while True:
		# PORT1: Read Distance sensor (2 byte in, 0 byte out)
		try:
			data=iolhat.pd(1,0,2,None)
			iolhat.led(1,iolhat.LED_GREEN)
			raw_data = struct.unpack("!H", data)
			pos_value = int(raw_data[0]>>2)

		except:
			print("e")
			closePorts()
			iolhat.led(0,iolhat.LED_RED)

		# PORT0: Compile output message to HMI,
		len0 = 14
		message_buffer0 = bytearray(len0)
		#line 1: Pos
		#line 2: Counter
		struct.pack_into("!HHBBBBBBBBBB",message_buffer0,0,pos_value,i,0,0,0,0,0,0,0,0,0,2)
		#                                                                                ^ Use 2-line display

		# PORT0: Read Buttons and output int values to HMI (1 byte in, 14 byte out)
		try:
			data=iolhat.pd(0,14,1,message_buffer0)
			iolhat.led(0,iolhat.LED_GREEN)
			# we do not need to parse as there is only 1 byte
			button_value  = int (data[0] & 0x0F)

		except:
			closePorts()
			iolhat.led(0,iolhat.LED_RED)

		# Print result to console
		print ("button_value=",button_value, ", pos_value=", pos_value, "     ") # end='\r')
		# Increment counter
		i+=1
		time.sleep(0.1)



# Python main function
if __name__ == '__main__':
	main()
