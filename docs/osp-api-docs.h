/* Open Sensor Platform Project
 * https://github.com/sensorplatforms/open-sensor-platform
 *
 * Copyright (C) 2013 Sensor Platforms Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*!
  \mainpage

 This documentation describes the Open-Sensor-Platform project
 for a resource constrained Sensor Hub or Smart Sensor devices. 

  \image html osp_framework.png
  \image latex osp_android_framework.png
  \image latex osp_hub_framework.png

<h2> API Reference</h2>
<a href="osp-api_8h.html#func-members">Functions</a>
<br>
<a href="annotated.html">Data Structures</a>
<br>


<h2>High Level Usage Overview</h2>
At boot time:
  - get sensor calibration data from non-volatile memory or the host
  - call OSP_Initialize()
  - let the Library know about what sensors are on the system by filling in a SensorDescriptor_t and calling OSP_RegisterInputSensor() for each sensor

During steady state operation:
  - queue data from sensor ISRs by calling OSP_SetData()
  - execute primary fusion computation by calling OSP_DoForegroundProcessing() at least twice as fast as your max expected output rate 
  - execute calibration computation by calling OSP_DoBackgroundProcessing() at least as fast as your min expected output rate
  - as enable commands come from the host construct an appropriate SensorDescriptor_t and call OSP_SubscribeOutputSensor() to start receiving data callbacks
  - as disable commands come from the host call OSP_UnsubscribeOutputSensor()
  - as setDelay commands come from the host do an appropriate OSP_UnsubscribeOutputSensor(), update the OutputDataRate, then call OSP_SubscribeOutputSensor()

Data will be returned via callbacks:
  - Sensor fusion will call your callbacks with output sensor data (Device Orientation Rotation Vector, Calibrated Magnetometer Data, etc)
  - ResourceManager will provide callbacks for enabling/disabling/rate changing input sensor drivers
  - Errors and warnings are also available via callbacks
  - When sensor calibration callbacks fire, store the calibration parameters to non-volatile memory locally or on the host


<h2>Examples</h2>
<h3> Step Example </h3>
Simplified example demonstrating step counting.  Uses simulated data, and can be used as a benchmark

<a href="example_2orientationExample_8c-example.html">example/orientationExample.c</a>


*/
/*!
 * \example main.c
 * Simplified example demonstrating step counting.  Uses simulated data, and can be used as a benchmark
 *
 *
 */