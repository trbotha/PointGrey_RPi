PointGrey_RPi
=============

PointGrey program for Raspberry Pi using hardware encoding

Most libraries used for this build should be installed on most distributions. A firmware update is generally 
required before it can be compiled. To compile simply type make and then run GPU_Encode.

Note that this code is far from complete but was asked to be submitted. The program uses sofware trigger to syncronise
two cameras. The code to set camera to software trigger mode is not included as camera was setup using a laptop and 
settings were save on camera.

Run on a 900MHz RPi two point grey chameleon cams were captured at 20-30fps each. This does depend on shutter time. 
The biggest speed limitation was found to be the colourspace convertion from grayscale to yuv420 this is required since the 
h264 encoder does not accept grayscale format even though the header files provides it as a possible input. 
For colour convertions the bayers image needs to be converted to an acceptable encoder input yuv,rgb see http://home.nouwen.name/RaspberryPi/documentation/ilcomponents/video_encode.html
for a list of acceptable input formats. The dc1394 library provides some colourspace convertion algorithm which can be used for
this.
