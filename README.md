# OSP Hello World
Open Sensor Platform (OSP) is open source software built to enable systems with sensor hubs. This is a branch of the open-sensor-platform on GitHub. The intent is to use the OSP framework for something other than what it was meant for since it is as good a framework as any to start building your fun projects!
OSP was released in 2014 by Sensor Platforms Inc. under the Apache 2.0 license. The current copyright owner of this project is [Knowles Electronics, LLC.](www.knowles.com)

### Trying it out

A reference "Hello World" example is provided for easily available hardware: 
  * An STM32F0 (Cortex-M0) based Nucleo-F091RC board that is readily available
  * More reference projects maybe added in the future. In the meantime you can also look at [STM32 Discovery-F4 MP3 Player](https://github.com/vermar/open-sensor-platform/tree/CMSIS-RTX/embedded/projects/Discovery_F4_MP3) which uses ASF for implementing a MP3 player on an STM32 Discovery-F4 board. The MP3 project is also a good reference for CMSIS-RTX and GCC toolchain.

The example can easily be ported to other Cortex-M processors and boards.

### Development Environment

The cross compiler toolchain and IDE used for the examples is the popular ARM Keil MDK that supports most Cortex-M and Cortex-R series microcontrollers. For the Cortex-M0 controllers manufactured by ST Microelectronics, Keil provides a special version of the MDK toolchain free of cost to the user – for both commercial and non-commercial use. This is a special arrangement between ST and ARM (owner of Keil). The toolchain and corresponding license can be found at the following weblink:

  * http://www2.keil.com/stmicroelectronics-stm32/mdk

*Note that when you first launch the Keil MDK program by double clicking on any Keil Project (e.g. Embedded\Projects\Nucleo_F091_Hello\Keil-MDK-Build\Nucleo_F091_Hello.uvprojx) the pack installer will be launched that will download and install the support for STM32F0xx devices. You need to have internet connection for the pack installer to work properly.*

The projects use the Keil RTX Real Time Operating system. Reference for CMSIS-RTX based implementation can be found in the MP3 project as mentioned above.

### Application Support Framework (ASF)

The Application Support Framework (ASF) is the basic starting point for application development on PH board. It stitches together the various system components of the Touch application to provide the final results for appliance control. It has been implemented keeping in mind the need for portability across hardware platforms, CPU architectures, and real-time operating systems.

The ASF provides APIs, macros, and utility functions for managing tasks, timers, and message passing in the application. These methods have been created to abstract the underlying kernel or RTOS and allow porting of the existing application firmware to different platforms with minimal effort. Documentation can be found at the following weblink:

  * https://github.com/sensorplatforms/open-sensor-platform/wiki/ASF

### “Hello World” Example

A simple Hello World example is provided in source form for STM32F091 Nucleo-F091RC board (and a Knowles proprietary board called Purple Haze or PH). While on the surface you may only see some flashing LEDs, the example highlights some useful features built into the application framework provided.

  * Console debug messages: The debug UART port of PH board provides high speed DMA based debug-printf messages on any Windows based terminal program (e.g. RealTerm, Tera Term, etc). This feature is highly useful during debug or just to provide informative messages for the developer for testing or understanding application flow. Due to its low-latency DMA based non-blocking implementation, the debug prints can even be used from ISRs during debugging. A simple text message (e.g. `D1_printf(“Hello World!\n”)`) would take only several microseconds to complete. Note that the default baud rate for UART on PH board is 921600 and on Nucleo board is 115200. Higher speeds are possible if supported by the terminal program and the cable used. Nucleo boards have built-in ST-Link debugger and the debugger’s USB interface also provides a UART bridge that is connected to the target processor’s UART. This makes it easier to get debug messages via the same USB interface instead of requiring an external UART cable (such as the FTDI cables). However the debugger based UART bridge is unable to support higher baud rates than 115200.

  * Console command interface: The `CmdHandler_T.c` file implements a serial command handler that can easily be customized as per application requirements. In PH Touch application, the command handler is used to interface with test-automation tool for sending and receiving run-time diagnostics and parameters to the application processor (STM32) and to the DSPs (via STM32). As command handler it is implemented as separate task, the command handling does not affect the execution of other more important tasks in the system.
  In the Hello World application you can try typing `log=10` in your terminal program console and the firmware will respond with “Received 6 bytes: log=10”. Thise simple command echo is implemented in `Console_Util.c` file in the function `CmdParse_User()`. More complex command parsing (e.g. using `sscanf`) can be implemented by customizing this function.

  * Task Statistics display: Probably the two most important question in any embedded application developed using a Real Time Operating System are “How much is my CPU usage for each task?” and “How much task stack is being consumed?”. The ASF provides this answer readily on demand by having built in CPU and Stack usage profiling. In the serial terminal console simple hit “Enter” key and you will be presented with a display similar to the one shown below:
```
------------------------------------------------------
InstrManagerTask: 0576/1024     56%     0.00%   16
     LED_On_Task: 0064/0256     25%     0.00%   138
    LED_Off_Task: 0064/0256     25%     0.00%   251
  CmdHandlerTask: 0112/1024     10%     0.00%   3
       IDLE TASK: 0064/0128     50%     99.99%  2503
    System Stack: 0264/0512     51%      -*-     -*-
------------------------------------------------------
```
  The various fields in the above profiling information are explained below:
  * 1st Column: Task Names that are automatically derived from the entry function names in the code
  * 2nd Column: Stack usage in the form <current-usage>/<max-stack-size> in bytes
  * 3rd Column: Stack usage in percentage
  * 4th Column: CPU Usage in percentage for each task
  * 5th Column: Number of times the task has run (scheduled)
  * Last Row: This is special and provides the system stack usage - i.e. stack used by `main()` and ISRs

### Additional Questions

Refer to the detailed [FAQ](https://github.com/sensorplatforms/open-sensor-platform/wiki) on the wiki section of this project.

### Project Structure
  * embedded - various project code 
  * external - Source code, library, documentation, algorithms, etc. from external sources available under open-source license

### Note on Copyright

You may see copyright notices in source files that may contain the following copyright owner names:
  * Sensor Platforms, Inc.
  * Audience, Inc.
  * Knowles
Sensor Platforms, Inc. was acquired by Audience in 2014 and subsequently Knowles acquired Audience in 2015. Thus all copyright for any sources – open or licensed – lies with Knowles Electronics, LLC.
