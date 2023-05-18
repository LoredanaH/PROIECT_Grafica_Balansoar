#include "glos.h"
#include "gl.h"
#include "glu.h"
#include "glaux.h"
#include "glut.h"

#include <math.h>
#include <stdio.h>
#include <stdbool.h>

static int direction;

#define stripeImageWidth 32
GLubyte stripeImage[3 * stripeImageWidth];

const int x = 0, y = 1, z = 2, w = 3;
const int A = 0, B = 1, C = 2, D = 3;
static float lightSourcePosition[4] = { -60, 50, -40, 1 };

static float modelX = 0;
static float modelZ = 0;
static float angle = 0;

float coeficientiPlanIarba[4];
float matriceUmbrire[4][4];

GLfloat punctePlanIarba[][3] = {
	{ -550.0f, -60.0f, -150.0f },
	{ -20.0f, -60.0f, 150.0f },
	{ 20.0f, -60.0f, 150.0f } ,
	{ 550.0f, -60.0f, -150.0f }
};

void CALLBACK mutaSursaFata() {
	if (lightSourcePosition[z] < 100) {
		lightSourcePosition[z] += 5;
	}
}

void CALLBACK mutaSursaSpate() {
	if (lightSourcePosition[z] > -100) {
		lightSourcePosition[z] -= 5;
	}
}

void CALLBACK mutaSursaDreapta() {
	if (lightSourcePosition[x] < 100) {
		lightSourcePosition[x] += 5;
	}
}

void CALLBACK mutaSursaStanga() {
	if (lightSourcePosition[x] > -100) {
		lightSourcePosition[x] -= 5;
	}
}

void CALLBACK mutaStanga()
{
	modelX = modelX - 2;
}

void CALLBACK mutaDreapta()
{
	modelX = modelX + 2;
}

void CALLBACK mutaSpate()
{
	modelZ = modelZ - 2;
}

void CALLBACK mutaFata()
{
	modelZ = modelZ + 2;
}

void computePlaneCoefficientsFromPoints(float points[3][3]) {
	// calculeaza 2 vectori din 3 puncte
	float v1[3]{ points[0][x] - points[1][x], points[0][y] - points[1][y], points[0][z] - points[1][z] };
	float v2[3]{ points[1][x] - points[2][x], points[1][y] - points[2][y], points[1][z] - points[2][z] };

	// produsul vectorial al celor 2 vectori => al 3lea vector cu componentele A,B,C chiar coef din ec. planului
	coeficientiPlanIarba[A] = v1[y] * v2[z] - v1[z] * v2[y];
	coeficientiPlanIarba[B] = v1[z] * v2[x] - v1[x] * v2[z];
	coeficientiPlanIarba[C] = v1[x] * v2[y] - v1[y] * v2[x];

	// determinam D - ecuatia planului in punctul random ales trebuie sa fie zero
	int randomPoint = 1; // poate fi orice punct de pe planul ierbii, asa ca alegem unul din cele 3 folosite pentru calcule
	coeficientiPlanIarba[D] =
		-(coeficientiPlanIarba[A] * points[randomPoint][x] +
			coeficientiPlanIarba[B] * points[randomPoint][y] +
			coeficientiPlanIarba[C] * points[randomPoint][z]);
}

void computeShadowMatrix(float points[3][3], float lightSourcePosition[4]) {
	// determina coef planului	
	computePlaneCoefficientsFromPoints(points);

	// temp = AxL + ByL + CzL + Dw
	float temp =
		coeficientiPlanIarba[A] * lightSourcePosition[x] +
		coeficientiPlanIarba[B] * lightSourcePosition[y] +
		coeficientiPlanIarba[C] * lightSourcePosition[z] +
		coeficientiPlanIarba[D] * lightSourcePosition[w];

	//prima coloana
	matriceUmbrire[0][0] = temp - coeficientiPlanIarba[A] * lightSourcePosition[x];
	matriceUmbrire[1][0] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[x];
	matriceUmbrire[2][0] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[x];
	matriceUmbrire[3][0] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[x];
	//a 2a coloana
	matriceUmbrire[0][1] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[y];
	matriceUmbrire[1][1] = temp - coeficientiPlanIarba[B] * lightSourcePosition[y];
	matriceUmbrire[2][1] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[y];
	matriceUmbrire[3][1] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[y];
	//a 3a coloana
	matriceUmbrire[0][2] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[z];
	matriceUmbrire[1][2] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[z];
	matriceUmbrire[2][2] = temp - coeficientiPlanIarba[C] * lightSourcePosition[z];
	matriceUmbrire[3][2] = 0.0f - coeficientiPlanIarba[D] * lightSourcePosition[z];
	//a 4a coloana
	matriceUmbrire[0][3] = 0.0f - coeficientiPlanIarba[A] * lightSourcePosition[w];
	matriceUmbrire[1][3] = 0.0f - coeficientiPlanIarba[B] * lightSourcePosition[w];
	matriceUmbrire[2][3] = 0.0f - coeficientiPlanIarba[C] * lightSourcePosition[w];
	matriceUmbrire[3][3] = temp - coeficientiPlanIarba[D] * lightSourcePosition[w];
}

void myInit(void)
{
	glClearColor(0.6, 0.7, 1.0, 1.0);

	//sursa lumina
	float lightAmbient[] = { 0.4f, 0.4f, 0.4f, 1.0f };
	float lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, lightSourcePosition);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	//iluminarea
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
}

void banca()
{
	glNormal3f(0, 1, 0);
	glBegin(GL_QUADS);
	{
		//Front
		glVertex3f(-1.5f, -1.0f, 1.5f);
		glVertex3f(1.5f, -1.0f, 1.5f);
		glVertex3f(1.5f, 1.0f, 1.5f);
		glVertex3f(-1.5f, 1.0f, 1.5f);

		//Right
		glVertex3f(1.5f, -1.0f, -1.5f);
		glVertex3f(1.5f, 1.0f, -1.5f);
		glVertex3f(1.5f, 1.0f, 1.5f);
		glVertex3f(1.5f, -1.0f, 1.5f);

		//Back
		glVertex3f(-1.5f, -1.0f, -1.5f);
		glVertex3f(-1.5f, 1.0f, -1.5f);
		glVertex3f(1.5f, 1.0f, -1.5f);
		glVertex3f(1.5f, -1.0f, -1.5f);

		//Left
		glVertex3f(-1.5f, -1.0f, -1.5f);
		glVertex3f(-1.5f, -1.0f, 1.5f);
		glVertex3f(-1.5f, 1.0f, 1.5f);
		glVertex3f(-1.5f, 1.0f, -1.5f);

		//Top
		glVertex3f(-1.5f, -1.0f, 1.5f);
		glVertex3f(1.5f, -1.0f, -1.5f);
		glVertex3f(-1.5f, -1.0f, -1.5f);
		glVertex3f(-1.5f, -1.0f, -1.5f);

		//Bottom
		glVertex3f(-1.5f, 1.0f, 1.5f);
		glVertex3f(1.5f, -1.0f, 1.5f);
		glVertex3f(1.5f, -1.0f, -1.5f);
		glVertex3f(-1.5f, 1.0f, -1.5f);
	}
	glEnd();

	glNormal3f(0, 0, -1);
	glBegin(GL_QUADS);
	{
		//Spatarul
		glVertex3f(-1.5f, 12.0f, 1.5f);
		glVertex3f(1.5f, 1.0f, 1.5f);
		glVertex3f(1.5f, 1.0f, -1.5f);
		glVertex3f(-1.5f, 12.0f, -1.5f);
	}
	glEnd();
}

void balansoar()
{
	GLUquadric* a = gluNewQuadric();

	glTranslatef(-130.0, 0.0, 0.0);
	{

		glRotatef(90, 1, 0, 0);
		glRotatef(20, 0, -1, 0);
		{
			//piciorul stang
			gluCylinder(a, 10, 10, 100, 50, 50);//cilindrul din fata

			glRotatef(45, 0, 1, 0);
			gluCylinder(a, 10, 10, 100, 50, 50);//cilindrul din spate
			glRotatef(45, 0, -1, 0);

			glTranslatef(0.0, 170.0, 0.0);//cilindrii translatati spre dreapta
			{
				//piciorul drept
				gluCylinder(a, 10, 10, 100, 50, 50);//cilindrul din fata

				glRotatef(45, 0, 1, 0);
				gluCylinder(a, 10, 10, 100, 50, 50);//cilindrul din spate
				glRotatef(45, 0, -1, 0);
			}
		}
		glRotatef(-20, 0, -1, 0);

		glRotatef(90, 1, 0, 0);
		gluCylinder(a, 10, 10, 170, 50, 50);//cilindrul de deasupra
	}

	//balansarea 
	glRotated(angle, 0.0, 0.0, 1.0);

	if (angle == 0.0) {
		direction = 1;
	}
	if (angle == 60.0) {
		direction = -1;
	}

	glRotatef(90, 1, 0, 0);
	glRotatef(130, 0, 1, 0);
	{
		//bratul drept
		glTranslatef(0.0, 30.0, 0.0);
		gluCylinder(a, 3, 3, 59, 50, 50);//cilindrul din spate

		glRotatef(40, 0, 1, 0);
		gluCylinder(a, 3, 3, 67, 50, 50);//cilindrul din fata
		glRotatef(40, 0, -1, 0);

		glTranslated(0.0, -30.0, 0.0);

		glTranslatef(0.0, -30.0, 0.0);
		glTranslatef(0.0, 170.0, 0.0);//cilindrii translatati spre stanga
		{
			//bratul stang
			gluCylinder(a, 3, 3, 59, 50, 50);//cilindrul din spate

			glRotatef(40, 0, 1, 0);
			gluCylinder(a, 3, 3, 67, 50, 50);//cilindrul din fata
			glRotatef(40, 0, -1, 0);
		}
		glTranslatef(0.0, 30.0, 0.0);
	}
	glRotatef(130, 0, -1, 0);


	glRotatef(180, 0, 1, 0);
	glRotatef(90, -1, 0, 0);
	glRotatef(180, 0, -1, 0);

	//Banca
	glTranslated(20.0, -70.0, 85.0);
	glScaled(10.0, 3.0, 40.0);
	glRotatef(20, 0.0, 0.0, -1.0);
	banca();
}

void desenareModel(bool umbra) {
	if (umbra) {
		glColor3f(0, 0.15, 0.05);
	}
	else {
		glColor3f(0.4 ,0.25, 0.05);
	}
	glPushMatrix();
	glTranslatef(modelX, -30, -30);
	glTranslatef(0, 0, modelZ);

	glTranslated(-40.0, 20.0, -100.0);
	glRotated(90, 0.0, 1.0, 0.0);
	glScaled(0.5, 0.5, 0.5);

	balansoar();
	
	glPopMatrix();
}

void desenareIarba() {
	glPushMatrix();
	glColor3f(0, 0.6, 0.1);
	glTranslatef(0, -0.1, 0);
	glBegin(GL_QUADS);
	{
		glNormal3f(0, 1, 0);
		for (int i = 0; i < 4; i++) {
			glVertex3fv(punctePlanIarba[i]);
		}
	}
	glEnd();
	glPopMatrix();
}

void deseneazaLumina()
{
	glPushMatrix();
	glTranslatef(lightSourcePosition[x], lightSourcePosition[y], lightSourcePosition[z]);
	glColor3f(1.0, 0.9, 0);
	auxSolidSphere(5);
	glPopMatrix();
}

void CALLBACK display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	glTranslatef(0, 0, -150);
	glRotatef(15, 1, 0, 0);

	computeShadowMatrix(punctePlanIarba, lightSourcePosition);

	glPushMatrix();
	glLightfv(GL_LIGHT0, GL_POSITION, lightSourcePosition);
	desenareIarba();
	desenareModel(false);
	glPopMatrix();

	//deseneaza umbra
	glDisable(GL_LIGHTING);
	{
		glPushMatrix();
		glMultMatrixf((GLfloat*)matriceUmbrire);
		desenareModel(true);
		glPopMatrix();
		deseneazaLumina();
	}
	glEnable(GL_LIGHTING);

	auxSwapBuffers();
}

void CALLBACK IdleFunction()
{
	angle += direction;

	display();
	Sleep(1000/ 60);
}

void CALLBACK myReshape(GLsizei w, GLsizei h)
{
	if (!h) return;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, 1.0 * (GLfloat)w / (GLfloat)h, 10, 300.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
	auxInitDisplayMode(AUX_DOUBLE | AUX_RGB | AUX_DEPTH24);
	auxInitPosition(10, 10, 1200, 800);
	{
		auxKeyFunc(AUX_RIGHT, mutaDreapta);
		auxKeyFunc(AUX_LEFT, mutaStanga);
		auxKeyFunc(AUX_UP, mutaSpate);
		auxKeyFunc(AUX_DOWN, mutaFata);
		auxKeyFunc(AUX_w, mutaSursaSpate);
		auxKeyFunc(AUX_s, mutaSursaFata);
		auxKeyFunc(AUX_d, mutaSursaDreapta);
		auxKeyFunc(AUX_a, mutaSursaStanga);
	}
	auxInitWindow("Balansoar");
	myInit();
	auxReshapeFunc(myReshape);
	auxIdleFunc(IdleFunction);
	auxMainLoop(display);
	return 0;
}