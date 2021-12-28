/*
 Sasha Farhat
 CSCI 4229
 Fall 2021

Final Project: Futurama
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include<time.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
//  OpenGL with prototypes for glext
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
// Tell Xcode IDE to not gripe about OpenGL deprecation
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#else
#include <GL/glut.h>
#endif
//  Default resolution
//  For Retina displays compile with -DRES=2
#ifndef RES
#define RES 1
#endif

#ifdef __cplusplus
extern "C" {
#endif
    unsigned int LoadTexBMP(const char* file);
    int  LoadOBJ(const char* file);
#ifdef __cplusplus
}
#endif

//------------move ship--------------
float move = 0;
float move2 = 0;
float move3  = 0;
float move4 = 0;

//------------Light Values-------------
unsigned int texture[7]; // Texture names
int text = 0;


int light     =   1;  // Lighting
int local     =   0;  // Local Viewer Model
int distance  =   350;  // Light distance
int smooth    =   1;  // Smooth/Flat shading
int emission  =   0;  // Emission intensity (%)
int ambient   =  10;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   100;  // Elevation of light
int inc       =  10;  // Ball increment

int th=65;          //  Azimuth of view angle
int ph=10;          //  Elevation of view angle
int axes=1;        //  Display axes
int mode=0;       //  Projection mode
int fp = 0;       // first person view

int fov=65;       //  Field of view (for perspective)
double asp=5;     //  Aspect ratio
double dim=300;   //  Size of world

int angleY = 14;
float pos_x = 0, pos_y = 10, pos_z = -0; //starting position. y wont change
float cam_x = 582, cam_y = 0, cam_z = 0;

//  Cosine and Sine in degrees
#define Cos(x) (cos((x)*3.14159265/180))
#define Sin(x) (sin((x)*3.14159265/180))

/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible
 */
#define LEN 8192  //  Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}

/*
 *  Check for OpenGL errors
 */
void ErrCheck(const char* where)
{
   int err = glGetError();
   if (err) fprintf(stderr,"ERROR: %s [%s]\n",gluErrorString(err),where);
}

/*
 *  Print message to stderr and exit
 */
void Fatal(const char* format , ...)
{
   va_list args;
   va_start(args,format);
   vfprintf(stderr,format,args);
   va_end(args);
   exit(1);
}
static void Project()
{
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  First Person or Perspective transformation
   if (fp)
      gluPerspective(fov,asp,0.01, 400);
    else if(mode)
        gluPerspective(fov,asp,dim/4,4*dim);   //  Orthogonal projection
   else
      glOrtho(-asp*dim,+asp*dim, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
    glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r*80,r*80,r*80);
   //  White ball with yellow specular
   float yellow[]   = {1.0,1.0,0.0,1.0};
   float Emission[] = {0.0,0.0,0.01*emission,1.0};
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (int ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (int th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transofrmations
   glPopMatrix();
}
//------------------------------Function to add windows -----------------------------------------

void frontWindow(float x,float z,float y)
{
    float i,j;
    for(i= y-2 ; i>0; i = i-10)
    {
        for(j = -x/2 + 2; j < (x/2);j = j+10)
        {
            glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(j-2, i+2, -z/2-0.3);
            glVertex3f(j+2,i+2,-z/2-0.3);
            glVertex3f(j+2,i-2,-z/2-0.3);
            glVertex3f(j-2,i-2,-z/2-0.3);
            glEnd();
        }
    }
}

void backWindow(float x,float z,float y)
{
    float i,j;
    for(i = y - 2; i>0; i = i - 10)
    {
        for(j = -x/2 + 2; j < (x/2); j = j+10)
        {
            glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(j-2,i+2,z/2+0.3);
            glVertex3f(j+2,i+2,z/2+0.3);
            glVertex3f(j+2,i-2,z/2+0.3);
            glVertex3f(j-2,i-2,z/2+0.3);
            glEnd();
        }
    }

}


void leftWindow(float x,float z,float y)
{
    float i,j;
    for(i = y-2; i > 0; i = i-10)
    {
        for(j = -z/2 + 2; j < (z/2); j = j+10)
        {
            glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(-x/2-0.3,i+2,j-2);
            glVertex3f(-x/2-0.3,i+2,j+2);
            glVertex3f(-x/2-0.3,i-2,j+2);
            glVertex3f(-x/2-0.3,i-2,j-2);
            glEnd();
        }
    }
}


void rightWindow(float x,float z,float y)
{
    float i,j;
    for(i = y-2;i>0;i=i-10)
    {
        for(j = -z/2 + 2; j < (z/2); j = j+10)
        {
            glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(x/2+0.3,i+2,j-2);
            glVertex3f(x/2+0.3,i+2,j+2);
            glVertex3f(x/2+0.3,i-2,j+2);
            glVertex3f(x/2+0.3,i-2,j-2);
            glEnd();
        }
    }
}

//--------------------------------Generic Apartment Building-------------------------------------
void GenericBuilding(float x,float z,float y, float r, float g, float bl, float shiftX, float shiftY, float shiftZ)
{
    int l = x;
    int b = z;
    int h = y;
    
    x = x/2;
    z = z/2;
    
    
    glPushMatrix();
    glTranslatef(shiftX, shiftY, shiftZ);
    //bottom
    glColor3f(r, g, bl);
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glVertex3f(x,0,z);
    glVertex3f(x,0,-z);
    glVertex3f(-x,0,-z);
    glVertex3f(-x,0,+z);
    glEnd();
    //front
    glColor3f(r, g, bl);
    glBegin(GL_QUADS);
    glNormal3f( 0, 0,-1);
    glVertex3f(x,y,-z);
    glVertex3f(x,0,-z);
    glVertex3f(-x,0,-z);
    glVertex3f(-x,y,-z);
    glEnd();
    
    frontWindow(l,b,h); //window
    
    //left
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glVertex3f(-x,y,-z);
    glVertex3f(-x,y,z);
    glVertex3f(-x,0,z);
    glVertex3f(-x,0,-z);
    glEnd();
    
    leftWindow(l,b,h);//window
    
    //back
    glColor3f(r, g, bl);
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(-x,y,z);
    glVertex3f(x,y,z);
    glVertex3f(x,0,z);
    glVertex3f(-x,0,z);
    glEnd();
    
    backWindow(l,b,h);//window
    
    //right
    glBegin(GL_QUADS);
    glNormal3f(1, 1, 0);
    glVertex3f(x,y,z);
    glVertex3f(x,0,z);
    glVertex3f(x,0,-z);
    glVertex3f(x,y,-z);
    glEnd();
    
    rightWindow(l,b,h); //window
    
    //top
    glColor3f(r*0.5, g*0.5, bl);
    glBegin(GL_QUADS);
    glNormal3f(0, +1, 0);
    glVertex3f(x,y,z);
    glVertex3f(x,y,-z);
    glVertex3f(-x,y,-z);
    glVertex3f(-x,y,z);
    glEnd();
    glPopMatrix();
}


//--------------------------------Basic Shapes----------------------------------------------------
//cylinder
void cylinder(float x, float y, float z, float shiftX, float shiftY, float shiftZ, float r, float g, float b, float a, int t)
{
    //enable texture
    float white[] = {1,1,1,1};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,text?GL_REPLACE:GL_MODULATE);
    
    int u;
    
    //resource mentioned in ReadME
    float angleIncrement = (2.0f * M_PI) / 65;
    float textureCoordinateIncrement = 1.0f / 65;
    
    glPushMatrix();
    //translate and scale
    glTranslatef(shiftX, shiftY, shiftZ);
    glScalef(x, y, z);
    glBindTexture(GL_TEXTURE_2D,texture[t]);
    glBegin(GL_QUAD_STRIP);
    glColor4f(r, g, b, a);
    for(u = 0; u <=65; u+=1){
        float c = cos(angleIncrement * u)*10;
        float s = sin(angleIncrement * u)*10;
        glNormal3f(c, 1, s);
        glTexCoord2f( textureCoordinateIncrement * u, 0); glVertex3f( c, -1.0f, s);
        glNormal3f(c, -1, s);
        glTexCoord2f( textureCoordinateIncrement * u, 1.0f); glVertex3f( c, 75, s);
    }
    glEnd();
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

}

//triangle
void triangle(int shiftX, int shiftY, int shiftZ, float scaleX, float scaleY, float scaleZ, float rd, float gr, float bl){
    glPushMatrix();
    glTranslated(shiftX, shiftY, shiftZ);
    glScaled(scaleX, scaleY, scaleZ);
    glColor3f(rd, gr, bl);
    glBegin(GL_TRIANGLES);
    glNormal3f(0,0,1);
    glVertex3d(0,1,0);
    glNormal3f(0,1,0);
    glVertex3d(1.0, 0.0, 0);
    glNormal3f(1,0,0);
    glVertex3d(-1.0, 0.0, 0);
    glEnd();
    glPopMatrix();
}
void cone(float r, float g, float b, int shiftX, int shiftY, int shiftZ, int scaleX, int scaleY, int scaleZ){
    int k;
    glTranslatef(shiftX,shiftY,shiftZ);
    glRotated(th,0,1,0);
    glScaled(scaleX, scaleY, scaleZ);
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    for(k = 0; k <= 360; k += 10){
        glColor3f(r, g, b);
        glNormal3f(0,1,0);
        glVertex3f(0, 4, 0);
        glNormal3f(Cos(k), 1, Sin(k));
        glVertex3f(Cos(k), 3, Sin(k));
        glNormal3f(Cos(k + 10), 1, Sin(k + 10));
        glVertex3f(Cos(k + 10), 3, Sin(k + 10));
        }
    glEnd();
    glPopMatrix();
}

static void quad(float r, float g, float b, double dx, double dy, double dz, double shiftX, double shiftY, double shiftZ, int t)
{
    float white[] = {1,1,1,1};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,text?GL_REPLACE:GL_MODULATE);
    glPushMatrix();
    //  Cube
    glColor3f(r,g,b);
    glBindTexture(GL_TEXTURE_2D,texture[t]);
    glTranslated(shiftX, shiftY, shiftZ);
    glScaled(dx, dy, dz);
    glBegin(GL_QUADS);
    //  Front
    glNormal3f(0, 0, 1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, 1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, 1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, 1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, 1);
    glEnd();

    //  Back
    glBegin(GL_QUADS);
     glColor3f(r,g,b);
    glNormal3f( 0, 0,-1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, -1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, -1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, -1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, -1);
    glEnd();
    //  Right
    glBegin(GL_QUADS);
    glColor3f(r,g,b);
    glNormal3f(1, 1, 0);
    glTexCoord3f(0,0,0); glVertex3f(+1,0,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(+1,+2,+1);
    glEnd();
    //  Left
    glBegin(GL_QUADS);
    glColor3f(r,g,b);
    glNormal3f(-1, 0, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(-1,0,+1);
    glTexCoord3f(1,1,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Top
    glBegin(GL_QUADS);
    glColor3f(r, g, b);
    glNormal3f(0, +1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,+2,+1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Bottom
    glBegin(GL_QUADS);
    glColor3f(r,g,b);
    glNormal3f(0, -1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,0,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,0,+1);
    glEnd();
    //  End
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void planetExpressGarage(int shiftX, int shiftY, int shiftZ, double x, double y, double z){
    glColor3f(0.963, 0.178, 0.335);
    float white[] = {1,1,1,1};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,text?GL_REPLACE:GL_MODULATE);
    glPushMatrix();
    //  Cube
    glBindTexture(GL_TEXTURE_2D,texture[3]);
    glTranslated(shiftX, shiftY, shiftZ);
    glScaled(x, y, z);
    glBegin(GL_QUADS);
    //  Front
    glNormal3f(0, 0, 1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, 1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, 1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, 1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, 1);
    glEnd();

    //  Back
    glBegin(GL_QUADS);
    glNormal3f( 0, 0,-1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, -1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, -1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, -1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, -1);
    glEnd();
    //  Right
    glBegin(GL_QUADS);
    glNormal3f(1, 1, 0);
    glTexCoord3f(0,0,0); glVertex3f(+1,0,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(+1,+2,+1);
    glEnd();
    //  Left
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(-1,0,+1);
    glTexCoord3f(1,1,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Top
    glBegin(GL_QUADS);
    glNormal3f(0, +1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,+2,+1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Bottom
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,0,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,0,+1);
    glEnd();
    //  End
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
    
    int u;
    glPushMatrix();
    glTranslatef(shiftX,shiftY+22,shiftZ-18);
    glScaled(1.5,1,.75);
    glRotated(90,0,0,1);
    glRotated(90,1,0,0);
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.596, 0.984, 0.596);
    for(u = 0; u <=360; u+=10){
        glNormal3f(Cos(u)*10, 1, (Sin(u)));
        glVertex3f(Cos(u)*10, +90, Sin(u)*10);
        glNormal3f(Cos(u), -1, Sin(u));
        glVertex3f(Cos(u)*10, 0, Sin(u)*10);
    }
    glEnd();
    glPopMatrix();
    
}

void rectangle(float x, float y, float z){
    glBegin(GL_POLYGON);
     glVertex3f(-x,0,z);
     glVertex3f(x,0,z);
     glVertex3f(x,0,-z);
     glVertex3f(-x,0,-z);
     glEnd();
}
//borrowed curved tube from https://stackoverflow.com/questions/52930454/opengl-glut-c-want-to-render-curved-cylinder

void curvedtube(int shiftX, int shiftY, int shiftZ, float x, float y, float z, float rd, float gr, float bl, float h, int one){
    GLfloat w0, w1, ang0, ang1, angle, x1, y1, xb, yb, zb;
    int i2, j2;

    int slices = 10;
    GLfloat bend_radius = 25.0f;

    GLfloat bend_angle, bend_ang0, bend_ang1;

    bend_angle = bend_radius * h; //for bent tube-> 20
    bend_ang0  = -bend_angle/2.0f;
    bend_ang1  = bend_angle/2.0f;

    for ( i2 = 0; i2 < slices; i2++ )
    {
        w0 = (float)i2 / (float)slices;
        w1 = (float)(i2+1) / (float)slices;

        ang0 = bend_ang0 + (bend_ang1-bend_ang0) * w0;
        ang1 = bend_ang0 + (bend_ang1-bend_ang0) * w1;
        
        glColor3f(rd, gr, bl);
        glPushMatrix();
        glTranslatef(shiftX, shiftY, shiftZ);
        glScaled(x, y, z);
        glRotatef(-90,one,0,0);
        glBegin(GL_QUAD_STRIP);

            for ( j2 = 0; j2 <= 360; ++ j2 )
            {
                angle = M_PI * (float)j2 * M_PI / 180.0f;
                x1 = 8 * cos(angle) + bend_radius;
                y1 = 8 * sin(angle);

                xb = sin( ang0 ) * x1;
                yb = y1;
                zb = cos( ang0 ) * x1;
                glNormal3f(xb, yb, zb);
                glVertex3f( xb, yb, zb );

                xb = sin( ang1 ) * x1;
                yb = y1;
                zb = cos( ang1 ) * x1;
                glNormal3f(xb, yb, zb);
                glVertex3f( xb, yb, zb );
            }
        glEnd();
        glPopMatrix();
    }
}

void road(){

    //pavement
    glColor3f(0,0,0);
    rectangle(20,0,20);

    //yellow strip
    glPushMatrix();
    glTranslatef(0,0.5,0);
    glColor3f(1,1,0);
    rectangle(20,0,0.5);
    glPopMatrix();
}

static void ground(){
    //  Set specular color to white
    float white[] = {1,1,1,1};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,text?GL_REPLACE:GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[4]);

    glColor4f(0.663, 0.663, 0.663, 0.8);
    glBegin(GL_POLYGON);
    glTexCoord3f(0,0,0); glVertex3f(-400,0,-400);
    glTexCoord3f(1,0,0); glVertex3f(-400,0,400);
    glTexCoord3f(1,1,0); glVertex3f(400,0,400);
    glTexCoord3f(0,1,0); glVertex3f(400,0,-400);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    //TODO: add water lines
    
    //water bay
    glColor3f(0.0,0.4,0.7);
    glBegin(GL_POLYGON);
    glVertex3f(-600,-10,-600);
    glVertex3f(-600,-10,600);
    glVertex3f(600,-10,600);
    glVertex3f(600,-10,-600);
    glEnd();
    glPopMatrix();
    
    
    
    //map
    float i;
    for(i=380;i>=-400;i-=40)
    {
        glPushMatrix();
        glTranslatef(i,0.5,0);
        road();
        glPopMatrix();
    }
    for(i=380;i>=-400;i-=40)
    {
        glPushMatrix();
        glTranslatef(i,0.5,55);
        glScaled(1,1,0.75);
        road();
        glPopMatrix();
    }
    for(i=380;i>=-400;i-=40)
    {
        glPushMatrix();
        glTranslatef(i,0.5,-55);
        glScaled(1,1,0.75);
        road();
        glPopMatrix();
    }
}

//--------------------------------BUILDING FUNCTIONS-----------------------------------------------
void twinBuilding(int x, int y, int z, int r, float shiftX, float shiftZ)
{
    glPushMatrix();
    cylinder(3,3,3, shiftX,0, shiftZ, 1,1,1,1,5);
    cylinder(3.2,0.27,3.2, shiftX,88, shiftZ, 0.2,0.4,1, 0.8,0);
    cylinder(3.2,0.27,3.2, shiftX,35, shiftZ, 0.2,0.4,1, 0.8,0);
    cylinder(3.2,0.27,3.2, shiftX,140, shiftZ, 0.2,0.4,1, 0.8,0);
    cylinder(3.2,0.27,3.2, shiftX,200, shiftZ, 0.2,0.4,1, 0.8,0);
    glPopMatrix();
    
    int i, j;
    GLfloat v[x * y][3];
    
    //half sphere
    for (i = 0; i < x; ++i) {
      for (j = 0; j < y; ++j) {
        v[i*y+j][0]=r*cos(j*2*M_PI/y)*cos(i*M_PI/(2*x));
        v[i*y+j][1]=r*sin(i*M_PI/(2*x));
        v[i*y+j][2]=r*sin(j*2*M_PI/y)*cos(i*M_PI/(2*x));
      }
    }
    glPushMatrix();
    glTranslated(shiftX,210, shiftZ);
    glBegin(GL_QUADS);
    glColor4f(0.2, 0.412, 1, 0.9);
      for (i=0; i<x-1; ++i) {
        for (j=0; j<y; ++j) {
            glNormal3f(Cos(i + 1), 1, Sin(i + 1));
            glVertex3fv(v[i*y+j]);
            glVertex3fv(v[i*y+(j+1)%y]);
            glVertex3fv(v[(i+1)*y+(j+1)%y]);
            glVertex3fv(v[(i+1)*y+j]);
            }
      }
    glEnd();
    glPopMatrix();

}

void planetExpress(int shiftX, int shiftY, int shiftZ, int x, int y, int z, int r, double lats, double longs){
        //sphere
        int i, j;
        for(i = 0; i <= lats; i++) {
            float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
            float z0  = sin(lat0);
            float zr0 =  cos(lat0);

            float lat1 = M_PI * (-0.5 + (float) i / lats);
            float z1 = sin(lat1);
            float zr1 = cos(lat1);
            glPushMatrix();
            glColor3f( 0.863, 0.078, 0.235);
            glTranslated(shiftX,shiftY+55,shiftZ);
            glBegin(GL_QUAD_STRIP);
            
            for(j = 0; j <= longs; j++) {
                float lng = 2 * M_PI * (float) (j - 1) / longs;
                float x = cos(lng);
                float y = sin(lng);
                glNormal3f(x * zr0, y * zr0, z0);
                glVertex3f(r * x * zr0, r * y * zr0, r * z0);
                glNormal3f(x * zr1, y * zr1, z1);
                glVertex3f(r * x * zr1, r * y * zr1, r * z1);
            }
            glEnd();
            glPopMatrix();
        }
    //balcony
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f(0.333, 0.420, 0.184);
        glTranslated(shiftX,shiftY+45,shiftZ);
        glScaled(1,0.5,1);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //balcony 2
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f(0.561, 0.737, 0.561);
        glTranslated(shiftX,shiftY+48,shiftZ);
        glScaled(1,0.3,1);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //body under the sphere (cylinder formula)
    glPushMatrix();
    glTranslated(shiftX,shiftY,shiftZ);
    int u1;
    glColor3f( 0.863, 0.078, 0.235);
    glBegin(GL_QUAD_STRIP);
    for(u1 = 0; u1 <=360; u1+=10){
        glNormal3f(Cos(u1)*10, 1, (Sin(u1)));
        glVertex3f(Cos(u1)*15, +40, Sin(u1)*15);
        glNormal3f(Cos(u1), -1, Sin(u1));
        glVertex3f(Cos(u1)*30, 0, Sin(u1)*30);
    }
    glEnd();
    glPopMatrix();
}

//planet express ship
void planetExpressShip(int shiftX, int shiftY, int shiftZ, int x, int y, int z, int r, double lats, double longs, float re, float g, float bl)
{
    //spaceship body
    int i, j;
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 0.000, 0.980, 0.604);
        glTranslated(shiftX,shiftY,shiftZ);
        glScaled(0.8,0.4,0.3);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //space ship head window
        for(i = 0; i <= lats; i++) {
            float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
            float z0  = sin(lat0);
            float zr0 =  cos(lat0);

            float lat1 = M_PI * (-0.5 + (float) i / lats);
            float z1 = sin(lat1);
            float zr1 = cos(lat1);
            glPushMatrix();
            glColor4f( 0,0,0, 0.7);
            glTranslated(shiftX-22,shiftY,shiftZ);
            glScaled(0.3,0.2,0.3);
            glBegin(GL_QUAD_STRIP);
            for(j = 0; j <= longs; j++) {
                float lng = 2 * M_PI * (float) (j - 1) / longs;
                float x = cos(lng);
                float y = sin(lng);

                glNormal3f(x * zr0, y * zr0, z0);
                glVertex3f(r * x * zr0, r * y * zr0, r * z0);
                glNormal3f(x * zr1, y * zr1, z1);
                glVertex3f(r * x * zr1, r * y * zr1, r * z1);
            }
            glEnd();
            glPopMatrix();
    }
    
    //space ship red design
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 1, 0, 0);
        glTranslated(shiftX,shiftY,shiftZ);
        glScaled(0.8,0.1,0.4);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    
    glPushMatrix();
    
    //----------engine------------
    glTranslated(shiftX+200, shiftY-130, shiftZ);
    glRotatef(90, 0, 1, 0);
    
    curvedtube(-4,130,-167, 0.2, 0.2,0.8, 0,1,0,120,0);
    curvedtube(4,130,-167, 0.2, 0.2,0.8, 0,1,0,120,0);
    
    curvedtube(4,130,-167, 0.3, 0.3,0.2, 0,0,0,120,0);//black
    curvedtube(-4,130,-167, 0.3, 0.3,0.2, 0,0,0,120,0);
    
    curvedtube(4,130,-163, 0.3, 0.3,0.2, 1,1,1,120,0);//white
    curvedtube(-4,130,-163, 0.3, 0.3,0.2, 1,1,1,120,0);
    
    curvedtube(4,130,-158, 0.1, 0.1,0.5, 1,0,0,250,0);//red
    curvedtube(-4,130,-158, 0.1, 0.1,0.5, 1,0,0,250,0);
    
    curvedtube(4,130,-156, 0.06, 0.06,0.5, 1,1,0,250,0);//yellow
    curvedtube(-4,130,-156, 0.06, 0.06,0.5, 1,1,0,250,0);
    glPopMatrix();
    
    //fin of spaceship
    glPushMatrix();
    triangle(shiftX,shiftY,shiftZ, 30, 30, 30, 0, 1, 0);
    glPopMatrix();
}

//traffic light
void trafficLight(int shiftX, int shiftY, int shiftZ, int x, int y, int z, int r, double lats, double longs, float re, float g, float bl)
{
    //body of the traffic light
    int i, j;
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( re, g, bl);
        glTranslated(shiftX,shiftY,shiftZ);
        glScaled(0.3,0.4,0.3);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //black sphere
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 0,0,0);
        glTranslated(shiftX-9,shiftY+5,shiftZ);
        glScaled(0.15,0.15,0.15);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //black sphere
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 0,0,0);
        glTranslated(shiftX-9,shiftY-5,shiftZ);
        glScaled(0.15,0.15,0.15);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //green light
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 0,1,0);
        glTranslated(shiftX-12,shiftY+5,shiftZ);
        glScaled(0.1,0.1,0.1);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    //red light
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( 1,0,0);
        glTranslated(shiftX-12,shiftY-5,shiftZ);
        glScaled(0.1,0.1,0.1);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
}

//future car
void futureCar(int shiftX, int shiftY, int shiftZ, int x, int y, int z, int r, double lats, double longs, float re, float g, float bl){
    //body of the car
    int i, j;
    for(i = 0; i <= lats; i++) {
        float lat0 = M_PI * (-0.5 + (float) (i - 1) / lats);
        float z0  = sin(lat0);
        float zr0 =  cos(lat0);

        float lat1 = M_PI * (-0.5 + (float) i / lats);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);
        glPushMatrix();
        glColor3f( re, g, bl);
        glTranslated(shiftX,shiftY,shiftZ);
        glScaled(0.75,0.2,0.5);
        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            float lng = 2 * M_PI * (float) (j - 1) / longs;
            float x = cos(lng);
            float y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(r * x * zr0, r * y * zr0, r * z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(r * x * zr1, r * y * zr1, r * z1);
        }
        glEnd();
        glPopMatrix();
    }
    
    //roof of the car
    glColor4f(0.902, 0.902, 0.980, 0.7); //transparent roof
    GLfloat v[x * y][3];
    for (i = 0; i < x; ++i) {
      for (j = 0; j < y; ++j) {
        v[i*y+j][0]=r*cos(j*2*M_PI/y)*cos(i*M_PI/(2*x));
        v[i*y+j][1]=r*sin(i*M_PI/(2*x));
        v[i*y+j][2]=r*sin(j*2*M_PI/y)*cos(i*M_PI/(2*x));
      }
    }
    glPushMatrix();
    glTranslated(shiftX,shiftY+3,shiftZ);
    glScaled(0.50,0.45,0.35);
    glBegin(GL_QUADS);
      for (i=0; i<x-1; ++i) {
        for (j=0; j<y; ++j) {
            glNormal3f(Cos(i + 1), 1, Sin(i + 1));
          glVertex3fv(v[i*y+j]);
          glVertex3fv(v[i*y+(j+1)%y]);
          glVertex3fv(v[(i+1)*y+(j+1)%y]);
          glVertex3fv(v[(i+1)*y+j]);
        }
      }
    glEnd();
    glPopMatrix();
}

//advertisement billboard
static void billboard(){
    quad(0, 0, 0, 2, 20, 25, -80,120,-120, 0);// back board
    quad(1, 1, 1, 1.5, 15, 18, -83,125,-120, 1); //Fry's face
    
    //pole
    int u;
    glPushMatrix();
    glTranslatef(-80,0,-120);
    glScaled(.70,1.35,.75);
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.596, 0.984, 0.596);
    for(u = 0; u <=360; u+=10){
        glNormal3f(Cos(u)*10, 1, (Sin(u)));
        glVertex3f(Cos(u)*10, +90, Sin(u)*10);
        glNormal3f(Cos(u), -1, Sin(u));
        glVertex3f(Cos(u)*10, 0, Sin(u)*10);
    }
    glEnd();
    glPopMatrix();
}

//orange building
void halfSphereBuilding(int shiftX, int shiftY, int shiftZ, int x, int y, int z, int r){
    int i, j;
    glColor3f(0.824, 0.412, 0.118);
    GLfloat v[x * y][3];
    
    //half sphere
    for (i = 0; i < x; ++i) {
      for (j = 0; j < y; ++j) {
        v[i*y+j][0]=r*cos(j*2*M_PI/y)*cos(i*M_PI/(2*x));
        v[i*y+j][1]=r*sin(i*M_PI/(2*x));
        v[i*y+j][2]=r*sin(j*2*M_PI/y)*cos(i*M_PI/(2*x));
      }
    }
    glPushMatrix();
    glTranslated(shiftX,shiftY,shiftZ);
    glBegin(GL_QUADS);
    glColor3f(0.824, 0.412, 0.118);
      for (i=0; i<x-1; ++i) {
        for (j=0; j<y; ++j) {
            glNormal3f(Cos(i + 1), 1, Sin(i + 1));
            glVertex3fv(v[i*y+j]);
            glVertex3fv(v[i*y+(j+1)%y]);
            glVertex3fv(v[(i+1)*y+(j+1)%y]);
            glVertex3fv(v[(i+1)*y+j]);
            }
      }
    glEnd();
    
    //cone
    int u;
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.624, 0.212, 0.018);
    for(u = 0; u <=360; u+=10){
        glNormal3f(Cos(u)*10, 1, (Sin(u)));
        glVertex3f(Cos(u)*7, +90, Sin(u)*7);
        glNormal3f(Cos(u), -1, Sin(u));
        glVertex3f(Cos(u)*7, 0, Sin(u)*7);
    }
    glEnd();
    glPopMatrix();

    //texture for the two flats
    float white[] = {1,1,1,1};
    float Emission[]  = {0.0,0.0,0.01*emission,1.0};
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
    
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,text?GL_REPLACE:GL_MODULATE);
    glColor3f(0.9, 0.5, 0.2);
    //Lower Flat Building
    glPushMatrix();
    glTranslatef(350,55,120);
    glScaled(10, 7, 20);
    glBindTexture(GL_TEXTURE_2D,texture[2]);
    glBegin(GL_QUADS);
    //  Front
    glNormal3f(0, 0, 1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, 1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, 1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, 1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, 1);
    glEnd();

    //  Back
    glBegin(GL_QUADS);
    glNormal3f( 0, 0,-1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, -1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, -1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, -1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, -1);
    glEnd();
    //  Right
    glBegin(GL_QUADS);
    glNormal3f(1, 1, 0);
    glTexCoord3f(0,0,0); glVertex3f(+1,0,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(+1,+2,+1);
    glEnd();
    //  Left
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(-1,0,+1);
    glTexCoord3f(1,1,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Top
    glBegin(GL_QUADS);
    glNormal3f(0, +1, 0);
    glVertex3f(-1,+2,+1);
    glVertex3f(+1,+2,+1);
    glVertex3f(+1,+2,-1);
    glVertex3f(-1,+2,-1);
    glEnd();
    //  Bottom
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,0,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,0,+1);
    glEnd();
    
    glPopMatrix();
    
    //Upper Flat Building
    glPushMatrix();
    glTranslatef(350,75,120);
    glScaled(10, 7, 20);
    glBindTexture(GL_TEXTURE_2D,texture[2]);
    glBegin(GL_QUADS);
    //  Front
    glNormal3f(0, 0, 1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, 1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, 1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, 1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, 1);
    glEnd();

    //  Back
    glBegin(GL_QUADS);
    glNormal3f( 0, 0,-1);
    glTexCoord3f(0,0,0); glVertex3f(-1,0, -1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0, -1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2, -1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2, -1);
    glEnd();
    //  Right
    glBegin(GL_QUADS);
    glNormal3f(1, 1, 0);
    glTexCoord3f(0,0,0); glVertex3f(+1,0,+1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,+2,-1);
    glTexCoord3f(0,1,0); glVertex3f(+1,+2,+1);
    glEnd();
    //  Left
    glBegin(GL_QUADS);
    glNormal3f(-1, 0, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(-1,0,+1);
    glTexCoord3f(1,1,0); glVertex3f(-1,+2,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,+2,-1);
    glEnd();
    //  Top
    glBegin(GL_QUADS);
    glNormal3f(0, +1, 0);
    glVertex3f(-1,+2,+1);
    glVertex3f(+1,+2,+1);
    glVertex3f(+1,+2,-1);
    glVertex3f(-1,+2,-1);
    glEnd();
    //  Bottom
    glBegin(GL_QUADS);
    glNormal3f(0, -1, 0);
    glTexCoord3f(0,0,0); glVertex3f(-1,0,-1);
    glTexCoord3f(1,0,0); glVertex3f(+1,0,-1);
    glTexCoord3f(1,1,0); glVertex3f(+1,0,+1);
    glTexCoord3f(0,1,0); glVertex3f(-1,0,+1);
    glEnd();
    
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

}

/*
 *  OpenGL (GLUT) calls this routine to display the scene
 */
void display()
{
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
    //enable alpha for color
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable( GL_BLEND );
    glClearColor(0.529, 0.808, 0.922, 1);
   //  Undo previous transformations
   glLoadIdentity();
    if (fp)
    {
        cam_x = +2*dim*Sin(th)*Cos(angleY);
        cam_y = +2*dim        *Sin(angleY);
        cam_z = -2*dim*Cos(th)*Cos(angleY);
        gluLookAt(pos_x,pos_y,pos_z, cam_x+pos_x,cam_y+pos_y,cam_z+pos_z, 0,1,0);
    }
    else if (mode)
    {
        double Ex = -2*dim*Sin(th)*Cos(ph);
        double Ey = +2*dim        *Sin(ph);
        double Ez = +2*dim*Cos(th)*Cos(ph);
        gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
    }
    //  Orthogonal - set world orientation
    else
    {
        glRotatef(ph,1,0,0);
        glRotatef(th,0,1,0);
    }
    
    glShadeModel(smooth ? GL_SMOOTH : GL_FLAT); //are there different shade models?
    //  Light switch
    if (light)
    {
        //  Translate intensity to color vectors
        float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
        //  Light position
        float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
        //  Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2] , 0.1);
        //  OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        //  Enable lighting
        glEnable(GL_LIGHTING);
        //  Location of viewer for specular calculations
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
        //  glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        //  Enable light 0
        glEnable(GL_LIGHT0);
        //  Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
    }
    else
        glDisable(GL_LIGHTING);
    
    //draw aexes
    glColor3f(1,1,1);
    if (axes)
    {
        const double len=1.5;  //  Length of axes
        glBegin(GL_LINES);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(len,0.0,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,len,0.0);
        glVertex3d(0.0,0.0,0.0);
        glVertex3d(0.0,0.0,len);
        glEnd();
        //  Label axes
        glRasterPos3d(len,0.0,0.0);
        Print("X");
        glRasterPos3d(0.0,len,0.0);
        Print("Y");
        glRasterPos3d(0.0,0.0,len);
        Print("Z");
    }
    
//------------------------------------Terrain------------------------------------
    ground();
//------------------------------------SKY SCRAPER------------------------------------
    twinBuilding(40, 50, 50, 35, 20, -180);
    twinBuilding(40, 50, 50, 35, 20, 300);
    
    //bridgesx
    glPushMatrix();
    glRotatef(90, 0,0,1);
    glRotatef(90, 1,0,0);
    cylinder(0.6,6,0.6, 210,-150, 0, 0.2,0.4,1, 0.4,0);
    cylinder(0.6,6,0.6, 160,-150, 0, 0.2,0.4,1, 0.4,0);
    glPopMatrix();
//------------------------------------PRIMITIVE APARTMENT BUIDLING------------------------------------
    int s;
    glPushMatrix();
    glTranslated(180,0,0);
    for(s = 215; s>= -100; s-=100){
        GenericBuilding(20, 20, 60, 0.000, 0.980, 0.604, s,0.00,35);
        GenericBuilding(20, 20, 50, 0.878, 1.000, 1.000, s-24,0.00,35);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45,0.00,35);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-65,0.00,35);
        GenericBuilding(20, 20, 80, 1.000, 0.498, 0.314, s-85, 0.00, 35);
        GenericBuilding(20, 20, 90, 0.000, 1.000, 1.000, s-105, 0.00, 35);
        GenericBuilding(20, 30, 80, 0.498, 1.000, 0.831, s-110, 0.00, 35);
        }
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(100,0,0);
    for(s = 100; s>= -100; s-=105){
        GenericBuilding(20, 20, 60,1.000, 0.647, 0.000, s,0.00,80);
        GenericBuilding(20, 20, 50, 0.878, 1.000, 1.000, s-24,0.00,80);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45,0.00,80);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-65,0.00,80);
        GenericBuilding(20, 20, 80, 0.780, 0.082, 0.522, s-85, 0.00, 80);
        GenericBuilding(20, 20, 90, 0.000, 1.000, 1.000, s-105, 0.00, 80);
        GenericBuilding(20, 30, 80, 0.498, 1.000, 0.831, s-110, 0.00, 80);
        }
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(100,0,0);
    for(s = 100; s>= -100; s-=105){
        GenericBuilding(20, 20, 60, 0.902, 0.902, 0.980, s,0.00,124);
        GenericBuilding(20, 20, 50, 0.878, 1.000, 1.000, s-24,0.00,125);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45, 0.00,125);
        GenericBuilding(20, 20, 90, 0.878, 1.000, 1.000, s-65, 0.00,125);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-85,0.00,125);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-105,0.00,125);
        GenericBuilding(20, 30, 80, 1.000, 0.388, 0.278, s-110, 0.00,125);
        }
    glPopMatrix();
    
    glPushMatrix();
    glTranslated(-200,0,0);
    for(s = 120; s>= -100; s-=105){
        GenericBuilding(20, 20, 60, 0.000, 0.980, 0.604, s,0.00,80);
        GenericBuilding(20, 20, 50, 1.000, 0.388, 0.278, s-24,0.00,80);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45, 0.00,80);
        GenericBuilding(20, 20, 90, 0.878, 1.000, 1.000, s-65, 0.00,80);
        GenericBuilding(20, 20, 80, 0.780, 0.082, 0.522, s-85,0.00,80);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-105,0.00,80);
        GenericBuilding(20, 30, 80, 0.498, 1.000, 0.831, s-110, 0.00,35);
        }
    glPopMatrix();

    glPushMatrix();
    glTranslated(-200,0,0);
    for(s = 120; s>= -100; s-=105){
        GenericBuilding(20, 20, 60, 0.902, 0.902, 0.980, s,0.00,35);
        GenericBuilding(20, 20, 50, 0.878, 1.000, 1.000, s-24,0.00,35);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45, 0.00,35);
        GenericBuilding(20, 20, 90, 0.878, 1.000, 1.000, s-65, 0.00,35);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-85,0.00,35);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-105,0.00,35);
        GenericBuilding(20, 30, 80, 1.000, 0.388, 0.278, s-110, 0.00,35);
        }
    glPopMatrix();


    //-----Left Side road---------

    for(s = 380; s>= -320; s-=105){
        GenericBuilding(20, 20, 60, 0.000, 0.980, 0.604, s,0.00,-35);
        GenericBuilding(20, 20, 50, 1.000, 0.388, 0.278, s-24,0.00,-35);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45, 0.00,-35);
        GenericBuilding(20, 20, 90, 0.878, 1.000, 1.000, s-65, 0.00,-35);
        GenericBuilding(20, 20, 80, 0.780, 0.082, 0.522, s-85,0.00,-35);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-105,0.00,-35);
        GenericBuilding(20, 30, 80, 0.498, 1.000, 0.831, s-110, 0.00,-35);
        }

    for(s = 380; s>= -320; s-=105){
        GenericBuilding(20, 20, 60, 0.902, 0.902, 0.980, s,0.00,-80);
        GenericBuilding(20, 20, 50, 0.878, 1.000, 1.000, s-24,0.00,-80);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-45, 0.00,-80);
        GenericBuilding(20, 20, 90, 0.878, 1.000, 1.000, s-65, 0.00,-80);
        GenericBuilding(20, 20, 80, 1.000, 0.894, 0.710, s-85,0.00,-80);
        GenericBuilding(20, 20, 30, 0.529, 0.808, 0.980, s-105,0.00,-80);
        GenericBuilding(20, 30, 80, 0.498, 1.000, 0.831, s-110, 0.00,-80);
        }
//------------------------------------miscelaneous------------------------------------
    //billboard
    billboard();
    //traffic light
    trafficLight(-50, 130, 0, 20, 40, 40, 40, 20, 20,0.855, 0.647, 0.125);
    glPushMatrix();
    glRotated(180,0,1,0);
    glTranslated(100,1,0);
    trafficLight(-50, 130, 0, 20, 40, 40, 40, 20, 20,0.855, 0.647, 0.125);
    glPopMatrix();
    
//------------------------------------FUTURAMA THEMED------------------------------------
    //planet express spacship
    glPushMatrix();
    glScaled(1.5, 1.5, 1.5);
    planetExpressShip(50, 20-move4, 120, 20, 40, 40, 40, 20, 20,0.855, 0.647, 0.125);
    glPopMatrix();
    //tiny cars
    futureCar(350+move, 50, 60, 20, 20, 20, 20, 10, 10,1, 0, 0);
    futureCar(0+move3, 100, 12, 20, 20, 20, 20, 10, 10,0, 1, 0);
    futureCar(200+move2, 100, 50, 20, 20, 20, 20, 10, 10,1.000, 1.000, 0.000);
    futureCar(200+move, 100, -50, 20, 20, 20, 20, 10, 10,1.0, 0.0, 0.0);
    futureCar(30+move2, 100, -12, 20, 20, 20, 20, 10, 10,1, 0, 0);

//------------------------------------Planet Express Building------------------------------------
    curvedtube(350,-10,225, 1.2, 1.6,1, 0.863, 0.078, 0.235,20,1);
    quad(0, 0, 0, 1.5, 10, 6, 320, 1, 200, 1);
    
    planetExpress(350, 0, 200, 20, 20, 20, 20, 20, 20);
    curvedtube(350,-10,180, 1.5,1.6,1, 0.863, 0.078, 0.235,20,1); //red curve tube
    curvedtube(350,-10,300, 1.2, 1.6,1, 0.863, 0.078, 0.235,20,1); //red curve tube
    planetExpressGarage(350, 0, 250,15, 10, 50); //building with green rough and brick bottom
    //------------------------------------Orange Building------------------------------------
    halfSphereBuilding(350, 0, 120, 40, 50, 50, 50);
    quad(0, 0, 0, 1.5, 10, 6, 295, 5, 120, 1);
    cone(0.824, 0.412, 0.118, 350, 60, 120, 7, 10, 7);
    
   //  Five pixels from the lower left corner of the window
   glWindowPos2i(8,8);
   //  Print the text string
    
    if (fp) Print("Angle: %d  Dim: %.1f  View: %s PosX: %.1f PosZ: %.1f th: %d ph: %d CamX: %.1f CamY: %.1f CamZ: %.1f", angleY, dim, "FP", pos_x, pos_z, th, ph, cam_x, cam_y, cam_z);
    else  Print("Angle=%d,%d  Dim=%.1f FOV=%d Projection=%s",th,ph,dim,fov,mode?"Perpective":"Orthogonal");
    //  Render the scene
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */

void special(int key,int x,int y)
{
    //  Right arrow key
    if (key == GLUT_KEY_RIGHT){
        if(fp){// look up
            th += 5;
        }
        else th += 5;
    }
            
    //  Left arrow key
    else if (key == GLUT_KEY_LEFT){
        if(fp){// look left
            th -= 5;
        }
        else th -= 5;
    }
            
    //  Up arrow key
    else if (key == GLUT_KEY_UP){
        if(fp){// look up
            angleY += 2;
        }
        else ph += 5;
    }
           
    //  Down arrow key
    else if (key == GLUT_KEY_DOWN){
        if(fp){//look down
            angleY -= 2;
        }
        else ph -= 5;
    }
    th %= 360;
    ph %= 360;
   //  Keep angles to +/-360 degrees
    Project();
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
    if(fp){
        if (ch == 'd' || ch == 'D')//move camera right
            {
                pos_x -= cam_z * 0.02;
                pos_z += cam_x * 0.02;
            }
        else if (ch == 'a' || ch == 'A') //move camera left
            {
                pos_x += cam_z * 0.02;
                pos_z -= cam_x * 0.02;
            }
        else if (ch == 'w' || ch == 'W')// move camera forward
            {
            pos_x += cam_x * 0.02;
            pos_z += cam_z * 0.02;
            }
        else if (ch == 's' || ch == 'S') //move camera back
           {
            pos_x -= cam_x * 0.02;
            pos_z -= cam_z * 0.02;
           }
        else if (ch == '1')
        {
            
            pos_x = -84;
            pos_z = -20;
            cam_x = 405;
            cam_y = 281;
            cam_z = 340;
            angleY = 28;
            th =  130;
        }
        else if (ch == '2')
        {
            
            pos_x = 169;
            pos_z = 218;
            cam_x = -420;
            cam_y = 244;
            cam_z = -352;
            angleY = 24;
            th =  -50;
        }
        else if (ch == '3')
        {
            
            pos_x = -232;
            pos_z = -175;
            cam_x = 497;
            cam_y = 244;
            cam_z = 231;
            angleY = 24;
            th =  120;
        }
        
}
    if(mode){
        if(ch == '1'){
            th = 50;
            ph = 15;
            fov = 48;
        }
        if (ch == '2'){
            th = 125;
            ph = 35;
            fov = 33;
        }
        if(ch == '3'){
            th = 315;
            ph = 10;
            fov = 57;
        }
    }

    
    //  Exit on ESC
    if (ch == 27)
       exit(0);
    //  Reset view angle
    else if (ch == '0')
       th = ph = 0;
    //  Toggle axes
    else if (ch == 'i' || ch == 'I')
       axes = 1 - axes;
    //  Switch display mode
    else if (ch == 'm' || ch == 'M')
       mode = 1 - mode;
    //  first person view
    else if (ch == 'f' || ch == 'F')
        fp = 1 - fp;
    //  Change field of view angle
    else if (ch == '-' && ch>1)
        {if(!fp) fov --;}
    else if (ch == '+' && ch<179)
        {if(!fp) fov ++;}
    //  Light elevation
    else if (ch=='[')
       ylight -= 2;
    else if (ch==']')
       ylight += 2;
    //  Diffuse level
    else if (ch=='j' && diffuse>0)
       diffuse -= 5;
    else if (ch=='J' && diffuse<100)
       diffuse += 5;
    else if (ch == 'l' || ch == 'L')
       light = 1-light;
    //  Reproject
    Project();
    //  Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
    //  Ratio of the width to the height of the window
    asp = (height>0) ? (double)width/height : 1;
    //  Set the viewport to the entire window
    glViewport(0,0, RES*width,RES*height);
    //  Set projection
    Project();
    
}
/*
 *  GLUT calls this routine when there is nothing else to do
 */
void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360);
    
   static float increment=-0.9;
   move+=increment;
   if (move>2 || move <-150)
       increment *= -1;
    
   static float increment2=1;
   move2+=increment2;
   if (move2 > 50 || move2 < -250)
       increment2 *= -1;
    
    static float increment3=-0.8;
    move3+=increment3;
    if (move3>100 || move3 <-100)
        increment3 *= -1;
   
    static float increment4=-0.2;
    move4+=increment4;
    if (move4>2 || move4 <-4)
        increment4 *= -1;
   glutPostRedisplay();
    
}

/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(1600,800);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Futurama");
#ifdef USEGLEW
   //  Initialize GLEW
   if (glewInit()!=GLEW_OK) Fatal("Error initializing GLEW\n");
#endif
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
    //  Tell GLUT to call "idle" when there is nothing else to do
    glutIdleFunc(idle);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT so it can interact with the user
    texture[0] = LoadTexBMP("img0.bmp");
    texture[1] = LoadTexBMP("img1.bmp");
    texture[2] = LoadTexBMP("img2.bmp");
    texture[3] = LoadTexBMP("img5.bmp");
    texture[4] = LoadTexBMP("ground.bmp");
    texture[5] = LoadTexBMP("apartmentwall.bmp");

    //Load Textures
   glutMainLoop();
   //Initialize();
   return 0;
}
