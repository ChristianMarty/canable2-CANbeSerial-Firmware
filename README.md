# CANable 2.0 CANbeSerial Firmware

This is an alternative firmware for the CANable 2.0 device. It replaces the slcan protocol with the CANbeSerial protocol.

Key benefits of this implementation are:
* Propper framing algorithm
* Frames are CRC checked
* Data is not encoded as strings
* Support of all standard baud rates for CAN and CAN-FD



## Building

Firmware builds with GCC. Specifically, you will need gcc-arm-none-eabi, which
is packaged for Windows, OS X, and Linux on
[Launchpad](https://launchpad.net/gcc-arm-embedded/+download). Download for your
system and add the `bin` folder to your PATH.

Your Linux distribution may also have a prebuilt package for `arm-none-eabi-gcc`, check your distro's repositories to see if a build exists. Simply compile by running `make`. 

## Flashing with the Bootloader

Plug in your CANable2 while pressing down the BOOT button. The blue LED should be dimly illuminated. Next, type `make flash` and your CANable will be updated to the latest firwmare. Unplug/replug the device after moving the boot jumper back, and your CANable will be up and running.


## License

See LICENSE.md
