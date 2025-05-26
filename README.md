# IOL HAT
Software repository for IOL HAT, an Raspberry Pi extension for IO-Link communication: https://www.pinetek-networks.com/iol-hat

![IOL HAT RPi (600 x 400 px)](https://github.com/user-attachments/assets/e64add7d-45a9-483a-b5c1-e328c57330e8)

# IOL HAT Overview
The IOL HAT provides IO-Link (SDCI) connectivity for Raspberry Pi and other single board computers. The application consists of a software part ("Master Application", iol-master-appl) and a hardware based on the MAX14819 (IOL HAT extension module). 
The communication between Master Application and IOL HAT is over SPI and GPIO lines. It is possible to connect two IOL HATs to one Raspberry Pi (one SPI line with 2 chip selects). 
The communication between the user application and the Master Application is established over a TCP connection. Examples are provided for both C and Python for this connection.

![image](https://github.com/Pinetek-Networks/iol-hat/assets/116767503/4d07e1c6-1d9f-4f4e-bbbb-611436dbf62c)

# IOL HAT Master Application 

The Master Application is based on the i-link stack from RT-Labs (dual license with the  GPLv3 license applied for this build): https://github.com/rtlabs-com/i-link
Each instance of the Master Application can serve **two** IOL ports. If you want to operate 4 ports, you need to start 2 instances of the Master Application using the correct parameters (see below).

# IOL HAT software and settings
The IOL HAT can be used with the binary iol-master-appl that is provided in the /bin/ folder. This binary has been compiled with the aarch-linux toolchain for 64-bit version. For other versions, please follow the instructions to build: https://github.com/Pinetek-Networks/iol-hat/blob/main/src-master-application/README.md

**Enable SPI communication**
The communication between Master Application iol-master-appl and the IOL HAT is done over SPI, so SPI communication needs to be enabled on the host.
To enable SPI for Raspberry Pi hosts, the following instructions can be used:
```
  -sudo raspi-config
  -go to: Interface Options
  -go to: SPI
  -select: Enable SPI
```
If you use another target, please verify with the documentation how to enable the SPI

**GPIO**
The interrupt line on between host and IOL HAT is a GPIO line. To operate, GPIOD is used as library.
To install GPIOD, please use the follwing command
```
sudo apt install gpiod	
```
# IOL HAT usage and examples
To communicate with the IO-Link devices, start the Master Application (iol-master-appl) and then your application. Using the examples is a good starting point to develop your application.
Further information especially on the TCP communication can be found here:
* IOL HAT webpage: https://pinetek-networks.com/en/iol-hat (IOL HAT product page)
* IOL HAT manuals (software and hardware): https://download.pinetek-networks.com/iol-hat/doc (IOL HAT manual download)
* IOL HAT Knowledge Base: https://www.pinetek-networks.com/knowledge-base/iol-hat (IOL HAT online manual)

**Important note:**
To work properly, the master application needs to be started **AFTER** 24V power is applied. Failure to do so will result in broken IO-Link communication.

The configuration of the Master Application (iol-hat-appl) is done over command line arguments:
```
  -h, --help      shows help message and exits 
  -v, --version   prints version information and exits 
  -e, --extclock  Use clock for MAX14819 from ext source (not used for IOL HAT)
  -r, --realtime  Run realtime on given kernel (requires root rights, see manual) 
  -i, --iolport   Specify the IOL port (12/34) (based on jumper settings on the hardware, see manual), usage e.g., -i 34 for IOL port 3-4.
  -t, --tcpport   Specify the TCP port, usage -t 1234 for port 1234. Standard ports are 12010 for IOL port 1-2, and 12011 for IOL port 3-4
```
**Timing considerations:**
IO-Link is a real-time protocol that requires fast response and cycle times to not run into timeouts. For running with sensors etc. that are queried 1-2 times per second, those timeouts are not crititcal. The iol-hat Master Application can run as user without further considerations.
If timeouts are critical (e.g., when HMI devices are connected that would change screen when timeouts occur), it is recommended to run the iol-hat in realtime mode. The solution is based on this description: https://forums.raspberrypi.com/viewtopic.php?t=228727
As preparation, one core needs to be reserved. This is done by adding the this argument to /boot/firmware/cmdline.txt (for older versions of Raspberry OS, the file is /boot/cmdline.txt):
```
isolcpus=3
```
where 3 is the core you want to reserve (can be 0..3). The core needs to be matching with the option that you are giving for iol-hat, e.g., for core 3 it would be 
```
 iol-hat -r 3
```


# Build the Master Application
If you need to build the Master Application for a different host, please folloe the instructons: [https://github.com/Pinetek-Networks/iol-hat/blob/main/src-master-application/README.md](https://github.com/Pinetek-Networks/iol-hat/blob/main/src-master-application/README.md)
