# Ardunio 6502 emulator

This emulator emulates the **w65c02** (CMOS 6502) and some of **w65c22** (VIA for CMOS 6502)

Every functionality based on the documents ([w65c02](https://eater.net/datasheets/w65c02s.pdf), [w65c22](https://eater.net/datasheets/w65c22.pdf)) and [ben eater's videos](https://www.youtube.com/watch?v=LnzuMJLZRdU&list=PLowKtXNTBypFbtuVMUVXNR0z1mu7dp7eH&index=1)

vasm make command: `make CPU=6502 SYNTAX=oldstyle`

Makefile ~~stolen~~ borrowed from [Marlin 3D Printer Firmware](https://github.com/MarlinFirmware/Marlin)'s [makefile](https://github.com/MarlinFirmware/Marlin/blob/2.1.x/Marlin/Makefile)