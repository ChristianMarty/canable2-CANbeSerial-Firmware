# CANable 2.0 CANbeSerial Firmware

This is an alternative firmware for the CANable 2.0 device. It replaces the slcan protocol with the CANbeSerial protocol.

For more information about the CANbeSerial protocol go the [CANbeSerial GitHub Page](https://github.com/ChristianMarty/CANbeSerial) or the [CANbeSerial page on my website](https://www.christian-marty.ch/electricthings/canbeserial.html) .

Key benefits of this implementation are:
* Proper framing algorithm
* Frames are CRC checked
* Data is not encoded as strings (may not be a benefit for your application)
* Support of all standard baud rates for CAN and CAN-FD (the reason why I made this firmware)
* Overall better error handling

## Hardware
This firmware was developed and tested using the Makerbase CANable V2.0 PRO S.
### Hardware Compatibility,
Regarding the hardware compatibility, the only observed difference is that the green LED is connected to a different pin than in the original CANable firmware. 
Apart from this, no hardware configuration changes have been made.

## Known Limitations
* The cbs_transmitAcknowledgment is returned when the CAN frame is place into the CAN tx-buffer not when the frame is transmitted on the BUS.

## Building -> from the original CANable project

Firmware builds with GCC. Specifically, you will need gcc-arm-none-eabi, which
is packaged for Windows, OS X, and Linux on
[Launchpad](https://launchpad.net/gcc-arm-embedded/+download). Download for your
system and add the `bin` folder to your PATH.

Your Linux distribution may also have a prebuilt package for `arm-none-eabi-gcc`, check your distro's repositories to see if a build exists. Simply compile by running `make`. 

## Flashing with the Bootloader

Plug in your CANable2 while pressing down the BOOT button. The blue LED should be dimly illuminated. Next, type `make flash` and your CANable will be updated to the latest firwmare. Unplug/replug the device after moving the boot jumper back, and your CANable will be up and running.

## License

See LICENSE.md
