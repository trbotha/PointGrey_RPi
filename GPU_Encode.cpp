/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/

/*Encoder uses OMX, all header files with OMX types and
function  prototypes can be found in /opt/vc/include/IL/ if latest
firmware has been installed*/

#include "GPU_Encode.h"

using namespace std;

/* Constructor */
GPU_Encode::GPU_Encode(void){
    debug = false;
    video_encode = NULL;
}

/* Destructor */
GPU_Encode::~GPU_Encode(void){
}

/*  Initialise GPU Encoder
    INPUTS
        1: image width
        2: image height
        3: bytes per pixel
*/
int GPU_Encode::Init(int width_, int height_, int bpp_){
    bcm_host_init();
    width = width_;
    height = height_;
    bpp = bpp_;
    img_size = width*height*bpp;

    memset(list, 0, sizeof(list));

    //initialise ilclient
    if((client = ilclient_init()) == NULL){
        printf("ilclient_init() failed\n");
        return -1;
    }

    //initialise OMX
    r = OMX_Init();
    if( r != OMX_ErrorNone){
        ilclient_destroy(client);
        printf("OMX_Init() failed\n");
        return r;
   }

   // create video_encode component
   r = (OMX_ERRORTYPE)ilclient_create_component(client, &video_encode, "video_encode", (ILCLIENT_CREATE_FLAGS_T)(ILCLIENT_DISABLE_ALL_PORTS | ILCLIENT_ENABLE_INPUT_BUFFERS | ILCLIENT_ENABLE_OUTPUT_BUFFERS));
   if (r != 0) {
      printf("ilclient_create_component() for video_encode failed with %x!\n", r);
      return r;
   }
   list[0] = video_encode;





   return 0;
}


/*
    Set the format of the input stream.
    INPUTS
        1: input format (OMX_COLOR_FORMATTYPE) see /opt/vc/include/IL/OMX_IVCommon.h
            for format types. Note only a few formats are supported some yuv420
        2: framerate to incode to
*/
int GPU_Encode::input_stream_format(OMX_COLOR_FORMATTYPE Format, int framerate){

    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex = 200;

    if (OMX_GetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamPortDefinition, &portdef) != OMX_ErrorNone) {
        printf("%s:%d: OMX_GetParameter(portdef) for video_encode port 200 failed!\n", __FUNCTION__, __LINE__);
        exit(1);
    }
    portdef.format.video.nFrameWidth = width;
    portdef.format.video.nFrameHeight = height;
    portdef.format.video.xFramerate = framerate << 16;
    portdef.format.video.nSliceHeight = ((height+15)&~15); //must be multiple of 16
    portdef.format.video.nStride = ((width+31)&~31); //must be multiple of 32
    portdef.format.video.eColorFormat = Format;
    printf(" nSlice = %i\n nStride = %i \n",((height+15)&~15),((width+31)&~31));
    r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamPortDefinition, &portdef);
    if (r != OMX_ErrorNone) {
        printf("%s:%d: OMX_SetParameter(OMX_IndexParamPortDefinition) for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__, r);
        return(r);
    }
    return 0;
}


/*
    Set the format of the encoder
    INPUTS
        1: input format OMX_VIDEO_CodingAVC is as far as I know the only supported
            format ie. H264.
*/
int GPU_Encode::encoding_format(OMX_VIDEO_CODINGTYPE encoder){

    memset(&format, 0, sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
    format.nSize = sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE);
    format.nVersion.nVersion = OMX_VERSION;
    format.nPortIndex = 201;
    format.eCompressionFormat = encoder;

    r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoPortFormat, &format);
    if (r != OMX_ErrorNone) {
        printf("%s:%d: OMX_SetParameter(VideoPortFormat) for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__, r);
        return r;
    }

    return 0;
}



/*
    Set Encoder settings effects quality of output video
    INPUTS
        1: bitrate
        2: num of P frames
        3: AVC profile
        4: avclevel
    I did not find much difference in changing the last two inputs
*/
int GPU_Encode::enoder_settings(unsigned long int bitrate, int numPFrames, OMX_VIDEO_AVCPROFILETYPE profile,OMX_VIDEO_AVCLEVELTYPE avclevel){

    memset(&avc_param, 0, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
    avc_param.nSize = sizeof(OMX_VIDEO_PARAM_AVCTYPE);
    avc_param.nPortIndex = 201;
    avc_param.nVersion.nVersion = OMX_VERSION;


    r = OMX_GetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoAvc, &avc_param);
    if ( r != OMX_ErrorNone) {
        printf("%s:%d: OMX_GetParameter(ParamVidAvc) for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__,r);
        return r;
    }

    avc_param.eProfile = profile;
    avc_param.eLevel = avclevel;
    avc_param.nPFrames = numPFrames;

    r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoAvc, &avc_param);
    if (r!= OMX_ErrorNone) {
        printf("%s:%d: OMX_SetParameter(AVC) for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__,r);
        return r;
    }
    r = OMX_GetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoAvc, &avc_param);
    if ( r != OMX_ErrorNone) {
        printf("%s:%d: OMX_GetParameter(ParamVidAvc) for video_encode port 200 failed with %x!\n", __FUNCTION__, __LINE__,r);
        return r;
    }


    err = ilclient_change_component_state(video_encode, OMX_StateLoaded);
    if ( err != 0) {
        printf("Error changing state loaded %i!\n",err);
        return r;
    }

    memset(&bitrate_param, 0, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
    bitrate_param.nSize = sizeof(OMX_VIDEO_PARAM_BITRATETYPE);
    bitrate_param.nPortIndex = 201;
    bitrate_param.nVersion.nVersion = OMX_VERSION;
    bitrate_param.eControlRate = OMX_Video_ControlRateVariable;
    bitrate_param.nTargetBitrate = bitrate;


    r = OMX_SetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexParamVideoBitrate, &bitrate_param);
    if (r!= OMX_ErrorNone) {
        printf("%s:%d: OMX_SetParameter(Bitrate) for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__,r);
        return r;
    }

    memset(&bitrate_config, 0, sizeof(OMX_VIDEO_CONFIG_BITRATETYPE));
    bitrate_config.nSize = sizeof(OMX_VIDEO_CONFIG_BITRATETYPE);
    bitrate_config.nPortIndex = 201;
    bitrate_config.nVersion.nVersion = OMX_VERSION;

    //return bitrate to check not needed can be removed
    r = OMX_GetParameter(ILC_GET_HANDLE(video_encode), OMX_IndexConfigVideoBitrate, &bitrate_config);
    if (r!= OMX_ErrorNone) {
        printf("%s:%d: OMX_SetParameter(AVC) for video_encode port 201 failed with %x!\n", __FUNCTION__, __LINE__,r);
        return r;
    }
    printf("Bitrate is = %lu\n",bitrate_config.nEncodeBitrate);

    return 0;

}


/*
    Set Encoder to execute

*/
int GPU_Encode::set_encoder_execute(void){
    printf("encode to idle...\n");
    if (ilclient_change_component_state(video_encode, OMX_StateIdle) == -1) {
        printf("%s:%d: ilclient_change_component_state(video_encode, OMX_StateIdle) failed with ", __FUNCTION__, __LINE__);
    }


    printf("enabling port buffers for 200...\n");
    err = ilclient_enable_port_buffers(video_encode, 200, NULL, NULL, NULL);
    if ( err != 0) {
        printf("enabling port buffers for 200 failed with %i!\n",err);
        return err;
    }

    printf("enabling port buffers for 201...\n");
    if (ilclient_enable_port_buffers(video_encode, 201, NULL, NULL, NULL) != 0) {
        printf("enabling port buffers for 201 failed!\n");
        return -1;
    }


    printf("encode to executing...\n");
    ilclient_change_component_state(video_encode, OMX_StateExecuting);

    return 0;
}

/*
    Set output file name
    INPUT
        1: string can be changed to char*
*/
int GPU_Encode::output_filename(string filename){

//open file buffer to save encoded frames to
   outf = fopen(filename.c_str(), "w");
   if (outf == NULL) {
      printf("Failed to open '%s' for writing video\n", filename.c_str());
      return -1;
   }
   return 0;


/*
    Encode frame
    INPUTS
        1: frame buffer to encode
        2: Size of input buffer in bytes
*/
int GPU_Encode::encode(unsigned char* buffer_in, unsigned long buffer_in_size){

    buf = ilclient_get_input_buffer(video_encode, 200, 1);
    if (buf == NULL) {
        printf("Doh, no buffers for me!\n");
    } else {
//         printf("Got buffer: %p %lu %lu %lu\n", buf, buf->nAllocLen, buf->nFilledLen, buf->nOffset);

         /* fill it */
        buf->pBuffer = buffer_in;
        buf->nFilledLen = buffer_in_size;
        r = OMX_EmptyThisBuffer(ILC_GET_HANDLE(video_encode), buf);
        if( r != OMX_ErrorNone) {
            printf("Error emptying buffer failed with %x!\n",r);
        }
        else {
//            printf("Buffer emptied!\n");
        }

        if(debug){
            printf("Requesting output...\n");
            fflush(stdout);
         }

         out = ilclient_get_output_buffer(video_encode, 201, 1);

         r = OMX_FillThisBuffer(ILC_GET_HANDLE(video_encode), out);
         if (r != OMX_ErrorNone) {
            printf("Error filling buffer: %x\n", r);
         }

        if (out != NULL) {
            if(debug)
               printf("Got it: %p %lu\n", out, out->nFilledLen);
  		if (out->nFlags & OMX_BUFFERFLAG_CODECCONFIG) {
                if(debug){
                    for (unsigned int i = 0; i < out->nFilledLen; i++)
                        printf("%x ", out->pBuffer[i]);
                    printf("\n");
                }
            }

        //write encoded frame to file buffer
        r = (OMX_ERRORTYPE)fwrite(out->pBuffer, 1, out->nFilledLen, outf);
        if ( (unsigned int)r != out->nFilledLen ) {
            if(debug)
                printf("fwrite: Error emptying buffer: %d!\n", r);
        } else {
            if(debug)
                printf("Buffer emptied!\n");
        }

        out->nFilledLen = 0;

         }
         else {
            printf("Not getting it :(\n");
         }

    }
    return 0;
}



/*
    retrieve encoded buffer
    INPUTS
        1: encoded H264 frame buffer
        2: Size of output buffer in bytes
*/
int GPU_Encode::get_encoded_buffer(unsigned char* buffer_out, unsigned long* buffer_out_sze){
    buffer_out = out->pBuffer;
    *buffer_out_sze = out->nFilledLen;
    return 0;
}

/*
    Shutdown encoder
    INPUT boolean
*/
int GPU_Encode::close_shutdown(bool closed){
    //close output file
    fclose(outf);
    printf ("Teardown.\n");
    if(!closed){
        printf("disabling port buffers for 200 and 201...\n");
        ilclient_disable_port_buffers(video_encode, 200, NULL, NULL, NULL);
        ilclient_disable_port_buffers(video_encode, 201, NULL, NULL, NULL);
    }
    //clean up
    ilclient_state_transition(list, OMX_StateIdle);
    ilclient_cleanup_components(list);
    OMX_Deinit();
    ilclient_destroy(client);
    return 0;
}
