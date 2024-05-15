# IOL HAT
Repository for IOL HAT

![image](https://github.com/Pinetek-Networks/iol-hat/assets/116767503/9132dac7-b8c2-4d44-91c0-e09cfc75237d)

# IOL HAT Overview
The IOL HAT provides IO-Link (SDCI) connectivity for Raspberry Pi and other single board computers. The IOL HAT consists of a software part ("Master Application") and a hardware based on the MAX 14819 as provided per schematics in this repository. 
It is possible to connect two IOL HAT to one Raspberry Pi (one SPI instance). The communication between the user application and the Master Application is established over a TCP connection. Examples are provided for both C and Python for this connection.

![image](https://github.com/Pinetek-Networks/iol-hat/assets/116767503/4d07e1c6-1d9f-4f4e-bbbb-611436dbf62c)

# IOL HAT Master Application 

The Master Application is based on the i-link stack from RT-Labs: https://github.com/rtlabs-com/i-link
For the i-link stack, the GPLv3 license is applied (dual license is available here).  
The original i-link setup builds a library and examples. The library build and corresponding files are included in the Master Application. The Master Application is based on the examples from the i-link stack. 
CMake is not used for the IOL HAT.

# IOL HAT usage and examples
Use the IOL HAT can be used with the binaries provided in the /bin/ folder. The binaries have been compiled with the aarch-linux toolchain for 64-bit version, and the arm-linux-gnueabihf for the 32-bit version.

The configuration of the Master Application (iol-hat binary) is done over command line arguments:
```
  -h, --help      shows help message and exits 
  -v, --version   prints version information and exits 
  -e, --extclock  Use clock for MAX14819 from ext source (not used for IOL HAT)
  -r, --realtime  Run realtime on given kernel (requires root rights, see manual) 
  -i, --iolport   Specify the IOL port (12/34) (based on jumper settings on the hardware, see schematics), usage e.g., -i 34 for IOL port 3-4.
  -t, --tcpport   Specify the TCP port, usage -t 1234 for port 1234. Standard ports are 12010 for IOL port 1-2, and 12011 for IOL port 3-4
```
**Timing considerations:**
IO-Link is a real-time protocol that requires fast response and cycle times to not run into timeouts. For running with sensors etc. that are queried 1-2 times per second, those timeouts are not crititcal. The iol-hat Master Application can run as user without further considerations.
If timeouts are critical (e.g., when HMI devices are connected that would change screen when timeouts occur), it is recommended to run the iol-hat in realtime mode. The solution is based on this description: https://forums.raspberrypi.com/viewtopic.php?t=228727
As preparation, one core needs to be reserved. This is done by adding the this argument to /boot/cmdline.txt:
```
isolcpus=3
```
where 3 is the core you want to reserve (can be 0..3). The core needs to be matching with the option that you are giving for iol-hat, e.g., for core 3 it would be 
```
 iol-hat -r 3
```

To use the communication stacks, run the binary and communicate via the TCP protocol with throught the binary with the SDCI devices. Further information can be found in the IOL HAT datasheet on the webpage: [pinetek-networks.com/en/iol-hat](https://pinetek-networks.com/en/iol-hat/)

# Build the Master Application
The master application can be build using the make command. If required, alter the makefiles for the target architecture etc.
