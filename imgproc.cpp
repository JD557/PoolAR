#include "imgproc.hpp"

void alphaErode(unsigned char *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	unsigned char *erodedBuffer=(unsigned char*)malloc(h*w);
	for(int i=0; i<h*w; ++i)  {erodedBuffer[i]=255;}

	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			if (y<=1 || x<=1 || x>=w-2 || y>=h-2) {
				erodedBuffer[pos]=data[4*pos+3];
			}
			else if (data[4*pos+3]==0) {
				for (int i=0;i<5;++i) {
					for (int j=0;j<5;++j) {
						size_t newPos=640*(y+i-2)+x+j-2;
						if (kernel[i][j]==1) {
							erodedBuffer[newPos]=0;
						}
					}
				}
			}
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=erodedBuffer[pos];
		}
	}
	free(erodedBuffer);
}

void alphaDilate(unsigned char *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	unsigned char *dilatedBuffer=(unsigned char*)malloc(h*w);
	for(int i=0; i<h*w; ++i)  {dilatedBuffer[i]=0;}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			if (y<=1 || x<=1 || x>=w-2 || y>=h-2) {
				dilatedBuffer[pos]=data[4*pos+3];
			}
			else if (data[4*pos+3]==255) {
				for (int i=0;i<5;++i) {
					for (int j=0;j<5;++j) {
						size_t newPos=640*(y+i-2)+x+j-2;
						if (kernel[i][j]==1) {
							dilatedBuffer[newPos]=255;
						}
					}
				}
			}
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=dilatedBuffer[pos];
		}
	}
	free(dilatedBuffer);
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

	unsigned char *smoothBuffer=(unsigned char*)malloc(h*w);
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			if (y<=1 || x<=1 || x>=w-2 || y>=h-2) {
				smoothBuffer[pos]=data[4*pos+3];
			}
			else {
				unsigned int accum=0;
				for (int i=0;i<5;++i) {
					for (int j=0;j<5;++j) {
						size_t newPos=640*(y+i-2)+x+j-2;
						accum+=kernel[i][j]*data[4*(newPos)+3];
					}
				}
				smoothBuffer[pos]=accum/scale;
			}
		}
	}
	for (int y=0;y<h;++y) {
		for (int x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=smoothBuffer[pos];
		}
	}
	free(smoothBuffer);
}