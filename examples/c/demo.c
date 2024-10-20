// demo.c
//
// This file is part of the iol-hat distribution
// (https://github.com/Pinetek-Networks/iol-hat/). Copyright (c) 2024 Pinetek
// Networks.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

// This example assumes the following connections
// Port 0:
// IFM HMI E30391: https://www.ifm.com/de/en/product/E30391
// 1 octets PD in input
// 14 octets PD out output
// PD layout see here:
// https://www.ifm.com/download/files/ifm-E30391_AB-20160613-IODD11-en-fix/$file/ifm-E30391_AB-20160613-IODD11-en-fix.pdf
//
// Port 1:
// IFM 6124 https://www.ifm.com/de/de/product/IF6124
// 2 octets PD out output
// 0 octeds PD in input
// PD layout see here:
// https://www.ifm.com/download/files/ifm-IF6123-20170707-IODD11-en/$file/ifm-IF6123-20170707-IODD11-en.pdf

#include "iol-hat.h"

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

// Shut down ports and turn off LEDs
void
close_ports() {
  printf("Closing ports and setting LEDs off...\n");
  power(0, 0);
  led(0, 0);
  power(1, 0);
  led(1, 0);
  exit(0);
}

int
main() {
  printf("Demo program for IOL HAT\n");
  printf("This program will demonstrate basic operations using the IOL HAT\n");

  // Set POWER ON and turn on LEDs for ports 0 and 1
  printf("Setting power and LEDs for ports...\n");
  if (power(0, 1) == CMD_SUCCESS) {
    led(0, 1); // LED_GREEN
  } else {
    led(0, 2); // LED_RED
  }

  if (power(1, 1) == CMD_SUCCESS) {
    led(1, 1); // LED_GREEN
  } else {
    led(1, 2); // LED_RED
  }

  // Give time for power to stabilize
  sleep(1);

  iol_status status;

  readStatus(0, & status);
  printStatus( & status);

  // Read the application-specific tag (index 24, subindex 0, length 32 bytes)
  printf("Reading parameters from port 1, index 24\n");
  uint8_t read_data[32];
  if (readParameter(1, 24, 0, 32, read_data) == CMD_SUCCESS) {
    printf("Read data: ");
    for (int i = 0; i < 32; i++) {
      printf("%02X ", read_data[i]);
    }
    printf("\n");
    printf("Read data string=%s\n", read_data);
  } else {
    printf("Failed to read data from port 1\n");
    close_ports();
  }

  // Write the application-specific tag (index 24, subindex 0, length 16 bytes)
  // The first write might fail due to the state of the IOL stack and/or the
  // device, therefore repeat twice
  printf("Writing data to port 1, index 24\n");
  uint8_t write_data[16] = "PinetekNetworks";
  printf("Write data string=%s\n", write_data);
  if (writeParameter(1, 24, 0, sizeof(write_data), write_data) != CMD_SUCCESS) {
    printf("Failed to write data to port 1\n");
    close_ports();
  }

  // Start loop to acquire input data and send output data
  // This loop demonstrates how to read from and write to the connected devices
  printf("Running main loop. Press 'Ctrl+C' to exit.\n");
  int i = 0;
  while (1) {

    uint16_t pos_value; // Pos value from sensor
    uint8_t button_value; // Pos value from sensor

    // PORT1: Read Distance sensor data (2 bytes in, 0 bytes out)

    // PD for port 1
    uint8_t pd_data_in1[64];
    uint8_t pd_data_out1[64];

    if (pd(1, 0, 2, pd_data_out1, pd_data_in1) == CMD_SUCCESS) {
      pos_value
        = ((uint16_t) pd_data_in1[0] << 8) | (uint16_t) pd_data_in1[1];
      pos_value >>= 2; // Adjust value
      printf("Position value: %d\n", pos_value);
    } else {
      printf("Failed to read process data from port 1\n");
      close_ports();
    }

    // PORT0: Prepare output message to HMI
    // Compile output data to be sent to HMI (14 bytes out)
    uint8_t pd_data_in0[64];
    uint8_t pd_data_out0[64] = {
      0
    };

    pd_data_out0[0] = (pos_value >> 8) & 0xFF; // Pos value high byte
    pd_data_out0[1] = pos_value & 0xFF; // Pos value  low byte

    pd_data_out0[2] = (i >> 8) & 0xFF; // Counter high byte
    pd_data_out0[3] = i & 0xFF; // Counter low byte

    pd_data_out0[13] = 2; // 2 line display

    // PORT0: Read buttons and output values to HMI (1 byte in, 14 byte out)
    if (pd(0, 14, 1, pd_data_out0, pd_data_in0) == CMD_SUCCESS) {
      button_value = pd_data_in0[0] & 0x0F;
      printf("Button value: %d\n", button_value);
    } else {
      printf("Failed to send process data to port 0\n");
      close_ports();
    }

    // Increment counter
    i++;
    sleep(1); // Some time needed for keyboard interrupt
  }

  return 0;
}
