# Telaire
Arduino C++ Code to communicate and measure Amphenol Advanced Sensors (AAS) Telaire Sensors including Carbon Dioxide (CO2), Dust (PM), and Humidity sensors.

Check Branches for different sensor types.

AAS Telaire T9602 series, ChipCap2 parts, T6700 series, Dust Sensors.

T6713, T6703, T6793, T6715

v1.4 Evaluation Board now monitors low output ouly, checks to see which sensors are present, and displays only them. Dust output is a 2 minute rolling average updated every 5 seconds.

NOTE: Any option with the dust sensor requires the pinchangeint library to be installed.
https://github.com/NicoHood/PinChangeInterrupt 
Or http://playground.arduino.cc/Main/PinChangeInt 

Sometimes there are issues with the driver file of the Arduino compatible and the Arduino IDE programme will not commi=unicate with the Arduino, either it requires a CH-340 driver, or see http://eestuffs.com/2017/01/19/fix-arduino-driver-install-problem/ for a fix.
