open-sensor-platform
====================

Open Sensor Platform (OSP) is open source software built to enable
systems with sensor hubs.  It is a framework specifically for sensor
data acquisition, communication, and interpretation.  The embedded
code is in C to allow easy compatibility with any CPU architecture or
real-time operating system.  The linux code allows easy implementation
of sensor hub data up to an Application Processor (AP) an Android
Hardware Abstraction Layer (HAL).  The framework is simple and
flexible enough to be extended to more use-cases.

This project can be used to support various embedded sensor
applications such as:
  * Android KitKat compliant buffering sensor hubs,
  * always-on context aware hubs,
  * multi-function wearable devices.

### Community Contributions

OSP encourages developers to focus on creating low-level applications
for sensors rather than reinventing the wheel on the sensor interface.
By open sourcing this software, community contributions can continue
to develop this framework.  Ports of OSP to other platforms can be
contributed to this project.

OSP is released by Sensor Platforms Inc. under the Apache 2.0 license.

### Trying it out

A reference sensor hub example is provided for easily available hardware: 
  * an STEVAL board with ARM-CortexM3 based microprocessor and 9-axis sensors attached,
  * a Nexus7 tablet with KitKat connected over I2C to the STEVAL board as a sensor hub.

This example can be ported to a Cortex-M0+, or any other MCU or DSP
architecture.

The example uses the Keil-MDK toolchain, but
future releases will utilize gcc under Keil as well as full
gcc/Makefile based build system.

The project uses the Keil RTX Real Time Operating system and will
eventually leverage the platform agnostic CMSIS APIs.

### Additional Questions

Refer to the detailed FAQ on the wiki section of this project:
https://github.com/sensorplatforms/open-sensor-platform/wiki 

### Project Structure
  * doc - Doxygen creation
  * embedded - sensor hub code 
  * external - Source code, library, documentation, algorithms, etc. from external sources available under open-source license
  * linux - Android application processor code
