/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/


#include "convert_colourspace.h"
/*A few colourspace convertions, more can be obtained from libdc1394 */
/*
   convertion from rgb888(24bit) to 8bit grascale specifying width and height
   three methods can be used LIGHTNESS, AVERAGE and LUMINOSITY. The latter
   is the most used
   INPUTS
        1: pointer to rgb buffer input
        2: pointer to grayscale buffer (output)
        3: image width
        4: image height
        5: method to use
        6: order RGB or BGR
    buffers need to be preallocated
*/
void rgb2gray(unsigned char* rgb, unsigned char* gray, int width, int height, RGB2GRAYMETHOD method, RGBORDER order){
    struct timespec start1, stop1;
    clock_gettime(CLOCK_MONOTONIC,&start1 );

    int R = 0;
    int G = 1;
    int B = 2;

    if ( order == BGR ){
        R = 2;
        G = 1;
        B = 0;
    }
    unsigned long int img_size = width*height;
    if( method == LIGHTNESS){
           for(int i = 0; i < img_size; i++){
                gray[i] = (max(max(rgb[i*3+R],rgb[i*3+1+G]),rgb[i*3+2+B])+min(min(rgb[i*3+R],rgb[i*3+1+G]),rgb[i*3+2+B]))/2;
           }
    }
    if( method == AVERAGE){
           for(int i = 0; i < img_size; i++){
                gray[i] = (rgb[i*3+R]+rgb[i*3+1+G]+rgb[i*3+2+B])/3;
           }
    }
    if( method == LUMINOSITY){
           for(int i = 0; i < img_size; i++){
                gray[i] = 0.21*rgb[i*3+R]+0.71*rgb[i*3+G]+0.07*rgb[i*3+B];
           }
    }
    clock_gettime(CLOCK_MONOTONIC,&stop1 );
   //  printf("Time taken rgb2gray%f \n",((float)stop1.tv_sec+(float)(stop1.tv_nsec)/1.0e9)-((float)start1.tv_sec+(float)(start1.tv_nsec)/1.0e9));
}

/*
   convertion from rgb888(24bit) to 8bit grascale specifying image size
   three methods can be used LIGHTNESS, AVERAGE and LUMINOSITY. The latter
   is the most used
   INPUTS
        1: pointer to rgb buffer input
        2: pointer to grayscale buffer (output)
        3: image size
        4: method to use
        5: order RGB or BGR
    buffers need to be preallocated
*/
void rgb2gray(unsigned char* rgb, unsigned char* gray, unsigned long int img_size , RGB2GRAYMETHOD method, RGBORDER order){
    struct timespec start1, stop1;
    clock_gettime(CLOCK_MONOTONIC,&start1 );
    int R = 0;
    int G = 1;
    int B = 2;

    if ( order == BGR ){
        R = 2;
        G = 1;
        B = 0;
    }

    if( method == LIGHTNESS){
           for(unsigned int i = 0; i < img_size; i++){
                gray[i] = (max(max(rgb[i*3+R],rgb[i*3+1+G]),rgb[i*3+2+B])+min(min(rgb[i*3+R],rgb[i*3+1+G]),rgb[i*3+2+B]))/2;
           }
    }
    if( method == AVERAGE){
           for(unsigned int i = 0; i < img_size; i++){
                gray[i] = (rgb[i*3+R]+rgb[i*3+1+G]+rgb[i*3+2+B])/3;
           }
    }
    if( method == LUMINOSITY){
           for(unsigned int i = 0; i < img_size; i++){
                gray[i] = 0.21*rgb[i*3+R]+0.71*rgb[i*3+G]+0.07*rgb[i*3+B];
           }
    }
    clock_gettime(CLOCK_MONOTONIC,&stop1 );
     printf("Time taken rgb2gray%f \n",((float)stop1.tv_sec+(float)(stop1.tv_nsec)/1.0e9)-((float)start1.tv_sec+(float)(start1.tv_nsec)/1.0e9));
}

/*
   convertion from grascale 8bit to rgb 24bit
   INPUTS
        1: pointer to rgb buffer (output)
        2: pointer to grayscale buffer (input)
        3: image size
    buffers need to be preallocated
*/

void gray2rgb(unsigned char* rgb, unsigned char* gray,unsigned long int img_size){
    struct timespec start1, stop1;
    clock_gettime(CLOCK_MONOTONIC,&start1 );
    #pragma omp for
    for(unsigned int i = 0; i < img_size; ++i){
       // for(int j = 0; j < 3; ++j){
            memcpy(&rgb[i*3],&gray[i],1);
           //rgb[i*3+j] = gray[i];
      //  }

    }
    clock_gettime(CLOCK_MONOTONIC,&stop1 );
     printf("Time taken gray2rgb %f \n",((float)stop1.tv_sec+(float)(stop1.tv_nsec)/1.0e9)-((float)start1.tv_sec+(float)(start1.tv_nsec)/1.0e9));
}

/*
   convertion from grascale 8bit to yuv420 semi-packed planar
   INPUTS
        1: pointer to rgb buffer (output)
        2: pointer to grayscale buffer (input)
        3: image size
    buffers need to be preallocated
*/
void gray2yuv420sp(unsigned char* yuv, unsigned char* gray,unsigned long int img_size){
      struct timespec start1, stop1;
    clock_gettime(CLOCK_MONOTONIC,&start1 );
    #pragma omp for
    memset(&yuv[img_size],128,img_size*0.5);
    memcpy(yuv,gray,img_size);

    clock_gettime(CLOCK_MONOTONIC,&stop1 );
 //    printf("Time taken gray2yuv420sp %f \n",((float)stop1.tv_sec+(float)(stop1.tv_nsec)/1.0e9)-((float)start1.tv_sec+(float)(start1.tv_nsec)/1.0e9));
}

