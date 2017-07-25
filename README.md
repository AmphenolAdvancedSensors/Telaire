# Telaire T66xx series family
Arduino C++ Code to communicate and measure AAS Telaire Sensors including T6613, T6615, and T6617 parts.

T6613 has I2C and UART available, as well as 0-4V analogue output. Analogue is linearly ranged as calibration, so 0V at 0ppm, and 4V at 2,000 ppm for example.

T6615 and T6617 only have UART and analogue outputs. Higher range calibrations will need to be multiplied by 10 to get ppm value output as we use 16bit register for output (MSB, LSB) which normally equates to 0 to 65,536ppm maximum.
