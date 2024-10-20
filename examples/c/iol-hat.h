#ifndef IOL_HAT_H
#define IOL_HAT_H

#include <stdint.h>

// Command success and failure return values
#define CMD_SUCCESS 1
#define CMD_FAIL - 1

// Function to set power on a specific port
// Parameters:
// - _port: The port number (0-3)
// - _status: The desired power status (0: OFF, 1: ON)
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise
int power(uint8_t _port, uint8_t _status);

// Function to handle process data communication for the specified port
// Parameters:
// - _port: The port number (0-3)
// - _len_out: The length of the output process data
// - _len_in: The expected length of the input process data
// - _pd_out: The output process data to be sent
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise
int pd(uint8_t _port, uint8_t _len_out, uint8_t _len_in, uint8_t * _pd_out, uint8_t * _pd_in);

// Function to read parameters from a specified port
// Parameters:
// - _port: The port number (0-3)
// - _index: The index of the parameter to read
// - _subindex: The subindex of the parameter to read
// - _length: The length of the data to read
// - return_data: Buffer to store the read data
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise
int readParameter(uint8_t _port, uint16_t _index, uint8_t _subindex, uint8_t _length, uint8_t * _read_data);

// Function to write parameters to a specified port
// Parameters:
// - _port: The port number (0-3)
// - _index: The index of the parameter to write
// - _subindex: The subindex of the parameter to write
// - _length: The length of the data to write
// - _writeData: The data to be written
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise
int writeParameter(uint8_t _port, uint16_t _index, uint8_t _subindex, uint8_t _length, uint8_t * _write_data);

// Function to read the status of a specified port
// Parameters:
// - _port: The port number (0-3)
// - status_data: Buffer to store the status data (13 bytes)
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise

typedef struct {

  uint8_t pdInValid;
  uint8_t pdOutValid;

  uint8_t transmissionRate;
  uint8_t masterCycleTime;

  uint8_t pdInLength;
  uint8_t pdOutLength;

  uint16_t vendorId;
  uint32_t deviceId;
  uint8_t power;
}
__attribute__((__packed__)) iol_status;

int readStatus(uint8_t _port, iol_status * _status);

// Function to set LED status for a specific port
// Parameters:
// - _port: The port number (0-3)
// - _status: The desired LED status (0: OFF, 1: GREEN, 2: RED)
// Returns:
// - CMD_SUCCESS if successful, CMD_FAIL otherwise
int led(uint8_t _port, uint8_t _status);

// Function to print the iol_status structure
// Parameters:
// - status: The iolStatus structure to be printed
void printStatus(const iol_status * status);

#endif // IOL_HAT_H
