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

import struct
import socket
import time


TCP_IP = '127.0.0.1'
BUFFER_SIZE = 1024

TCP_PORT1 = 12010
TCP_PORT2 = 12011

def power (port, status):

	if (port not in [0,1,2,3]):
		raise ValueError("Port out of range")

	if (status not in [0,1]):
		raise ValueError("Status value out of range")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	#CMD PWR = 1
	message = struct.pack("!BBB",1, port, status);
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	try:
		s.connect((TCP_IP, tcp_port))

		s.send(message)
		data = s.recv(BUFFER_SIZE)

		data_len=len(data)

		#print ("received data:", data, ", len=", data_len)

		#ERROR
		if (data_len == 2):
			raw_data = struct.unpack("!BB", data)
			print (f"Data error (port={port}, status={status}):", int (raw_data[1]))

		else:
			raw_data = struct.unpack("!BBB", data)
			#print (f"Power set: port={port}, status={status}")

	except Exception as e:
		print (f"Set power error (port={port}, status={status}): {e}")
	
	finally:
		s.close()


#CMD PD
def pd (port, len_out, len_in, pd_out):
	if (port not in [0,1,2,3]):
		print("port out of range")
		raise ValueError("Port out of range")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	return_data = bytearray(len_in)
	message_buffer0 = bytearray(4)
	struct.pack_into("!BBBB",message_buffer0, 0, 3,port, len_out, len_in)

	
	out_buffer = message_buffer0
	if (len_out > 0):
		out_buffer += pd_out

	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((TCP_IP, tcp_port))
		#print (port, "out=", out_buffer)
		s.send(out_buffer)
		data = s.recv(BUFFER_SIZE)
		return_data = data[4:]

	except Exception as e:
		print (f"PD error (port={port}, pd_out={pd_out}): {e}")
		raise ValueError("PD error")
	finally:
		s.close()

	time.sleep(4/1000)
	return return_data


#CND LED

LED_OFF = 0
LED_GREEN= 1
LED_RED = 2


def led (port, status):

	if (port not in [0,1,2,3]):
		raise ValueError("Port out of range")

	if (status not in [0,1,2,3]):
		raise ValueError("LED value out of range")


	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	#CMD LED = 2
	message = struct.pack("!BBB",2, port, status);
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))

		s.send(message)
		data = s.recv(BUFFER_SIZE)

		data_len=len(data)

		#print ("received data:", data, ", len=", data_len)

		#ERROR
		if (data_len == 2):
			raw_data = struct.unpack("!BB", data)
			print (f"Data error (port={port}, status={status}):", int (raw_data[1]))

		else:
			raw_data = struct.unpack("!BBB", data)
			#print (f"LED set: port={port}, status={status}")

	except Exception as e:
		print (f"Set LED error (port={port}, status={status}): {e}")
	
	finally:
		s.close()

	## Give some time to prevent overload
	time.sleep(4/1000)







