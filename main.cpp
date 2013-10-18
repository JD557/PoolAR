#ifdef _WIN32
#include <windows.h>
#include <GL/glew.h>
#else
#include <GL/gl.h>
#endif
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <cmath>
#include "model.hpp"

#include "bullet/include/btBulletCollisionCommon.h"

#if AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGB
	#define CHANNELS 3
	#define CH1 r
	#define CH2 g
	#define CH3 b
#elif AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGR
	#define CHANNELS 3
	#define CH1 b
	#define CH2 g
	#define CH3 r
#elif AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_RGBA
	#define CHANNELS 4
	#define CH1 r
	#define CH2 g
	#define CH3 b
#elif AR_DEFAULT_PIXEL_FORMAT == AR_PIXEL_FORMAT_BGRA
	#define CHANNELS 4
	#define CH1 b
	#define CH2 g
	#define CH3 r
#endif

#define MODEL_DEBUG true

//
// Camera configuration.
//
#ifdef _WIN32
char			*vconf = "Data\\WDM_camera_flipV.xml";
char            *cparam_name = "Data\\camera_para.dat";
char            *config_name = "Data\\marker.dat";
#else
char			*vconf = "v4l2src device=/dev/video0 use-fixed-fps=false ! ffmpegcolorspace ! capsfilter caps=video/x-raw-rgb,bpp=24,width=640,height=480 ! identity name=artoolkit ! fakesink";
char            *cparam_name    = "Data/camera_para.dat";
char            *config_name = "Data/marker.dat";
#endif

int             xsize, ysize;
int             thresh = 100;
int             icount = 0;

ARParam         cparam;
//char                *config_name = "Data/multi/marker.dat";
ARMultiMarkerInfoT  *config;

int             patt_id;
double          patt_width     = 80.0;
double          patt_center[2] = {0.0, 0.0};
double          patt_trans[3][4];

double    gl_para[16];
GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

static void   init(void);
static void   cleanup(void);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static void   drawObject( double trans1[3][4], double trans2[3][4], size_t marker );
static void   draw_table_always();

Model hole;
Model table;
Model ball;
GLuint videoTexture;

//mode debug
float xy_aspect = (float)640 / (float)720;
double cam_up_vec[] = { 0 , 1 , 0};
int VIEW_MODE=0, NR_VIEW_MODE=2;


double model_debug_camera[2][6]  = {0,0,192,-35,59,0,
								   0,-127,93,-38,79,-2};

double tx=model_debug_camera[VIEW_MODE][0],
	   ty=model_debug_camera[VIEW_MODE][1],
	   tz=model_debug_camera[VIEW_MODE][2],
	   mx=model_debug_camera[VIEW_MODE][3],
	   my=model_debug_camera[VIEW_MODE][4],
	   mz=model_debug_camera[VIEW_MODE][5];

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	init();

	table = Model("Assets/pool.obj");
	ball = Model("Assets/ball.obj");
	hole = newHole(25,6.15,6.15);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &videoTexture);
	glBindTexture(GL_TEXTURE_2D, 1);
	glDisable(GL_TEXTURE_2D);

    arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
	return (0);
}



static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1b ) {
        printf("*** %f (frame/sec)\n", (double)icount/arUtilTimer());
        cleanup();
        exit(0);
    }
	/*
	 if( key == 't' ) {
        printf("*** %f (frame/sec)\n", (double)icount/arUtilTimer());
        printf("Enter new threshold value (current = %d): ", thresh);
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
        icount = 0;
    }
	*/
    /* turn on and off the debug mode with right mouse */
    if( key == 'd' ) {
        printf("*** %f (frame/sec)\n", (double)icount/arUtilTimer());
        arDebug = 1 - arDebug;
        if( arDebug == 0 ) {
            glClearColor( 0.0, 0.0, 0.0, 0.0 );
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
        }
        icount = 0;
    }

	if( key == '\\' ) {
        if(++VIEW_MODE==NR_VIEW_MODE)
			VIEW_MODE=0;

	   tx=model_debug_camera[VIEW_MODE][0];
	   ty=model_debug_camera[VIEW_MODE][1];
	   tz=model_debug_camera[VIEW_MODE][2];
	   mx=model_debug_camera[VIEW_MODE][3];
	   my=model_debug_camera[VIEW_MODE][4];
	   mz=model_debug_camera[VIEW_MODE][5];
    }
	
	if( key == 'q' ) {
        printf("x: %f\n", ++tx);
    }
	if( key == 'w' ) {
        printf("x: %f\n", ++ty);
    }
	if( key == 'e' ) {
        printf("x: %f\n", ++tz);
    }
	if( key == 'z' ) {
        printf("x: %f\n", --tx);
    }
	if( key == 'x' ) {
        printf("y: %f\n", --ty);
    }
	if( key == 'c' ) {
        printf("z: %f\n", --tz);
    }

	if( key == 'r' ) {
        printf("xxx: %f\n", ++mx);
    }
	if( key == 't' ) {
        printf("yyy: %f\n", ++my);
    }
	if( key == 'y' ) {
        printf("zzz: %f\n", ++mz);
    }
	if( key == 'v' ) {
        printf("xxx: %f\n", --mx);
    }
	if( key == 'b' ) {
        printf("yyy: %f\n", --my);
    }
	if( key == 'n' ) {
        printf("zzz: %f\n", --mz);
    }
	
}

void alphaErode(ARUint8 *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	ARUint8 *erodedBuffer=(ARUint8*)malloc(h*w);
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
	for (size_t y=0;y<h;++y) {
		for (size_t x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=erodedBuffer[pos];
		}
	}
	free(erodedBuffer);
}

void alphaDilate(ARUint8 *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=0;kernel[0][1]=1;kernel[0][2]=1;kernel[0][3]=1;kernel[0][4]=0;
	kernel[1][0]=1;kernel[1][1]=1;kernel[1][2]=1;kernel[1][3]=1;kernel[1][4]=1;
	kernel[2][0]=1;kernel[2][1]=1;kernel[2][2]=1;kernel[2][3]=1;kernel[2][4]=1;
	kernel[3][0]=1;kernel[3][1]=1;kernel[3][2]=1;kernel[3][3]=1;kernel[3][4]=1;
	kernel[4][0]=0;kernel[4][1]=1;kernel[4][2]=1;kernel[4][3]=1;kernel[4][4]=0;

	ARUint8 *dilatedBuffer=(ARUint8*)malloc(h*w);
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
	for (size_t y=0;y<h;++y) {
		for (size_t x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=dilatedBuffer[pos];
		}
	}
	free(dilatedBuffer);
}

void alphaHisteresis(ARUint8 *data, int w, int h,int lowThres, int highThres) {
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

void alphaGaussianBlur(ARUint8 *data, int w, int h) {
	int kernel[5][5];
	kernel[0][0]=1;kernel[0][1]=4;kernel[0][2]=7;kernel[0][3]=4;kernel[0][4]=1;
	kernel[1][0]=4;kernel[1][1]=16;kernel[1][2]=26;kernel[1][3]=16;kernel[1][4]=4;
	kernel[2][0]=7;kernel[2][1]=26;kernel[2][2]=41;kernel[2][3]=26;kernel[2][4]=7;
	kernel[3][0]=4;kernel[3][1]=16;kernel[3][2]=26;kernel[3][3]=16;kernel[3][4]=4;
	kernel[4][0]=1;kernel[4][1]=4;kernel[4][2]=7;kernel[4][3]=4;kernel[4][4]=1;
	int scale=273;

	ARUint8 *smoothBuffer=(ARUint8*)malloc(h*w);
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
	for (size_t y=0;y<h;++y) {
		for (size_t x=0;x<w;++x) {			
			size_t pos=640*y+x;
			data[4*pos+3]=smoothBuffer[pos];
		}
	}
	free(smoothBuffer);
}

void generateOverMask(ARUint8 *dataIn,ARUint8 *dataOut,int w, int h,int minSat,int maxSat) {
	for (size_t i=0;i<w*h;i++) {
		ARUint8 CH1=dataIn[CHANNELS*i];
		ARUint8 CH2=dataIn[CHANNELS*i+1];
		ARUint8 CH3=dataIn[CHANNELS*i+2];
		dataOut[4*i]=r;
		dataOut[4*i+1]=g;
		dataOut[4*i+2]=b;
		ARUint8 max=0;
		ARUint8 min=255;
		if (r>max) {max=r;}
		if (g>max) {max=g;}
		if (b>max) {max=b;}
		if (r<min) {min=r;}
		if (g<min) {min=g;}
		if (b<min) {min=b;}
		int hue;
		int sat=max-min;
		int value=max;
		if (max==r) {hue=60.0/255.0*(g-b);}
		else if (max==g) {hue=120+60.0/255.0*(b-r);}
		else if (max==b) {hue=240+60.0/255.0*(r-g);}
		if (hue<0) {hue+=360;}
		dataOut[4*i+3]=0;
		if (sat>minSat && (hue>300 || hue<60) && value>50) {
			int alpha=(sat-minSat)*255/maxSat;
			if (alpha<0) {alpha=0;}
			if (alpha>255) {alpha=255;}
			dataOut[4*i+3]=alpha;
		}
	}

	alphaGaussianBlur(dataOut,w,h);
	alphaHisteresis(dataOut,w,h,125,150);
	alphaDilate(dataOut,w,h);
	alphaErode(dataOut,w,h);
	alphaGaussianBlur(dataOut,w,h);
	/* ALPHA DEBUG */
	/*for (size_t i=0;i<w*h;i++) {
		dataOut[4*i]=dataOut[4*i+3];
		dataOut[4*i+1]=dataOut[4*i+3];
		dataOut[4*i+2]=dataOut[4*i+3];
	}*/

}

ARUint8 dataPtr2[640*480*4];



/* main loop */
static void mainLoop(void)
{
	if( !MODEL_DEBUG ){
		ARUint8         *dataPtr;
		//ARUint8         *dataPtr2=(ARUint8*)malloc(sizeof(ARUint8)*640*480*4);
		ARMarkerInfo    *marker_info;
		int             marker_num;
		double          err;

   
		if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
			arUtilSleep(2);
			return;
		}
		if( icount == 0 ) arUtilTimerReset();
		icount++;

		argDrawMode2D();
		argDispImage( dataPtr, 0,0 );

    
		if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
			cleanup();
			exit(0);
		}
	
		arVideoCapNext();

		if( (err=arMultiGetTransMat(marker_info, marker_num, config)) < 0 ) {
			argSwapBuffers();
			return;
		}
    
		if(err > 100.0 ) {
			argSwapBuffers();
			return;
		}
	
		argDrawMode3D();
		argDraw3dCamera( 0, 0 );
		glClearDepth( 1.0 );
		glClear(GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	
		for(int i = 0; i < config->marker_num; i++ ) {
		   drawObject( config->trans, config->marker[i].trans, i );
		}

		glDisable( GL_LIGHTING );
		glDisable( GL_DEPTH_TEST );

		generateOverMask(dataPtr,dataPtr2,640,480,15,30);
		argDrawMode2D();
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_ALPHA);
		glEnable(GL_BLEND);
		glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBindTexture(GL_TEXTURE_2D,videoTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataPtr2);
		glEnable(GL_COLOR_MATERIAL);
		int deltaY=240;
		glBegin(GL_QUADS);
			glTexCoord2d(0,1); glVertex2f(0, 0+deltaY);
			glTexCoord2d(1,1); glVertex2f(640, 0+deltaY);
			glTexCoord2d(1,0); glVertex2f(640, 480+deltaY);
			glTexCoord2d(0,0); glVertex2f(0, 480+deltaY);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_COLOR_MATERIAL);
		glDisable(GL_ALPHA);
		glDisable(GL_BLEND);
	
		argSwapBuffers();
	} else {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();	
		glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 500.0 );
		gluLookAt(tx,ty,tz,0,0,0,cam_up_vec[0],cam_up_vec[1],cam_up_vec[2]);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		draw_table_always();
		glutSwapBuffers();
		glFlush();
	}
}

static void init( void )
{

	 ARParam  wparam;

    /* open the video path */
    if( arVideoOpen( vconf ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (config = arMultiReadConfigFile(config_name)) == NULL ) {
        printf("config data load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 2, 1, 0 );
    arFittingMode   = AR_FITTING_TO_IDEAL;
    arImageProcMode = AR_IMAGE_PROC_IN_HALF;
    argDrawMode     = AR_DRAW_BY_TEXTURE_MAPPING;
    argTexmapMode   = AR_DRAW_TEXTURE_HALF_IMAGE;
}

/* cleanup function called when program exits */
static void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

static void drawObject( double trans1[3][4], double trans2[3][4], size_t marker )
{
    
	glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd( gl_para );
    argConvGlpara(trans2, gl_para);
    glMultMatrixd( gl_para );

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMatrixMode(GL_MODELVIEW);

	
	if (marker==0) {table.render();}

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glColorMask(0,0,0,0);
	hole.render();
	glCullFace(GL_FRONT);
	glColorMask(1,1,1,1);
	hole.render();
	glCullFace(GL_BACK);
	glDisable(GL_CULL_FACE);

}

GLfloat ambi2[]   = {0.5, 0.5, 0.5, 0.5};
static void draw_table_always(){

	
	glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi2);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    //glMatrixMode(GL_MODELVIEW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	glClearColor(0,1,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);

	table.render();
	glTranslated(mx,my,mz);
	table.render();
	
	//glColor3d(255,0,0);
	//glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	//glScaled(3,3,3);
	ball.render();
	//hole.render();
}