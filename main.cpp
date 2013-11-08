#ifdef _WIN32
#include <windows.h>
#include <GL/glew.h>
#pragma comment(lib,"glew32.lib")
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <GL/glut.h>
#include <AR/gsub.h>
#include <AR/video.h>
#include <AR/param.h>
#include <AR/ar.h>
#include <AR/arMulti.h>
#include <cmath>
#include "model.hpp"
#include "physics.hpp"
#include "imgproc.hpp"
#include "light.hpp"

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
#define MODEL_DEBUG 0

//
// Camera configuration.
//
#ifdef _WIN32
string vconf       = "Data\\WDM_camera_flipV.xml";
string cparam_name = "Data\\camera_para.dat";
string config_name = "Data\\marker.dat";
char *club_path      = "Data\\club.pat";
#else
string vconf          = "v4l2src device=/dev/video1 use-fixed-fps=false ! ffmpegcolorspace ! capsfilter caps=video/x-raw-rgb,bpp=24,width=640,height=480 ! identity name=artoolkit ! fakesink";
string cparam_name    = "Data/camera_para.dat";
string config_name    = "Data/marker.dat";
char *club_path       = "Data/club.pat";
#endif

int             club_id;
double          club_width     = 20.0;
double          club_center[2] = {0.0, 0.0};
double          club_trans[3][4];

int xsize, ysize;
int thresh = 100;
int icount = 0;

int FRAME_RATE     = 60;
btScalar TICK_RATE = 1.0/FRAME_RATE;
int  DELTA_T       = 1000.0/FRAME_RATE;


GLUquadric* glQ;
ARParam cparam;
ARMultiMarkerInfoT *config;

double    gl_para[16];

void init(void);
void cleanup(void);
void keyEvent( unsigned char key, int x, int y);
void mainLoop(void);
void drawObject( double trans1[3][4], double trans2[3][4], int renderMode);
void tick(int a);
void loadModels();

Model hole;
Model table;
Model ball;
Model club;

GLuint videoTexture;

Light mainLight;

Physics world;

	//world. ->setInternalTickCallback(myTickCallback);

ARUint8 overlayBuffer[640*480*4];
ARUint8 *doubleBuffer;

//mode debug
#if MODEL_DEBUG
void draw_table_always(int renderMode);
float xy_aspect = (float)640 / (float)720;
double cam_up_vec[] = { 0 , 1 , 0};
int VIEW_MODE=0, NR_VIEW_MODE=2;


double model_debug_camera[2][6]  = {{0,0,192,-35,59,0},
								   {0,-127,93,-38,79,-2}};

double tx=model_debug_camera[VIEW_MODE][0],
	   ty=model_debug_camera[VIEW_MODE][1],
	   tz=model_debug_camera[VIEW_MODE][2],
	   mx=model_debug_camera[VIEW_MODE][3],
	   my=model_debug_camera[VIEW_MODE][4],
	   mz=model_debug_camera[VIEW_MODE][5];
#endif

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	init();

    arVideoCapStart();
    argMainLoop( NULL, keyEvent, mainLoop );
	return (0);
}

double club_x=0,club_y=0,club_z=0;

void   keyEvent( unsigned char key, int x, int y)
{
    switch (key) {
    	case 0x1b: // ESC - quit
	        printf("*** %f (frame/sec)\n", (double)icount/arUtilTimer());
        	cleanup();
        	exit(0);
        	break;
        case 'd': // Debug mode
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
        	break;
		case '1': // Move ball
    		world.getBalls()[0]->forceActivationState(1);
			world.getBalls()[0]->setLinearVelocity(btVector3(-1,0,-1)*40);
			break;
	    case '2': // Move ball
			world.getBalls()[0]->forceActivationState(1);
			world.getBalls()[0]->setLinearVelocity(btVector3(1,0,1)*40);
		break;
		case 'o':club_x=20;club_y=20;club_z=20;;world.updateClub(club_x,club_y,club_z);break;
		case 'k':club_x=10;club_y=10;club_z=10;;world.updateClub(club_x,club_y,club_z);break;
		case 'l':--club_x;world.updateClub(club_x,club_y,club_z);break;
		case '�':--club_z;world.updateClub(club_x,club_y,club_z);break;
    #if MODEL_DEBUG // MODEL DEBUG ONLY KEYS
        case '\\': // Change view mode
        	if(++VIEW_MODE==NR_VIEW_MODE) {VIEW_MODE=0;}
			
			tx=model_debug_camera[VIEW_MODE][0];
			ty=model_debug_camera[VIEW_MODE][1];
			tz=model_debug_camera[VIEW_MODE][2];
			mx=model_debug_camera[VIEW_MODE][3];
			my=model_debug_camera[VIEW_MODE][4];
			mz=model_debug_camera[VIEW_MODE][5];
			break;

		case 'q':printf("x: %f\n", ++tx);break;
		case 'w':printf("x: %f\n", ++ty);break;
		case 'e':printf("x: %f\n", ++tz);break;
		case 'z':printf("x: %f\n", --tx);break;
		case 'x':printf("y: %f\n", --ty);break;
		case 'c':printf("y: %f\n", --ty);break;
		case 'r':printf("xxx: %f\n", ++mx);break;
		case 't':printf("yyy: %f\n", ++my);break;
		case 'y':printf("zzz: %f\n", ++mz);break;
		case 'v':printf("xxx: %f\n", --mx);break;
		case 'b':printf("yyy: %f\n", --my);break;
		case 'n':printf("zzz: %f\n", --mz);break;
		/*case 'o':club_x+=20;world.updateClub(club_x,club_y,club_z);break;
		case 'k':++club_z;world.updateClub(club_x,club_y,club_z);break;
		case 'l':--club_x;world.updateClub(club_x,club_y,club_z);break;
		case '�':--club_z;world.updateClub(club_x,club_y,club_z);break;*/
	#endif
    }
}

void generateOverMask(ARUint8 *dataIn,ARUint8 *dataOut,int w, int h,int minSat,int maxSat) {
	for (int i=0;i<w*h;i++) {
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
		int hue=0;
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
	
	downscaleAlpha(dataOut,w,h);
	alphaGaussianBlur(dataOut,w/2,h/2);
	alphaHisteresis(dataOut,w/2,h/2,125,150);
	alphaDilate(dataOut,w/2,h/2);
	alphaErode(dataOut,w/2,h/2);
	upscaleAlpha(dataOut,w,h);
	alphaGaussianBlur(dataOut,w,h);
	//alphaHisteresis(dataOut,w,h,125,150);
	/* ALPHA DEBUG */
	/*for (size_t i=0;i<w*h;i++) {
		dataOut[4*i]=dataOut[4*i+3];
		dataOut[4*i+1]=dataOut[4*i+3];
		dataOut[4*i+2]=dataOut[4*i+3];
	}*/

}

void print_arr(double arr[3][4]) {
	for(int i=0;i< 3;++i) {
		for(int j = 0; j< 4; ++j) {
			printf("%f ", arr[i][j]);
		}

		printf("\n");
	}
	printf("\n");
}

void drawClub(double b1[3][4]){
	
	double mat1[3][4],mat2[3][4],mat3[3][4];

	
	//printf("\n");
	//print_arr(b2);

	arUtilMatInv(b1, mat2);
	arUtilMatMul(mat2, club_trans , mat1);

    //arUtilMatInv(mat1, mat2);
	//arUtilMatInv(mat1,mat3);
	//arUtilMatMul(club_trans, mat2 , mat1);
	//double x = mat3[0][3];
	//double y = mat3[1][3];
	//double z = mat3[2][3];
   // printf("%f %f %f\n",x,y, z);
	
	glPushMatrix();


		glRotated(90,0,0,1);

		argConvGlpara(mat1,gl_para);
		glLoadMatrixd(gl_para);
		
		GLfloat glMatrix[4][4];
		glGetFloatv(GL_MODELVIEW_MATRIX, &glMatrix[0][0]);

		double x = glMatrix[3][0];
		double y = glMatrix[3][1];
		double z = glMatrix[3][2];

		if(z<0)z=0;
			
		printf("%f %f %f\n",x,y, z);
	glPopMatrix();

	world.updateClub(x,z,-y);
	//world.updateClub(x,y,z);
	/*
	btScalar	m[16];
	glPushMatrix();
	btRigidBody* body=btRigidBody::upcast(world.club);
	if(body&&body->getMotionState())
	{
		btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
		myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
		glRotated(90,1,0,0);
			glMultMatrixf(m);
		ball.render();
	}
	glPopMatrix();
	printf("club: %f %f %f\n", world.club->getCenterOfMassPosition()[0],world.club->getCenterOfMassPosition()[1],world.club->getCenterOfMassPosition()[2]);
	*/
    //argConvGlpara(club_trans, gl_para);
    //glLoadMatrixd( gl_para );

	argConvGlpara(mat1, gl_para);
	glMultMatrixd(gl_para);

	glPushMatrix();
	glRotated(-90,0,0,1);
	club.render();
	glPopMatrix();
}

/* main loop */
void mainLoop(void)
{
	
#if MODEL_DEBUG == 0 //Normal Mode
	ARUint8         *dataPtr;
	ARMarkerInfo    *marker_info;
	int             marker_num;
	double          err;
	bool needsUpdate = false;


	if( (dataPtr = (ARUint8 *)arVideoGetImage()) == NULL ) {
		if(doubleBuffer==NULL) return;
		dataPtr = doubleBuffer;
	}
	else {
		doubleBuffer = dataPtr;
		needsUpdate = true;
	}
	if( icount == 0 ) arUtilTimerReset();
	icount++;
	argDrawMode2D();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

	bool club_visible=false;
	for(int i=0; i<marker_num; ++i){
		if(marker_info[i].id==club_id){
			arGetTransMat(&marker_info[i], club_center, club_width, club_trans);
			club_visible=true;
		}
	}
	

	if (needsUpdate) {
		generateOverMask(dataPtr,overlayBuffer,640,480,15,30);
		glBindTexture(GL_TEXTURE_2D,videoTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); // Linear Filtering
		glTexImage2D(GL_TEXTURE_2D, 0, 4, 640, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, overlayBuffer);
	}

	argDrawMode3D();
	argDraw3dCamera( 0, 0 );
	//glClearDepth( 1.00 );
	glEnable(GL_ALPHA);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_LIGHTING);

	glMatrixMode(GL_MODELVIEW);
    argConvGlpara(config->trans, gl_para);
    glLoadMatrixd( gl_para );

    glGetFloatv(GL_MODELVIEW_MATRIX, Model::cameraMatrix);
	

	//CLUB
	if(club_visible) {
		glPushMatrix();
		drawClub(config->trans);
		glPopMatrix();
	}

	// RENDER SCENE
	glPushMatrix();
	drawObject( config->trans, config->marker[0].trans, 0);
	glPopMatrix();
	

	// RENDER SHADOWS

    glEnable(GL_STENCIL_TEST);
	glEnable(GL_CULL_FACE);
	glColorMask(0,0,0,0);
	glStencilFunc(GL_ALWAYS,0,0xFFFFFFFFL);
	glDepthMask( GL_FALSE );
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	glCullFace(GL_BACK);
	glPushMatrix();
	drawObject( config->trans, config->marker[0].trans, 1);
	glPopMatrix();
	glCullFace(GL_FRONT);
	glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
	glPushMatrix();
	drawObject( config->trans, config->marker[0].trans, 1);
	glPopMatrix();
	glColorMask(1,1,1,1);
	glStencilFunc( GL_NOTEQUAL, 0, 0xFFFFFFFFL );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	glDisable( GL_LIGHTING );
	glDisable( GL_DEPTH_TEST );

	glPushMatrix();
    glLoadIdentity();
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glColor4f(0.0,0.0,0.0,0.5);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_QUADS );
        glVertex3f(-30, 30,50);
        glVertex3f(-30,-30,50);
        glVertex3f( 30,-30,50);
        glVertex3f( 30, 30,50);
    glEnd();
    glDisable(GL_BLEND);
    glPopMatrix();
	glDepthMask( GL_TRUE );
	glDisable(GL_STENCIL_TEST);
	glColor3f(1.0,1.0,1.0);
	glDepthMask( GL_TRUE );

	argDrawMode2D();
	int deltaY=240;
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_ALPHA);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindTexture(GL_TEXTURE_2D,videoTexture);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glEnable(GL_COLOR_MATERIAL);
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
#else // Debug Mode
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();	
	glFrustum( -xy_aspect*.04, xy_aspect*.04, -.04, .04, .1, 500.0 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt(tx,ty,tz,0,0,0,cam_up_vec[0],cam_up_vec[1],cam_up_vec[2]);
	glEnable(GL_LIGHTING);
	mainLight.use();
	glPointSize(10.0);
	glBegin(GL_POINTS);
		glVertex3f(mainLight.getPos().x,mainLight.getPos().y,mainLight.getPos().z);
	glEnd();
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, Model::cameraMatrix);
	glDisable(GL_STENCIL_TEST);
	glCullFace(GL_FRONT);
	draw_table_always(0);
	glPopMatrix();

	/*glPushMatrix();
	//glCullFace(GL_FRONT);
	draw_table_always(1);
	glPopMatrix();*/

	glEnable(GL_CULL_FACE);
	glColorMask(0,0,0,0);
	glStencilFunc(GL_ALWAYS,0,0xFFFFFFFFL);
	glDepthMask( GL_FALSE );
	glStencilOp(GL_KEEP,GL_KEEP,GL_INCR);
	glCullFace(GL_BACK);
	glPushMatrix();
	draw_table_always(1);
	glPopMatrix();
	glCullFace(GL_FRONT);
	glStencilOp(GL_KEEP,GL_KEEP,GL_DECR);
	glPushMatrix();
	draw_table_always(1);
	glPopMatrix();
	glColorMask(1,1,1,1);
	glStencilFunc( GL_NOTEQUAL, 0, 0xFFFFFFFFL );
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

   	glPushMatrix();
    glLoadIdentity();
    glDisable(GL_CULL_FACE);
    glDisable(GL_LIGHTING);
    glColor4f(0.0,0.0,0.0,0.5);
    glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glBegin( GL_TRIANGLE_STRIP );
        glVertex3f(-0.1f, 0.1f,-0.10f);
        glVertex3f(-0.1f,-0.1f,-0.10f);
        glVertex3f( 0.1f, 0.1f,-0.10f);
        glVertex3f( 0.1f,-0.1f,-0.10f);
    glEnd();
    glDisable(GL_BLEND);
    glPopMatrix();
	glDepthMask( GL_TRUE );
	glutSwapBuffers();
	glFlush();
	glDisable(GL_STENCIL_TEST);
	glutSwapBuffers();
	glFlush();
#endif
	
}

void init( void )
{

	 ARParam  wparam;

    /* open the video path */
    if( arVideoOpen( &vconf[0] ) < 0 ) exit(0);
    /* find the size of the window */
    if( arVideoInqSize(&xsize, &ysize) < 0 ) exit(0);
    printf("Image size (x,y) = (%d,%d)\n", xsize, ysize);

    /* set the initial camera parameters */
    if( arParamLoad(cparam_name.c_str(), 1, &wparam) < 0 ) {
        printf("Camera parameter load error !!\n");
        exit(0);
    }
    arParamChangeSize( &wparam, xsize, ysize, &cparam );
    arInitCparam( &cparam );
    printf("*** Camera Parameter ***\n");
    arParamDisp( &cparam );

    if( (config = arMultiReadConfigFile(config_name.c_str())) == NULL ) {
        printf("config data load error !!\n");
        exit(0);
    }

	if( (club_id=arLoadPatt(club_path)) < 0 ) {
        printf("pattern load error !!\n");
        exit(0);
    }

    /* open the graphics window */
    argInit( &cparam, 1.0, 0, 2, 1, 0 );
    arFittingMode   = AR_FITTING_TO_IDEAL;
    arImageProcMode = AR_IMAGE_PROC_IN_HALF;
    argDrawMode     = AR_DRAW_BY_TEXTURE_MAPPING;
    argTexmapMode   = AR_DRAW_TEXTURE_HALF_IMAGE;

    // Setup Physics
	glutTimerFunc(0, tick, 0);

	// Setup VBOs
	glewInit();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	// Setup Lights
	//mainLight.setPosition(100.0,-200.0,200.0);
	mainLight.setPosition(200.0,-50.0,200.0);
	mainLight.setAmbient(0.1, 0.1, 0.1);
	mainLight.setColor(0.9, 0.9, 0.9);

	// Setup Models
	glQ = gluNewQuadric();
	loadModels();

	// Setup overdraw
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &videoTexture);
	glBindTexture(GL_TEXTURE_2D, 1);
	glDisable(GL_TEXTURE_2D);


}

/* cleanup function called when program exits */
void cleanup(void)
{
    arVideoCapStop();
    arVideoClose();
    argCleanup();
}

void renderHoles() {

	glPushMatrix();
	glTranslated(0.0,0.0,0.0);
	hole.render();
	glPopMatrix();

	glPushMatrix();
	glTranslated(74.92,0.0,0.0);
	hole.render();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0,-59.23,0.0);
	hole.render();
	glPopMatrix();

	glPushMatrix();
	glTranslated(74.92,-59.23,0.0);
	hole.render();
	glPopMatrix();


	glPushMatrix();
	glTranslated(74.92,-118.47,0.0);
	hole.render();
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0,-118.47,0.0);
	hole.render();
	glPopMatrix();
}

void drawObject( double trans1[3][4], double trans2[3][4], int renderMode)
{
    
    if (renderMode==0) {

	    glEnable(GL_LIGHTING);
	    mainLight.use();

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glColorMask(0,0,0,0);
		renderHoles();
		glColorMask(1,1,1,1);

		table.render();

		btScalar	m[16];
		for(int i=0; i< world.getBalls().size(); ++i){
			glPushMatrix();
			btRigidBody* body=btRigidBody::upcast(world.getBalls()[i]);
			if(body&&body->getMotionState())
			{
				btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
				myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			}
			glRotated(90,1,0,0);
			glMultMatrixf(m);
			ball.render();
			glPopMatrix();
		}

		glCullFace(GL_FRONT);
		renderHoles();
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
	}
	else if (renderMode==1) {

	    glDisable(GL_LIGHTING);

		glEnable(GL_CULL_FACE);

		//table.renderShadow(mainLight.getPos());

		btScalar	m[16];
		for(int i=0; i< world.getBalls().size(); ++i){
			glPushMatrix();
			btRigidBody* body=btRigidBody::upcast(world.getBalls()[i]);
			if(body&&body->getMotionState())
			{
				btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
				myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			}
			glRotated(90,1,0,0);
			glMultMatrixf(m);
			ball.renderShadow(mainLight.getPos());
			glPopMatrix();
		}
	}
}

#if MODEL_DEBUG
void draw_table_always(int renderMode){

	if (renderMode==0) {
		glTranslated(mx,my,mz);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glColorMask(0,0,0,0);
		hole.render();
		glColorMask(1,1,1,1);
		table.render();

		btScalar	m[16];
		
		for(int i=0; i< world.getBalls().size(); ++i){
			glPushMatrix();

			btRigidBody* body=btRigidBody::upcast(world.getBalls()[i]);
			if(body&&body->getMotionState())
			{
				btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
				myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			}
		
			glRotated(90,1,0,0);
			glMultMatrixf(m);
		
			ball.render();
			glPopMatrix();
		}
		glCullFace(GL_FRONT);
		hole.render();
		glCullFace(GL_BACK);
		glDisable(GL_CULL_FACE);
	}
	else if (renderMode==1) {
		glTranslated(mx,my,mz);

		table.renderShadow(mainLight.getPos());

		btScalar	m[16];
		
		for(int i=0; i< world.getBalls().size(); ++i){
			glPushMatrix();

			btRigidBody* body=btRigidBody::upcast(world.getBalls()[i]);
			if(body&&body->getMotionState())
			{
				btDefaultMotionState* myMotionState = (btDefaultMotionState*)body->getMotionState();
				myMotionState->m_graphicsWorldTrans.getOpenGLMatrix(m);
			}
		
			glRotated(90,1,0,0);
			glMultMatrixf(m);
		
			ball.renderShadow(mainLight.getPos());
			glPopMatrix();
		}
	}
}
#endif

void loadModels() {
	table = Model("Assets/pool.obj");
	ball = Model("Assets/ball.obj");
	club = Model("Assets/club.obj");
	hole = newHole(25,6.15,6.15);
}

size_t lastTick = 0;

void tick(int a){
	//world.dynamicsWorld->stepSimulation(TICK_RATE);
	world.dynamicsWorld->stepSimulation((glutGet(GLUT_ELAPSED_TIME)-lastTick)/1000.0,7);
	lastTick = glutGet(GLUT_ELAPSED_TIME);
	glutTimerFunc(DELTA_T, tick, 0);
}