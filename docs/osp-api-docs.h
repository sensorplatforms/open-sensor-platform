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

With respect to terminology, remember that everything coming in is considered a 
<b>Sensor</b>, everything going out is a <b>Result</b>.
<br>


<h2> API Reference</h2>
<a href="osp-api_8h.html#func-members">Functions</a>
<br>
<a href="annotated.html">Data Structures</a>
<br>


<h2>High Level Usage Overview</h2>
At boot time:
  - read sensor calibration data from non-volatile memory
  - call OSP_Initialize()
  - let the Library know about what sensors are on the system by filling in a SensorDescriptor_t and calling OSP_RegisterSensor() for each sensor
  - configure what sensor fusion products to compute by filling in a ResultRequestDescriptor_t and call OSP_SubscribeResult() for each ResultType_t desired

During steady state operation:
  - queue data from sensor ISRs by calling OSP_SetData()
  - execute primary fusion computation by calling OSP_DoForegroundProcessing() at least twice as fast as your expected output rate 
  - execute calibration computation by calling OSP_DoBackgroundProcessing() at least one tenth as fast as your expected output rate

Data will be returned via callbacks:
  - Sensor fusion will call your callbacks with result data (Device Orientation Rotation Vector, Calibrated Magnetometer Data, etc)
  - ResourceManager will provide recommendations for enabling/disabling/rate changing sensor drivers
  - Errors and warnings are also available via callbacks
  - When sensor calibration callbacks fire, store the calibration parameters to non-volatile memory


<h2>Examples</h2>
<h3> Context Example </h3>
Similar to the orientation example but uses Android HAL 1.0 specific results. Uses simulated data, and can be used as a benchmark

<a href="example_2androidHalExample_8c-example.html">example/androidHalExample.c</a>

<h3> Context Example </h3>
Simplified context example demonstrating Posture and Step Counting. Uses simulated data, and can be used as a benchmark

<a href="example_2contextExample_8c-example.html">example/contextExample.c</a>

<h3> Orientation Example </h3>
Simplified example demonstrating device orientation.  Uses simulated data, and can be used as a benchmark

<a href="example_2orientationExample_8c-example.html">example/orientationExample.c</a>


*/
/*!
 * \example example/contextExample.c
Simplified context example which demonstrates Posture and Step Counting. Uses simulated data, and can be used as a benchmark

 * \example example/orientationExample.c
Simplified example demonstrating device orientation.  Uses simulated data, and can be used as a benchmark

 * \example example/androidHalExample.c
Similar to the orientation example but uses Android HAL 1.0 specific results. Uses simulated data, and can be used as a benchmark

 *
 */
