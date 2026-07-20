INSTALL SOFTWARE:
1)Install PSOC programmer (Update firmeware) : close it
2)Install PSOC creator

LOAD THE FIRMWIRE INTO PSOC:
3)Open Trigger_PSOC5.cywrk allocated inside Psoc_creator_project folder with PSOC creator softwer from cypress. Program board Debug-->Program in the menu.

PYTHON INTERFACE:
4)Use trigger.py to comunicate through serial port with the board and change trigger count.
Example of communication: in trigger_example.ipynb 

NOTE: Baudrate of the serial port is already configured to 115200bps, when trigger object is created just pass the COM used.