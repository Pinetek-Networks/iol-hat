# Master application
This Master Application is based on the i-link stack from RT-Labs: https://github.com/rtlabs-com/i-link
The branches "main" and "public" are merged. 
For the i-link stack (main and public branch), the GPLv3 license is applied (dual license option):
Main: https://github.com/rtlabs-com/i-link/blob/main/LICENSE.md (a4ec0653b1af2ca248e3fcff01dcc7c603671c7a)
Public: https://github.com/rtlabs-com/i-link/blob/public/LICENSE.md (4faf4d8ff54a60227c94fcb4c2e03b078d242ac9)

The fork of the stack can be found here: 
https://github.com/Pinetek-Networks/i-link
https://github.com/Pinetek-Networks/i-link-public

# Info
- The Master Application does not use the original i-link CMake build
- The sources have been merged and extended.
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
```
To build the Release:
```
make
```
<<<<<<< HEAD

To build the Debug:
```
make debug
```

If you switch between the Release and Debug, you need to clean the build before:
```
make clean
```

After build, you can use the **iol-master-appl** binary in the "bin/release" or in "bin/debug" folder, depending on the selected build. The application can now be started with the options as described on the main page.
=======
After build, you can use the **iol-master-appl** binary with the options as described on the main page.
>>>>>>> 1c4e3e19af3e9e90593fa9cd9b9903a299ad4871
