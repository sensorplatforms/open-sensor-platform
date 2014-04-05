sensorhubd
--------------------------------------------
Sensorhubd bridges from a serial UART into Input Event virtual sensor device nodes.


### Data Path
It creates three input event devices : accel, mag, and gyro.  They are integer values in android conventions,
but need to be scaled down by 1000 to be put into floating point values.

Find their event IDs by doing a cat /proc/bus/input/devices or using getevent with no arguments

### Control Path
For sensor enable/disable and setDelay sensorhubd reads on named pipes it creates, specifically:

    /data/misc/sensorhubd/osp-accel/enable
    /data/misc/sensorhubd/osp-accel/delayMs

    /data/misc/sensorhubd/osp-mag/enable
    /data/misc/sensorhubd/osp-mag/delayMs

    /data/misc/sensorhubd/osp-gyro/enable
    /data/misc/sensorhubd/osp-gyro/delayMs


### usage
usage: sensorhubd 
         
### verifying functionality
assuming that no daemons are started in init.rc, adb shell logs in as root

*hook up the serial port and start sensor hub daemon*

      adb shell
      sensorhubd &

*read out accelerometer event data from another shell*

      adb shell cat /proc/bus/input/devices | awk '/osp-accel/,/event/ {d=$1; if (/N:/) {split($2,n,"=")}; if(/H:/) {split($2,e," "); print $3 "\t" n[2];}}'
      adb shell getevent `adb shell cat /proc/bus/input/devices | awk '/osp-accel/,/event/ {d=$1; if (/N:/) {split($2,n,"=")}; if(/H:/) {split($2,e," "); print "/dev/input/" $3 ;}}'` #no events come, so press <ctrl-c>
      adb shell echo 1 > /data/misc/sensorhubd/osp-accel/enable
      adb shell getevent `adb shell cat /proc/bus/input/devices | awk '/osp-accel/,/event/ {d=$1; if (/N:/) {split($2,n,"=")}; if(/H:/) {split($2,e," "); print "/dev/input/" $3 ;}}'`
