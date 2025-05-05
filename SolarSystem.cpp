#include <GL/glut.h>
#include <math.h>

struct Star {
    float x, y, z;
    float baseBrightness;
    float twinkleSpeed;
};

const int NUM_STARS = 2000;
Star stars[NUM_STARS];

struct Asteroid {
    float angle, radius, yOffset, size, spin;
};

const int NUM_ASTEROIDS = 300;
Asteroid belt[NUM_ASTEROIDS];

float sunRotation = 0.0;
float earthOrbit = 0.0;
float marsOrbit = 0.0;
float mercuryOrbit = 0.0;
float venusOrbit = 0.0;
float jupiterOrbit = 0.0;
float saturnOrbit = 0.0;
float uranusOrbit = 0.0;
float neptuneOrbit = 0.0;
float moonOrbit = 0.0;
float zoom = 20.0f;  // initial camera distance
bool paused = false;
float cameraAngleX = 20.0f;  // vertical angle (elevation)
float cameraAngleY = 0.0f;   // horizontal angle (azimuth)
float lastMouseX, lastMouseY;
bool isDragging = false;

float twinkleTime = 0.0f;

void init() {
    glEnable(GL_DEPTH_TEST); // Depth buffer
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Light source (Sun position)
    GLfloat lightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // Point light at origin
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    GLfloat ambientLight[] = { 0.1f, 0.1f, 0.1f, 1.0f };
    GLfloat diffuseLight[] = { 1.0f, 1.0f, 0.8f, 1.0f }; // yellowish sun

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    glShadeModel(GL_SMOOTH); // smooth shading

    glClearColor(0.0, 0.0, 0.0, 1.0); // Black background
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	
	for (int i = 0; i < NUM_STARS; i++) {
	    stars[i].x = (rand() % 400 - 200) / 10.0f;
	    stars[i].y = (rand() % 400 - 200) / 10.0f;
	    stars[i].z = (rand() % 400 - 200) / 10.0f;
	    stars[i].baseBrightness = 0.6f + static_cast<float>(rand() % 40) / 100.0f; // 0.6 - 1.0
    	stars[i].twinkleSpeed = 0.5f + static_cast<float>(rand() % 100) / 100.0f; // varies how fast it twinkles
	}
	
	for (int i = 0; i < NUM_ASTEROIDS; i++) {
	    belt[i].angle = rand() % 360;
	    belt[i].radius = 9.0f + static_cast<float>(rand() % 30) / 10.0f;  // between Mars & Jupiter
	    belt[i].yOffset = (rand() % 100 - 50) / 300.0f;  // small vertical offset
	    belt[i].size = 0.05f + static_cast<float>(rand() % 10) / 100.0f;
	    belt[i].spin = rand() % 360;
	}

}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w/h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

void drawOrbit(float radius) {
    glDisable(GL_LIGHTING); // Orbit lines don't need lighting
    glColor3f(0.5, 0.5, 0.5); // Gray lines

    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 360; i++) {
        float theta = i * (M_PI / 180.0f);
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        glVertex3f(x, 0.0, z);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void drawStarfield() {
    glDisable(GL_LIGHTING);
    glPointSize(1.5);
    glBegin(GL_POINTS);

    for (int i = 0; i < NUM_STARS; ++i) {
        float pulse = 0.2f * sin(twinkleTime * stars[i].twinkleSpeed) + 0.8f; // 0.6 - 1.0 range
        float brightness = stars[i].baseBrightness * pulse;

        glColor3f(brightness, brightness, brightness);
        glVertex3f(stars[i].x, stars[i].y, stars[i].z);
    }

    glEnd();
    glEnable(GL_LIGHTING);
}

void drawSun() {
    glColor3f(1.0, 1.0, 0.0); // Yellow sun
    glutSolidSphere(2.0, 50, 50); // radius, slices, stacks
    
    GLfloat emission[] = { 1.0, 1.0, 0.0, 1.0 }; // Yellow glow
	glMaterialfv(GL_FRONT, GL_EMISSION, emission);
	glutSolidSphere(2.0, 50, 50);
	GLfloat noEmission[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);

}

void drawSaturnRings() {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 360; i++) {
        float angle = i * M_PI / 180.0f;
        float innerRadius = 1.1f;
        float outerRadius = 1.8f;

        float xInner = innerRadius * cos(angle);
        float zInner = innerRadius * sin(angle);
        float xOuter = outerRadius * cos(angle);
        float zOuter = outerRadius * sin(angle);

        // Gradient color for a subtle ring look
        glColor4f(0.9f, 0.8f, 0.5f, 0.3f); // Outer ring (fainter)
        glVertex3f(xOuter, 0.0, zOuter);

        glColor4f(0.7f, 0.6f, 0.4f, 0.6f); // Inner ring (stronger)
        glVertex3f(xInner, 0.0, zInner);
    }
    glEnd();

    glDisable(GL_BLEND);
    glEnable(GL_LIGHTING);
}

void keyboard(unsigned char key, int x, int y) {
    if (key == '+') {
        zoom -= 1.0f;
        if (zoom < 5.0f) zoom = 5.0f;
    }
    else if (key == '-') {
        zoom += 1.0f;
        if (zoom > 50.0f) zoom = 50.0f;
    }
    else if (key == 'p' || key == 'P') {
    paused = !paused;
	}
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            isDragging = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            isDragging = false;
        }
    }
}

void motion(int x, int y) {
    if (isDragging) {
        float dx = x - lastMouseX;
        float dy = y - lastMouseY;

        cameraAngleY += dx * 0.5f;
        cameraAngleX += dy * 0.5f;

        if (cameraAngleX > 89.0f) cameraAngleX = 89.0f;
        if (cameraAngleX < -89.0f) cameraAngleX = -89.0f;

        lastMouseX = x;
        lastMouseY = y;

        glutPostRedisplay();
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // ?? Draw stars BEFORE setting up camera
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();  // Ignore camera transforms
    drawStarfield();   // Draw them here, static
    glPopMatrix();
    
    // ?? Now setup camera
    glLoadIdentity();    
   // Convert spherical coordinates to cartesian
	float camX = zoom * sinf(cameraAngleY * M_PI / 180.0f) * cosf(cameraAngleX * M_PI / 180.0f);
	float camY = zoom * sinf(cameraAngleX * M_PI / 180.0f);
	float camZ = zoom * cosf(cameraAngleY * M_PI / 180.0f) * cosf(cameraAngleX * M_PI / 180.0f);
	
	gluLookAt(camX, camY, camZ,
	          0.0, 0.0, 0.0,
	          0.0, 1.0, 0.0);

    // Sun
    glPushMatrix();
    glRotatef(sunRotation, 0.0, 1.0, 0.0);
    glColor3f(1.0, 1.0, 0.0); // Yellow
    glutSolidSphere(2.0, 50, 50);
    // Glow aura around the Sun
	glColor4f(1.0, 0.8, 0.0, 0.2); // glowing yellow with alpha
	glutSolidSphere(2.5, 50, 50);  // slightly larger than sun
    glPopMatrix();

    // Mercury
    drawOrbit(4.0);
    glPushMatrix();
    glRotatef(mercuryOrbit, 0.0, 1.0, 0.0);
    glTranslatef(4.0, 0.0, 0.0);
    glRotatef(0.03f, 0.0, 0.0, 1.0);            // axial tilt
    glColor3f(0.7, 0.7, 0.7); // Gray
    glutSolidSphere(0.3, 20, 20);
    glPopMatrix();
    
    // Venus
    drawOrbit(5.5); // Venus
	glPushMatrix();
	glRotatef(venusOrbit, 0.0, 1.0, 0.0);
	glTranslatef(5.5, 0.0, 0.0);
	glRotatef(177.4f, 0.0, 0.0, 1.0);            // axial tilt
	glColor3f(1.0, 0.5, 0.0); // Orange
	glutSolidSphere(0.5, 20, 20);
	glPopMatrix();

    // Earth
    drawOrbit(7.0); // Earth
	glPushMatrix();
	glutSolidSphere(0.5, 20, 20);
	glRotatef(earthOrbit, 0.0, 1.0, 0.0);
	glTranslatef(7.0, 0.0, 0.0);
	glRotatef(23.5f, 0.0, 0.0, 1.0);            // axial tilt
	glColor3f(0.0, 0.0, 1.0); // Earth blue
	glutSolidSphere(0.5, 20, 20);
	
	// Moon around Earth
	glPushMatrix();
	glRotatef(moonOrbit, 0.0, 1.0, 0.0); // orbit around earth
	glTranslatef(1.0, 0.0, 0.0); // distance from Earth
	glColor3f(0.8, 0.8, 0.8); // Light gray
	glutSolidSphere(0.15, 15, 15);
	glPopMatrix();
	
	glPopMatrix();


    // Mars
    drawOrbit(9.0); // Mars
    glPushMatrix();
    glRotatef(marsOrbit, 0.0, 1.0, 0.0);
    glTranslatef(9.0, 0.0, 0.0);
    glRotatef(25.2f, 0.0, 0.0, 1.0);            // axial tilt
    glColor3f(1.0, 0.2, 0.2); // Red
    glutSolidSphere(0.4, 20, 20);
    glPopMatrix();
    
	
	// Jupiter
	drawOrbit(11.5); // Jupiter
	glPushMatrix();
	glRotatef(jupiterOrbit, 0.0, 1.0, 0.0);
	glTranslatef(11.5, 0.0, 0.0);
	glRotatef(3.1f, 0.0, 0.0, 1.0);            // axial tilt
	glColor3f(0.7, 0.4, 0.2); // Brown
	glutSolidSphere(1.0, 30, 30);
	glPopMatrix();
	
	// Saturn
	drawOrbit(14.0); // Saturn
	glPushMatrix();
	glRotatef(saturnOrbit, 0.0, 1.0, 0.0);
	glTranslatef(14.0, 0.0, 0.0);
	glColor3f(1.0, 1.0, 0.5); // Light Yellow
	glRotatef(25.0f, 1.0, 0.0, 0.0); // Saturn's axial tilt
	drawSaturnRings();
	glutSolidSphere(0.9, 25, 25);
	glPopMatrix();
	
	// Uranus
	drawOrbit(16.5); // Uranus
	glPushMatrix();
	glRotatef(uranusOrbit, 0.0, 1.0, 0.0);
	glTranslatef(16.5, 0.0, 0.0);
	glRotatef(97.8f, 0.0, 0.0, 1.0);            // axial tilt
	glColor3f(0.5, 1.0, 1.0); // Cyan
	glutSolidSphere(0.7, 20, 20);
	glPopMatrix();
	
	// Neptune
	drawOrbit(18.5); // Neptune
	glPushMatrix();
	glRotatef(neptuneOrbit, 0.0, 1.0, 0.0);
	glTranslatef(18.5, 0.0, 0.0);
	glRotatef(28.3f, 0.0, 0.0, 1.0);            // axial tilt
	glColor3f(0.2, 0.2, 1.0); // Dark Blue
	glutSolidSphere(0.7, 20, 20);
	glPopMatrix();
	
	//Asteroid belt
	for (int i = 0; i < NUM_ASTEROIDS; i++) {
	    glPushMatrix();
	    float rad = belt[i].angle * M_PI / 180.0f;
	    float x = belt[i].radius * cos(rad);
	    float z = belt[i].radius * sin(rad);
	    glTranslatef(x, belt[i].yOffset, z);
	    glRotatef(belt[i].spin, 0.0, 1.0, 0.0);
	    glColor3f(0.5, 0.5, 0.5); // gray rocks
	    glutSolidSphere(belt[i].size, 6, 6);
	    glPopMatrix();
	}

    glutSwapBuffers();
}

void update(int value) {
    if (!paused) {
        sunRotation += 0.5f;
        mercuryOrbit += 2.5f;
        venusOrbit += 1.8f;
        earthOrbit += 1.0f;
        marsOrbit += 0.8f;
        jupiterOrbit += 0.5f;
        saturnOrbit += 0.4f;
        uranusOrbit += 0.3f;
        neptuneOrbit += 0.2f;
        moonOrbit += 3.0f;
        twinkleTime += 0.2f;  // Adjust speed if needed

        // Wrap around
        if (sunRotation > 360) sunRotation -= 360;
        if (mercuryOrbit > 360) mercuryOrbit -= 360;
        if (venusOrbit > 360) venusOrbit -= 360;
        if (earthOrbit > 360) earthOrbit -= 360;
        if (marsOrbit > 360) marsOrbit -= 360;
        if (jupiterOrbit > 360) jupiterOrbit -= 360;
        if (saturnOrbit > 360) saturnOrbit -= 360;
        if (uranusOrbit > 360) uranusOrbit -= 360;
        if (neptuneOrbit > 360) neptuneOrbit -= 360;
        if (moonOrbit > 360) moonOrbit -= 360;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Solar System");
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
	glutMotionFunc(motion);

    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);
    glutMainLoop();
    return 0;
}

