// FBO_Scene.cpp : Defines the entry point for the console application.
//
 
#include <windows.h>
#include <gl/glew.h>
#include <gl/glut.h>
 
#define TEXTURE_WIDTH        256
#define TEXTURE_HEIGHT        256
 
//        Global Section     //
/////////////////////////////
GLuint fboID;
GLuint textureID;
GLuint rboID;
double ang;
bool isMSAA = false;
GLuint depthID;
GLuint frameBufID;
GLuint fbo2ID;
GLuint colorBuffer, depthBuffer, mfbo, texture1, fbo;
int width = 500, height = 500;
/////////////////////////////
 
 
void createFBO();
void drawFBO();

void drawScene()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);  
	glDisable(GL_TEXTURE_2D);
 
    float amb[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
 
    float diff[] = {1.0, 1.0, 1.0, 0.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diff);
 
    float spec[] = {0.0, 1.0, 0.0, 0.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, spec);
 
    const GLfloat blue[4] = { 0.0f, 0.3f, 1.f, 1.f };
    glMaterialfv( GL_FRONT, GL_DIFFUSE, blue );
    glMaterialfv( GL_FRONT, GL_SPECULAR, blue );
    glMaterialf( GL_FRONT, GL_SHININESS, 120.0);
 
    float position[] = {15, 15, 15, 1.0};
    //glLightfv(GL_LIGHT0, GL_POSITION, position);
 
    glColor3f(0.0, 1.0, 0.0);
    glutSolidTeapot(4.0);   
}
 
void drawQuad()
{
    glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2d(-8, -8);
        glTexCoord2f(0, 1); glVertex2d(-8, 8);
        glTexCoord2f(1, 1); glVertex2d(8, 8);
        glTexCoord2f(1, 0); glVertex2d(8, -8);
    glEnd();
}
 

void idle()
{
    glutPostRedisplay();
    Sleep(10);
}
 
void display()
{
 
     drawFBO();
	 glutSwapBuffers();
}
 
void createTexture()
{       
    // create a texture object
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGenerateMipmap  (GL_TEXTURE_2D);        

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); // No data.
    glBindTexture(GL_TEXTURE_2D, 0); // Not yet bound to anything.
 
    // create a depth texture object
    glGenTextures(1, &depthID);
    glBindTexture(GL_TEXTURE_2D, depthID);
    
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, TEXTURE_WIDTH,
		          TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL );
}
 

void drawFBO()
{
    // rendering procedure
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, mfbo);
    glPushAttrib(GL_VIEWPORT_BIT);
    
    glViewport(0, 0, width, height);
    
    // Draw scene.
    glClearColor(1.0, 1.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(ang += 1, 1, 1, 1);

    drawScene();

    glPopAttrib();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, mfbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    // you have to unbind all fbos before you can render to the main window
    glBindFramebuffer(GL_FRAMEBUFFER, 0);    
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //glGenerateMipmap(GL_TEXTURE_2D);
    glBegin(GL_QUADS); 
        glTexCoord2i(0, 0); glVertex2i(-5, -5);
        glTexCoord2i(0, 1); glVertex2i(-5, 5);
        glTexCoord2i(1, 1); glVertex2i(5, 5);
        glTexCoord2i(1, 0); glVertex2i(5, -5);
    glEnd();
    
}

void createFBO()
{
    // multi sampled color buffer
    glGenRenderbuffers(1, &colorBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4 /* 4 samples */, GL_RGBA8, width, height);

    // multi sampled depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4 /* 4 samples */, GL_DEPTH_COMPONENT, width, height);

    // create fbo for multi sampled content and attach depth and color buffers to it
    glGenFramebuffers(1, &mfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, mfbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // create texture
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // set your texture parameters here if required ...

    // create final fbo and attach texture to it
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER_EXT, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);        
}
 
void init()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -10, 10);
 
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
 
    //createTexture();
    createFBO();
}
 
void keyboard(unsigned char c, int x, int y)
{
        isMSAA = !isMSAA;
 
        if (isMSAA) {
                glEnable(GL_MULTISAMPLE);
        } else {
                glDisable(GL_MULTISAMPLE);
        }
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
 
    glutInitWindowSize(500, 500);
    glutCreateWindow("FBO");
 
    glewInit();
    init();
 
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboard);
 
    glutMainLoop();
 
    return 0;
}
