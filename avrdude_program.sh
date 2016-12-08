#!bin/bash
avrdude -c usbasp -p t25 -U flash:w:heater.hex
