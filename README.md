linefollower1
=============

Arduino code for a line following robot, with object detection, as teached at Ingegno Team 2013-2014 workshop, see [ingegno.be](http://ingegno.be).

## Lasercut frame

To come

## BOM

To come

## Calibration

Use different scripts to calibrate 

### Engine calibration

Use sketch `calib_motors_forward` to verify your motors run straight, and you turn correctly

### Light sensor calibration

Use sketch `calib_light_sensor_setpoint_line_follow` to verify the light sensor can see lines. Use lines on different colors to determine accuracy needed and to see where your midpoint is.

### Line following

Use sketch `calib_forward_line_follow` to have robot follow a line. Use values of previous calibration, and set here delay and anticipation `Kp`, `Kd`, `Ki`, as well as `max_speed` and `min_speed`that works best for your robot
