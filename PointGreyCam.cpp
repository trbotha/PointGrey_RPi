/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/

#include "PointGreyCam.h"

using namespace std;
//clean up exit
    void cleanup_and_exit(dc1394camera_t *camera)
    {
        dc1394_video_set_transmission(camera, DC1394_OFF);
        dc1394_capture_stop(camera);
        dc1394_camera_free(camera);
        exit(1);
    }

    PointGreyCam::PointGreyCam(void){}
    PointGreyCam::~PointGreyCam(void){
        delete [] camera;
        delete [] frame;
    }

/*
Initialise Point grey structure
The program is hardcoded to work with upto 2 cameras
can be changed to be the number of cameras picked up
i.e. NumCameras.
*/
    int PointGreyCam::init(void){
        start_cap = false;
        d = dc1394_new ();
        if (!d)
            return -1;
        err=dc1394_camera_enumerate (d, &list);
        DC1394_ERR_RTN(err,"Failed to enumerate cameras");
        if (list->num == 0) {
            dc1394_log_error("No cameras found");
            return -2;
        }

        //find number of cameras connected and create structures
        NumCameras = list->num;
        camera = new dc1394camera_t*[2];
        frame = new dc1394video_frame_t*[2];

        //initialise camera structure
        for(int i = 0; i < NumCameras; ++i){
            camera[i] = dc1394_camera_new (d, list->ids[i].guid);
            if (!camera[i]) {
                dc1394_log_error("Failed to initialize camera with guid %i", list->ids[i].guid);
                return 1;
            }
               printf("Using camera with GUID %llu\n", camera[i]->guid);
        }
        dc1394_camera_free_list (list);

        return NumCameras;
    }

/*  Setup camera
    INPUT
        1: dc1394video_mode_t see libdc1394 for details
*/
    int PointGreyCam::setupcamera( dc1394video_mode_t video_mode){

        for(int i = 0; i < NumCameras; ++i){

            err=dc1394_video_set_mode(camera[i],   video_mode);
            DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not set video mode");

            //set up camera with 4 frame buffers max of chameleon I believe
            err=dc1394_capture_setup(camera[i], 4, DC1394_CAPTURE_FLAGS_DEFAULT);
            DC1394_ERR_RTN(err,"Could not setup capture");
            err=dc1394_video_set_transmission(camera[i], DC1394_ON);
            DC1394_ERR_RTN(err,"Could not start transmission");

            /*-----------------------------------------------------------------------
             *  report camera's features
             *-----------------------------------------------------------------------*/
            printf("Get Camera Features\n");
            err=dc1394_feature_get_all(camera[i],&features);
            if (err!=DC1394_SUCCESS) {
                dc1394_log_warning("Could not get feature set");
            }
            else {
               dc1394_feature_print_all(&features, stdout);
            }
        }
        for(int i = 0; i < NumCameras; i++){
            //For this code to work the camera needs to be in software trigger mode, I did this with my laptop
            //below is a prototype of changing the camera mode. This is set to setup free streaming
           // dc1394error_t dc1394_external_trigger_set_mode   ( 	dc1394camera_t *  	camera,		dc1394trigger_mode_t  	mode
        )
        //set shutter open time
            err = dc1394_feature_set_value(camera[i],DC1394_FEATURE_SHUTTER,300);
            DC1394_ERR_RTN(err,"Could not set shutter time");
            unsigned int val;
            err = dc1394_feature_get_value(camera[i],DC1394_FEATURE_SHUTTER,&val);
            printf("Shutter value of cam %i is %lu \n",i,val);
        }
        return 0;
    }


/*  get frames from camera
 INPUT 1:
    Input array of frame structures

*/
    int PointGreyCam::getFrames(dc1394video_frame_t ** &frames_){
        struct timespec start, stop;

    //set sofware trigger high is this is the first frame, else the
    // trigger will be set high from outside of class, this was found
    // to increase fps
        if(!start_cap){
            clock_gettime(CLOCK_MONOTONIC,&start );
            for(int i = 0; i < NumCameras; ++i){
                err = dc1394_software_trigger_set_power(camera[i], DC1394_ON);
              //  printf("Set Software Trigger state HIGH for Cam %i Error is \n",i,err);
            }
             clock_gettime(CLOCK_MONOTONIC,&stop );
            printf("Time taken trigger = %f \n",((float)stop.tv_sec+(float)(stop.tv_nsec)/1.0e9)-((float)start.tv_sec+(float)(start.tv_nsec)/1.0e9));
        }
             clock_gettime(CLOCK_MONOTONIC,&start );

    //dequeue frame i.e. get from camera frame buffer and set software trigger low
            for(int i = 0; i < NumCameras; ++i){
                err=dc1394_capture_dequeue(camera[i], DC1394_CAPTURE_POLICY_WAIT, &frame[i]);
                err = dc1394_software_trigger_set_power(camera[i], DC1394_OFF);
                DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not capture a frame");

            }
            //if trigger is set high now it was found the camera did not notice the low setting
            // thus the camera did not see a change in trigger

            clock_gettime(CLOCK_MONOTONIC,&stop );
         //   printf("Time taken dequee = %f \n",((float)stop.tv_sec+(float)(stop.tv_nsec)/1.0e9)-((float)start.tv_sec+(float)(start.tv_nsec)/1.0e9));

            frames_ = frame;
            clock_gettime(CLOCK_MONOTONIC,&start );

            //for every frame dequeue it needs to be enqueed allowing new frame to be save to it
            for(int i = 0; i < NumCameras; i++)
                err = dc1394_capture_enqueue(camera[i],frame[i]);
            clock_gettime(CLOCK_MONOTONIC,&stop );
          //  printf("Time taken Enqueue = %f \n",((float)stop.tv_sec+(float)(stop.tv_nsec)/1.0e9)-((float)start.tv_sec+(float)(start.tv_nsec)/1.0e9));

          //set start_cap true disabling the initial triggering for first frame
            start_cap = true;
            return 0;
    }

/*Exit*/
    int PointGreyCam::exit(void){
         for(int i = 0; i < NumCameras; ++i){
            err=dc1394_video_set_transmission(camera[i],DC1394_OFF);
            /*-----------------------------------------------------------------------
             *  close camera
             *-----------------------------------------------------------------------*/
            dc1394_video_set_transmission(camera[i], DC1394_OFF);
            dc1394_capture_stop(camera[i]);
            dc1394_camera_free(camera[i]);

        }
        start_cap = false;
        dc1394_free (d);

        return 0;
    }

/*Set software trigger low*/
	int PointGreyCam::trigger_low(void){
        for(int i = 0; i < NumCameras; ++i){
          int  err = dc1394_software_trigger_set_power(camera[i], DC1394_OFF);
          //  printf("Set Software Trigger state HIGH for Cam %i Error is \n",i,err);
        }
        return err;
	}


/*Set software trigger high*/
	int PointGreyCam::trigger_high(void){
        for(int i = 0; i < NumCameras; ++i){
           int err = dc1394_software_trigger_set_power(camera[i], DC1394_ON);
          //  printf("Set Software Trigger state HIGH for Cam %i Error is \n",i,err);
        }
        return err;
	}
