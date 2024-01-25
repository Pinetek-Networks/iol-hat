# IOL HAT
Repository for IOL HAT

The Master Application is based on the i-link stack from RT-Labs: https://github.com/rtlabs-com/i-link
For the i-link stack, the GPLv3 license is applied (dual license is available here).  
The Master Application uses the library based on the i-link Stack from RT-Labs. The fork of the stack can be found here: https://github.com/Pinetek-Networks/i-link
The Master Application is also based on the i-link stack. However, it does not use the CMake build.

# IOL HAT usage and examples
Use the IOL HAT can be used with the binaries provided in the /bin/ folder. The binaries have been compiled with the aarch-linux toolchain for 64-bit version, and the arm-linux-gnueabihf for the 32-bit version.
There are two binaries per target architecture available, one for port 1+2 and one for port 3+4. Please refer to the IOL HAT manual on how to set the ports.

To use the communication stacks, run the binary and communicate via the TCP protocol with throught the binary with the SDCI devices. Further information can be found in the IOL HAT datasheet on the webpage: [pinetek-networks.com/en/iol-hat](https://pinetek-networks.com/en/iol-hat/)

# Build the library for Linux
To build the library follow these steps:
1. Install cmake (if not already installed): `sudo apt install cmake`
2. Clone the i-link fork in a local repository `git clone https://github.com/Pinetek-Networks/i-link`
3. Initialize the submodules: `git submodule update --init --recursive`  
4. Modify the crosscompile-rpi.cmake file; change the staging directory and provide the locations of the build tools
5. Generate Makefile:\
  `mkdir build`\
  `cmake -B build -DCMAKE_TOOLCHAIN_FILE=crosscompile-rpi.cmake`
6. Build the application:\
  `cd build`\
  `make`\
  `make install`

The header files and library are then available in the staging directory as privided in crosscompile-rpi.cmake. Include them in your application to use the stack.
