/*

 */

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <windows.h>
#include <GL/glut.h>
#endif

#include <cmath>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include "triple.hpp"
#include "rgb.hpp"
#include "triangle.hpp"
#include "fractalTerrain.hpp"
#include "tgaClass.h"

typedef std::vector<double> triple;
typedef std::vector< std::vector<Triple > > mapPointMatrix;
typedef std::vector<std::vector<Triple > > tripleMatrix;
typedef std::vector< std::vector<RGB1> > rgbMatrix;
typedef std::vector< std::vector<double> > shadeMatrix;

#define PI 3.14159

// trackball variables
GLfloat trackball_angle = 0;   /* in degrees */
GLfloat trackball_angle2 = 0;   /* in degrees */
int trackball_moving=0, trackball_startx=0, trackball_starty=0;
float navigation_damper = 0.3f;
GLint win_width = 400;			// Window dimensions
GLint win_height = 300;

// state variables
int recompute = 0;
enum { COLOR_MODE=0, TEXTURE_MODE, COLOR_AND_TEXTURE_MODE, DIA_SQ_MODE };
int appearanceMode = COLOR_MODE;
int view_mode = 0;
bool terrainHasWater = false;
float g_FogDensity = 0.2f;
bool g_bFog = false;

// navigation variables
GLfloat eyex = 0.5;
GLfloat eyey = 0.5;
GLfloat eyez = 0.5;

// direction vector of the observer; used as the center parameters in gluLookAt
GLfloat center_x;
GLfloat center_y;
GLfloat center_z;

double exaggeration = .7;
int lod = 7;
int steps = 1 << lod;
int numTriangles = (steps * steps * 2);
std::vector<Triangle> triangles;
mapPointMatrix map;
rgbMatrix colors;
tripleMatrix normals;

// texture variables
#define MAX_TEXTURES 6
#define GRASS_ID 0
#define MOUNTAIN_ID 1
#define SNOWCAP_ID 2
#define WATER_ID 3
unsigned int textureArray[MAX_TEXTURES] = {0};
unsigned int skyTexture[MAX_TEXTURES] = {0};

// skybox texture variables
#define BACK_ID 0 // The texture ID for the back side of the cube
#define FRONT_ID 1  // The texture ID for the front side of the cube
#define BOTTOM_ID 2  // The texture ID for the bottom side of the cube
#define TOP_ID 3  // The texture ID for the top side of the cube
#define LEFT_ID 4 // The texture ID for the left side of the cube
#define RIGHT_ID 5

using namespace std;

void set_projection ();
void origin();
void computeTerrain();
void createTexture(unsigned int textureArray[], char * strFileName, int textureID);
void CreateSkyBox(float x, float y, float z, float width, float height, float length);
void InitFog();

void init()
{
  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  glDepthMask(true);
  InitFog();

  glColorMaterial(GL_FRONT, GL_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  // init a light; positioned in display()
  glEnable(GL_LIGHTING);
  GLfloat ambient[4] = {1.0, 1.0, 1.0, 1.0};
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, ambient);

  // init terrain object
  computeTerrain();

  // init terrain textures
  createTexture(textureArray, "grass2.tga", GRASS_ID);
  createTexture(textureArray, "mountainTexture.tga", MOUNTAIN_ID);
  createTexture(textureArray, "snowcapTexture.tga", SNOWCAP_ID);
  createTexture(textureArray, "waterTexture.tga", WATER_ID);

  // init skybox texture
  createTexture(skyTexture, "SCBACK.tga", BACK_ID );
  createTexture(skyTexture, "SCFRONT.tga", FRONT_ID );
  createTexture(skyTexture, "SCBOTTOM.tga", BOTTOM_ID );
  createTexture(skyTexture, "SCTOP.tga", TOP_ID );
  createTexture(skyTexture, "SCLEFT.tga", LEFT_ID );
  createTexture(skyTexture, "SCRIGHT.tga", RIGHT_ID );
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // load a new terrain
    if (recompute) {
      computeTerrain();
      recompute = false;
    }

    // depending on view mode (first-person nav or observer view), set projection and gluLookAt
    if (view_mode == 0){
        set_projection();
        gluLookAt(eyex, eyey, eyez, eyex + center_x, eyey + center_y, eyez + center_z, 0.0, 1.0, 0.0);
    }
    else if (view_mode == 1 ){
        set_projection();
        gluLookAt(0.5, 1.5, 1.5, 0.5, 0.5, 0.5, 0.0, 1.0, 0.0);
    }
    glMatrixMode(GL_MODELVIEW);

    GLfloat light_pos[4] = {-3.6, 3.9, 1.0, 1.0};
    glTranslatef(0.5, 0.0, 0.5);
    glRotatef(trackball_angle, 0.0, 1.0, 0.0);
    glTranslatef(-0.5, 0.0,-0.5);
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);


    if (appearanceMode == COLOR_MODE) {
      glDisable(GL_TEXTURE_2D);
    } else {
      glEnable(GL_TEXTURE_2D);
    }

    // for each triangle in triangles
    for (int k = 0; k < numTriangles; k++) {

      bool hasTexture = false;
      int currentTexture = NULL;
      // if we are in any of the 3 modes that require texture, proceed with texturing
      if (appearanceMode != COLOR_MODE) {
        hasTexture = true;
        // if this triangle (specifically, the first vertex) is below 0.3, apply water or grass texture
        if (map[triangles[k].get_i(0)][triangles[k].get_j(0)][1] <= (0.3 * exaggeration)) {
          // if water mode enabled, apply water texture
          if (terrainHasWater) {
            currentTexture = WATER_ID;
          }
          // if water mode off, apply grass texture
          else {
            currentTexture = GRASS_ID;
          }
        }
        // if the first vertex of this triangle is between 0.3 (inclusive) and 0.5 (exclusive), apply the mountain texture
        else if (map[triangles[k].get_i(0)][triangles[k].get_j(0)][1] < 0.5) {
          currentTexture = MOUNTAIN_ID;
        }
        // if the first vertex of this triangle is above 0.5 (inclusive), apply the snowcapped mountain texture
        else {
          currentTexture = SNOWCAP_ID;
        }
        glBindTexture(GL_TEXTURE_2D, textureArray[currentTexture]);
      }

      glBegin(GL_TRIANGLES);
      // for each vertex of each triangle
      for (int n = 0; n < 3; n++) {
        // load info for this vertex
        Triple mapEntry = map[triangles[k].get_i(n)][triangles[k].get_j(n)];
        Triple normalEntry = normals[triangles[k].get_i(n)][triangles[k].get_j(n)];
        RGB1 colorEntry = colors[triangles[k].get_i(n)][triangles[k].get_j(n)];

        // if current appearance mode uses color, set color
        GLfloat mat[4] = {static_cast<GLfloat>(colorEntry.r), static_cast<GLfloat>(colorEntry.g), static_cast<GLfloat>(colorEntry.b), 1.0};
        if (appearanceMode == COLOR_MODE || appearanceMode == COLOR_AND_TEXTURE_MODE) {
          glColor4fv(mat);
        }

        int texCoordFactor = 1;
        if (currentTexture == GRASS_ID || currentTexture == WATER_ID) {
          // a factor to multiply the texCoords by in order to make more copies of certain textures
          texCoordFactor = 4;
        }
        // if this vertex has texture
        if (hasTexture) {
          // if current appearanceMode is DIA_SQ_MODE, use i, j coordinates for tex coords
          if (appearanceMode == DIA_SQ_MODE) {
            glTexCoord2d(triangles[k].get_i(n)/4, triangles[k].get_j(n)/4);
          }
          // otherwise, use x, z coordinate as tex coords
          else {
            glTexCoord2d(mapEntry.x * texCoordFactor, mapEntry.z * texCoordFactor);
          }
        }

        // set normal
        glNormal3f(normalEntry.x, normalEntry.y, normalEntry.z);

        // set vertex
        glVertex3f(mapEntry.x, mapEntry.y, mapEntry.z);

      }
      glEnd();
    }

    CreateSkyBox(0, 0, 0, 4, 3, 4);
    glutSwapBuffers();
}

void computeTerrain() {
  // init containers
  FractalTerrain terrain = FractalTerrain(lod, 0.5, terrainHasWater);
  map = mapPointMatrix(steps+1, vector<Triple>(steps+1));
  colors = rgbMatrix(steps+1, vector<RGB1>(steps+1));
  // for each i
  for (int i = 0; i <= steps; ++ i) {
    // for each j
    for (int j = 0; j <= steps; ++ j) {
      double x = 1.0 * i / steps, z = 1.0 * j / steps;
      double altitude = terrain.getAltitude (x, z);
      map[i][j] = Triple(x, altitude * exaggeration, z);
      colors[i][j] = terrain.getColor(x, z);
    }
  }
  // init triangles
  int currentTriangle = 0;
  triangles = vector<Triangle>(numTriangles);
  // for each i
  for (int i = 0; i < steps; ++ i) {
    // for each j
    for (int j = 0; j < steps; ++ j) {
      Triangle *triangle1 = new Triangle (i, j, colors[i][j], i + 1, j, colors[i+1][j], i, j + 1, colors[i][j+1]);
      Triangle *triangle2 = new Triangle (i + 1, j, colors[i+1][j], i + 1, j + 1, colors[i+1][j+1], i, j + 1, colors[i][j+1]);
      triangles[currentTriangle++] = *triangle1;
      triangles[currentTriangle++] = *triangle2;
    }
  }

  // init normal variables
  double ambient = .9;
  double diffuse = 1.0;
  normals = tripleMatrix(steps+1, vector<Triple>(steps+1, Triple(0.0, 0.0, 0.0)));
  Triple sun = Triple (-3.6, 3.9, 1.0);

  /* compute triangle normals and vertex averaged normals */
  for (int i = 0; i < numTriangles; ++ i) {
    Triple v0 = map[triangles[i].get_i(0)][triangles[i].get_j(0)];
    Triple v1 = map[triangles[i].get_i(1)][triangles[i].get_j(1)];
    Triple v2 = map[triangles[i].get_i(2)][triangles[i].get_j(2)];
    Triple normal = v0.subtract(v1).cross(v2.subtract(v1)).normalize();

    triangles[i].set_n(normal);
    for (int j = 0; j < 3; ++ j) {
      normals[triangles[i].get_i(j)][triangles[i].get_j(j)] =
        normals[triangles[i].get_i(j)][triangles[i].get_j(j)].add (normal);
    }
  }


 shadeMatrix shade = shadeMatrix(steps+1, vector<double>(steps+1));
  for (int i = 0; i <= steps; ++ i) {
    for (int j = 0; j <= steps; ++ j) {
      shade[i][j] = 1.0;
      Triple vertex = map[i][j];
      Triple ray = sun.subtract (vertex);
      double distance = steps * sqrt (ray.getComponent(0) * ray.getComponent(0) + ray.getComponent(2) * ray.getComponent(2));
      /* step along ray in horizontal units of grid width */
      for (double place = 1.0; place < distance; place += 1.0) {
        Triple sample = vertex.add (ray.scale (place / distance));
        double sx = sample.getComponent(0), sy = sample.getComponent(1), sz = sample.getComponent(2);
        if ((sx < 0.0) || (sx > 1.0) || (sz < 0.0) || (sz > 1.0)){
          break; /* steppd off terrain */
        }
        double ground = exaggeration * terrain.getAltitude (sx, sz);
        if (ground >= sy) {
          shade[i][j] = 0.0;
          break;
        }
      }
    }
  }

  for (int i = 0; i < numTriangles; ++ i) {
    RGB1 avg = RGB1 (0.0, 0.0, 0.0);
    for (int j = 0; j < 3; ++j) {
      int k = triangles[i].get_i(j), l = triangles[i].get_j(j);
      Triple vertex = map[k][l];
      RGB1 color = colors[k][l];
      // normals[k][l].display();
      Triple normal = normals[k][l].normalize ();
      Triple light = vertex.subtract (sun);
      double distance2 = light.length2 ();
      double dot = light.normalize ().dot (normal);
      double shadow = shade[k][l];
      double lighting = ambient + diffuse * ((dot < 0.0) ? - dot : 0.0) / distance2 * shadow;
      color = color.scale (lighting);
      triangles[i].setRgb(j, color);
      avg = avg.add (color);
    }
    triangles[i].color = (avg.scale (1.0 / 3.0));
  }

}

// init a texture
void createTexture(unsigned int textureArray[], char * strFileName, int textureID)
{
    char buffer[30];    //filename holder
    int status = 0;        //error codes for file read
    TGA myTGAImage;

    if(!strFileName)                                    // Return from the function if no file name was passed in
        return;

    sprintf (buffer,strFileName);        //load buffer with filename
    status = myTGAImage.readTGA(buffer);// Load into a TGA object
    if(status > 1) {
        cout <<"Error occurred = " << status <<strFileName<< endl;
        exit(0);
  }

  glGenTextures((GLsizei)1, (GLuint *) &textureArray[textureID]);

	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);

  gluBuild2DMipmaps(GL_TEXTURE_2D, 3,  myTGAImage.width,
					  myTGAImage.height, myTGAImage.format, GL_UNSIGNED_BYTE, myTGAImage.data);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

}

// create skybox
void CreateSkyBox(float x, float y, float z, float width, float height, float length)
{

    // Bind the BACK texture of the sky map to the BACK side of the cube
    glEnable(GL_TEXTURE_2D);
    glPushAttrib(GL_CURRENT_BIT);
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBindTexture(GL_TEXTURE_2D, skyTexture[BACK_ID]);

    // This centers the sky box around (x, y, z)
    x = x - width  / 2;
    y = y - height / 2;
    z = z - length / 2;

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the BACK Side
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z);
    glEnd();

    // Bind the FRONT texture of the sky map to the FRONT side of the box
    glBindTexture(GL_TEXTURE_2D, skyTexture[FRONT_ID]);

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the FRONT Side
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z + length);
    glEnd();

    // Bind the BOTTOM texture of the sky map to the BOTTOM side of the box
    glBindTexture(GL_TEXTURE_2D, skyTexture[BOTTOM_ID]);

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the BOTTOM Side
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
    glEnd();

    // Bind the TOP texture of the sky map to the TOP side of the box
    glBindTexture(GL_TEXTURE_2D, skyTexture[TOP_ID]);

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the TOP Side
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glEnd();

    // Bind the LEFT texture of the sky map to the LEFT side of the box
    glBindTexture(GL_TEXTURE_2D, skyTexture[LEFT_ID]);

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the LEFT Side
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x, y, z);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x, y, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x, y + height, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x, y + height, z);
    glEnd();

    // Bind the RIGHT texture of the sky map to the RIGHT side of the box
    glBindTexture(GL_TEXTURE_2D, skyTexture[RIGHT_ID]);

    // Start drawing the side as a QUAD
    glBegin(GL_QUADS);

    // Assign the texture coordinates and vertices for the RIGHT Side
    glTexCoord2f(0.0f, 0.0f); glVertex3f(x + width, y, z);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(x + width, y, z + length);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(x + width, y + height, z + length);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(x + width, y + height, z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopAttrib();
}

// draws axes and grid lines as developer guides
void origin(){
    glPushAttrib(GL_CURRENT_BIT);
    glPushMatrix();

    glBegin(GL_LINES);
        glColor3f(1.0, 0.0, 0.0);
        GLfloat x_mat[3] = {1.0, 0.0, 0.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, x_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, x_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, x_mat);
          glVertex3f(-100.0, 0, 0);
          glVertex3f(100.0, 0, 0);
        glColor3f(0.0, 1.0, 0.0);
        GLfloat y_mat[3] = {0.0, 1.0, 0.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, y_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, y_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, y_mat);
          glVertex3f(0, -100.0, 0);
          glVertex3f(0, 100.0, 0);
        glColor3f(0.0, 0.0, 1.0);
        GLfloat z_mat[3] = {0.0, 0.0, 1.0};
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, z_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, z_mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, z_mat);
          glVertex3f(0, 0, -100.0);
          glVertex3f(0, 0, 100.0);
      glEnd();

      // draw grid lines
      GLfloat x_grid_mat[3] = {3.333, 3.333, 3.333};
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, x_grid_mat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, x_grid_mat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, x_grid_mat);
      glBegin(GL_LINES);
        for (int k=1; k < 50; k++) {
          glVertex3f(k, 0, -50.0);
          glVertex3f(k, 0, 50.0);

          glVertex3f(-k, 0, -50.0);
          glVertex3f(-k, 0, 50.0);
        }
      glEnd();

      GLfloat z_grid_mat[3] = {3.333, 3.333, 3.333};
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, z_grid_mat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, z_grid_mat);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, z_grid_mat);
      glBegin(GL_LINES);
        for (int k=1; k < 50; k++) {
          glVertex3f(-50.0, 0, k);
          glVertex3f(50.0, 0, k);

          glVertex3f(-50.0, 0, -k);
          glVertex3f(50.0, 0, -k);
        }
      glEnd();

    glPopAttrib();
    glPopMatrix();
}

// set fog parameters
void InitFog() {
    float fogColor[4] = {0.03f,0.03f,0.03f,1.0f}; // blue fog

    glFogi(GL_FOG_MODE, GL_EXP2); // Fog Mode

    glFogfv(GL_FOG_COLOR, fogColor); // Set Fog Color

    glFogf(GL_FOG_DENSITY, g_FogDensity);  // How Dense Will The Fog Be

    glHint(GL_FOG_HINT, GL_DONT_CARE);  // The Fog's calculation accuracy

    glFogf(GL_FOG_START, 0.5); // Fog Start Depth

    glFogf(GL_FOG_END, 0.4f); // Fog End Depth

    glEnable(GL_FOG); // This enables our OpenGL Fog
}

// set trackball state vars according to button presses
static void mouse (int button, int state, int x, int y)
{
    // if in first person mode, adjust trackball vars so that motion while mouse is held down
    if (view_mode == 0) {
      trackball_startx = x;
      trackball_starty = y;
    } else {
      if (button == GLUT_LEFT_BUTTON){
        if (state == GLUT_DOWN){
          trackball_moving = 1;
          trackball_startx = x;
          trackball_starty = y;
        }
        if (state == GLUT_UP){
          trackball_moving = 0;
        }
      }

    }
}

static void mouse_idle (int x, int y)
{
  if (view_mode == 0) {
    float delta_x = x - trackball_startx;
    float delta_y = y - trackball_starty;
    trackball_startx = x;
    trackball_starty = y;
    trackball_angle += delta_y * 0.01;
    trackball_angle2 += delta_x * 0.01;
    center_x = sin(trackball_angle2);
    center_y = sin(trackball_angle);
    center_z = -cos(trackball_angle2);
    glutPostRedisplay();
  }
}

static void motion (int x, int y)
{
    if(trackball_moving){
        trackball_angle = (trackball_angle + (( x - trackball_startx) / 5));

        trackball_angle2 = (trackball_angle2 + ((y - trackball_starty) / 5));

        trackball_startx = x;
        trackball_starty = y;
        glutPostRedisplay();
    }
}

void set_projection () {

    GLfloat aspect = GLfloat(win_width) / GLfloat(win_height);
    // Reset the projection when zoom setting or window shape changes.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 300.0f);

}
void myReshape(int w, int h)
{
    // Window reshaping function.
    win_width = w;
    win_height = h;
    glViewport(0, 0, win_width, win_height);

}

void keyFunction(unsigned char key, int x, int y) {
    switch (key) {
        float yrot_rads;
        case 'w':
        case 'W':
                eyex += (center_x * navigation_damper) / 5.0;
                eyez += (center_z * navigation_damper) / 5.0;
            break;
        case 's':
        case 'S':
            // backward motion is allowed regardless of collision status
            eyex -= (center_x * navigation_damper) / 5.0;
            eyez -= (center_z * navigation_damper) / 5.0;
            break;
        case 'g':
        case 'G':
            // toggle guides drawn
            //drawGuides = !drawGuides;
            break;
        case 'p':
        case 'P':
            // reset to default position
            eyex = 0.5;
            eyey = 0.5;
            eyez = 0.5;
            break;
        case 'r':
        case 'R':
            recompute = true;
            break;
        case 'v':
        case 'V':
            if (view_mode == 0){
                view_mode = 1;
            }
            else if (view_mode == 1){
                view_mode = 0;
            }
           /* else if (view_mode == 2){
                view_mode = 0;
            } */
            break;
      case 'f':
      case 'F':                                    // Check if we hit F
          g_bFog = !g_bFog;                            // This turns our bool to the opposite value
          if(g_bFog)                                // If g_bFog is true
              glEnable(GL_FOG);                    // Turn on fog
          else
              glDisable(GL_FOG);                    // Else turn OFF the fog
          break;
      case '+':
          g_FogDensity += 0.045f;                    // Increase the fog density
          // How Dense Will The Fog Be
          if(g_FogDensity > 1) g_FogDensity = 1.0;    // Make sure we don't go above 1
          glFogf(GL_FOG_DENSITY, g_FogDensity);glutPostRedisplay();
          break;
      case '-':
          g_FogDensity -= 0.045f;                    // Decrease the fog density
          // How Dense Will The Fog Be
          if(g_FogDensity < 0) g_FogDensity = 0.0;    // Make sure we don't go below 0
          glFogf(GL_FOG_DENSITY, g_FogDensity);glutPostRedisplay();
          break;
        case 'q':
        case 'Q':
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void mySpecialFunc (int key, int x, int y) {

    switch(key)
    {

        case GLUT_KEY_LEFT:
            // rotate view
            trackball_angle2 -= 0.05f;
            center_x = sin(trackball_angle2);
            center_z = -cos(trackball_angle2);
            break;
        case GLUT_KEY_RIGHT:
            // rotate view
            trackball_angle2 += 0.05f;
            center_x = sin(trackball_angle2);
            center_z = -cos(trackball_angle2);
            break;
        case GLUT_KEY_UP:
            eyey+=0.05;
            break;
        case GLUT_KEY_DOWN:
            eyey-=0.05;
            break;
    }

    glutPostRedisplay();

}

void appearanceModeMenu (int id) {
  appearanceMode = id;
  if (id != COLOR_MODE) {
    glEnable(GL_TEXTURE_2D);
  } else {
    glDisable(GL_TEXTURE_2D);
  }

  if (id == TEXTURE_MODE || id == DIA_SQ_MODE) {
    glColor4f(1.0, 1.0, 1.0, 1.0);
  }
  glutPostRedisplay();
}

void hasWaterMenu (int id) {
  terrainHasWater = id;
  recompute = 1;
  glutPostRedisplay();
}

void mainMenu (int id) {
  return;
}

int main(int argc, char **argv)
{

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(900, 700);
    glutCreateWindow("Project 1");

    /* Register assorted GLUT callback routines. */
    init();
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(mouse_idle);
    glutSpecialFunc(mySpecialFunc);
    glutKeyboardFunc(keyFunction);

    int appearanceModeMenuId = glutCreateMenu(appearanceModeMenu);
    glutAddMenuEntry("Color", COLOR_MODE);
    glutAddMenuEntry("Texture", TEXTURE_MODE);
    glutAddMenuEntry("Color & Texture", COLOR_AND_TEXTURE_MODE);
    glutAddMenuEntry("Diamond Square Vizualization", DIA_SQ_MODE);

    int hasWaterMenuId = glutCreateMenu(hasWaterMenu);
    glutAddMenuEntry("Water On", true);
    glutAddMenuEntry("Water Off", false);

    int mainMenuId = glutCreateMenu(mainMenu);
    glutAddSubMenu("Appearance Mode", appearanceModeMenuId);
    glutAddSubMenu("Toggle Water", hasWaterMenuId);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    /* Enter GLUT's main loop; callback dispatching begins. */
    glutMainLoop();
    return 0;
}

