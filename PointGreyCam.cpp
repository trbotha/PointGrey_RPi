/* Written by Theunis Richard Botha, South Africa, December 2012
You are free to use this for educational/non-commercial use*/

#include "PointGreyCam.h"

using namespace std;

void cleanup_and_exit(dc1394camera_t *camera)
{
    dc1394_video_set_transmission(camera, DC1394_OFF);
    dc1394_capture_stop(camera);
    dc1394_camera_free(camera);
    exit(1);
}

PointGreyCam::PointGreyCam(void):software_trigger(false),start_cap(false){}
PointGreyCam::~PointGreyCam(void){}


/*
Initialise Point grey structure
The program is hardcoded to work with upto 2 cameras
can be changed to be the number of cameras picked up
i.e. NumCameras.
*/
int PointGreyCam::Init(void){
    start_cap = false;
    d = dc1394_new ();
    if (!d)
        return -1;
    err=dc1394_camera_enumerate (d, &list);
    DC1394_ERR_RTN(err,"Failed to enumerate cameras.\n");
    if (list->num == 0) {
        dc1394_log_error("No cameras found.\n");
        return -2;
    }

    //find number of cameras connected and create structures
    NumCameras = list->num;
    camera = new dc1394camera_t*[NumCameras];
    frame = new dc1394video_frame_t*[NumCameras];

     //initialise camera structure
    for(int i = 0; i < NumCameras; ++i){
        camera[i] = dc1394_camera_new (d, list->ids[i].guid);
        if (!camera[i]) {
            dc1394_log_error("Failed to initialize camera with guid %i.\n", list->ids[i].guid);
            return 1;
        }
           printf("Using camera with GUID %llu\n", camera[i]->guid);
           dc1394_video_set_iso_speed( camera[i], DC1394_ISO_SPEED_400 );
    }

    dc1394_camera_free_list (list);

    return NumCameras;
}

/* Setup camera
This setup camera is mainly used to setup of software triggering since framerate is not set
however can also bes used to setup streaming, FPS will then be 30fps
INPUT
    1: dc1394video_mode_t see libdc1394 for details
    2: dc1394trigger_mode_t typically DC1394_TRIGGER_MODE_3 (streaming) or DC1394_TRIGGER_MODE_0 (sofware settable i.r. sofware trigger)
    3: dc1394trigger_source_t if DC1394_TRIGGER_MODE_0 this does not have any affect
*/
int PointGreyCam::SetupCamera( dc1394video_mode_t video_mode,dc1394trigger_mode_t trigger_mode, dc1394trigger_source_t trigger_source){
   int ret = SetupCamera(  video_mode, trigger_mode,  trigger_source,  DC1394_FRAMERATE_30);
   return ret;
}

/* Setup camera
This setup camera is mainly used to setup of streaming  since framerate is set
however can also bes used to setup sofware triggering
INPUT
    1: dc1394video_mode_t see libdc1394 for details
    2: dc1394trigger_mode_t typically DC1394_TRIGGER_MODE_3 (streaming) or DC1394_TRIGGER_MODE_0 (sofware settable i.r. sofware trigger)
    3: dc1394trigger_source_t if DC1394_TRIGGER_MODE_0 this does not have any affect
    3: dc1394framerate_t framerate desired from camera i.e. DC1394_FRAMERATE_10, DC1394_FRAMERATE_30 etc..
*/
int PointGreyCam::SetupCamera( dc1394video_mode_t video_mode,dc1394trigger_mode_t trigger_mode, dc1394trigger_source_t trigger_source, dc1394framerate_t fps){

    float shutter = 10;
    if( (trigger_mode == DC1394_TRIGGER_MODE_0) && (trigger_source==DC1394_TRIGGER_SOURCE_SOFTWARE) )
        software_trigger = true;
    for(int i = 0; i < NumCameras; i++){
        err = dc1394_feature_set_power(camera[i],DC1394_FEATURE_TRIGGER,DC1394_ON);
        DC1394_ERR_RTN(err, "Could not set trigger on.");
        err = dc1394_feature_set_power(camera[i],DC1394_FEATURE_TRIGGER_DELAY,DC1394_OFF);
        DC1394_ERR_RTN(err, "Could not set trigger delay off.");
        err=dc1394_external_trigger_set_mode(camera[i], trigger_mode);
        DC1394_ERR_RTN(err, "Could not set trigger mode.");
        err=dc1394_external_trigger_set_source(camera[i], trigger_source);
        DC1394_ERR_RTN(err, "Could not set trigger source.");
    }



    for(int i = 0; i < NumCameras; i++){
        dc1394_feature_set_power(camera[i],DC1394_FEATURE_FRAME_RATE,DC1394_ON);
        DC1394_ERR_RTN(err, "Could not set Frame rate power ON.");
        dc1394_video_set_framerate(camera[i],fps);
        DC1394_ERR_RTN(err, "Could not set Frame rate.");

    }


    for(int i = 0; i < NumCameras; ++i){
        err=dc1394_video_set_mode(camera[i], video_mode);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not set video mode");

        //set up camera with 4 frame buffers max of chameleon I believe
        err=dc1394_capture_setup(camera[i], 4, DC1394_CAPTURE_FLAGS_DEFAULT);
        DC1394_ERR_RTN(err,"Could not setup capture");
        err=dc1394_video_set_transmission(camera[i], DC1394_ON);
        DC1394_ERR_RTN(err,"Could not start transmission");
    }

    return 0;
}
/* SetTransmission
    Set transmission to pwr state either DC1394_OFF or DC1394_ON
    INPUT
        1: dc1394switch_t either DC1394_OFF or DC1394_ON
*/
int PointGreyCam::SetTransmission(dc1394switch_t pwr){
    for(int i = 0; i < NumCameras; ++i)
        err=dc1394_video_set_transmission(camera[i], pwr);
}

/* SetCameraSettings
    Set camera settings currently a structure is passed with desired settings
    this structure currently ony contains shutter since most others are on auto
    this function can be used as a template to change other settings see below
    for two websited detailing features and settings
    http://damien.douxchamps.net/ieee1394/libdc1394/
    http://libdc1394-22.sourcearchive.com/documentation/2.0.2/main.html
        INPUT
        1: struct CamSettings only has shutter to control shutter in ms
*/
int PointGreyCam::SetCameraSettings(CamSettings settings ){
    unsigned int val;
    for(int i = 0; i < NumCameras; i++){
        err = dc1394_feature_set_mode(camera[i],DC1394_FEATURE_SHUTTER,DC1394_FEATURE_MODE_MANUAL);
        DC1394_ERR_RTN(err,"Could not set shutter time\n");
        err = dc1394_feature_set_absolute_value(camera[i],DC1394_FEATURE_SHUTTER,settings.shutter/1000);
        DC1394_ERR_RTN(err,"Could not set shutter time\n");
        err = dc1394_feature_get_value(camera[i],DC1394_FEATURE_SHUTTER,&val);
        printf("Shutter value of cam %i is %lu \n",i,val);
    }
}
/* PrintCameraSettings
    Print camera settings and capabilities of each camera
*/
int PointGreyCam::PrintCameraSettings(){
       for(int i = 0; i < NumCameras; ++i){
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
}

/* GetFrames
    Get frames from cameras
    INPUT
        1: pointer to array of dc1394video_frame_t pointers
*/
int PointGreyCam::GetFrames(dc1394video_frame_t ** &frames_){

    //send sofware trigger if this is thefirst frame, else the
    // trigger will be send after frame is grabbed
    if( !start_cap && software_trigger ){
            SendTrigger();
    }

//dequeue frame i.e. get from camera frame buffer
    for(int i = 0; i < NumCameras; ++i){
        err=dc1394_capture_dequeue(camera[i], DC1394_CAPTURE_POLICY_WAIT, &frame[i]);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not capture a frame");

    }
    //send sofware trigger
    if(software_trigger)
        SendTrigger();


    frames_ = frame;
    //for every frame dequeue it needs to be enqueed allowing new frame to be save to it
    for(int i = 0; i < NumCameras; i++)
        err = dc1394_capture_enqueue(camera[i],frame[i]);

//set start_cap true disabling the initial triggering for first frame
    start_cap = true;
    return 0;
}

/*Exit*/
int PointGreyCam::Exit(void){
     for(int i = 0; i < NumCameras; ++i){
        /*-----------------------------------------------------------------------
        *  close camera
        *-----------------------------------------------------------------------*/
        err=dc1394_video_set_transmission(camera[i],DC1394_OFF);
        DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not stop camera transmission.");

        dc1394_capture_stop(camera[i]);
        dc1394_camera_free(camera[i]);

    }
    start_cap = false;
    dc1394_free (d);

    return 0;
}


/*Set software trigger low*/
	int PointGreyCam::TriggerLow(void){
        for(int i = 0; i < NumCameras; ++i){
            err = dc1394_software_trigger_set_power(camera[i], DC1394_OFF);
          //  printf("Set Software Trigger state HIGH for Cam %i Error is \n",i,err);
        }
        return err;
	}


/*Set software trigger high*/
	int PointGreyCam::TriggerHigh(void){
        for(int i = 0; i < NumCameras; ++i){
           err = dc1394_software_trigger_set_power(camera[i], DC1394_ON);
          //  printf("Set Software Trigger state HIGH for Cam %i Error is \n",i,err);
        }
        return err;
	}

/*SendTrigger
Send trigger wait till camera knows its high and set trigger low
I did experienced a time that the system runs the while loop infinately
I have changed setting and the occurences dissapeared, one was to set Frame
rate to 30 irrespective of whether sofware trigger is used, not sure if it correted
the problem, but afterwards there was no problem.
*/
	int PointGreyCam::SendTrigger(void){
	    dc1394switch_t power;
	    int i;
            for(i = 0; i < NumCameras; ++i){
               err = dc1394_software_trigger_set_power(camera[i], DC1394_ON);
                DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not set trigger power high.");
            }
            for(i = 0; i < NumCameras; ++i){
                power = DC1394_OFF;
                while( (power==DC1394_OFF)){
                    err = dc1394_software_trigger_get_power(camera[i], &power);
                    DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not get trigger power.");

                }
                err = dc1394_software_trigger_set_power(camera[i], DC1394_OFF);
                DC1394_ERR_CLN_RTN(err,cleanup_and_exit(camera[i]),"Could not set trigger power low.");
        }
        return 0;
	}
