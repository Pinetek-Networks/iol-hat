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
import sys



TCP_IP = '127.0.0.1'
BUFFER_SIZE = 1024

TCP_PORT1 = 12010
TCP_PORT2 = 12011

CMD_SUCCESS = 1
CMD_FAIL = 0

verbose = False #True

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

		if (verbose):
			print ("received data:", data, ", len=", data_len)

		#ERROR
		if (data_len == 2):
			raw_data = struct.unpack("!BB", data)
			print (f"Data error (port={port}, status={status}):", int (raw_data[1]))

		else:
			raw_data = struct.unpack("!BBB", data)
			if (verbose):
				print (f"Power set: port={port}, status={status}")

	except Exception as e:
		s.close()
		print (f"Set power error (port={port}, status={status}): {e}")
		raise Exception("Parameter write error (TCP message)")
		
	s.close()
	return CMD_SUCCESS


#CMD PD
def pd (port, len_out, len_in, pd_out):
	print ("*** CMD PD, port=",port, ", len_out=", len_out, ", len_in=", len_in)

	if (port not in [0,1,2,3]):
		print("PD: port out of range")
		raise ValueError("Port out of range")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	return_data = bytearray(len_in)
	message_buffer0 = bytearray(4)
	struct.pack_into("!BBBB",message_buffer0, 0, 3,port, len_out, len_in)

	if (verbose):
		print("PD: pd_out", pd_out)

	
	out_buffer = message_buffer0
	if (len_out > 0):
		out_buffer += pd_out

	if (verbose):
		print("PD: Send buffer", out_buffer)

	try:
		s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		s.connect((TCP_IP, tcp_port))
		#print (port, "out=", out_buffer)
		s.send(out_buffer)
		rcv_buffer = s.recv(BUFFER_SIZE)

		if (verbose):
			print("PD Receive buffer", rcv_buffer)
		return_data = rcv_buffer[4:]

	except Exception as e:
		s.close()
		print (f"PD: error (port={port}, pd_out={pd_out}): {e}")
		raise ValueError("PD error")

	s.close()
	time.sleep(4/1000)
	return return_data

#CND LED

LED_OFF = 0
LED_GREEN= 1
LED_RED = 2


def led (port, status):

	print ("*** CMD LED, port=",port)
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
			print (f"LED: TCP command error (port={port}, status={status}):", getErrorMessage (int (raw_data[1])))
			raise Exception("Parameter write error (TCP message)")

		else:
			raw_data = struct.unpack("!BBB", data)
			if (verbose):
				print (f"LED: port={port}, status={status}")

	except Exception as e:
		print (f"LED: error (port={port}, status={status}): {e}")
		raise Exception({e})
	
	s.close()
	## Give some time to prevent overload
	time.sleep(4/1000)
	return CMD_SUCCESS



def read (port, index, subindex, length):
	print ("*** CMD read, port=",port,", index=",index,", subindex=",subindex,", length=",length)
	return_data = ""

	if (port not in [0,1,2,3]):
		raise ValueError("READ: Port out of range")

	if(index > 0xFFFF):
		raise ValueError("READ: Index value too high")

	if(index < 0):
		raise ValueError("READ: Index value too high")

	if(subindex > 0xFF):
		raise ValueError("READ: Subindex value too high")

	if(subindex < 0):
		raise ValueError("READ: Subindex value too high")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	#CMD READ = 4
	message = struct.pack("!BBHBB",4, port, index, subindex, length);
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))

		s.send(message)
		data = s.recv(BUFFER_SIZE)

		data_len=len(data)
		if (verbose):
			print ("READ: received data:", data, ", len=", data_len)

		#ERROR (TCP message)
		if (data_len == 2):
			s.close()
			raw_data = struct.unpack("!BB", data)
			print (f"READ: TCP message error ", getErrorMessage (int (raw_data[1])))
			raise Exception("READ: TCP message error", getErrorMessage (int (raw_data[1])))

		#ERROR (IO-Link error)
		elif (data_len == 4):
			s.close()
			raw_data = struct.unpack("!BBH", data)
			print (f"READ: IO-Link error=", hex (raw_data[2]))
			return CMD_FAIL

		else:
			#print (f"Read: port={port}, status={status}")
			return_data = data[6:]

	except Exception as e:
		s.close()
		print (f"READ exception:", e)
		raise Exception({e})

	
	finally:
		s.close()
		return return_data

	## Give some time to prevent overload
	time.sleep(4/1000)
	return CMD_SUCCESS


def write (port, index, subindex, length, writeData):
	print ("*** CMD WRITE, port=",port,", index=",index,", subindex=",subindex,", length=",length, ", writeData=", writeData)

	if (port not in [0,1,2,3]):
		raise ValueError("WRITE: Port out of range")

	if(index > 0xFFFF):
		raise ValueError("WRITE: Index value too high")

	if(index < 0):
		raise ValueError("WRITE: Index value too high")

	if(subindex > 0xFF):
		raise ValueError("WRITE: Subindex value too high")

	if(subindex < 0):
		raise ValueError("WRITE: Subindex value too high")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	#CMD WRITE = 5
	snd_message = struct.pack("!BBHBB%ds" % len(writeData),5, port, index, subindex, length, writeData)

	if (verbose):
		print (snd_message)

	#Open a socket, connect and send the message
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))

		if (verbose):
			print ("WRITE: send message=", snd_message, ", len=", len(snd_message))

		s.send(snd_message)
		rcv_message = s.recv(BUFFER_SIZE)

		rcv_len=len(rcv_message)

		if (verbose):
			print ("WRITE: received message=", rcv_message, ", len=", rcv_len)

		#ERROR (TCP-Message)
		if (rcv_len == 2):
			raw_data = struct.unpack("!BB", rcv_message)
			print (f"WRITE: TCP message error ", getErrorMessage(int (raw_data[1])))
			raise Exception("WRITE: TCP message error", getErrorMessage(int (raw_data[1])))

		#ERROR (IO-Link)
		elif (rcv_len == 4):

			raw_data = struct.unpack("!BBH", rcv_message)
			print (f"WRITE: IO-Link error=", hex (raw_data[2]))
			return CMD_FAIL

	except Exception as e:
		s.close()
		print (f"WRITE exception:", e)
		raise Exception({e})

	s.close()
	## Give some time to prevent overload
	time.sleep(4/1000)
	return CMD_SUCCESS


# Class for the IOL status

class IolStatus:
	def __init__(self, pd_in_valid=0, pd_out_valid=0, transmission_rate=0, master_cycle_time=0,
				 pd_in_length=0, pd_out_length=0, vendor_id=0, device_id=0, power=0, error=None):
		"""
		Initializes the IolStatus object with default values.

		Parameters:
		pd_in_valid (int):        Indicates if the process data input is valid (0 or 1)
		pd_out_valid (int):       Indicates if the process data output is valid (0 or 1)
		transmission_rate (int):  COM speed (0=invalid, 1=COM1, 2=COM2, 3=COM3)
		master_cycle_time (int):  Master cycle time in ms
		pd_in_length (int):       Length of process data input in bytes
		pd_out_length (int):      Length of process data output in bytes
		vendor_id (int):          Vendor ID
		device_id (int):          Device ID
		power (int):              L+ enabled (0 or 1)
		error (int|None):         REG_ChanStatA/B bits 0-2 (CMD_STATUS2 only, None for CMD_STATUS)
		"""
		self.pd_in_valid = pd_in_valid
		self.pd_out_valid = pd_out_valid
		self.transmission_rate = transmission_rate
		self.master_cycle_time = master_cycle_time
		self.pd_in_length = pd_in_length
		self.pd_out_length = pd_out_length
		self.vendor_id = vendor_id
		self.device_id = device_id
		self.power = power
		self.error = error  # None if parsed from CMD_STATUS (13 bytes), int if from CMD_STATUS2 (14 bytes)

	@classmethod
	def from_buffer(cls, buffer):
		"""
		Parses a buffer into an IolStatus object.
		Accepts both CMD_STATUS payloads (13 bytes, no error field)
		and CMD_STATUS2 payloads (14 bytes, with error field).

		Parameters:
		buffer (bytes): Payload bytes after stripping the 2-byte [CMD][port] header.

		Returns:
		IolStatus: An instance of IolStatus populated with the parsed data.
		"""
		if len(buffer) < 13:
			raise ValueError(f"Buffer too short to parse IolStatus (got {len(buffer)}, need at least 13)")

		pd_in_valid      = buffer[0]
		pd_out_valid     = buffer[1]
		transmission_rate = buffer[2]
		master_cycle_time = buffer[3]
		pd_in_length     = buffer[4]
		pd_out_length    = buffer[5]
		vendor_id        = int.from_bytes(buffer[6:8],  byteorder='little')
		device_id        = int.from_bytes(buffer[8:12], byteorder='little')
		power            = buffer[12]
		error            = buffer[13] if len(buffer) >= 14 else None

		return cls(pd_in_valid, pd_out_valid, transmission_rate, master_cycle_time,
				   pd_in_length, pd_out_length, vendor_id, device_id, power, error)

	def print_status(self):
		"""
		Prints the current status of the IolStatus object.
		"""
		print(f"pd_in_valid:       {self.pd_in_valid}")
		print(f"pd_out_valid:      {self.pd_out_valid}")
		print(f"transmission_rate: {self.transmission_rate}")
		print(f"master_cycle_time: {self.master_cycle_time}")
		print(f"pd_in_length:      {self.pd_in_length}")
		print(f"pd_out_length:     {self.pd_out_length}")
		print(f"vendor_id:         0x{self.vendor_id:04X}")
		print(f"device_id:         0x{self.device_id:08X}")
		print(f"power:             {self.power}")
		if self.error is not None:
			print(f"error:             0x{self.error:02X}")

	def __repr__(self):
		error_str = f", error=0x{self.error:02X}" if self.error is not None else ""
		return (f"IolStatus(pd_in_valid={self.pd_in_valid}, pd_out_valid={self.pd_out_valid}, "
				f"transmission_rate={self.transmission_rate}, master_cycle_time={self.master_cycle_time}, "
				f"pd_in_length={self.pd_in_length}, pd_out_length={self.pd_out_length}, "
				f"vendor_id=0x{self.vendor_id:04X}, device_id=0x{self.device_id:08X}, "
				f"power={self.power}{error_str})")


def readStatus(port):
	print ("*** CMD STATUS, port=",port)

	if (port not in [0,1,2,3]):
		raise ValueError("STATUS: Port out of range")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	# CMD_STATUS = 6, response: [CMD][port] + 13 bytes payload = 15 bytes total
	message = struct.pack("!BB", 6, port)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))
		s.send(message)
		data = s.recv(BUFFER_SIZE)
		data_len = len(data)

		if verbose:
			print("received data:", data, ", len=", data_len)

		if data_len == 2:
			s.close()
			raw_data = struct.unpack("!BB", data)
			print(f"STATUS: TCP message error ", getErrorMessage(int(raw_data[1])))
			raise Exception("STATUS: TCP message error", getErrorMessage(int(raw_data[1])))

		elif data_len != 15:
			s.close()
			print(f"STATUS: unexpected response length, expected 15, got {data_len}")
			raise Exception("STATUS: unexpected response length")

		else:
			return IolStatus.from_buffer(data[2:])

	except Exception as e:
		s.close()
		print(f"STATUS: exception:", e)
		raise

	s.close()


def readStatus2(port):
	print ("*** CMD STATUS2, port=",port)

	if (port not in [0,1,2,3]):
		raise ValueError("STATUS2: Port out of range")

	if (port < 2):
		tcp_port = TCP_PORT1
	else:
		tcp_port = TCP_PORT2
		port=port-2

	# CMD_STATUS2 = 8, response: [CMD][port] + 14 bytes payload = 16 bytes total
	message = struct.pack("!BB", 8, port)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))
		s.send(message)
		data = s.recv(BUFFER_SIZE)
		data_len = len(data)

		if verbose:
			print("received data:", data, ", len=", data_len)

		if data_len == 2:
			s.close()
			raw_data = struct.unpack("!BB", data)
			print(f"STATUS2: TCP message error ", getErrorMessage(int(raw_data[1])))
			raise Exception("STATUS2: TCP message error", getErrorMessage(int(raw_data[1])))

		elif data_len != 16:
			s.close()
			print(f"STATUS2: unexpected response length, expected 16, got {data_len}")
			raise Exception("STATUS2: unexpected response length")

		else:
			return IolStatus.from_buffer(data[2:])

	except Exception as e:
		s.close()
		print(f"STATUS2: exception:", e)
		raise

	s.close()


def readStatus3(chip):
	"""
	Reads the chip-level REG_Status (0x1E) from the MAX14819.
	This is a chip-level command (not per-port) — it returns the combined live +
	latched fault register for the entire chip (both channels).

	The lower nibble contains live bits (not cleared on read).
	The upper nibble contains clear-on-read (COR) bits, which are set on the
	0->1 transition of the corresponding live bit and cleared when the register
	is read. The firmware latches the COR bits in software so they are not lost
	between interrupt and this read.

	Parameters:
	chip (int): 0 = chip on TCP_PORT1 (ports 0-1), 1 = chip on TCP_PORT2 (ports 2-3)

	Returns:
	int: reg_status byte with the following bit mapping:
	     Bit 0: VCCWarn    — VCC supply voltage warning (<18V), live
	     Bit 1: VCCUV      — VCC undervoltage (<9V), live
	     Bit 2: ThWarn     — Die temperature warning (>135°C), live
	     Bit 3: ThShdn     — Thermal shutdown (>150°C), live
	     Bit 4: VCCWarnCOR — VCC supply voltage warning, clear-on-read
	     Bit 5: VCCUVCOR   — VCC undervoltage, clear-on-read
	     Bit 6: ThWarnCOR  — Die temperature warning, clear-on-read
	     Bit 7: ThShdnCOR  — Thermal shutdown, clear-on-read
	"""
	print("*** CMD STATUS3, chip=", chip)

	if chip not in [0, 1]:
		raise ValueError("STATUS3: chip must be 0 or 1")

	tcp_port = TCP_PORT1 if chip == 0 else TCP_PORT2

	# CMD_STATUS3 = 9, no port byte, response: [CMD][reg_status] = 2 bytes
	message = struct.pack("!B", 9)
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

	try:
		s.connect((TCP_IP, tcp_port))
		s.send(message)
		data = s.recv(BUFFER_SIZE)
		data_len = len(data)

		if verbose:
			print("received data:", data, ", len=", data_len)

		if data_len != 2:
			s.close()
			print(f"STATUS3: unexpected response length, expected 2, got {data_len}")
			raise Exception("STATUS3: unexpected response length")

		if data[0] == 0xFF:
			s.close()
			raw_data = struct.unpack("!BB", data)
			print(f"STATUS3: TCP message error ", getErrorMessage(int(raw_data[1])))
			raise Exception("STATUS3: TCP message error", getErrorMessage(int(raw_data[1])))

		reg_status = data[1]
		if verbose:
			print(f"reg_status=0x{reg_status:02X}")
		return reg_status

	except Exception as e:
		s.close()
		print(f"STATUS3: exception:", e)
		raise

	s.close()



def getErrorMessage(error_code):
	"""
	Returns the plain text message for the given error code.

	Arguments:
		error_code (int): The error code for which the message is needed.

	Returns:
		str: A human-readable error message.
	"""
	error_messages = {
		0xFF: "General error",
		0x01: "Error: Invalid length",
		0x02: "Error: Function not supported",
		0x03: "Error: Power failure",
		0x04: "Error: Invalid port ID",
		0x05: "Error: Internal error"
	}

	return error_messages.get(error_code, "Unknown error code")


