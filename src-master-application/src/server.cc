// TCP Server
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include "osal.h"
#include "osal_log.h"
#include "iolink.h"
#include "iolink_smi.h"
#include "iolink_max14819.h"
#include "iolink_pl.h"
#include "iolink_port.h"

#include "server.h"
#include "common.h"


#define PORT_NUMBER 2

os_mutex_t *mtx_cmd0;
os_mutex_t *mtx_cmd1;
iolCmd cmd0, cmd1;
iolStatus status[PORT_NUMBER];


void *runServer (void *_arg)
{	
  // Start TCP Server
	
	mtx_cmd0 = os_mutex_create();
	mtx_cmd1 = os_mutex_create();
	
	in_port_t myPort;

	socket_thread_t * socketArgs = (socket_thread_t *)_arg;
	myPort = socketArgs->tcpPort;
		
	int serverFd, newSocket, valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	uint8_t buffer[1024] = { 0 };
	uint8_t dataBuffer[1024] = { 0 };
	
	LOG_DEBUG (IOLINK_PL_LOG, "Create Socket\n");
	
	// Creating socket file descriptor
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		LOG_DEBUG (IOLINK_PL_LOG, "socket failed\n");
		exit(EXIT_FAILURE);
	}
	

	const int enable = 1;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int)) < 0)
	{
		LOG_DEBUG (IOLINK_PL_LOG,"setsockopt O_REUSEADDR | SO_REUSEPORT failed\n");    
		exit(EXIT_FAILURE);
	}
		
		
	address.sin_family = AF_INET;
	// Bind to localhost
	inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr);
	address.sin_port = htons(myPort);

	// Forcefully attaching socket to the port 
	if (bind(serverFd, (struct sockaddr*)&address,	sizeof(address))< 0)
	{
		LOG_DEBUG (IOLINK_PL_LOG,"bind failed\n");
		exit(EXIT_FAILURE);
	}
		
	#define RET_ERROR 0xFF
	#define RET_ERROR_LEN	0x01
	#define RET_ERROR_FUNC 0x02
	#define RET_ERROR_PWR 0x03
	#define RET_ERROR_PORT_ID 0x04
	#define RET_ERROR_INTERNAL 0x05
		
	while (1)
	{
		LOG_DEBUG (IOLINK_PL_LOG,"Listen on port %d\n", myPort);
		
		if (listen(serverFd, 1) < 0) // Allow only 1 connection
		{
			LOG_ERROR (IOLINK_PL_LOG, "listen failed\n");
			exit(EXIT_FAILURE);
		}
		
		if ((newSocket	= accept(serverFd, (struct sockaddr*)&address,	(socklen_t*)&addrlen))< 0) 
		{
			LOG_ERROR (IOLINK_PL_LOG,"accept failed\n");
			exit(EXIT_FAILURE);
		}
				
		LOG_DEBUG (IOLINK_PL_LOG, "Accept ok\n");
		
		valread = read(newSocket, buffer, 1024);

				
	 char rxBuf[1024*5] = ""; 
	 
	 for (int i=0; i<valread; i++)
	 {
		 char tempBuffer[10];
		 sprintf(tempBuffer, "%02X ", buffer[i]);
		 strcat(rxBuf, tempBuffer);
	 } 
	 
		LOG_DEBUG (IOLINK_PL_LOG, "READ %d bytes:%s\n", valread, rxBuf);
		
		// Structure is CMD | PORT | ...
		
		// Too less bytes
		if (valread < 2)
		{
			LOG_ERROR(IOLINK_PL_LOG, "ERROR: Command len\n");
			uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
			send(newSocket, myErrorMessage, 2, 0);
		}
		
		// Port not in range 0..1
		else if (buffer[1] > 1)
		{
			char myErrorMessage[] = "ERROR: port out of range\n"; 
			LOG_ERROR(IOLINK_PL_LOG, "%s\n",myErrorMessage);
			send(newSocket, myErrorMessage, strlen(myErrorMessage), 0);
		}	
		
		else
		{
			uint8_t myPort = buffer[1];
			LOG_DEBUG(IOLINK_PL_LOG, "1\n");
			iolink_app_port_ctx_t * app_port = &iolink_app_master.app_port[buffer[1]];
			LOG_DEBUG(IOLINK_PL_LOG, "app_port=%p\n", app_port);
			LOG_DEBUG(IOLINK_PL_LOG, "buffer[0]=%d\n", buffer[0]);
			// Switch CMD
			switch (buffer[0])
			{
				LOG_DEBUG(IOLINK_PL_LOG, "CMD_PWR\n");
				// Power
				case CMD_PWR:
				// [CMD] [port] [status]
				// Returns [CMD] [port] [status]
				{
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_PWR");
					if (valread != 3)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PWR len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PWR port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					LOG_DEBUG(IOLINK_PL_LOG, "iolink_hw=%p\n", socketArgs->iolink_hw);
					LOG_DEBUG(IOLINK_PL_LOG, "iolink_pl_max14819_set_power=%p\n", socketArgs->iolink_hw->ops->set_power);
					
					// Set the power
					socketArgs->iolink_hw->ops->set_power(socketArgs->iolink_hw, (uint8_t) buffer[1], (bool) buffer[2]);
					// Return status
					send(newSocket, buffer, 3, 0);						
				}
				break;
				
				// LED
				case CMD_LED: 
				// [CMD] [port] [LEDs] 0x01=R 0x02=G
				// Returns [CMD] [port] [LEDs] 0x01=G 0x02=R
				{
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_LED");
					if (valread != 3)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_LED len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}

					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_LED port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
			
					socketArgs->iolink_hw->ops->set_led(socketArgs->iolink_hw, (uint8_t) buffer[1], (bool)(buffer[2]&0x01)?1:0, (bool) (buffer[2]&0x02)?1:0);
					
					send(newSocket, buffer, 3, 0);		
				}
				break;
				
				//Process data
				case CMD_PD: 
				// [CMD] [port] [LenOut] [LenIn] [DataOut..]
				// Returns [CMD] [port] [LenOut] [LenIn] [DataIn...]
				{					
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_PD\n");
					
					if (valread < 4) // 4 is minimum with LenOut=0
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PI len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PD port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					iolCmd *myCmd;
					os_mutex_t *myMutex;
					
					if (myPort == 0)
					{
						myCmd = &cmd0;
						myMutex = mtx_cmd0;

					}
					else
					{
						myCmd = &cmd1;
						myMutex = mtx_cmd1;

					}

					os_mutex_lock (myMutex);
									
					// Zero out structure
					memset(myCmd->dataOut, 0x00, sizeof (myCmd->dataOut));
					
					myCmd->command = CMD_PD;
					myCmd->port = myPort;
					
					myCmd->lenOut = buffer [2];
					myCmd->lenIn = buffer[3];
					
					memcpy(myCmd->dataOut, &buffer[4], myCmd->lenOut);
					memcpy(&buffer[4], myCmd->dataIn, myCmd->lenIn);
					
					buffer[3] = myCmd->lenIn;						
					send(newSocket, buffer, myCmd->lenIn+4, 0);
					
					os_mutex_unlock (myMutex);
				}
								
				break;
				
				
				#ifdef HISTORY
				//Process data with history
				case CMD_PD_HISTORY: 
				// [CMD] [port] [LenOut] [LenIn] [DataOut..]
				// Returns [CMD] [port] [LenOut] [LenIn] [HistoryCount] [DataIn...] [DataInHistory..]
				{					
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_PD_HISTORY\n");
					
					if (valread < 4) // 4 is minimum with LenOut=0
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PI len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_PD port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					iolCmd *myCmd;
					os_mutex_t *myMutex;
					std::deque<pd_in_struct> *myQueue;
					
					if (myPort == 0)
					{
						myCmd = &cmd0;
						myMutex = mtx_cmd0;
						myQueue = &pd_in_queue0;

					}
					else
					{
						myCmd = &cmd1;
						myMutex = mtx_cmd1;
						myQueue = &pd_in_queue1;
					}

					os_mutex_lock (myMutex);
									
					// Zero out structure
					memset(myCmd->dataOut, 0x00, sizeof (myCmd->dataOut));
					
					myCmd->command = CMD_PD;
					myCmd->port = myPort;
					
					myCmd->lenOut = buffer [2];
					myCmd->lenIn = buffer[3];
					
					// Copy to command for PD_OUT
					memcpy(myCmd->dataOut, &buffer[4], myCmd->lenOut);
					
					// Copy queue (includes last PD_IN)
					uint8_t myPos = 0;
					
					for (const auto& item : *myQueue)
					{
						memcpy(&buffer[5+((myPos)*myCmd->lenIn)], item.data_in, myCmd->lenIn);
						myPos++;
					}
					
					buffer[3] = myCmd->lenIn;						
					buffer[4] = myQueue->size();
					
					// Len is 5 + Current PD IN + History PD_IN
					send(newSocket, buffer, 5+ myQueue->size()*myCmd->lenIn, 0);
					
					myQueue->clear();					
					os_mutex_unlock (myMutex);
				}
								
				break;
				#endif // HISTORY


				case CMD_READ:
				{
					// [CMD] [port] [index 16b] [subindex] [len]					
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_READ\n");
					
					if (valread != 6)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_READ len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_READ port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					uint16_t index = ((uint16_t) buffer [2] << 8) | buffer [3];
					uint8_t subindex = buffer[4];
					uint8_t lenRequested = buffer[5];					
					
					uint8_t myActualLen = 0;
					
					LOG_DEBUG (LOG_STATE_ON, "CMD_READ i%d s%d l%d\n", index, subindex, lenRequested);
				

					iolink_smi_errortypes_t myError = do_smi_device_read(app_port, index, subindex, lenRequested, &buffer[6], &myActualLen);
					
					LOG_DEBUG (LOG_STATE_ON, "Read returned %d\n", (uint16_t)myError);
					
			
					if (myError != IOLINK_SMI_ERRORTYPE_NONE) {
						LOG_WARNING(LOG_STATE_ON, "%s: Failed to read on port %u\n", __func__, app_port->portnumber);
						
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_INTERNAL, 0, 0};
						
						myErrorMessage[2] = (myError>>8) & 0xFF;
						myErrorMessage[3] = myError & 0xFF;						
						
						send(newSocket, myErrorMessage, 4, 0);				
						break;
					}
					
					buffer[5] = myActualLen;					

					// Send success message
					send(newSocket, buffer, 6+myActualLen,0);						
				}					
				
				break;
				
				case CMD_WRITE:
				{
					// [CMD] [port] [index 16b] [subindex] [len] [dataOut]				
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_WRITE\n");
					
					if (
						(valread < 7) || // 7 is minimum for 1 octet of write data
						(valread != buffer[5] + 6) // Len + overhead
					)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_WRITE len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_WRITE port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					uint16_t index = ((uint16_t) buffer [2] << 8) | buffer [3];
					uint8_t subindex = buffer[4];
					uint8_t lenOut = buffer[5];
					
					
					memcpy(dataBuffer, &buffer[6], lenOut );
						
					LOG_DEBUG (LOG_STATE_ON, "CMD_WRITE i%d s%d l%d\n", index, subindex, lenOut);
					
					for (int i=0; i<lenOut; i++)
					{
						LOG_DEBUG(LOG_STATE_ON, "data[%d]=%02X\n", i, buffer[6+i]);
					}
					
					iolink_smi_errortypes_t myError = do_smi_device_write(app_port, index, subindex, lenOut, dataBuffer) ;

					if (myError!= IOLINK_SMI_ERRORTYPE_NONE) {
						LOG_WARNING(LOG_STATE_ON, "%s: Failed to write on port %u, error = %u\n", __func__, app_port->portnumber, myError);
						
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_INTERNAL, 0, 0};
						
						myErrorMessage[2] = (myError>>8) & 0xFF;
						myErrorMessage[3] = myError & 0xFF;
						send(newSocket, myErrorMessage, 4, 0);
						break;
					}
					
					send(newSocket, buffer, 6,0);
				}
				
				break;
				
				
				case CMD_STATUS:
				{
					// [CMD] [port]					
					LOG_DEBUG(IOLINK_PL_LOG, "CMD_STATUS\n");
					
					if (valread != 2)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_STATUS len\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_LEN};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					if (buffer[1] > PORT_NUMBER)
					{
						LOG_ERROR(IOLINK_PL_LOG, "ERROR: CMD_STATUS port id out of range\n");
						uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_PORT_ID};
						send(newSocket, myErrorMessage, 2, 0);
						break;
					}
					
					bool myPower; 
					uint8_t myBaudrate;
					uint8_t myPort = buffer[1];
					
					socketArgs->iolink_hw->ops->get_status(socketArgs->iolink_hw, myPort, &myPower, &myBaudrate);
					iolink_port_t *myPort_t  = iolink_get_port (iolink_app_master.master, app_port->portnumber);
					
					uint8_t myPortStatus =  app_port->app_port_state;
					
					// Port not active => Set transmission rate 0 (invalid)
					if (myPortStatus != IOL_STATE_RUNNING)
					{
						status[myPort].transmissionRate = 0;					
					}
					
					else 
					{
						status[myPort].transmissionRate = (uint8_t) myBaudrate;
					}										
					
					status[myPort].power = myPower;
					
					uint8_t myCycletime = 0;

					// Check if cycle time can be determined
					if ((myPort == 0) && (mode_ch_0 == iolink_mode_SDCI))
								myCycletime = iolink_pl_get_cycletime(myPort_t);
								
					if ((myPort == 1) && (mode_ch_1 == iolink_mode_SDCI))
								myCycletime = iolink_pl_get_cycletime(myPort_t);

					status[myPort].masterCycleTime = myCycletime;
					
					iolink_app_port_status_t * port_status = &app_port->status;

					status[myPort].vendorId = port_status->vendorid;
					status[myPort].deviceId = port_status->deviceid;
					
					LOG_DEBUG(LOG_STATE_ON, "port=%d\n", myPort);
					LOG_DEBUG(LOG_STATE_ON, "pdInValid=%d\n", status[myPort].pdInValid);
					LOG_DEBUG(LOG_STATE_ON, "pdOutValid=%d\n", status[myPort].pdOutValid);
					LOG_DEBUG(LOG_STATE_ON, "transmissionRate=0x%X\n", status[myPort].transmissionRate);					
					LOG_DEBUG(LOG_STATE_ON, "cycleTime=0x%X\n", status[myPort].masterCycleTime);
					LOG_DEBUG(LOG_STATE_ON, "pdInLength=%d\n", status[myPort].pdInLength);
					LOG_DEBUG(LOG_STATE_ON, "pdOutLength=%d\n", status[myPort].pdOutLength);
					LOG_DEBUG(LOG_STATE_ON, "vendorId=0x%X\n", status[myPort].vendorId);
					LOG_DEBUG(LOG_STATE_ON, "deviceId=0x%X\n", status[myPort].deviceId);
					LOG_DEBUG(LOG_STATE_ON, "power=%d\n", status[myPort].power);
					
					
					memcpy(&buffer[2], &status[myPort], sizeof(iolStatus));
					
					send(newSocket, buffer, 2+sizeof(iolStatus),0);		
				}
				break;
				
				default:
				{
					LOG_ERROR(IOLINK_PL_LOG, "ERROR: function not supported\n");
					uint8_t myErrorMessage[] = {RET_ERROR, RET_ERROR_FUNC};
					send(newSocket, myErrorMessage, 2, 0);
					break;
				}				
			}
		}
	
		// closing the connected socket
		LOG_DEBUG (IOLINK_PL_LOG,"Close socket\n");
		close(newSocket);

	}
	
	//closing the listening socket
	//shutdown(server_fd, SHUT_RDWR);
	
}