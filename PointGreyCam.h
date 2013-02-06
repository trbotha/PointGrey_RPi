/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/

#pragma once
#ifndef __PointGreyCam_h
#define __PointGreyCam_h
#include <stdio.h>
#include <stdint.h>
#include <dc1394/dc1394.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cstring>
#include <iostream>
#include <time.h>
using namespace std;

class PointGreyCam{

private:
    FILE* imagefile;
    dc1394camera_t **camera;
    dc1394featureset_t features;
    dc1394framerates_t framerates;
    dc1394video_modes_t video_modes;
    dc1394framerate_t framerate;
    dc1394video_mode_t video_mode;
    dc1394color_coding_t coding;
    unsigned int width, height;
    dc1394video_frame_t **frame;
    dc1394_t * d;
    dc1394camera_list_t * list;
    dc1394error_t err;
    dc1394switch_t pwr;
    int NumCameras;
    bool start_cap;
public:
    PointGreyCam(void);
  ~PointGreyCam(void);
	int init(void);
	int exit(void);
	int setupcamera( dc1394video_mode_t video_mode);
	int getFrames(dc1394video_frame_t ** &frames_);
	int trigger_low(void);
	int trigger_high(void);
};
#endif
