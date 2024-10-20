#include "iol-hat.h"

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <arpa/inet.h>

#include <unistd.h>

#include <stdbool.h>

// IP address of the IO-Link Master for TCP communication
#define TCP_IP "127.0.0.1"

// Buffer size for receiving data
#define BUFFER_SIZE 1024

// TCP port number for communication with ports 0 and 1 of the IO-Link device
#define TCP_PORT1 12010

// TCP port number for communication with ports 2 and 3 of the IO-Link device
#define TCP_PORT2 12011

// Verbose flag for enabling/disabling detailed logging (set to 1 to enable, 0 to disable)
#define VERBOSE

bool verbose = false;

// ***********************************************************************************************************
// Function to print log messages if verbose mode is enabled
void log_message(const char * message) {
  #ifdef VERBOSE
  printf("%s\n", message);
  #endif
}

// ***********************************************************************************************************
// Function to get a human-readable error message based on the error code
const char * get_error_message(uint8_t error_code) {
  switch (error_code) {
  case 0xFF:
    return "General error";
  case 0x01:
    return "Error: Invalid length";
  case 0x02:
    return "Error: Function not supported";
  case 0x03:
    return "Error: Power failure";
  case 0x04:
    return "Error: Invalid port ID";
  case 0x05:
    return "Error: Internal error";
  default:
    return "Unknown error code";
  }
}
// ***********************************************************************************************************
// Function to set power on a specific port
int power(uint8_t _port, uint8_t _status) {
  if (_port > 3) {
    fprintf(stderr, "Port out of range\n");
    return CMD_FAIL;
  }

  if (_status > 1) {
    fprintf(stderr, "Status value out of range\n");
    return CMD_FAIL;
  }

  int tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command PWR = 1
  unsigned char message[3] = {
    1,
    _port,
    _status
  };
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, sizeof(message), 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    perror("Receive error");
  } else if (bytes_received == 2) {
    // Handle error response
    fprintf(stderr, "Data error (port=%d, status=%d): %s\n", _port, _status, get_error_message(buffer[1]));
  } else {
    // Success
    log_message("Power set successfully");
  }

  close(sock);

  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to handle process data communication for the specified port
int pd(uint8_t _port, uint8_t _len_out, uint8_t _len_in, uint8_t * _pd_out, uint8_t * _pd_in) {

  printf("pd function called with: _port=%d, _len_out=%d, _len_in=%d\n", _port, _len_out, _len_in);

  if (_port > 3) {
    fprintf(stderr, "Port out of range\n");
    return CMD_FAIL;
  }

  int tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command PD = 3
  uint8_t message[BUFFER_SIZE];
  message[0] = 3; // Command ID for PD
  message[1] = _port;
  message[2] = _len_out;
  message[3] = _len_in;
  memcpy( & message[4], _pd_out, _len_out);

  #ifdef VERBOSE
  printf("PD: Message sent as bytes (%d): ", (4 + _len_out));
  for (int i = 0; i < 4 + _len_out; i++) {
    printf("%02X ", message[i]);
  }
  printf("\n");
  #endif

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, 4 + _len_out, 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    perror("Receive error");
  } else {
    #ifdef VERBOSE
    printf("PD: Message received as bytes (%d): ", bytes_received);
    for (int i = 0; i < bytes_received; i++) {
      printf("%02X ", buffer[i]);
    }
    printf("\n");
    #endif

    memcpy(_pd_in, & buffer[4], _len_in);
  }

  close(sock);
  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to read parameters from a specified port
int readParameter(uint8_t _port, uint16_t _index, uint8_t _subindex, uint8_t _length, uint8_t * _read_data) {

  printf("readParameter called with: _port=%d, _index=%d, _subindex=%d, _length=%d\n", _port, _index, _subindex, _length);

  if (_port > 3) {
    fprintf(stderr, "READ: Port out of range\n");
    return CMD_FAIL;
  }

  uint16_t tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command READ = 4
  uint8_t message[6];
  message[0] = 4; // Command ID for read
  message[1] = _port;
  message[2] = (_index >> 8) & 0xFF;
  message[3] = _index & 0xFF;
  message[4] = _subindex;
  message[5] = _length;

  #ifdef VERBOSE
  printf("READ: Message send as bytes: ");
  for (int i = 0; i < 6; i++) {
    printf("%02X ", message[i]);
  }
  printf("\n");
  #endif

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, sizeof(message), 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    perror("Receive error");
    close(sock);
    return CMD_FAIL;
  } else {
    #ifdef VERBOSE
    printf("READ: Message received as bytes: ");
    for (int i = 0; i < bytes_received; i++) {
      printf("%02X ", buffer[i]);
    }
    printf("\n");
    #endif
  }

  memcpy(_read_data, & buffer[6], buffer[5]);

  close(sock);
  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to write parameters to a specified port
int writeParameter(uint8_t _port, uint16_t _index, uint8_t _subindex, uint8_t _length, uint8_t * _writeData) {
  printf("writeParameter called with: _port=%d, _index=%d, _subindex=%d, _length=%d\n", _port, _index, _subindex, _length);

  if (_port > 3) {
    fprintf(stderr, "WRITE: Port out of range\n");
    return CMD_FAIL;
  }

  uint16_t tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command WRITE = 5
  uint8_t message[BUFFER_SIZE];
  message[0] = 5; // Command ID for write
  message[1] = _port;
  message[2] = (_index >> 8) & 0xFF;
  message[3] = _index & 0xFF;
  message[4] = _subindex;
  message[5] = _length;
  memcpy( & message[6], _writeData, _length);

  #ifdef VERBOSE
  printf("WRITE: Message send as bytes: ");
  for (int i = 0; i < (6 + _length); i++) {
    printf("%02X ", message[i]);
  }
  printf("\n");
  #endif
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, (6 + _length), 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);

  #ifdef VERBOSE
  printf("WRITE: Message received as bytes: ");
  for (int i = 0; i < bytes_received; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\n");
  #endif

  if (bytes_received < 0) {
    perror("Receive error");
    close(sock);
    return CMD_FAIL;
  } else if (bytes_received == 2) {
    char errorMsgBuffer[255];
    sprintf(errorMsgBuffer, "WRITE: TCP message error: %s", get_error_message(buffer[1]));
    log_message(errorMsgBuffer);
    close(sock);
    return CMD_FAIL;
  } else if (bytes_received == 3) {
    char errorMsgBuffer[255];
    sprintf(errorMsgBuffer, "WRITE: IO Link error: 0x%02X%02X\n", buffer[2], buffer[3]);
    log_message(errorMsgBuffer);
    close(sock);
    return CMD_FAIL;
  }

  close(sock);
  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to set LED status for a specific port
int led(uint8_t _port, uint8_t _status) {
  if (_port > 3) {
    fprintf(stderr, "LED: Port out of range\n");
    return CMD_FAIL;
  }

  if (_status > 2) {
    fprintf(stderr, "LED: Status value out of range\n");
    return CMD_FAIL;
  }

  int tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command LED = 2
  unsigned char message[3] = {
    2,
    _port,
    _status
  };
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, sizeof(message), 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    perror("Receive error");
  } else if (bytes_received == 2) {
    // Handle error response
    fprintf(stderr, "LED: TCP command error (port=%d, status=%d): %s\n", _port, _status, get_error_message(buffer[1]));
  } else {
    // Success
    log_message("LED status set successfully");
  }

  close(sock);
  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to read the status of a specified port
int readStatus(uint8_t _port, iol_status * status_data) {
  printf("readStatus called with: _port=%d\n", _port);

  if (_port > 3) {
    fprintf(stderr, "STATUS: Port out of range");
    return CMD_FAIL;
  }

  uint16_t tcp_port = (_port < 2) ? TCP_PORT1 : TCP_PORT2;
  if (_port >= 2) {
    _port -= 2;
  }

  // Command STATUS = 6
  uint8_t message[2];
  message[0] = 6; // Command ID for status
  message[1] = _port;

  #ifdef VERBOSE
  printf("STATUS: Message send as bytes: ");
  for (int i = 0; i < 2; i++) {
    printf("%02X ", message[i]);
  }
  printf("\n");
  #endif

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation error");
    return CMD_FAIL;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(tcp_port);
  if (inet_pton(AF_INET, TCP_IP, & server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    close(sock);
    return CMD_FAIL;
  }

  if (connect(sock, (struct sockaddr * ) & server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    close(sock);
    return CMD_FAIL;
  }

  if (send(sock, message, sizeof(message), 0) < 0) {
    perror("Send error");
    close(sock);
    return CMD_FAIL;
  }

  uint8_t buffer[BUFFER_SIZE];
  int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
  if (bytes_received < 0) {
    perror("Receive error");
    close(sock);
    return CMD_FAIL;
  } else if (bytes_received != 15) {
    fprintf(stderr, "STATUS: Invalid response length\n");
    close(sock);
    return CMD_FAIL;
  }

  #ifdef VERBOSE
  printf("STATUS: Message received as bytes: ");
  for (int i = 0; i < bytes_received; i++) {
    printf("%02X ", buffer[i]);
  }
  printf("\n");
  #endif

  memcpy(status_data, & buffer[2], 13);

  close(sock);
  return CMD_SUCCESS;
}

// ***********************************************************************************************************
// Function to print the status of a specified port

// Function to print the iolStatus structure
void printStatus(const iol_status * status) {
  printf("IOL Status:");
  printf("pdInValid: %d\n", status -> pdInValid);
  printf("pdOutValid: %d\n", status -> pdOutValid);
  printf("transmissionRate: %d\n", status -> transmissionRate);
  printf("masterCycleTime: %d\n", status -> masterCycleTime);
  printf("pdInLength: %d\n", status -> pdInLength);
  printf("pdOutLength: %d\n", status -> pdOutLength);
  printf("vendorId: %d\n", status -> vendorId);
  printf("deviceId: %u\n", status -> deviceId);
  printf("power: %d\n", status -> power);
}
