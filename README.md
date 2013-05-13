PointGrey_RPi
=============

PointGrey program for Raspberry Pi using hardware encoding

Most libraries used for this build should be installed on most distributions. A firmware update is generally 
required before it can be compiled. To compile simply type make and then run GPU_Encode.

The program should be able to use as many cameras as the buss will support. Currently a maximum of two has been used. 
The Cameras can be used in either streaming mode where the framerate is set, or in software synching which can be 
used for stereovision purposes to obtain syncronised images of two or more cameras. 

Run on a 900MHz RPi two point grey chameleon cams were captured at 20-30fps each for both streaming and software
triggered cameras. This does depend however on shutter time.  Currently the shutter time (which is currently the only 
camera setting to be adjusted with this code, however camera settings are easily added with the
dc1394 library api) is specified using an external filewhich is read at the start else the shutter is default 20ms.

The biggest speed limitation was found to be the colourspace convertion from grayscale to yuv420 this is required since the 
h264 encoder does not accept grayscale format even though the header files provides it as a possible input. 
For colour convertions the bayers image needs to be converted to an acceptable encoder input yuv,rgb see 
http://home.nouwen.name/RaspberryPi/documentation/ilcomponents/video_encode.html
for a list of acceptable input formats. The dc1394 library provides some colourspace convertion algorithm which can 
be used for this.
