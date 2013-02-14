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
#include <unistd.h>
using namespace std;

struct CamSettings {
  float shutter;
} ;

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
    bool software_trigger;
public:
    PointGreyCam(void);
	~PointGreyCam(void);
	int Init(void);
	int Exit(void);
	int SetupCamera( dc1394video_mode_t video_mode,dc1394trigger_mode_t trigger_mode, dc1394trigger_source_t trigger_source,dc1394framerate_t fps);
	int SetupCamera( dc1394video_mode_t video_mode,dc1394trigger_mode_t trigger_mode, dc1394trigger_source_t trigger_source);
	int SetTransmission(dc1394switch_t pwr);
	int SetCameraSettings(CamSettings settings );
	int PrintCameraSettings();
	int GetFrames(dc1394video_frame_t ** &frames_);
	int TriggerLow(void);
	int TriggerHigh(void);
	int SendTrigger(void);
};
#endif
