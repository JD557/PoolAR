#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#ifndef __APPLE__
#include <GL/gl.h>
#include <GL/glut.h>
#else
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#endif
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <cmath>
#include "model.hpp"


//
// Camera configuration.
//
#ifdef _WIN32
char			*vconf = "Data\\WDM_camera_flipV.xml";
char            *cparam_name = "Data\\camera_para.dat";
char            *config_name = "Data\\marker.dat";
#else
char			*vconf = "v4l2src device=/dev/video1 use-fixed-fps=false ! ffmpegcolorspace ! capsfilter caps=video/x-raw-rgb,bpp=24,width=640,height=480 ! identity name=artoolkit ! fakesink";
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

Model hole;
Model table;
GLuint videoTexture;

int main(int argc, char **argv)
{

	glutInit(&argc, argv);
	init();

	table = Model("Assets/pool.obj");
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
}

void generateOverMask(ARUint8 *dataIn,ARUint8 *dataOut,int w, int h,int minSat,int maxSat) {
	for (size_t i=0;i<w*h;i++) {
		ARUint8 r=dataIn[3*i];
		ARUint8 g=dataIn[3*i+1];
		ARUint8 b=dataIn[3*i+2];
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
		if ((max-min>minSat && (hue>300 || hue<60) && value>50)) {
			int alpha=(sat-minSat)*255/maxSat;
			if (alpha<0) {alpha=0;}
			if (alpha>255) {alpha=255;}
			dataOut[4*i+3]=alpha;
		}
	}

}

/* main loop */
static void mainLoop(void)
{
    ARUint8         *dataPtr;
	ARUint8         *dataPtr2=(ARUint8*)malloc(640*480*4);
    ARMarkerInfo    *marker_info;
    int             marker_num;
	double          err;

    /* grab a video frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    if( icount == 0 ) arUtilTimerReset();
    icount++;

    argDrawMode2D();
    argDispImage( dataPtr, 0,0 );

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }
	//printf("num = %d\n", marker_num);
    arVideoCapNext();

	if( (err=arMultiGetTransMat(marker_info, marker_num, config)) < 0 ) {
		argSwapBuffers();
        return;
    }
    //printf("err = %f\n", err);
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
	generateOverMask(dataPtr,dataPtr2,640,480,20,30);
	argDrawMode2D();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D,videoTexture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
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
	free(dataPtr2);
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
