PointGrey_RPi
=============

PointGrey program for Raspberry Pi using hardware encoding

Most libraries used for this build should be installed on most distributions. A firmware update is generally 
required before it can be compiled. To compile simply type make and then run GPU_Encode.

Note that this code is far from complete but was asked to be submitted. The program uses sofware trigger to syncronise
two cameras. The code to set camera to software trigger mode is not included as camera was setup using a laptop and 
settings were save on camera.

Run on a 900MHz RPi two point grey chameleon cams were captured at 20-30fps each. This does depend on shutter time. 
