# beamposition_controller
A PI-Controller to control the beam position implemented digitally on an Arduino which can be mounted on a PCB which contains the necessary level-conversions and a multi-channel DAC.

This repository contains all the necessary files to replicate the implemented controller. In ```devices``` one finds the ```.ino``` and ```.cpp``` libraries for the controller itself as well as a ```.py``` script to communicate with it.
The ```KiCaD```-folder contains the ```.kicad_pro``` for the arduino shield and its ```.gbr``` files. 
