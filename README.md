linefollower1
=============

Arduino code for a line following robot, with object detection, as teached at Ingegno Team 2013-2014 workshop, see [ingegno.be](http://ingegno.be).

_Arduino code voor een lijn volg robot met object detectie, zoals geleerd bij Ingegno Team 2013-2014 workshop, zie [ingegno.be](http://ingegno.be)._

## Lasercut frame

To come / komt

## BOM

To come / komt

## Calibration

Use different scripts to calibrate your robot and see that all works as intended, eg connecting correct pins!

_Gebruik verschillende scripts om je robot te caliberen en te verifieren dat alles werkt zoals bedoeld, bv correcte pinnen verbinden!_

### Engine calibration - _Motor calibratie_

Use sketch `calib_motors_forward` to verify your motors run straight, and you turn correctly.

_Gebruik schets_ `calib_motors_forward`  _om te verifieren dat de motoren rechtdoor gaan en je juist draait. Gebruik seriele motor indien nodig._

### Light sensor calibration - _Lichtsensor calibratie_

*Use the serial monitor to see the results of the calibration!*
Draw different thick lines on a paper, and put your lightsensor on top if it.
Use sketch `calib_light_sensor_setpoint_line_follow` to verify the light sensor can see lines. Use lines on different colors to determine accuracy needed and to see where your midpoint is (around 2000).

*_Gebruik de seriele monitor om de resultaten van de calibratie te zien!_*
_Teken verschillende dikke lijnen op papier, en plaats je lichtsensor erboven. Gebruik schets_ `calib_light_sensor_setpoint_line_follow` _om te verifieren dat de lichtsensor lijnen kan zien. Gebruik lijnen op verschillende gekleurde bladen om de accuraatheid nodig te bepalen en te zien waar je middelpunt is (rond 2000)._

### Line following - _Lijn volgen_

Draw a tick line in black on a white paper, your circuit. Start simple, no sharp angles. Put your robot on it.
Use sketch `calib_forward_line_follow` to have robot follow a line. Use values of previous calibration, and set here delay and anticipation `Kp`, `Kd`, `Ki`, as well as `max_speed` and `min_speed`that works best for your robot.

_Teken een dikke lijn in zwart op wit papier, jouw circuit. Begin simpel, geen scherpe hoeken! Plaats je robot erboven. Gebruik schets_ `calib_forward_line_follow` _om de robot de lijn te laten volgen. Gebruik de waarden van je eerdere calibratie en bepaal hier vertraging en anticipatie:_  `Kp`, `Kd`, `Ki`, _alsook de_ `max_speed` _en_ `min_speed` _die best werkt voor je robot._
