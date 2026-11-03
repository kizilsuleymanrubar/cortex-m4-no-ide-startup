# cortex-m4-no-ide-startup
 
A bare-metal firmware project for the **STM32F411** (Arm Cortex-M4). It is built with the **Arm GNU Toolchain** only. No IDE is used.
 
Almost everything was written from scratch for this project:
 
- the Makefile
- the linker script
- the startup code (vector table and reset handler)
- all source and header files
Only two parts come from outside:
 
- **CMSIS**: the standard Arm/ST header files
- **`syscalls.c`**: system call stubs provided by ST
The goal is to learn what an IDE normally hides: building, linking, memory layout, start-up, flashing and debugging.
 
## What the Program Does
 
- Blinks the LED on pin PA5 once per second.
- Reads the chip's internal temperature sensor once per second.
- Prints the result over UART (USART2, 115200 baud) with `printf`.
## Features
 
- **Startup code** (`src/stm32_startup.c`): the full interrupt vector table and a `Reset_Handler`. The reset handler copies `.data` to RAM, clears `.bss`, sets up the C library and calls `main()`.
- **Linker script** (`stm32_ls.ld`) for the STM32F411.
- **Makefile** with two build modes: `nano` (default) and `semihosting`. Each mode has its own build folder.
- **Drivers written with registers only** (no HAL): UART, SysTick, LED and the temperature sensor.
- **Flashing and debugging** with OpenOCD and `arm-none-eabi-gdb`.
## Repository Layout
 
```
.
├── CMSIS/              # CMSIS header files (external)
├── inc/
│   ├── led.h
│   ├── sensor.h
│   ├── systick.h
│   └── uart.h
├── src/
│   ├── main.c
│   ├── led.c
│   ├── sensor.c        # temperature sensor (ADC1)
│   ├── systick.c
│   ├── uart.c          # USART2 driver and printf/scanf support
│   └── stm32_startup.c # vector table and Reset_Handler
├── Makefile
├── stm32_ls.ld         # linker script
├── syscalls.c          # newlib system calls (external, from ST)
├── gdb_client          # GDB commands to flash and run the board
└── LICENSE             # GPL-3.0
```
 
## How It Works
 
The chip runs from its internal 16 MHz oscillator (HSI). The clock is not changed.
 
- **`main()`** sets up the LED, SysTick and UART. Then it runs a loop forever. The loop calls `temp_sensor_task()` and `led_task()`.
- **Both tasks are small state machines.** They never wait in a long loop, so one task does not block the other.
- **SysTick** fires once per second. Its interrupt handler only sets two flags (`temp_sensor_isr()` and `led_isr()`). The real work is done in the main loop.
- **UART**: `printf` output goes to USART2. The function `__io_putchar` turns each `\n` into `\r\n`, so your `printf` strings only need `\n`.
- **Semihosting build**: the UART driver is left out. The output is sent to the host computer through OpenOCD instead. In this mode `Reset_Handler` does not call `__libc_init_array()`.
### Memory Layout
 
| Region | Start address | Size |
|--------|---------------|------|
| FLASH  | `0x08000000`  | 512 KB |
| SRAM   | `0x20000000`  | 128 KB |
 
- The stack starts at the top of SRAM (`_estack`) and grows down.
- **FLASH** holds the vector table (placed first), the code (`.text`), constants (`.rodata`) and the init/fini arrays.
- **SRAM** holds `.data` and `.bss`. The initial values of `.data` are stored in FLASH. `Reset_Handler` copies them to SRAM at start-up.
- The symbol `end` marks the end of `.bss`. A heap can start there.
## Requirements
 
- Arm GNU Toolchain: `arm-none-eabi-gcc` (with newlib-nano and `rdimon` support), `arm-none-eabi-gdb` and `arm-none-eabi-objdump`
- `make`
- OpenOCD
- A NUCLEO-F4 board with an STM32F411 (OpenOCD config: `board/st_nucleo_f4.cfg`)
- A serial terminal such as `picocom`. On the board, the ST-LINK USB connection also provides the serial port for USART2.
## Build
 
All output goes to the `build/` folder.
 
| Command | What it does | Output |
|---------|--------------|--------|
| `make` | Builds with newlib-nano (default) | `build/final_nano.elf` |
| `make semihosting` | Builds with semihosting (adds `-DSEMIHOSTING`) | `build/final_sh.elf` |
| `make load` | Starts OpenOCD inside `build/` | - |
| `make info` | Shows the section sizes of the ELF files | - |
| `make clean` | Deletes the `build/` folder | - |
 
Object files and the linker map file go to `build/nano/` or `build/semihosting/`.
 
Main compiler and linker options:
 
- `-mcpu=cortex-m4 -mthumb -mfloat-abi=soft`
- `-std=gnu11 -Wall -O0`
- `--specs=nano.specs` (default mode) or `--specs=rdimon.specs` (semihosting)
- `-Wl,-u,_printf_float` in the default mode, so `printf` can print `%f` values
## Flash and Debug with GDB
 
OpenOCD is the GDB server. `arm-none-eabi-gdb` is the client. You need two terminals.
 
**Terminal 1: start OpenOCD**
 
```bash
make                # or: make semihosting
make load           # OpenOCD listens on localhost:3333
```
 
**Terminal 2: start GDB**
 
OpenOCD runs inside `build/`. So file names in `monitor flash write_image` are relative to that folder.
 
```bash
cd build
arm-none-eabi-gdb final_nano.elf     # for the semihosting build: final_sh.elf
```
 
Then type these commands in GDB. They are also listed in the `gdb_client` file.
 
```
target remote localhost:3333
monitor reset init
monitor flash write_image erase final_nano.elf
monitor arm semihosting enable       # only for the semihosting build
monitor reset
monitor halt
monitor resume
```
 
To finish:
 
```
monitor shutdown
quit
```
 
| Command | What it does |
|---------|--------------|
| `target remote localhost:3333` | Connects GDB to OpenOCD |
| `monitor reset init` | Resets the chip and runs the board setup |
| `monitor flash write_image erase <elf>` | Erases the flash and writes the program |
| `monitor arm semihosting enable` | Turns on semihosting (semihosting build only) |
| `monitor reset`, `monitor halt`, `monitor resume` | Reset, stop and continue the program |
| `monitor shutdown` | Closes OpenOCD |
 
> The `gdb_client` file uses the name `final.elf`. Change it to `final_nano.elf` or `final_sh.elf`, depending on your build.
 
To see the UART output (the port name can be different on your computer):
 
```bash
picocom -b 115200 /dev/ttyACM0
```
 
## License
 
This project uses the [GPL-3.0](LICENSE) license. CMSIS and `syscalls.c` have their own licenses.
