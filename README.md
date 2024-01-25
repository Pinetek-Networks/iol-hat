# IOL HAT
Repository for IOL HAT

The Master Application is based on the i-link Stack from RT-Labs: https://github.com/rtlabs-com/i-link
The Master Application uses the library based on the i-link Stack from RT-Labs. The fork of the stack can be found here: https://github.com/Pinetek-Networks/i-link

# IOL HAT usage and examples
Use the IOL HAT can be used with the binaries provided in the /bin/ folder. The libraries have been copiled with the aarch-linux toolchain for 64-bit version, and the arm-linux-gnueabihf For the 32-bit version.
There are two binaries per target architecture available, one for port 1+2 and one for port 3+4. Please refer to the IOL HAT manual on how to set the ports.

To use the communication stacks, run the binary and communicate via the TCP protocol with throught the binary with the SDCI devices.

# Build the library for Linux
To build the library follow these steps:
1. Clone the i-link fork in a local repository https://github.com/Pinetek-Networks/i-link
2.  
