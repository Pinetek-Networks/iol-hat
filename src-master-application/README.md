# Master application
This Master Application is based on the i-link stack from RT-Labs: https://github.com/rtlabs-com/i-link
For the i-link stack, the GPLv3 license is applied (dual license option). The fork of the stack can be found here: https://github.com/Pinetek-Networks/i-link

# Info
- The Master Application does not use the original i-link CMake build
- Modifications in the source files are marked with comments
- Additional files (e.g., the TCP server) have been created. Those are not seperately marked

# Building the Master Application 
The following instructions describe how to build the Master Application on the target itself, e.g., on the Raspberry Pi

## Log in on the target
Usually, this is done over ssh

## Enable SPI communication
_The following commands apply for Raspberry Pi. If you use another target, please verify with the documentation how to enable the SPI_

```
sudo raspi-config
```
+ Interface Options
+ SPI
+ Enable SPI

## Install the target toolchain, git and GPIO drivers
```
sudo apt update
sudo apt install -y build-essential
sudo apt install git
sudo apt install gpiod	
```

## Clone the repository locally to the target
```
git clone https://github.com/Pinetek-Networks/iol-hat.git
```

## Go to the cloned folder and build the application
```
cd iol-hat/src-master-application
make
```
After build, you can use the **iol-master-appl** binary with the options as described on the main page.
