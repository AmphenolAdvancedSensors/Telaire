# Telaire
Arduino Code to communicate and measure AAS Telaire IR Laser source Dust Sensors (SM-UART-01L)

Output to serial and 0.94" Serial OLED display.

Int - Uses Interupts to measure the LPO and then average over time.
Count - Counts the low pulses, is simpler and offers possibilities to combine other sensors that require timers.
