# Telaire
Arduino Code to communicate and measure AAS Telaire Dust Sensors

Choice of two methodologies, counting the pulses (count), or measuring the pulses and averaging the
outut using interupts (int) to create an assimulation of µg/m3 readings.

Output to serial.

If using the Interrupt ensure the pininterrupt library is installed.
