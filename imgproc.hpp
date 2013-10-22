#ifndef _IMGPROC_H_
#define _IMGPROC_H_
#include <stdlib.h>

void alphaErode(unsigned char *data, int w, int h);
void alphaDilate(unsigned char *data, int w, int h);
void alphaHisteresis(unsigned char *data, int w, int h,int lowThres, int highThres);
void alphaGaussianBlur(unsigned char *data, int w, int h);

#endif
