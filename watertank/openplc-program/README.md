# Sample program files for usage with openPLC
It is quite nice to use [OpenPLC](https://www.openplcproject.com/) to control the process via the modbus gateway. I have included to openPLC programs to do this.
The mobdus gateway has to be added as a modbus slave to the OpenPLC for this to work. 

## simple-control.st
This is a simple control programs which sets the outflow to 15 m/s when the level is below 800mm and to 25m/s when the level is above 1200mm.

## simple-control3.st
This is a simple control program similar to simple-control.st, however it has controls for the maxium and minimum setpoints at Holding register 1 and 2 in the openPLC.
