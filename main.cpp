/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include "GPU_Encode.h"
#include "convert_colourspace.h"
#include "PointGreyCam.h"

#define ESCAPE 27
using namespace std;

int read_raw_rgb(unsigned char* buf, unsigned long* filledLen, int filenumber,int stream);
int frame_to_rgb(unsigned char* buf, unsigned long* filledLen,dc1394video_frame_t *frame);
unsigned long img_size[2] = {640*480, 640*480}; //frame size

int main (int argc, char **argv){
    string filename;
    if (argc < 2){
            printf("No File name supplied will use deafult = file*.mp4");
            filename = "file";
    }
    else{
        filename = argv[1];
        printf("Filname is %s*.mp4\n",filename.c_str());
    }

//set thread priority to maximum
    setpriority(PRIO_PGRP, 0, -20);

    //initialise cameras to mode DC1394_VIDEO_MODE_640x480_MONO8
    PointGreyCam cameras;
    int numCams = cameras.init();
    cameras.setupcamera(DC1394_VIDEO_MODE_640x480_MONO8);
    dc1394video_frame_t **frames = 0;


    struct timespec start, stop,start1,stop1; //timer things used to test performance
    int err;
    GPU_Encode video1, video2; //create encoder for both cams
    GPU_Encode* video = new GPU_Encode[numCams];

    for(int i = 0; i <numCams; ++i){
        err = video[i].Init(640,480,3); //set video encoder settings
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }


//set input stream format to OMX_COLOR_FormatYUV420PackedPlanar
//convertion to YUV was faster than to rgb
//libdc1394 does have some conbertion functions as well
    for(int i = 0; i <numCams; ++i){
        err = video[i].input_stream_format(OMX_COLOR_FormatYUV420PackedPlanar);
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }

//set encoder to H264
    for(int i = 0; i <numCams; ++i){
        err = video[i].encoding_format(OMX_VIDEO_CodingAVC);
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }

//set quality of encoding
    for(int i = 0; i <numCams; ++i){
        err = video[i].enoder_settings(5000000, 250, OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31);
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }


//set encoder to execute, ready to recieve frames
    for(int i = 0; i <numCams; ++i){
        err = video[i].set_encoder_execute();
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }


//create video file names
    for(int i = 0; i <numCams; ++i){
        string outputfile = filename+"_Cam";
        string buf ="1000000";
        sprintf((char*)buf.c_str(),"%d.h264",i);
        outputfile.append(buf);
        printf("\nFilename is %s \n",outputfile.c_str());
        err = video[i].output_filename(outputfile.c_str());
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }


    //frame buffer for yuv420 (12bit i.e 1.5 bytes per pixel)
    unsigned char buffer_in[img_size[0]*3/2]; //640*480*1.5
    unsigned long buffer_in_size;
    clock_gettime(CLOCK_MONOTONIC,&start );
    int c = 0;
    bool run = true;
    int nFrames = 0;

    //only 100 frames are recorded for now can be set up to have
    // stop criteria or wait for keyinput
    while(nFrames<100){

        int errr = cameras.getFrames(frames); //get frames
        errr = cameras.trigger_high(); //set trigger high

        //convert frame to yuv420 and send to encoder
        for(int i = 0; i <numCams; ++i){
            frame_to_rgb(buffer_in,&buffer_in_size,frames[i]); //actually uses gray2yuv420sp
            err = video[i].encode(buffer_in, buffer_in_size); //send to encoder
                if( err!=0 ){
                    printf("Error %i\n",err);
                    return err;
                }
        }

        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
        ++nFrames;
    }//while nFrames

    clock_gettime(CLOCK_MONOTONIC,&stop );
    printf("Time taken %f \n",((float)stop.tv_sec+(float)(stop.tv_nsec)/1.0e9)-((float)start.tv_sec+(float)(start.tv_nsec)/1.0e9));

    //close encoders
    video[0].close_shutdown(true);
    for(int i = 1; i <numCams; ++i)
        video[i].close_shutdown(false);


//create system calls to delete old file with raw h264 named .h264
//create system call to add header to raw .h264 to create .mp4 file
//which can be viewd with a movie player
    for(int i = 0; i <numCams; ++i){
       string h264 = filename+"_Cam";
       string buf_h264 ="1000000";
       string mp4 = filename+"_Cam";
       string buf_mp4 ="1000000";
       sprintf((char*)buf_h264.c_str(),"%d.h264",i);
       h264.append(buf_h264);

       sprintf((char*)buf_mp4.c_str(),"%d.mp4",i);
       mp4.append(buf_mp4);
       char command_del[255];
       sprintf(command_del,"rm %s",mp4.c_str());
       char command_create[255];
       sprintf(command_create,"ffmpeg -f h264 -i %s -vcodec copy %s",h264.c_str(),mp4.c_str());

       system(command_del);
       system(command_create);

    }
    return 0;
}



int frame_to_rgb(unsigned char* buf, unsigned long* filledLen,dc1394video_frame_t *frame){
  //  printf("Getting rgb frame");
  //convert grayscale to yuv420sp
    gray2yuv420sp(buf,frame->image,640*480);
    *filledLen = 640*480*3/2;
    return 0;
}


//was used to test system before cameras was used to convert picture to gray and to yuv
//not needed can be removed
int read_raw_rgb(unsigned char* buf, unsigned long* filledLen, int filenumber,int stream){
  char filename[256];
  FILE *f;
  size_t ret;
  unsigned char grayimage[img_size[stream]];
  unsigned char yuv420[img_size[stream]*3/2];
  unsigned char rgb[img_size[stream]*3];
  snprintf(filename, 256, "%08d.png.raw", filenumber);

  f = fopen(filename, "r");
  if (!f) {
  printf("Failed to open '%s'\n", filename);
	return 0;
  }
  unsigned long int size_buf = img_size[stream]*3;

  ret = fread(rgb, 1, size_buf, f);
  if (ret < size_buf) {
	printf("Failed to read '%s': %d\n", filename, ret);
	return 0;
  }
    rgb2gray(rgb,grayimage,img_size[stream],LUMINOSITY,RGB);
    gray2yuv420sp(buf,grayimage,img_size[stream]);
   // gray2rgb(buf,grayimage,img_size[stream]/3);
  *filledLen = img_size[stream]*3/2;

  fclose(f);
  return 1;
}
