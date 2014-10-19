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
unsigned hardware_threads;

typedef enum {
    BASIC_FLOW, RING_FLOW,
    NUM_FLOWS
} FlowType;

std::string flow_type_strings[] = {
    "basic", "ring"
};

FlowType flow_type = BASIC_FLOW;
unsigned vortons_per = 5;
unsigned tracers_per = 80000;
unsigned leaf_coarseness = tracers_per / 10;

G308_Point mousePos;
G308_Point lastMousePos;
quaternion cameraRotation;


std::unique_ptr<basicflow> flow;
std::vector<vorton> vortons;
std::vector<particle> tracers; // what get affected by the vortons

// TIMING
auto gAnimationPeriod = std::chrono::duration_cast<std::chrono::high_resolution_clock::duration>(std::chrono::milliseconds(1000/30));
std::chrono::high_resolution_clock::duration gFramePeriod; // actual time in between

float gTime;
float gTimeStep = 0.008;

// DEBUGGING
bool particle_lines = false;
bool display_vortons= false;
bool show_info      = true;
bool paused         = false;

// FUNCTION DECLARATIONS
void display();
void idle();
void init();
void reshape(int x, int y);
void mouse(int state, int button, int x, int y);
void mouseDrag(int x, int y);
void keyboard( unsigned char key, int x, int y );
void special_keys( int key, int x, int y );
void print_info();

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
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutReshapeFunc(reshape);
    
    init();
    
    glutMainLoop();
    return 0;
}

void special_keys( int key, int x, int y ) {
    switch (key) {
        case GLUT_KEY_UP:
            vortons_per++;
            break;
        case GLUT_KEY_DOWN:
            vortons_per--;
            break;
            
        case GLUT_KEY_LEFT:
            tracers_per -= 1000;
            break;
        case GLUT_KEY_RIGHT:
            tracers_per += 1000;
            break;
            
        default:
            break;
    }
}

void keyboard( unsigned char key, int x, int y ) {
    switch (key) {
        case 13: // enter
            show_info = !show_info;
            break;
            
     /*   case 'f': {
            flow_type = (FlowType)((flow_type+1)%NUM_FLOWS);
            switch (flow_type) {
                case BASIC_FLOW:
                    flow = std::unique_ptr<curlnoise>(new basicflow);
                    break;
                    
                case RING_FLOW:
                    flow = std::unique_ptr<curlnoise>(new ringflow);
                    break;
                    
                default:
                    break;
            }
            break;
        }*/
        case 'v':
            display_vortons = !display_vortons;
            break;
            
        case '[':
            gTimeStep -= 0.0005;
            break;
            
        case ']':
            gTimeStep += 0.0005;
            break;
            
        case 'p':
            particle_lines = !particle_lines;
            break;
            
        case '9':
            leaf_coarseness -= 1000;
            if (leaf_coarseness == 0)
                leaf_coarseness = 1;
            break;
        case '0':
            if (leaf_coarseness == 1)
                leaf_coarseness = 1000;
            else
                leaf_coarseness += 1000;
            
        case ' ':
            paused = !paused;
            break;
            
        case '8':
            flow->dv *= 10;
            break;
            
        case '7':
            flow->dv /= 10;
            break;
            
        case '6':
            flow->dx *= 10;
            break;
        case '5':
            flow->dx /= 10;
            break;
            
        case 'o':
            flow->sphere = !flow->sphere;
            break;
        case 'i':
            flow->surface = !flow->surface;
            
        default:
            break;
    }
}

void display() {
    // test the perlin noise
    glClear(GL_COLOR_BUFFER_BIT);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    {
        float mat[16];
        cameraRotation.toMatrix(mat);
        glMultMatrixf(mat);
        
        if (display_vortons) {
            /*glEnable(GL_LIGHT0);
             glEnable(GL_LIGHTING);*/
            for (vorton &v : vortons) {
                //glBegin(GL_POINTS);
                //glVertex3f(v.mPos[0], v.mPos[1], v.mPos[2]);
                glColor3f(0.0,0.5,0.5);
                glPushMatrix();
                glTranslatef(v.mPos[0], v.mPos[1], v.mPos[2]);
                glutSolidSphere(.03, 8,8);
                glPopMatrix();
                glBegin(GL_LINES);
                {
                    glColor3f(1.0, 0.0, 0.0);
                    glVertex3f(v.mPos[0], v.mPos[1], v.mPos[2]);
                    glVertex3f(v.mPos[0]+v.mVel[0]*0.1,
                               v.mPos[1]+v.mVel[1]*0.1,
                               v.mPos[2]+v.mVel[2]*0.1);
                    glColor3f(0.0, 1.0, 0.0);
                    glVertex3f(v.mPos[0], v.mPos[1], v.mPos[2]);
                    glVertex3f(v.mPos[0]+v.mVorticity[0]*0.1,
                               v.mPos[1]+v.mVorticity[1]*0.1,
                               v.mPos[2]+v.mVorticity[2]*0.1);
                }
                glEnd();
            }
        }
        
        glDisable(GL_LIGHTING);
        glEnable(GL_POINT_SMOOTH);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(3);
        for (particle &p : tracers) {
            float col = (200-p.mLife)/200.0;
            glColor4f(col, col, col, (1-col)*0.05);
            if (particle_lines) {
                glBegin(GL_LINES);
                glVertex3f(p.mPos[0], p.mPos[1], p.mPos[2]);
                glVertex3f(p.mPos[0]-p.mVel[0]*gTimeStep,
                           p.mPos[1]-p.mVel[1]*gTimeStep,
                           p.mPos[2]-p.mVel[2]*gTimeStep);
                glEnd();
            } else {
                glBegin(GL_POINTS);
                glVertex3f(p.mPos[0], p.mPos[1], p.mPos[2]);
                glEnd();
            }
        }
    }
    glPopMatrix();
    
    if (show_info)
        print_info();
    
    glutSwapBuffers();
}

void print_info() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, gWidth, gHeight, 0);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_BLEND);
    glColor3f(0.0, 0.0, 0.0);
    
    glRasterPos2i(5, 15);
    
    std::chrono::milliseconds millis = std::chrono::duration_cast<std::chrono::milliseconds>(gFramePeriod);
    std::stringstream s;
    s << "Framerate: " << 1000.0/millis.count();
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 30);
    s.str("");// = std::stringstream();
    s.clear();
    s << "Tracers: " << tracers.size();
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 45);
    s.str("");
    s.clear();
    s << "Vortons: " << vortons.size();
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    
    glRasterPos2i(5, 60);
    s.clear();
    s.str("");
    s << "Vortons per: " << vortons_per;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 75);
    s.str("");
    s.clear();
    s << "Tracers per: " << tracers_per;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    
    
    glRasterPos2i(5, 90);
    s.str("");
    s.clear();
    s << "Tracers per thread: " << leaf_coarseness;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 105);
    s.str("");
    s.clear();
    s << "Timestep: " << gTimeStep;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 120);
    s.str("");
    s.clear();
    s << "Vel spread: " << flow->dx;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 135);
    s.str("");
    s.clear();
    s << "Vort spread: " << flow->dv;
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 150);
    s.str("");
    s.clear();
    s << "Tracer shape: " << (flow->sphere? "sphere" : "torus");
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glRasterPos2i(5, 165);
    s.str("");
    s.clear();
    s << "Tracer surface: " << (flow->surface? "true" : "false");
    printtoscreen(GLUT_BITMAP_HELVETICA_12, s.str());
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
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
        
        if (glutGetModifiers() == GLUT_ACTIVE_SHIFT) {
            flow->seed_particles(vortons_per, tracers_per, vortons, tracers);
        }
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
    p.mVel[1] = 0.5;//1;
    p.mVel[2] = 0;
    for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
        v.get_velocity_contribution(p.mVel, p.mPos);
    }
    
    // midpoint integration again, may as well be consistent
    midx = p.mPos + 0.5f*gTimeStep*p.mVel;
    for (vorton &v: vortons) { //TODO: optmise with spatial partitioning
        v.get_velocity_contribution(p.mVel, midx);
    }
    p.mPos = p.mPos + (gTimeStep*p.mVel);
    p.mLife--;
    
}

bool is_dead(particle &p) {
	return p.mLife < 0;
}

void idle() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    if (!paused) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        if (currentTime - lastTime > gAnimationPeriod) {
            gFramePeriod = currentTime - lastTime;
            lastTime = currentTime;
            gTime += gTimeStep;
            
            vec3 midx; // for the midpoint integration, which is more stable
            for (vorton &v : vortons) {
                flow->vorticity_velocity(v.mPos, v.mVel, v.mVorticity);
                midx = v.mPos + 0.5f*gTimeStep*(v.mVel*0.5);
                flow->get_velocity(midx, v.mVel);
                v.mPos = v.mPos + (gTimeStep*(v.mVel*0.5));
                v.mLife--;
            }
            
            flow->advance_time(gTimeStep);
            concurrent_tools::itparallel_for(0, tracers.size(), update_tracer,
                                           leaf_coarseness);//tracers.size()/(3*hardware_threads));
            
            // probably parallelise this
            tracers.erase( std::remove_if( tracers.begin(),
                                          tracers.end(),
                                          is_dead ),
                          tracers.end() );
            vortons.erase( std::remove_if( vortons.begin(),
                                          vortons.end(),
                                          is_dead ),
                          vortons.end() );
            
            glutPostRedisplay();
        }
    }
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    reshape(gWidth, gHeight);
    
    //flow = std::unique_ptr<curlnoise>(new ringflow());
    flow = std::unique_ptr<basicflow>(new basicflow());
    flow->seed_particles(vortons_per, tracers_per, vortons, tracers);
    
    hardware_threads = std::thread::hardware_concurrency();
    if (hardware_threads == 0)
        hardware_threads = 4;
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
