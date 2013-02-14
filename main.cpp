/* Written by Theunis Richard Botha, South Africa, December 2012
You are free to use this for educational/non-commercial use*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <iostream>
#include <fstream>
#include <termios.h>
#include "GPU_Encode.h"
#include "convert_colourspace.h"
#include "PointGreyCam.h"

//defines for keyboard input
#define ESCAPE 27
#define s 115
#define S 83
using namespace std;

int read_raw_rgb(unsigned char* buf, unsigned long* filledLen, int filenumber,int stream);
int frame_to_rgb(unsigned char* buf, unsigned long* filledLen,dc1394video_frame_t *frame);
void set_conio_terminal_mode();
void reset_terminal_mode();
int getch();
int kbhit();

unsigned long img_size[2] = {640*480, 640*480};
struct termios orig_termios;


int main (int argc, char **argv){ //input file name to save to default = file_Cam*.mp4
    CamSettings SetStruct;
    tcgetattr(0, &orig_termios);
    string filename;
    if (argc < 2){
            printf("No File name supplied will use deafult = file*.mp4\n");
            filename = "file";
    }
    else{
        filename = argv[1];
        printf("Filname is %s*.mp4\n",filename.c_str());
    }

    //read camera settings currently only used for shutter opening time
    ifstream camera_settings;
    camera_settings.open("camera_settings.txt");
    string setting;
    getline(camera_settings,setting);
    printf("string is %s\n",setting.c_str());
    if(!setting.compare("shutter")){
        camera_settings>>SetStruct.shutter;
        printf("shutter is %f\n",SetStruct.shutter);
    }

    camera_settings.close();


    //set thread priority to maximum
    setpriority(PRIO_PGRP, 0, -20);

     //initialise cameras to mode DC1394_VIDEO_MODE_640x480_MONO8
    PointGreyCam cameras;
    int numCams = cameras.Init();
    //cameras.SetupCamera(DC1394_VIDEO_MODE_640x480_MONO8,DC1394_TRIGGER_MODE_3,DC1394_TRIGGER_SOURCE_SOFTWARE,DC1394_FRAMERATE_30); //streaming example
    cameras.SetupCamera(DC1394_VIDEO_MODE_640x480_MONO8,DC1394_TRIGGER_MODE_0,DC1394_TRIGGER_SOURCE_SOFTWARE); //polling example
    cameras.SetCameraSettings(SetStruct );  //set settings only shutter at the moment
    cameras.PrintCameraSettings();          //print camera settings to screen
    dc1394video_frame_t **frames = 0;       //framer buffers


    struct timespec start, stop,start1,stop1;//timer things used to test performance
    int err;

    GPU_Encode* video = new GPU_Encode[numCams]; //create encoder for numCams

//The following code is for setting up encoder settings this can be grouped
// into one for loop.

//set resolutions of images
    for(int i = 0; i <numCams; ++i){
        err = video[i].Init(640,480,3);
        if( err!=0 ){
            printf("Error %i\n",err);
            return err;
        }
    }

//set input stream format to OMX_COLOR_FormatYUV420PackedPlanar
//convertion to YUV was faster than to rgb
//libdc1394 does have some conbertion functions as well
    for(int i = 0; i <numCams; ++i){
        err = video[i].input_stream_format(OMX_COLOR_FormatYUV420PackedPlanar, 30);
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

//create video file names .h264 are tempory file names
//with raw h264 encoding. A header will be added amd
//converted to .mo4 which can be played
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
    unsigned char buffer_in[img_size[0]*3/2];
    unsigned long buffer_in_size;

// while loop flags
    int c = 0;
    bool run = false;
    bool wait = true;

    set_conio_terminal_mode(); //sets input stream, allows for input to be received without enter
    printf("\nPress ""S"" to start and stop recording.\n");
    while(wait){

        if(kbhit()){ //if keyboard hit
            c = getch();
            if( (c==s) || (c==S) ){ //if Sample
                wait = false;
                run = true;
            }
            else if( c==ESCAPE ){ //if exit
                run = false;
                wait = false;
            }
        }
    }
    reset_terminal_mode(); //reset to original to display to screen
    unsigned int Frame_counter = 0;

    //Main loop
    if(run){
        cameras.SetTransmission(DC1394_ON); //set cameras on
        printf("\nSampling Press ""S"" to stop.\n");
        set_conio_terminal_mode(); //sets input stream, allows for input to be received without enter
        clock_gettime(CLOCK_MONOTONIC,&start ); //start timer
        while(run){

            int errr = cameras.GetFrames(frames); //get frame buffers

//convert frame to yuv420 and send to encoder
            for(int i = 0; i <numCams; ++i){
                frame_to_rgb(buffer_in,&buffer_in_size,frames[i]); //actually uses gray2yuv420sp
                err = video[i].encode(buffer_in, buffer_in_size); //send to encoder
                if( err!=0 ){
                    printf("Error %i\n",err);
                    return err;
                }
            }
            ++Frame_counter;
            if( err!=0 ){
                printf("Error %i\n",err);
                return err;
            }
            if(kbhit()){
                c = getch();
                if( (c==s) || (c==S) || (c==ESCAPE) )
                    run = false;
            }

        }//while run

        clock_gettime(CLOCK_MONOTONIC,&stop );
        reset_terminal_mode();
        //show statistics. For some reason when adding header the file always has two less frames than Frame_counter
        float time = ((float)stop.tv_sec+(float)(stop.tv_nsec)/1.0e9)-((float)start.tv_sec+(float)(start.tv_nsec)/1.0e9);
        printf("Frames taken %u, at an Average FPS %f \n",Frame_counter-2, float(Frame_counter-2)/time);

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
        reset_terminal_mode(); //reset terminal
        printf("Exiting.\n");
        cameras.Exit();
        video[0].close_shutdown(true);
        for(int i = 1; i <numCams; ++i)
            video[i].close_shutdown(false);
    }// if run
    else{
        reset_terminal_mode();
        printf("Exiting.\n");
        cameras.Exit();
        video[0].close_shutdown(true);
        for(int i = 1; i <numCams; ++i)
            video[i].close_shutdown(false);
    }


    return 0;
}



int frame_to_rgb(unsigned char* buf, unsigned long* filledLen,dc1394video_frame_t *frame){
  // printf("Getting rgb frame");
  //convert grayscale to yuv420sp
    int img_size = 640*480;
    gray2yuv420sp(buf,frame->image,640*480);
    *filledLen = (img_size*3)/2;
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

//reset terminal
void reset_terminal_mode()
{
    tcsetattr(0, TCSANOW, &orig_termios);
}
//set terminal
void set_conio_terminal_mode()
{
    struct termios new_termios;

    /* take two copies - one for now, one for later */

    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    /* register cleanup handler, and set the new terminal mode */
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

//detect keyboard input
int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

//get character input
int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}
