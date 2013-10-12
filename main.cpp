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
#include <math.h>


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
int             count = 0;

ARParam         cparam;
//char                *config_name = "Data/multi/marker.dat";
ARMultiMarkerInfoT  *config;

int             patt_id;
double          patt_width     = 80.0;
double          patt_center[2] = {0.0, 0.0};
double          patt_trans[3][4];

double    gl_para[16];
GLfloat   mat_ambient[]     = {0.7, 0.7, 0.7, 1.0};
GLfloat   mat_diffuse[]     = {0.9, 0.9, 0.9, 1.0};
GLfloat   mat_flash[]       = {0.3, 0.3, 0.3, 1.0};
GLfloat   mat_zero[]        = {0.1, 0.1, 0.1, 1.0};
GLfloat   mat_flash_shiny[] = {5.0};
GLfloat   light_position[]  = {100.0,-200.0,200.0,0.0};
GLfloat   ambi[]            = {0.1, 0.1, 0.1, 0.1};
GLfloat   lightZeroColor[]  = {0.9, 0.9, 0.9, 0.1};

static void   init(void);
static void   cleanup(void);
static void   keyEvent( unsigned char key, int x, int y);
static void   mainLoop(void);
static void   draw( double trans1[3][4], double trans2[3][4], int mode );

void holeMaterial() {
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
};
void zeroMaterial() {
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_zero);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_zero);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_zero);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_zero);
};

void drawHole(int sides, double radius, double depth) {
	int i=0;
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glColorMask(0,0,0,0);
	for (i=0;i<sides;++i) {
		double angle=2.0*3.14159*(double)i/(double)sides;
		double nextangle=2.0*3.14159*(double)(i+1)/(double)sides;
		glBegin(GL_QUADS);
			glVertex3f(radius*cos(angle),radius*sin(angle),0.0f);
			glVertex3f(radius*cos(angle),radius*sin(angle),-1.0*depth);
			glVertex3f(radius*cos(nextangle),radius*sin(nextangle),-1.0*depth);
			glVertex3f(radius*cos(nextangle),radius*sin(nextangle),0.0f);
		glEnd();
	}
	glColorMask(1,1,1,1);
	glCullFace(GL_FRONT);
	glBegin(GL_QUADS);
	for (i=0;i<sides;++i) {
		double angle=2.0*3.14159*(double)i/(double)sides;
		double nextangle=2.0*3.14159*(double)(i+1)/(double)sides;
		glNormal3f(-cos(angle),-sin(angle),0.0f);
		holeMaterial();
		glVertex3f(radius*cos(angle),radius*sin(angle),0.0f);
		zeroMaterial();
		glVertex3f(radius*cos(angle),radius*sin(angle),-1.0*depth);
		glNormal3f(-cos(nextangle),-sin(nextangle),0.0f);
		glVertex3f(radius*cos(nextangle),radius*sin(nextangle),-1.0*depth);
		holeMaterial();
		glVertex3f(radius*cos(nextangle),radius*sin(nextangle),0.0f);
	}
	glEnd();
	glCullFace(GL_BACK);
	glBegin(GL_POLYGON);
	zeroMaterial();
	for (i=0;i<sides;++i) {
		double angle=2.0*3.14159*(double)i/(double)sides;
		double nextangle=2.0*3.14159*(double)(i+1)/(double)sides;
		glNormal3f(0.0,0.0,1.0);
		glVertex3f(radius*cos(angle),radius*sin(angle),-1.0*depth);
	}
	glEnd();
	glDisable(GL_CULL_FACE);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	init();

    arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
	return (0);
}

static void   keyEvent( unsigned char key, int x, int y)
{
    /* quit if the ESC key is pressed */
    if( key == 0x1b ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        cleanup();
        exit(0);
    }
	/*
	 if( key == 't' ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        printf("Enter new threshold value (current = %d): ", thresh);
        scanf("%d",&thresh); while( getchar()!='\n' );
        printf("\n");
        count = 0;
    }
	*/
    /* turn on and off the debug mode with right mouse */
    if( key == 'd' ) {
        printf("*** %f (frame/sec)\n", (double)count/arUtilTimer());
        arDebug = 1 - arDebug;
        if( arDebug == 0 ) {
            glClearColor( 0.0, 0.0, 0.0, 0.0 );
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
            glClear(GL_COLOR_BUFFER_BIT);
            argSwapBuffers();
        }
        count = 0;
    }
}

/* main loop */
static void mainLoop(void)
{
    ARUint8         *dataPtr;
    ARMarkerInfo    *marker_info;
    int             marker_num;
    //int             j, k;
	double          err;
    int             i;

    /* grab a vide frame */
    if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
        arUtilSleep(2);
        return;
    }
    if( count == 0 ) arUtilTimerReset();
    count++;

    argDrawMode2D();
    argDispImage( dataPtr, 0,0 );

    /* detect the markers in the video frame */
    if( arDetectMarker(dataPtr, thresh, &marker_info, &marker_num) < 0 ) {
        cleanup();
        exit(0);
    }
	printf("num = %d\n", marker_num);
    arVideoCapNext();

	if( (err=arMultiGetTransMat(marker_info, marker_num, config)) < 0 ) {
		argSwapBuffers();
        return;
    }
    printf("err = %f\n", err);
    if(err > 100.0 ) {
        argSwapBuffers();
        return;
    }

    /* check for object visibility */
    /*k = -1;
    for( j = 0; j < marker_num; j++ ) {
        if( patt_id == marker_info[j].id ) {
            if( k == -1 ) k = j;
            else if( marker_info[k].cf < marker_info[j].cf ) k = j;
        }
    }
    if( k == -1 ) {
        argSwapBuffers();
        return;
    }
	*/
    /* get the transformation between the marker and the real camera */
    /*arGetTransMat(&marker_info[k], patt_center, patt_width, patt_trans);

    draw();*/

	argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
	
    for( i = 0; i < config->marker_num; i++ ) {
       draw( config->trans, config->marker[i].trans, 0 );
    }

    argSwapBuffers();
}

static void init( void )
{
	/*
    ARParam  wparam;
	
   
    if( arVideoOpen( vconf ) < 0 ) exit(0);
   
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    
    if( arParamLoad(cparam_name, 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (patt_id=arLoadPatt(patt_1)) < 0 ) {
        printf("pattern load error !!\n");
        exit(0);
    }

    
    argInit( &cparam, 1.0, 0, 0, 0, 0 );
*/

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

static void draw( double trans1[3][4], double trans2[3][4], int mode )
{
    argDrawMode3D();
    argDraw3dCamera( 0, 0 );
    glClearDepth( 1.0 );
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
	glMatrixMode(GL_MODELVIEW);
    argConvGlpara(trans1, gl_para);
    glLoadMatrixd( gl_para );
    argConvGlpara(trans2, gl_para);
    glMultMatrixd( gl_para );

    /* load the camera transformation matrix 
    argConvGlpara(patt_trans, gl_para);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd( gl_para );
*/

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_flash);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_flash_shiny);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMatrixMode(GL_MODELVIEW);

	drawHole(25,15.0,15.0);

    glDisable( GL_LIGHTING );

    glDisable( GL_DEPTH_TEST );
}
