#
# J-LINK GDB SERVER initialization
#
# This connects to a GDB Server listening
# for commands on localhost at tcp port 2331
target remote localhost:2331
# Set JTAG speed to 30 kHz
monitor flash device = STM32F103RC
monitor endian little
# Reset the chip to get to a known state.
monitor reset
monitor reg r13 = (0x20000000)
monitor reg pc = (0x20000004)
#
# CPU core initialization (to be done by user)
#
# Set the processor mode
#monitor reg cpsr = 0xd3
# Set auto JTAG speed
monitor speed auto
# Setup GDB FOR FASTER DOWNLOADS
set remote memory-write-packet-size 1024
set remote memory-write-packet-size fixed
# Load the program executable called "image.elf"
load rtthread-stm32.axf
