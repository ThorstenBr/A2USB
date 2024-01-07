# A2USB: Apple II USB Interface & Apple II Mouse Interface Card
![A2USB](Photos/A2USB_MouseAndMore.jpg)

This projects provides alternate firmware for A2VGA cards to support a USB interface instead of VGA output. It currently supports a USB mouse and fully emulates an *Apple II Mouse Interface Card*.

A normal USB mouse can be used with the Apple II. The emulated *Mouse Interface Card* works well with applications like...

* ... **MousePaint**:

    ![MousePaint](Photos/A2USB_MousePaint.jpg)

* ... or **A2Desktop**:

    ![A2Desktop](Photos/A2USB_A2Desktop.jpg)

# Hardware
## Required basics

* You need a simple **MicroUSB to USB-A adpater**, so you can connect a standard USB mouse to the PICO's USB connector.

    * Either use an adapter cable:

        ![Adapter Cable](Photos/A2USB_AdapterCable2.jpg)

    * Or a simple plug adapter:

        ![Plug Adapter](Photos/A2USB_AdapterPlug.jpg)

* And you need a **standard USB mouse**. Anything conforming to the common "USB HID" standard should work.
* You can also use a **wireless mouse**. Their tiny transceiver plugs also implement a standard USB HID interface.

## Supported PCBs
The following boards can be used to run the A2USB firmware. Some of them require modification:

* [A2VGA PCBs](https://github.com/rallepalaveev/analog) by Ralle Palaveev, using DIP ICs. PCB Rev 1.6 and newer already have the options required for A2USB (see jumper options).
  * His v1.5 PCBs (or older) can also be used, however, they need to be modified to support A2USB. See [modifications](HwModding.md).

     ![A2VGA](Photos/A2VGA_Palaveev.jpg)

* David Kuder's [A2analog PCBs](https://github.com/V2RetroComputing/analog) can also be used, but currently also require the [modifications](HwModding.md).

     ![A2analog](Photos/V2analog.jpg)

* More supported hardware platforms are coming! Watch this spot... :)

# Installation
* Download the latest A2USB firmware ZIP file from the [Releases](/Releases) section.
   * ZIP file contains separate firmware for NTSC/PAL regions. Difference is only the default mouse interrupt frequency (PAL 50Hz / NTSC 60Hz).
   * According to Apple II Technical Notes, the original Mouse Interface Card was also shipped with variants for PAL / NTSC, with differing default interrupt rates.
* **Remove the PICO (or entire card) from the Apple II**.
* Connect the PICO's USB to your PC/MAC **while pressing the BOOTSEL button**.
* Drag & drop the A2USB firmware file **A2USB-MOUSE-...-4ns.uf2** from the Releases ZIP archive to your PICO.
* Wait a second.
* Disconnect and reinstall in your Apple II. Route the USB adapter cable through an opening in the back. Connect a USB mouse directly (sorry, no USB HUB support yet).
* **No change to the PAL/CPLD logic is required.**

# Acknowledgements
This is an alternate firmware project for A2VGA cards to support a USB interface instead of VGA output.
It is based on work of...

* ... Mark Aikens: [Apple II VGA project](https://github.com/markadev/AppleII-VGA/)
* ... David Kuder: [A2analog project](https://github.com/V2RetroComputing/analog)
* ... and Ralle Palaveev: [A2VGA project](https://github.com/rallepalaveev/analog)

Many thanks for the excellent work, which is a base for the A2USB project!
