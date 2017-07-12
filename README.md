# Telaire

Arduino Code to communicate and measure AAS Telaire IR LED Dust Sensors (SM-PWM-01C)

Choice of two methodologies, counting the pulses (count), or measuring the pulses and averaging the outut using interupts (int) to create an assimulation of µg/m3 readings.

Output to serial.

If using the Interrupt ensure the pininterrupt library is installed.

If using the count method then AAS.h, IntFilter.h, LowPassFilter.h, and LowPassFilter.cpp are required also. Either create a library AAS or put in code directory.
