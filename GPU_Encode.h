/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/

#pragma once
#ifndef __GPU_Encode_h
#define __GPU_Encode_h
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
extern "C" {
  #include "bcm_host.h"
	#include "ilclient.h"
}

using namespace std;

class GPU_Encode{

private:
    int width, height, bpp; //height width and bytes per pixel
    unsigned long int img_size;
    OMX_VIDEO_PARAM_PORTFORMATTYPE format;
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    COMPONENT_T *video_encode;
    COMPONENT_T *list[5];
    OMX_BUFFERHEADERTYPE *buf;
    OMX_BUFFERHEADERTYPE *out;
    OMX_ERRORTYPE r;
    int err;
    ILCLIENT_T *client;
    int status;
    FILE *outf;
    OMX_VIDEO_PARAM_AVCTYPE avc_param;
    OMX_VIDEO_PARAM_BITRATETYPE bitrate_param;
    OMX_VIDEO_CONFIG_BITRATETYPE bitrate_config;
    bool debug;

public:
    GPU_Encode(void);
	~GPU_Encode(void);
	int Init(int width_, int height_, int bpp_);
	int input_stream_format(OMX_COLOR_FORMATTYPE Format, int framerate);
	int encoding_format(OMX_VIDEO_CODINGTYPE encoder);
	int enoder_settings(unsigned long int bitrate, int numPFrames, OMX_VIDEO_AVCPROFILETYPE profile,OMX_VIDEO_AVCLEVELTYPE avclevel);
    int set_encoder_execute(void);
    int output_filename(string filename);
    int encode(unsigned char* buffer_in, unsigned long buffer_in_size);
    int get_encoded_buffer(unsigned char* buffer_out, unsigned long* buffer_out_sze);
    int close_shutdown(bool closed);
};

#endif
