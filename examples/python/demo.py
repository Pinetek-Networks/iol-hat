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


def main():

	# Set POWER ON
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

	i = 0
	pos_value = 0
	button_value = 0

	try:
		data=iolhat.read(0,58,0,1)
		print (data)

	except Exception as e:
		print("read exception: "+e)
		iolhat.led(0,iolhat.LED_RED)

	try:
		data=iolhat.write(0,58,0,1, b'\02') #bytes("test", 'utf-8'))
		print (data)

	except Exception as e:
		print("read exception: "+e)
		iolhat.led(0,iolhat.LED_RED)


	return

	# Start loop to aquire and output data
	while True:
		# PORT1: Read Distance sensor
		try:
			data=iolhat.pd(1,0,2,None)
			iolhat.led(1,iolhat.LED_GREEN)
			raw_data = struct.unpack("!H", data)
			pos_value = int(raw_data[0]>>2)

		except:
			print("e")
			iolhat.led(0,iolhat.LED_RED)

		# PORT0: Compile output message to HMI, read back buttons
		len0 = 14
		message_buffer0 = bytearray(len0)
		#line 1: Pos
		#line 2: Counter
		struct.pack_into("!HHBBBBBBBBBB",message_buffer0,0,pos_value,i,0,0,0,0,0,0,0,0,0,2)
		#                                                                           ^ Use 2-line display

		try:
			data=iolhat.pd(0,len0,1,message_buffer0)
			iolhat.led(0,iolhat.LED_GREEN)
			# we do not need to parse as there is only 1 byte
			button_value  = int (data[0] & 0x0F)

		except:
			iolhat.led(0,iolhat.LED_RED)

		# Print result
		print ("button_value=",button_value, ", pos_value=", pos_value, "     ", end='\r')
		# Increment counter
		i+=1


if __name__ == '__main__':
	main()
