/* Written by Theunis Richard Botha, South Africa, December 2012
   You are free to use this for educational/non-commercial use*/
#ifndef __convert_colourspace_h
#define __convert_colourspace_h
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <algorithm>
#include <time.h>
#include <string.h>
using namespace std;

   enum RGB2GRAYMETHOD
   {
      LIGHTNESS,
      AVERAGE,
      LUMINOSITY,
   };
    enum RGBORDER
   {
      RGB,
      BGR,
   };
void rgb2gray(unsigned char* rgb, unsigned char* gray, int width, int height, RGB2GRAYMETHOD method, RGBORDER order); //rgb888 to grayscale8
void rgb2gray(unsigned char* rgb, unsigned char* gray, unsigned long int img_size , RGB2GRAYMETHOD method, RGBORDER order);//rgb888 to grayscale8
void gray2rgb(unsigned char* rgb, unsigned char* gray,unsigned long int img_size);//gray8 to rgb888
void gray2yuv420sp(unsigned char* yuv, unsigned char* gray,unsigned long int img_size); //gray8 to yuv420 semi packed planar

#endif
