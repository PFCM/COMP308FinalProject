//
//  main.cpp
//  noisepart
//
//  Created by Paul Francis Cunninghame Mathews on 7/10/14.
//  Copyright (c) 2014 pfcm. All rights reserved.
//

#include "noise.h"
#include "vorton.h"
#include "quaternion.h"
#include "define.h"
#include "concurrencytools.h"

#include <GL/glut.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_set>

GLuint gMainWindow;
GLint gWidth=500, gHeight=500;

G308_Point mousePos;
G308_Point lastMousePos;
quaternion cameraRotation;


basicflow flow;
std::vector<vorton> vortons;
std::vector<particle> tracers; // what get affected by the vortons

// TIMING
auto gAnimationPeriod = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::milliseconds(1000/60));
std::chrono::high_resolution_clock::duration gFramePeriod; // actual time in between

float gTime;
float gTimeStep = 0.005;

// DEBUGGING
bool particle_lines = false;

// FUNCTION DECLARATIONS
void display();
void idle();
void init();
void reshape(int x, int y);
void mouse(int state, int button, int x, int y);
void mouseDrag(int x, int y);

void update_tracer(unsigned); // updates a single tracer

// prints to screen
void printtoscreen(void *font, std::string s);
// length of G308_Point
float length(G308_Point &p);


int main(int argc, char * argv[])
{
    glutInit(&argc, argv);
    
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);
    glutInitWindowSize(gWidth, gHeight);
    gMainWindow = glutCreateWindow("Hello?");
    
    GLuint texName;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    
    

    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseDrag);
    
    init();
    
    glutMainLoop();
    return 0;
}

void display() {
//    unsigned char pixels[512*512*3];
//    vec3 pos,n;
//    for (unsigned i = 0; i < 512; i++) {
//        for (unsigned j = 0;j < 512; j++) {
//            float a = i/128.0;
//            float b = j/128.0;
//            
//            pos[0] = a;
//            pos[1] = b;
//            pos[2] = gTime;
//            
//            flow.get_velocity(pos, n);
//            
//            pixels[(j*512 + i)*3] = n[0] * 255;
//            pixels[(j*512 + i)*3+1] = n[1] * 255;
//            pixels[(j*512 + i)*3+2] = n[2] * 255;
//        }
//    }
    
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB,
//                 GL_UNSIGNED_BYTE, pixels);
    
    // test the perlin noise
    glClear(GL_COLOR_BUFFER_BIT);
//    glColor3f(1.0,1.0,1.0);
//    glEnable(GL_TEXTURE_2D);
//    glBegin(GL_QUADS);
//    glTexCoord2d(0, 0);glVertex3f(-1, -1, 0);
//    glTexCoord2d(0, 1);glVertex3f(-1,  1, 0);
//    glTexCoord2d(1, 1);glVertex3f( 1,  1, 0);
//    glTexCoord2d(1, 0);glVertex3f( 1, -1, 0);
//    glEnd();
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    float mat[16];
    cameraRotation.toMatrix(mat);
    glMultMatrixf(mat);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glPointSize(1);
    glColor3f(0.0,0.5,0.5);
    /*for (vorton &v : vortons) {
        //glBegin(GL_POINTS);
        //glVertex3f(v.mPos[0], v.mPos[1], v.mPos[2]);
        glPushMatrix();
        glTranslatef(v.mPos[0], v.mPos[1], v.mPos[2]);
        glutSolidSphere(.03, 8,8);
        glPopMatrix();
        //glEnd();
	}*/
    
    glDisable(GL_LIGHTING);
    glEnable(GL_POINT_SMOOTH);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0, 0.2, 0.2, 0.1);
    for (particle &p : tracers) {
        if (particle_lines) {
        glBegin(GL_LINES);
        glVertex3f(p.mPos[0], p.mPos[1], p.mPos[2]);
        glVertex3f(p.mPos[0]+p.mVel[0]*.06,
                   p.mPos[1]+p.mVel[1]*.06,
                   p.mPos[2]+p.mVel[2]*.06);
        glEnd();
        } else {
            glBegin(GL_POINTS);
            glVertex3f(p.mPos[0], p.mPos[1], p.mPos[2]);
            glEnd();
        }
    }
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, gWidth, 0, gHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glRasterPos2i(10, 10);
    glColor4f(0.0, 0.0, 0.0, 1.0);
    
    std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(gFramePeriod);
    std::stringstream s;
    s << 1000.0/millis.count();
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glutSwapBuffers();
}

// prints at current rasterpos
void printtoscreen(void *font, std::string s) {
    for (auto c : s) {
        glutBitmapCharacter(font, c);
    }
}

float length(G308_Point &p) {
	return sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
}

void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN) {
		mousePos.x = x;
		mousePos.y = y;
	}
}
void mouseDrag(int x, int y) {
	lastMousePos = mousePos;
	mousePos.x = x;
	mousePos.y = y;

	// arcball all over again
	G308_Point scaledMouse;
	G308_Point scaledLastMouse;
	scaledMouse.x = ((mousePos.x / gWidth) - .5) * .9; // arcball radius .9
	scaledMouse.y = ((mousePos.y / gHeight) - .5) * .9; 
	scaledLastMouse.x = ((lastMousePos.x / gWidth) - .5) * .9;
	scaledLastMouse.y = ((lastMousePos.y / gHeight) - .5) * .9;
	scaledMouse.z = 0;
	scaledLastMouse.z = 0; // so the length doesn't get stucj with a garbage value

	scaledMouse.z = sqrt(1 - pow(length(scaledMouse),2));
	scaledLastMouse.z = sqrt(1 - pow(length(scaledLastMouse),2));

	quaternion q(scaledLastMouse, scaledMouse);
	cameraRotation = (cameraRotation * q).normalise();
	glutPostRedisplay();
}

void update_tracer(unsigned i) {
    particle &p = tracers[i];
    vec3 midx;
    p.mVel[0] = 0;
    p.mVel[1] = 1;
    p.mVel[2] = 0;
    for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
        v.get_velocity_contribution(p.mVel, p.mPos);
    }
    
    // midpoint integration again, may as well be consistent
    midx = p.mPos + 0.5f*gTimeStep*p.mVel;
    for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
        v.get_velocity_contribution(p.mVel, midx);
    }
    p.mPos = p.mPos + (gTimeStep*p.mVel)*0.8;
    
}

void idle() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    if (currentTime - lastTime > gAnimationPeriod) {
        gFramePeriod = currentTime - lastTime;
        lastTime = currentTime;
        gTime += gTimeStep;
        
        vec3 midx; // for the midpoint integration, which is more stable
        for (vorton &v : vortons) {
            flow.vorticity_velocity(v.mPos, v.mVel, v.mVorticity);
            //flow.get_velocity(v.mPos, v.mVel);
            midx = v.mPos + 0.5f*gTimeStep*v.mVel;
            flow.get_velocity(midx, v.mVel);
            //v.mVorticity = v.mVorticity.normalise();
            //v.mVel[1] += 1;
            v.mPos = v.mPos + (gTimeStep*v.mVel)*0.8;
        }
        
        flow.advance_time(gTimeStep);
        concurrent_tools::parallel_for(0, tracers.size(), update_tracer, 1000);
//        std::unordered_set<particle*> dead
//        for (particle &p : tracers) {
//            p.mVel[0] = 0;
//            p.mVel[1] = 1;
//            p.mVel[2] = 0;
//            for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
//                v.get_velocity_contribution(p.mVel, p.mPos);
//            }
//            
//            // midpoint integration again, may as well be consistent
//            midx = p.mPos + 0.5f*gTimeStep*p.mVel;
//            for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
//                v.get_velocity_contribution(p.mVel, midx);
//            }
//            p.mPos = p.mPos + (gTimeStep*p.mVel)*0.8;
//            
	    /*  if (p.mPos[0] > 1.0 || p.mPos[0] < -1.0 ||
                p.mPos[1] > 1.0 || p.mPos[1] < -1.0 ||
                p.mPos[2] > 1.0 || p.mPos[1] < -1.0)
                dead.insert(&p);*/
 //       }
	/* tracers.erase(std::remove_if(tracers.begin(), tracers.end(),
                                  [&](particle &p) {
                                      return dead.find(&p) != dead.end();
				      }));*/
        
        glutPostRedisplay();
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    reshape(gWidth, gHeight);
    
    
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<float> dist(-0.5,0.5);
    point3 o;
    o[1] = -0.5f;
    for (unsigned i = 0; i < 10; i++) {
        vorton v;
	/* v.mPos[0] += dist(rng);
        v.mPos[1] += dist(rng)-0.5;
        v.mPos[2] += dist(rng);*/
	v.mPos = randutils::sphere_point(o, 0.25, false);
        
        v.mVorticity[0] += dist(rng);
        v.mVorticity[1] += dist(rng);
        v.mVorticity[2] += dist(rng);
        v.mVorticity = v.mVorticity.normalise();
        vortons.push_back(v);
    }
    for (unsigned i = 0; i < 100000; i++) {
        particle p;
        /*p.mPos[0] += dist(rng);
        p.mPos[1] += dist(rng)-0.5;
        p.mPos[2] += dist(rng);*/
	
	p.mPos = randutils::sphere_point(o, 0.5f,false);
        tracers.push_back(p);
    }
}

void reshape(int w, int h) {
    if (h == 0)
        h =1;
    glViewport(0,0,w,h);
    gWidth = w;
    gHeight = h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50, (GLfloat)w/(GLfloat)h, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,0.0,-3,
              0.0,0.0,0.0,
              0.0,1.0,0.0);
    glutPostRedisplay();
    std::cout << "reshaped\n";
}
