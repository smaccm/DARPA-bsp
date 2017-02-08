# TK1-SOM Daughterboard HOWTO #

## Prerequisites ##

### Hardware ###
- TK1-SOM and power plug
- daughterboard
- appropriate cables and connectors
- tools: screwdriver, needle-nose pliers, soldering iron (optional)
- (optional) IRIS+: including Pixhawk, battery, tools to disassemble IRIS+

## Assembling TK1-SOM and Daughterboard ##

The instructions for putting together the TK1-SOM and daughterboard are on
[the wiki](https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board)

The key steps are:

1. disassembling the TK1-SOM [disassemble\_tk1][1]
2. soldering missing connectors on TK1-SOM [Tk1SomMods][2]
3. mounting the TK1-SOM on the daughterboard: [attach\_daughterboard][3]
4. connecting serial [jumper\_uart][4] and reset cables [jumper\_reset][5]
5. connecting power cables [connect\_power][6]
6. connection debug output to host [debug\_ground][7]
7. (optional) connecting pixhawk: 
	* CAN [attach\_can][8]
	* power [attach\_pix\_pwr][9]
8. (optional) connecting 3DR radio [attach\_telem][10]
9. (optional) connecting battery [connect\_battery\_psu][11]

[1]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#disassemble_tk1
[2]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Tk1SomMods
[3]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#attach_daughterboard
[4]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#jumper_uart
[5]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#jumper_reset
[6]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#connect_power
[7]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#debug_ground
[8]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#attach_can
[9]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#attach_pix_pwr
[10]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#attach_telem
[11]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM/Daughter-Board#connect_battery_psu


## Flashing TK1-SOM U-Boot ##

Use the upstream U-Boot to enable Hyp mode and dfu booting capabilities.

[Instructions][12] for flashing U-Boot on TK1-SOM.
[12]: https://wiki.sel4.systems/Hardware/CEI_TK1_SOM#uboot


Note that upstream U-Boot does not support the USB 3 controller on the TK1-SOM, so only one USB port is available to boot from.

The [CEI U-Boot][13] does support the USB 3 port, but does not enable Hyp mode nor have have dfu or fastboot support.  See our [internal wiki page][14] for more info and CEI FTP access details.
[13]: ftp://cei_ftp%41coloradoengineering.com@coloradoengineering.com
[14]: https://confluence.csiro.au/pages/viewpage.action?pageId=348787875

## Building seL4 ##

Build a TK1-SOM compatible system in the usual way.  For example: the `vm` app from [https://github.com/smaccm/darpa-bsp](https://github.com/smaccm/darpa-bsp).

## Loading and Running seL4 ##

There are various ways to load and run an image on the TK1-SOM.

All require a host computer to be connected to the debug UART and to run a terminal program on it to access the U-Boot prompt in order to enter and execute U-Boot commands.

In the following we will assume that the image we are loading is called `system.img`.

### Prerequisite: Serial console access ###

Connect an appropriate serial cable to UART1 on the TK1-SOM (if you have a 1.8V USB-Serial converter use the ports on the TK1-SOM, if you have a 3.3V then use the ports on the daughterboard).

Start a serial console program (e.g. `screen`, `minicom`, etc.) listening to the serial port. The appropriate settings are: 115200 bits/s, 8N1, no hardware flow control.

To access the U-Boot console, boot the TK1-SOM then press any key to stop the boot process.  You should see a prompt, something like:

```
Tegra124 (TK1-SOM) # 
```

### Option 1: Using tftpboot ###

1. set up a tftp server on the host
2. connect TK1-SOM to network. If you are connecting the TK1-SOM directly to the host, then manually set the host address to `192.168.0.1`.  We will manually give the TK1-SOM the address `192.168.0.2`
3. in U-Boot do:
```
pci enum
setenv ipaddr 192.168.0.2
tftpboot ${loadaddr} 192.168.0.1:system.img
bootelf ${loadaddr}
```

Optionally, you can also set up a DHCP server and automatically provide the TK1-SOM an address.  In this case the U-Boot steps become:
```
pci enum
dhcp ${loadaddr} 192.168.0.1:system.img
bootelf ${loadaddr}
```

#### Starting a tftp server on a Linux ####

...

#### Starting a tftp server on a Mac ####

```
sudo launchctl load -F /System/Library/LaunchDaemons/tftp.plist
sudo launchctl start com.apple.tftpd
```

The default tftp file path is `/private/tftpboot`

Stop the server with:
```
sudo launchctl unload -F /System/Library/LaunchDaemons/tftp.plist
```

### Option 2: Using dfu ###

See the [full instructions](https://wiki.sel4.systems/Hardware/CEI_TK1_SOM#dfu)

The summary is:

Configuration steps (do only once):

1. Configure Linux by installing dfu-utils:
```
sudo apt-get install dfu-utils
```
2. Configure U-Boot to do dfu boot:
```
setenv dfu_alt_info "kernel ram $loadaddr 0x1000000"
saveenv
```

Load and boot seL4 image:

1. attach a USB cable between the TK1-SOM's OTG port and your host
2. in U-Boot
```
dfu 0 ram 0
bootelf ${loadaddr}
```
3. on host Linux
```
sudo dfu-util --device 0955:701a -a kernel -D system.img -R
```

### Option 3: Using USB storage ###

Copy image to USB storage:

1. plug a USB storage device into host
2. copy image file (e.g. `system.img`) to USB storage
3. unmount and unplug

Load and boot seL4 image:

1. plug USB storage device into TK1-SOM USB2 port
2. in U-Boot:
```
usb start
fatload usb 0:1 ${loadaddr} system.img
bootelf ${loadaddr}
```

Note, if the USB storage is not formatted with FAT, but, e.g., EXT2 then use an appropriate load command, e.g.:
```
ext2load usb 0:1 ${loadaddr} system.img
```


### Option 4: Using MMC storage ###

See the full instructions on https://wiki.sel4.systems/Hardware/CEI_TK1_SOM#mmc

The summary is:

Copy image to MMC storage:

1. attach a USB cable between the TK1-SOM's OTG port and your host
2. in U-Boot do:
```
ums mmc 0
```
A spinning wheel will appear on the console, and the entire MMC will be presented as a USB storage device to your host. 
3. mount MMS on host if necessary
4. copy image file (e.g. `system.img`) to USB storage
5. unmount and reboot TK1-SOM

Load and boot seL4 image:

1. in U-Boot do:
```
mmc init
mmcinfo
fatload mmc 0 ${loadaddr} system.img
bootelf ${loadaddr}
```

Note, if the MMC storage is not formatted with FAT, but, e.g., EXT2 then use an appropriate load command, e.g.:
```
ext2load mmc 0 ${loadaddr} system.img
```

### Setting U-Boot environment variables for convenience and quicker boot ###

Instead of retyping the U-Boot commands everytime you boot, you can save them as envrionment variables and run them witha  single (short) command, or even automatically on boot.

For example, to save the tftpboot command, do:

```
setenv bootsel4 "tftpboot ${loadaddr} 192.168.0.1:system.img; bootelf ${loadaddr}"
saveenv
```

Then you can run it with:
```
run bootsel4
```

Or set it to be the default boot command:
```
setenv bootcmd "run bootsel4"
saveenv
```

