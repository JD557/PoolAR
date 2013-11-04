#include "imgproc.hpp"
#include <omp.h>

unsigned char *tempBuffer=NULL;
size_t wAlloc = 0;
size_t hAlloc = 0;

void alloc(int w,int h) {
	if (wAlloc*hAlloc<w*h*4) {
		if (tempBuffer!=NULL) {free(tempBuffer);}
		tempBuffer=(unsigned char*)malloc(h*w);
		wAlloc=w;hAlloc=h;
	}
}

void downscaleAlpha(unsigned char *data, int w, int h) {
	for (int y=0;y<h;y+=2) {
		for (int x=0;x<w;x+=2) {
			unsigned int accum =
				data[4*(w*y+x)+3] +	
				data[4*(w*y+x+1)+3] +
				data[4*(w*(y+1)+x)+3] +
				data[4*(w*(y+1)+x+1)+3];
			accum = accum/4;
			data[2*(w*y+x)+3]=accum;
		}
	}
}

void upscaleAlpha(unsigned char *data, int w, int h) {
	// NOTE: W and H -> original image size
	alloc(w,h);

	for (int y=0;y<h;y++) {
		for (int x=0;x<w;x++) {
			int _x = x/2;
			int _y = y/2;
			tempBuffer[w*y+x]=data[4*(w*_y+_x)+3];
		}
	}

	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {
			size_t pos=w*y+x;
			data[4*pos+3]=tempBuffer[pos];
		}
	}
}

void alphaErode(unsigned char *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	alloc(w,h);

	#pragma omp parallel for shared(data, kernel, tempBuffer, w, h)
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			int pos=w*y+x;
			int min=255;
			for (int i=0;i<5 && min!=0;++i) {
				int _y=y+i-2;
				if (_y>=0 || _y<h) {
					for (int j=0;j<5 && min!=0;++j) {
						int _x=x+j-2;
						if (kernel[i][j] && (_x>=0 || _x<w)) {
							int newPos = w*_y+_x;
							min = data[4*pos+3]<min?data[4*pos+3]:min;
						}
					}
				}
			}
			tempBuffer[pos]=min;
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=w*y+x;
			data[4*pos+3]=tempBuffer[pos];
		}
	}
}

void alphaDilate(unsigned char *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	alloc(w,h);

	#pragma omp parallel for shared(data, kernel, tempBuffer, w, h)
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			int pos=w*y+x;
			int max=0;
			for (int i=0;i<5 && max!=255;++i) {
				int _y=y+i-2;
				if (_y>=0 || _y<h) {
					for (int j=0;j<5 && max!=255;++j) {
						int _x=x+j-2;
						if (kernel[i][j] && (_x>=0 || _x<w)) {
							int newPos = w*_y+_x;
							max = data[4*pos+3]>max?data[4*pos+3]:max;
						}
					}
				}
			}
			tempBuffer[pos]=max;
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=w*y+x;
			data[4*pos+3]=tempBuffer[pos];
		}
	}
}

void alphaHisteresis(unsigned char *data, int w, int h,int lowThres, int highThres) {
	// First Pass
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			if (data[4*(w*y+x)+3]>=highThres) {
				data[4*(w*y+x)+3]=255;
				if (x!=w-1 && data[4*(w*y+x+1)+3]>=lowThres) {data[4*(w*y+x+1)+3]=255;}
				if (y!=h-1 && data[4*(w*(y+1)+x)+3]>=lowThres) {data[4*(w*(y+1)+x)+3]=255;}
				if (x!=w-1 && y!=h-1 && data[4*(w*(y+1)+x+1)+3]>=lowThres) {data[4*(w*(y+1)+x+1)+3]=255;}
			}
		}
	}
	// Second Pass
	for (int y=h-1;y>=0;--y) {
		for (int x=w-1;x>=0;--x) {			
			if (data[4*(w*y+x)+3]>=highThres) {
				data[4*(w*y+x)+3]=255;
				if (x!=0 && data[4*(w*y+x-1)+3]>=lowThres) {data[4*(w*y+x-1)+3]=255;}
				if (y!=0 && data[4*(w*(y-1)+x)+3]>=lowThres) {data[4*(w*(y-1)+x)+3]=255;}
				if (x!=0 && y!=0 && data[4*(w*(y-1)+x-1)+3]>=lowThres) {data[4*(w*(y-1)+x-1)+3]=255;}
			}
		}
	}
	// Cleanup
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			if (data[4*(w*y+x)+3]!=255) {
				data[4*(w*y+x)+3]=0;
			}
		}
	}
}

void alphaGaussianBlur(unsigned char *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=1;kernel[0][1]=4;kernel[0][2]=7;kernel[0][3]=4;kernel[0][4]=1;
	kernel[1][0]=4;kernel[1][1]=16;kernel[1][2]=26;kernel[1][3]=16;kernel[1][4]=4;
	kernel[2][0]=7;kernel[2][1]=26;kernel[2][2]=41;kernel[2][3]=26;kernel[2][4]=7;
	kernel[3][0]=4;kernel[3][1]=16;kernel[3][2]=26;kernel[3][3]=16;kernel[3][4]=4;
	kernel[4][0]=1;kernel[4][1]=4;kernel[4][2]=7;kernel[4][3]=4;kernel[4][4]=1;
	int scale=273;

	alloc(w,h);
	#pragma omp parallel for shared(data, kernel, tempBuffer, w, h)
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=w*y+x;
			if (y<=1 || x<=1 || x>=w-2 || y>=h-2) {
				tempBuffer[pos]=data[4*pos+3];
			}
			else {
				unsigned int accum=0;
				for (int i=0;i<5;++i) {
					for (int j=0;j<5;++j) {
						size_t newPos=w*(y+i-2)+x+j-2;
						accum+=kernel[i][j]*data[4*(newPos)+3];
					}
				}
				tempBuffer[pos]=accum/scale;
			}
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=w*y+x;
			data[4*pos+3]=tempBuffer[pos];
		}
	}
}