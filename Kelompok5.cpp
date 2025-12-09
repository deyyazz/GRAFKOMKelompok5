#define GLUT_DISABLE_ATEXIT_HACK 

#include <GL/glut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio> 
#include <string.h>

// --- LIBRARY ---
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "freeglut.lib")

// =========================================================================
// 1. VARIABLE GLOBAL & KAMERA
// =========================================================================
float camX = 0.0f;
float camY = 80.0f;
float camZ = 700.0f;

float camYaw = 0.0f;
float camPitch = -10.0f;
float speed = 5.0f;

// --- TAMBAHAN DI BAGIAN GLOBAL VARIABLE ---
float lastMouseX = -1.0f;
float lastMouseY = -1.0f;
bool firstMouse = true;

bool keyStates[256] = { false };
bool specialStates[256] = { false };

const float PI = 3.14159265359f;
GLUquadric* gQuadric = nullptr;

// WARNA-WARNA UMUM
float maroon[] = { 0.6f, 0.0f, 0.0f };
float white[] = { 1.0f, 1.0f, 1.0f };
float wallWhite[] = { 0.95f, 0.95f, 0.90f };
float darkGlass[] = { 0.15f, 0.2f, 0.3f };
float gold[] = { 1.0f, 0.8f, 0.0f };
float roofBrown[] = { 0.65f, 0.35f, 0.15f };
float brightRed[] = { 0.9f, 0.1f, 0.1f };
float asphaltGray[] = { 0.25f, 0.25f, 0.28f };
float grassGreen[] = { 0.15f, 0.4f, 0.15f };

// =========================================================================
// 2. HELPER MATERIAL
// =========================================================================

void resetMaterial() {
    GLfloat defaultAmb[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat defaultDiff[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat defaultSpec[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpec);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void setMaterial(float r, float g, float b) {
    glColor3f(r, g, b);
}

// Helper Material Khusus Danau (Sesuai Kode Asli Danau)
void setDanauMaterial(float r, float g, float b, float shininess) {
    GLfloat ambient[] = { r * 0.25f, g * 0.25f, b * 0.25f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glColor3f(r, g, b);
}

// =========================================================================
// 3. HELPER GAMBAR DASAR
// =========================================================================

void drawCoordinates() {
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, 1200, 0, 700);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity(); glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    char text[99];
    sprintf_s(text, sizeof(text), "WASD: Jalan | PANAH: Lihat | Q/E: Terbang | POS: %.0f, %.0f", camX, camZ);
    glColor3f(1.0f, 1.0f, 0.0f); glRasterPos2i(10, 670);
    for (int i = 0; text[i] != '\0'; i++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
    glEnable(GL_LIGHTING); glEnable(GL_DEPTH_TEST);  glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
}

void drawCube(float width, float height, float depth, float r, float g, float b) {
    glColor3f(r, g, b);
    glPushMatrix(); glScalef(width, height, depth); glutSolidCube(1.0f); glPopMatrix();
}

void drawTabung(float radius, float height, float r, float g, float b) {
    glColor3f(r, g, b);
    glPushMatrix(); glTranslatef(0.0f, -height / 2.0f, 0.0f);
    glPushMatrix(); glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); gluCylinder(gQuadric, radius, radius, height, 32, 1);
    glPushMatrix(); glRotatef(180.0f, 1.0f, 0.0f, 0.0f); gluDisk(gQuadric, 0.0f, radius, 32, 1); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, 0.0f, height); gluDisk(gQuadric, 0.0f, radius, 32, 1); glPopMatrix();
    glPopMatrix(); glPopMatrix();
}

void drawSingleTriangle(float base, float height, bool inverted, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES); glNormal3f(0, 0, 1);
    if (inverted) { glVertex3f(-base / 2, height / 2, 0); glVertex3f(base / 2, height / 2, 0); glVertex3f(0, -height / 2, 0); }
    else { glVertex3f(0, height / 2, 0); glVertex3f(-base / 2, -height / 2, 0); glVertex3f(base / 2, -height / 2, 0); }
    glEnd();
    glDisable(GL_LIGHTING); glColor3f(0.9f, 0.9f, 0.9f); glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    if (inverted) { glVertex3f(-base / 2, height / 2, 0.1f); glVertex3f(base / 2, height / 2, 0.1f); glVertex3f(0, -height / 2, 0.1f); }
    else { glVertex3f(0, height / 2, 0.1f); glVertex3f(-base / 2, -height / 2, 0.1f); glVertex3f(base / 2, -height / 2, 0.1f); }
    glEnd(); glEnable(GL_LIGHTING);
}

void drawText3D(const char* text, float x, float y, float z, float scale) {
    glPushMatrix(); glTranslatef(x, y, z); glScalef(scale, scale, scale); glLineWidth(2.0f);
    for (const char* c = text; *c != '\0'; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    glLineWidth(1.0f); glPopMatrix();
}

// =========================================================================
// 3. LINGKUNGAN GLOBAL (RUMPUT & JALAN)
// =========================================================================
void drawGlobalEnvironment(float totalDist) {
    resetMaterial();
    // 1. Rumput Luas
    glPushMatrix(); glTranslatef(0.0f, -2.5f, 0.0f);
    drawCube(totalDist * 3.0f + 800.0f, 1.0f, 1200.0f, grassGreen[0], grassGreen[1], grassGreen[2]);
    glPopMatrix();

    // 2. Jalan Aspal Utama
    glPushMatrix(); glTranslatef(0.0f, -1.8f, 180.0f);
    drawCube(totalDist * 3.0f + 400.0f, 0.5f, 80.0f, asphaltGray[0], asphaltGray[1], asphaltGray[2]);
    glPopMatrix();

    // 3. Jalan Penghubung
    float pathThick = 0.6f; float pathY = -1.7f;
    // Path Hukum (Kiri)
    glPushMatrix(); glTranslatef(-totalDist, pathY, 100.0f); drawCube(100.0f, pathThick, 150.0f, asphaltGray[0], asphaltGray[1], asphaltGray[2]); glPopMatrix();
    // Path FC (Tengah)
    glPushMatrix(); glTranslatef(0.0f, pathY, 130.0f); drawCube(100.0f, pathThick, 100.0f, asphaltGray[0], asphaltGray[1], asphaltGray[2]); glPopMatrix();
    // Path FC Baru (Tengah-Kanan)
    glPushMatrix(); glTranslatef(250.0f, pathY, 100.0f); drawCube(50.0f, pathThick, 150.0f, asphaltGray[0], asphaltGray[1], asphaltGray[2]); glPopMatrix();
    // Path Danau (Kanan)
    glPushMatrix(); glTranslatef(totalDist + 50.0f, pathY, 100.0f); drawCube(80.0f, pathThick, 150.0f, asphaltGray[0], asphaltGray[1], asphaltGray[2]); glPopMatrix();
}

// =========================================================================
// 4. BAGIAN GEDUNG HUKUM (POSISI KIRI)
// =========================================================================

void drawLimasRoof(float width, float height, float depth) {
    glColor3fv(roofBrown);
    float ridgeLen = width * 0.6f;
    glBegin(GL_TRIANGLES);
    glNormal3f(0, 0.5, 1); glVertex3f(-width / 2, 0, depth / 2); glVertex3f(-ridgeLen / 2, height, 0); glVertex3f(ridgeLen / 2, height, 0);
    glVertex3f(-width / 2, 0, depth / 2); glVertex3f(ridgeLen / 2, height, 0); glVertex3f(width / 2, 0, depth / 2);
    glNormal3f(0, 0.5, -1); glVertex3f(width / 2, 0, -depth / 2); glVertex3f(ridgeLen / 2, height, 0); glVertex3f(-ridgeLen / 2, height, 0);
    glVertex3f(width / 2, 0, -depth / 2); glVertex3f(-ridgeLen / 2, height, 0); glVertex3f(-width / 2, 0, -depth / 2);
    glNormal3f(-1, 0.5, 0); glVertex3f(-width / 2, 0, -depth / 2); glVertex3f(-ridgeLen / 2, height, 0); glVertex3f(-width / 2, 0, depth / 2);
    glNormal3f(1, 0.5, 0); glVertex3f(width / 2, 0, depth / 2); glVertex3f(ridgeLen / 2, height, 0); glVertex3f(width / 2, 0, -depth / 2);
    glEnd();
}

void GedungHukumBelakang() {
    float W = 50.0f;  float H = 40.0f; float D = 200.0f;
    drawCube(W, H, D, wallWhite[0], wallWhite[1], wallWhite[2]);
    glPushMatrix(); glTranslatef(0.0f, H / 2 + 0.1f, 0.0f); drawLimasRoof(W + 10.0f, 20.0f, D + 10.0f); glPopMatrix();
    for (float z = -D / 2 + 20; z < D / 2 - 20; z += 30) {
        glPushMatrix(); glTranslatef(-W / 2 - 0.5f, 0.0f, z); drawCube(1.0f, 15.0f, 12.0f, darkGlass[0], darkGlass[1], darkGlass[2]); glPopMatrix();
        glPushMatrix(); glTranslatef(W / 2 + 0.5f, 0.0f, z); drawCube(1.0f, 15.0f, 12.0f, darkGlass[0], darkGlass[1], darkGlass[2]); glPopMatrix();
    }
}

void drawWingOrnaments(float groundY, float buildingH, float buildingD, float mainW) {
    float sectionW = 60.0f; float sectionH = 25.0f; float yPos = groundY + buildingH - 12.0f; float triBase = sectionW / 3.0f;
    float positions[] = { -(mainW / 2 - sectionW / 2 - 2.0f), (mainW / 2 - sectionW / 2 - 2.0f) };
    for (int i = 0; i < 2; i++) {
        glPushMatrix(); glTranslatef(positions[i], yPos, buildingD / 2 + 0.1f);
        glPushMatrix(); glTranslatef(-triBase, 0.0f, 0.0f); drawSingleTriangle(triBase, sectionH, true, maroon[0], maroon[1], maroon[2]); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f); drawSingleTriangle(triBase, sectionH, false, white[0], white[1], white[2]); glPopMatrix();
        glPushMatrix(); glTranslatef(triBase, 0.0f, 0.0f); drawSingleTriangle(triBase, sectionH, true, maroon[0], maroon[1], maroon[2]); glPopMatrix();
        glPopMatrix();
    }
}

void drawDoor(float x, float y, float z) {
    glPushMatrix(); glTranslatef(x, y, z);
    drawCube(14.0f, 10.0f, 1.0f, 0.1f, 0.1f, 0.1f);
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.2f); drawCube(13.0f, 9.5f, 0.5f, 0.3f, 0.3f, 0.3f); glPopMatrix();
    glPushMatrix(); glTranslatef(-3.2f, 0.0f, 0.3f); drawCube(6.0f, 9.0f, 0.2f, 0.1f, 0.2f, 0.4f); glPopMatrix();
    glPushMatrix(); glTranslatef(3.2f, 0.0f, 0.3f); drawCube(6.0f, 9.0f, 0.2f, 0.1f, 0.2f, 0.4f); glPopMatrix();
    glPopMatrix();
}

void Gedung1_Hukum() {
    resetMaterial();
    float groundY = 0.0f; float mainW = 160.0f; float mainH = 50.0f; float mainD = 60.0f;

    // UTAMA
    glPushMatrix(); glTranslatef(0.0f, groundY + mainH / 2, 0.0f); drawCube(mainW, mainH, mainD, wallWhite[0], wallWhite[1], wallWhite[2]); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, groundY + mainH, 0.0f); drawLimasRoof(mainW + 10.0f, 25.0f, mainD + 10.0f); glPopMatrix();

    // JENDELA SAMPING BAWAH SEGITIGA (1 BARIS)
    setMaterial(darkGlass[0], darkGlass[1], darkGlass[2]);
    float winY = groundY + 12.0f; float winZ = mainD / 2 + 1.0f; float sideOffset = mainW / 2 - 30.0f;
    glPushMatrix(); glTranslatef(-sideOffset, winY, winZ); for (int col = 0; col < 5; col++) { glPushMatrix(); float xPos = (col - 2) * 10.0f; glTranslatef(xPos, 0.0f, 0.0f); drawCube(8.0f, 5.0f, 2.0f, darkGlass[0], darkGlass[1], darkGlass[2]); glPopMatrix(); } glPopMatrix();
    glPushMatrix(); glTranslatef(sideOffset, winY, winZ); for (int col = 0; col < 5; col++) { glPushMatrix(); float xPos = (col - 2) * 10.0f; glTranslatef(xPos, 0.0f, 0.0f); drawCube(8.0f, 5.0f, 2.0f, darkGlass[0], darkGlass[1], darkGlass[2]); glPopMatrix(); } glPopMatrix();
    resetMaterial();

    // Fasad
    float centerD = 10.0f;
    glPushMatrix(); glTranslatef(0.0f, groundY + 30, mainD / 2 + centerD / 2); drawCube(45.0f, 60.0f, centerD, white[0], white[1], white[2]); glPopMatrix();
    drawDoor(0.0f, groundY + 5.0f, mainD / 2 + centerD + 5.0f);
    glColor3fv(white); drawText3D("FAKULTAS HUKUM", -12.0f, groundY + 17.0f, mainD / 2 + centerD + 8.2f, 0.02f);

    for (int i = -2; i <= 3; i += 4) {
        glPushMatrix(); glTranslatef(10.0f * i, groundY + 25, mainD / 2 + centerD + 5.0f);
        drawTabung(4.0f, 50.0f, maroon[0], maroon[1], maroon[2]);
        glTranslatef(0.0f, 30, 0.0f); glColor3f(gold[0], gold[1], gold[2]); glutSolidSphere(5.0f, 30, 30);
        glPopMatrix();
    }
    glPushMatrix(); glTranslatef(0.0f, groundY + 18.0f, mainD / 2 + centerD + 6.0f); drawCube(28.0f, 6.0f, 1.0f, maroon[0], maroon[1], maroon[2]); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, groundY + 15.0f, mainD / 2 + centerD + 8.0f);
    drawCube(40.0f, 1.0f, 10.0f, brightRed[0], brightRed[1], brightRed[2]); glTranslatef(0.0f, -1.0f, 0.0f); drawCube(40.0f, 1.0f, 10.0f, white[0], white[1], white[2]); glPopMatrix();

    drawWingOrnaments(groundY, mainH, mainD, mainW);
    glPushMatrix(); glTranslatef(0, 40, 42); drawSingleTriangle(25, 25, true, maroon[0], maroon[1], maroon[2]); glPopMatrix();

    // Teks Bawah
    glPushMatrix(); glTranslatef(45, 0.5, 90); drawCube(40, 1, 1, white[0], white[1], white[2]); glPopMatrix();
    glColor3fv(maroon); drawText3D("FAKULTAS HUKUM", 28, 1, 90, 0.03f);

    // GEDUNG BELAKANG (Mundur)
    resetMaterial();
    glPushMatrix(); glTranslatef(-70.0f, 40.0f / 2, -180.0f); GedungHukumBelakang(); glPopMatrix();
    glPushMatrix(); glTranslatef(70.0f, 40.0f / 2, -180.0f); GedungHukumBelakang(); glPopMatrix();
}

// =========================================================================
// 5. BAGIAN FOODCOURT (POSISI TENGAH)
// =========================================================================

void drawFlatOctagonRoof(float radius, float height) {
    glColor3f(0.85f, 0.85f, 0.85f);
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= 8; ++i) {
        float ang = (float)i * 6.28318f / 8;
        glVertex3f(cos(ang) * radius, 0, sin(ang) * radius);
    }
    glEnd();
    glColor3f(0.96f, 0.45f, 0.06f);
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= 8; ++i) {
        float ang = (float)i * 6.28318f / 8;
        float x = cos(ang) * radius; float z = sin(ang) * radius;
        glVertex3f(x, -1, z); glVertex3f(x, 0, z);
    }
    glEnd();
}

void drawTableSet(float x, float z) {
    glPushMatrix(); glTranslatef(x, 0.5f, z);
    glColor3f(0.92f, 0.92f, 0.92f); glutSolidSphere(2.4, 32, 24);
    for (int i = 0; i < 3; ++i) {
        float ang = i * 6.28318f / 3.0f;
        glPushMatrix(); glTranslatef(cos(ang) * 4.2f, 0.9f, sin(ang) * 4.2f);
        glColor3f(0.18f, 0.18f, 0.20f); glutSolidSphere(1.1, 24, 16);
        glPopMatrix();
    }
    glPopMatrix();
}

void drawKioskInterior() {
    glColor3f(1.0f, 0.96f, 0.85f); glPushMatrix(); glTranslatef(0.0f, 3.0f, -2.0f); glScalef(14.0f, 6.0f, 0.5f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.05f, 0.45f, 0.55f); glPushMatrix(); glTranslatef(0.0f, 1.5f, 1.0f); glScalef(14.0f, 3.0f, 0.5f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.98f, 0.96f, 0.86f); glPushMatrix(); glTranslatef(0.0f, 4.2f, 1.0f); glScalef(14.0f, 1.6f, 0.5f); glutSolidCube(1.0f); glPopMatrix();
}

void drawKioskRow(bool leftSide) {
    float baseX = leftSide ? -32.0f : 32.0f;
    float rotY = leftSide ? 90.0f : -90.0f;
    for (int i = 0; i < 4; ++i) {
        glPushMatrix(); glTranslatef(baseX, 0.4f, 6.0f - i * 11.0f); glRotatef(rotY, 0, 1, 0); drawKioskInterior(); glPopMatrix();
    }
}

void drawSmallFrontKiosk(float x, float z) {
    glPushMatrix(); glTranslatef(x, 1.8f, z);
    glColor3f(0.95f, 0.92f, 0.82f); glPushMatrix(); glScalef(14.0f, 6.0f, 6.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.86f, 0.86f, 0.86f); glPushMatrix(); glTranslatef(0.0f, 3.3f, 0.0f); glScalef(15.0f, 1.0f, 7.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.05f, 0.45f, 0.55f); glPushMatrix(); glTranslatef(0.0f, 0.5f, 3.1f); glScalef(11.0f, 3.0f, 0.8f); glutSolidCube(1.0f); glPopMatrix();
    glPopMatrix();
}

void drawEntrancePlanterAndGate() {
    // Planters
    glColor3f(0.10f, 0.10f, 0.10f); glPushMatrix(); glTranslatef(-32.0f, 0.8f, 88.0f); glScalef(26.0f, 1.6f, 6.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.05f, 0.45f, 0.25f); glPushMatrix(); glTranslatef(-32.0f, 2.0f, 88.0f); glScalef(24.0f, 1.0f, 5.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.10f, 0.10f, 0.10f); glPushMatrix(); glTranslatef(32.0f, 0.8f, 88.0f); glScalef(26.0f, 1.6f, 6.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.05f, 0.45f, 0.25f); glPushMatrix(); glTranslatef(32.0f, 2.0f, 88.0f); glScalef(24.0f, 1.0f, 5.0f); glutSolidCube(1.0f); glPopMatrix();

    // Gerbang
    glPushMatrix(); glTranslatef(-20.0f, 3.5f, 82.0f); glRotatef(18.0f, 0, 1, 0);
    glColor3f(0.07f, 0.18f, 0.46f); glPushMatrix(); glScalef(1.4f, 8.0f, 1.4f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(7.0f, 0.0f, 0.0f); glScalef(1.4f, 8.0f, 1.4f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(3.5f, 4.2f, 0.0f); glScalef(9.0f, 1.2f, 1.2f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.98f, 0.45f, 0.12f); for (float x = 1.0f; x <= 6.0f; x += 0.7f) { glPushMatrix(); glTranslatef(x, -0.3f, 0.0f); glScalef(0.4f, 6.5f, 0.6f); glutSolidCube(1.0f); glPopMatrix(); }
    glPopMatrix();

    glPushMatrix(); glTranslatef(20.0f, 3.5f, 82.0f); glRotatef(-18.0f, 0, 1, 0);
    glColor3f(0.07f, 0.18f, 0.46f); glPushMatrix(); glScalef(1.4f, 8.0f, 1.4f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-7.0f, 0.0f, 0.0f); glScalef(1.4f, 8.0f, 1.4f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(-3.5f, 4.2f, 0.0f); glScalef(9.0f, 1.2f, 1.2f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.98f, 0.45f, 0.12f); for (float x = -1.0f; x >= -6.0f; x -= 0.7f) { glPushMatrix(); glTranslatef(x, -0.3f, 0.0f); glScalef(0.4f, 6.5f, 0.6f); glutSolidCube(1.0f); glPopMatrix(); }
    glPopMatrix();

    glPushMatrix(); glTranslatef(-32.0f, 0.0f, 55.0f); glRotatef(90.0f, 0, 1, 0); drawSmallFrontKiosk(0, 0); glPopMatrix();
    glPushMatrix(); glTranslatef(32.0f, 0.0f, 55.0f); glRotatef(-90.0f, 0, 1, 0); drawSmallFrontKiosk(0, 0); glPopMatrix();
}

void drawFrontArcStalls(float W) {
    float maxX = W * 0.30f; float curve = 4.0f;
    for (int i = -2; i <= 2; ++i) {
        float t = i / 2.0f; float x = t * maxX; float z = 40.0f + curve * (1.0f - t * t);
        glColor3f(0.05f, 0.45f, 0.55f); glPushMatrix(); glTranslatef(x, 2.7f, z); glScalef(18.0f, 3.4f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
        glColor3f(1.0f, 0.96f, 0.85f); glPushMatrix(); glTranslatef(x, 5.6f, z); glScalef(18.0f, 3.0f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
    }
}

void Gedung2_Foodcourt() {
    resetMaterial();
    float W = 150.0f; float D = 70.0f; float H = 14.0f;

    // Dinding & Atap
    float wallDepth = D - 22.0f; float wallWidth = W * 0.5f; float wallCenterZ = 18.0f;
    glColor3f(1.0f, 0.96f, 0.85f); glPushMatrix(); glTranslatef(0.0f, H * 0.5f, wallCenterZ); glScalef(wallWidth, H, wallDepth); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0.0f, H + 3.0f, 18.0f); drawFlatOctagonRoof(65.0f, 22.0f); glPopMatrix();
    glColor3f(0.97f, 0.97f, 0.97f); glPushMatrix(); glTranslatef(0.0f, H - 1.5f, 42.0f); glScalef(W * 0.85f, 1.0f, 18.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.82f, 0.82f, 0.82f); glPushMatrix(); glTranslatef(0.0f, 0.4f, 42.0f); glScalef(W * 0.82f, 0.5f, 17.0f); glutSolidCube(1.0f); glPopMatrix();

    // Lobby
    glColor3f(0.65f, 0.65f, 0.65f); glPushMatrix(); glTranslatef(0.0f, 0.45f, 55.0f); glScalef(40.0f, 0.4f, 35.0f); glutSolidCube(1.0f); glPopMatrix();
    glColor3f(0.82f, 0.82f, 0.82f); glPushMatrix(); glTranslatef(-32.0f, 0.42f, 55.0f); glScalef(25.0f, 0.4f, 30.0f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(32.0f, 0.42f, 55.0f); glScalef(25.0f, 0.4f, 30.0f); glutSolidCube(1.0f); glPopMatrix();

    // Kolom Biru
    float maxX = W * 0.30f; float curve = 4.0f; glColor3f(0.07f, 0.18f, 0.46f);
    for (int i = -3; i <= 3; ++i) {
        float t = i / 3.0f; float x = t * maxX; float z = 38.0f + curve * (1.0f - t * t);
        glPushMatrix(); glTranslatef(x, H * 0.5f, z); glScalef(3.0f, H + 1.5f, 3.0f); glutSolidCube(1.0f); glPopMatrix();
    }

    // Bagian Belakang
    float newBackEdge = wallCenterZ - wallDepth / 2.0f; float platformDepth = 12.0f; float platformCenterZ = newBackEdge - platformDepth / 2.0f;
    glColor3f(0.72f, 0.72f, 0.72f); glPushMatrix(); glTranslatef(0.0f, 0.4f, platformCenterZ); glScalef(wallWidth * 0.6f, 0.5f, platformDepth); glutSolidCube(1.0f); glPopMatrix();
    float kioskZ = newBackEdge - 3.1f;
    glPushMatrix(); glTranslatef(-25.0f, 0.0f, kioskZ); drawSmallFrontKiosk(0, 0); glPopMatrix();
    glPushMatrix(); glTranslatef(25.0f, 0.0f, kioskZ); drawSmallFrontKiosk(0, 0); glPopMatrix();

    drawKioskRow(true); drawKioskRow(false);
    drawFrontArcStalls(W);
    drawTableSet(-30.0f, 32.0f); drawTableSet(-10.0f, 32.0f); drawTableSet(10.0f, 32.0f); drawTableSet(30.0f, 32.0f);
    drawEntrancePlanterAndGate();
}

// =========================================================================
// 6. BAGIAN GEDUNG 4 (NEW FOODCOURT - DETAILED)
// =========================================================================

// --- Data Konfigurasi FC Baru ---
const int   FCNEW_COLS = 8;
const int   FCNEW_ROWS = 6;
const float FCNEW_TILE = 2.0f;
const float FCNEW_GAP = 0.25f;
const float WOOD_R = 0.70f, WOOD_G = 0.45f, WOOD_B = 0.20f;
const float UMB_R = 0.65f, UMB_G = 0.45f, UMB_B = 0.25f;

void newFC_drawBox(float sx, float sy, float sz) { glPushMatrix(); glScalef(sx, sy, sz); glutSolidCube(1.0f); glPopMatrix(); }

void drawSimpleFoodLogo(float x, float y, float z, int type) {
    glPushMatrix(); glTranslatef(x, y, z);
    glScalef(1.5f, 1.5f, 1.5f);
    glColor3f(0.6f, 0.4f, 0.2f); glPushMatrix(); glTranslatef(0.0f, 0.4f, -0.2f); glScalef(1.8f, 1.8f, 0.1f); glutSolidCube(1.0f); glPopMatrix(); // Papan
    GLfloat mat_amb[] = { 0.8f, 0.8f, 0.8f, 1.0f }; glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb);
    if (type == 0) { // BURGER
        glColor3f(0.8f, 0.5f, 0.2f); glPushMatrix(); glScalef(1.2f, 0.2f, 1.2f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
        glColor3f(0.4f, 0.2f, 0.1f); glPushMatrix(); glTranslatef(0, 0.25f, 0); glScalef(1.25f, 0.15f, 1.25f); glutSolidCube(0.8f); glPopMatrix();
        glColor3f(0.2f, 0.8f, 0.2f); glPushMatrix(); glTranslatef(0, 0.4f, 0); glScalef(1.3f, 0.05f, 1.3f); glutSolidCube(0.8f); glPopMatrix();
        glColor3f(0.9f, 0.6f, 0.3f); glPushMatrix(); glTranslatef(0, 0.55f, 0); glScalef(1.2f, 0.4f, 1.2f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
    }
    else if (type == 1) { // TELOR CEPLOK
        glColor3f(1.0f, 1.0f, 1.0f); glPushMatrix(); glTranslatef(0, 0.2f, 0); glScalef(1.3f, 0.1f, 1.1f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
        glColor3f(1.0f, 0.7f, 0.0f); glPushMatrix(); glTranslatef(0.1f, 0.3f, 0); glScalef(0.5f, 0.3f, 0.5f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
    }
    else if (type == 2) { // AYAM GORENG
        glColor3f(0.8f, 0.5f, 0.1f); glPushMatrix(); glTranslatef(0, 0.5f, 0); glRotatef(30, 0, 0, 1); glScalef(0.7f, 1.2f, 0.7f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
        glColor3f(0.9f, 0.85f, 0.7f); glPushMatrix(); glTranslatef(-0.3f, 0.1f, 0.0f); glRotatef(30, 0, 0, 1); glScalef(0.2f, 0.8f, 0.2f); glutSolidCube(1.0f); glPopMatrix();
    }
    else if (type == 3) { // KOPI
        glColor3f(0.2f, 0.2f, 0.8f); glPushMatrix(); glTranslatef(0, 0.4f, 0); glRotatef(-90, 1, 0, 0); glutSolidCone(0.4f, 0.8f, 16, 1); glPopMatrix();
        glColor3f(1.0f, 1.0f, 1.0f); glPushMatrix(); glTranslatef(0, 0.85f, 0); glScalef(1.0f, 0.1f, 1.0f); glutSolidSphere(0.42f, 16, 16); glPopMatrix();
        glColor3f(1.0f, 0.0f, 0.0f); glPushMatrix(); glTranslatef(0.1f, 1.0f, 0); glScalef(0.05f, 0.6f, 0.05f); glutSolidCube(1.0f); glPopMatrix();
    }
    else { // BAKSO
        glColor3f(0.9f, 0.9f, 0.9f); glPushMatrix(); glTranslatef(0, 0.3f, 0); glScalef(1.2f, 0.6f, 1.2f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
        glColor3f(0.8f, 0.6f, 0.1f); glPushMatrix(); glTranslatef(0, 0.35f, 0); glScalef(1.0f, 0.1f, 1.0f); glutSolidSphere(0.5f, 16, 16); glPopMatrix();
        glColor3f(0.6f, 0.5f, 0.4f); glPushMatrix(); glTranslatef(0.2f, 0.45f, 0.2f); glutSolidSphere(0.15f, 8, 8); glPopMatrix(); glPushMatrix(); glTranslatef(-0.2f, 0.45f, -0.1f); glutSolidSphere(0.15f, 8, 8); glPopMatrix();
    }
    glPopMatrix();
}

void newFC_drawCourtyard() {
    float totalWidth = FCNEW_COLS * FCNEW_TILE + (FCNEW_COLS + 1) * FCNEW_GAP;
    float totalHeight = FCNEW_ROWS * FCNEW_TILE + (FCNEW_ROWS + 1) * FCNEW_GAP;
    float startX = -totalWidth / 2.0f;
    float startZ = -totalHeight / 2.0f;
    glColor3f(0.55f, 0.75f, 0.35f); glPushMatrix(); glScalef(totalWidth + 4.0f, 0.02f, totalHeight + 4.0f); glutSolidCube(1.0f); glPopMatrix();
    for (int r = 0; r < FCNEW_ROWS; ++r) {
        for (int c = 0; c < FCNEW_COLS; ++c) {
            float x = startX + FCNEW_GAP + c * (FCNEW_TILE + FCNEW_GAP) + FCNEW_TILE / 2.0f;
            float z = startZ + FCNEW_GAP + r * (FCNEW_TILE + FCNEW_GAP) + FCNEW_TILE / 2.0f;
            glColor3f(0.93f, 0.88f, 0.80f); glPushMatrix(); glTranslatef(x, 0.02f, z); newFC_drawBox(FCNEW_TILE, 0.02f, FCNEW_TILE); glPopMatrix();
        }
    }
}

void newFC_drawEntrancePath() {
    float totalHeight = FCNEW_ROWS * FCNEW_TILE + (FCNEW_ROWS + 1) * FCNEW_GAP;
    float startZ = -totalHeight / 2.0f;
    glColor3f(0.80f, 0.55f, 0.30f); glPushMatrix(); glTranslatef(0.0f, 0.025f, startZ - 2.5f); newFC_drawBox(4.0f, 0.02f, 5.0f); glPopMatrix();
}

void newFC_drawTableSet(bool withUmbrella) {
    glColor3f(WOOD_R, WOOD_G, WOOD_B); glPushMatrix(); glTranslatef(0.0f, 0.7f, 0.0f); newFC_drawBox(1.6f, 0.08f, 1.0f); glPopMatrix();
    float legX = 0.65f, legZ = 0.45f;
    for (int i = -1; i <= 1; i += 2) for (int j = -1; j <= 1; j += 2) { glPushMatrix(); glTranslatef(i * legX, 0.35f, j * legZ); newFC_drawBox(0.1f, 0.7f, 0.1f); glPopMatrix(); }
    for (int side = -1; side <= 1; side += 2) {
        glPushMatrix(); glTranslatef(side * 1.0f, 0.4f, 0.0f); newFC_drawBox(0.35f, 0.06f, 1.5f); glPopMatrix();
        for (int k = -1; k <= 1; k += 2) { glPushMatrix(); glTranslatef(side * 1.0f, 0.2f, k * 0.4f); newFC_drawBox(0.08f, 0.4f, 0.08f); glPopMatrix(); }
    }
    if (withUmbrella) {
        glColor3f(0.45f, 0.30f, 0.18f); glPushMatrix(); glTranslatef(0.0f, 1.2f, 0.0f); newFC_drawBox(0.08f, 2.2f, 0.08f); glPopMatrix();
        glColor3f(UMB_R, UMB_G, UMB_B); glPushMatrix(); glTranslatef(0.0f, 2.3f, 0.0f); glRotatef(-90, 1, 0, 0); glutSolidCone(1.2f, 0.7f, 16, 4); glPopMatrix();
    }
}

void newFC_drawMiddleTables() {
    float rowZ[2] = { 2.2f, 0.0f }; float colX[4] = { -5.2f, -1.7f, 1.7f, 5.2f };
    for (int r = 0; r < 2; ++r) for (int c = 0; c < 4; ++c) { glPushMatrix(); glTranslatef(colX[c], 0.0f, rowZ[r]); newFC_drawTableSet(true); glPopMatrix(); }
}

void newFC_drawBackShops() {
    float totalW = FCNEW_COLS * FCNEW_TILE + (FCNEW_COLS + 1) * FCNEW_GAP;
    float totalH = FCNEW_ROWS * FCNEW_TILE + (FCNEW_ROWS + 1) * FCNEW_GAP;
    float endZ = totalH / 2.0f;
    glColor3f(0.86f, 0.86f, 0.88f); glPushMatrix(); glTranslatef(0.0f, 0.1f, endZ + 1.0f); newFC_drawBox(totalW, 0.2f, 4.0f); glPopMatrix();
    glColor3f(0.9f, 0.9f, 0.95f); glPushMatrix(); glTranslatef(0.0f, 2.0f, endZ + 3.0f); newFC_drawBox(totalW, 4.0f, 0.5f); glPopMatrix();
    glColor3f(0.7f, 0.3f, 0.2f); glPushMatrix(); glTranslatef(0.0f, 3.8f, endZ + 1.5f); glRotatef(-15, 1, 0, 0); newFC_drawBox(totalW + 1.0f, 0.3f, 5.5f); glPopMatrix();
    int numBays = 6; float step = totalW / numBays;
    for (int i = 0; i < numBays; i++) {
        float cx = -totalW / 2.0f + step / 2.0f + i * step;
        glColor3f(0.95f, 0.95f, 0.95f); glPushMatrix(); glTranslatef(cx, 0.8f, endZ + 0.5f); newFC_drawBox(step - 0.4f, 0.8f, 0.8f); glPopMatrix();
        if (i % 3 == 0) glColor3f(0.9f, 0.4f, 0.4f); else if (i % 3 == 1) glColor3f(0.4f, 0.8f, 0.4f); else glColor3f(0.4f, 0.6f, 0.9f);
        glPushMatrix(); glTranslatef(cx, 0.8f, endZ + 0.1f); newFC_drawBox(step - 0.5f, 0.6f, 0.1f); glPopMatrix();
        if (i < numBays - 1) { glColor3f(0.7f, 0.7f, 0.75f); glPushMatrix(); glTranslatef(cx + step / 2.0f, 2.0f, endZ + 1.5f); newFC_drawBox(0.2f, 4.0f, 3.5f); glPopMatrix(); }
        drawSimpleFoodLogo(cx, 5.8f, endZ + 1.5f, i % 5);
    }
    glColor3f(0.7f, 0.7f, 0.75f);
    glPushMatrix(); glTranslatef(-totalW / 2.0f, 2.0f, endZ + 1.5f); newFC_drawBox(0.5f, 4.0f, 3.5f); glPopMatrix();
    glPushMatrix(); glTranslatef(totalW / 2.0f, 2.0f, endZ + 1.5f); newFC_drawBox(0.5f, 4.0f, 3.5f); glPopMatrix();
}

void newFC_drawSideBuildingRight() {
    float totalW = FCNEW_COLS * FCNEW_TILE + (FCNEW_COLS + 1) * FCNEW_GAP;
    float startX = -totalW / 2.0f;
    glColor3f(0.88f, 0.88f, 0.90f); glPushMatrix(); glTranslatef(startX + totalW + 2.0f, 1.8f, 0.0f); newFC_drawBox(4.0f, 3.6f, 12.0f); glPopMatrix();
    glColor3f(0.60f, 0.60f, 0.63f); glPushMatrix(); glTranslatef(startX + totalW + 2.0f, 3.85f, 0.0f); newFC_drawBox(4.4f, 0.5f, 13.0f); glPopMatrix();
    glColor3f(0.4f, 0.4f, 0.4f); for (float z = -5.0f; z <= 5.0f; z += 2.5f) { glPushMatrix(); glTranslatef(startX + totalW + 0.1f, 1.8f, z); newFC_drawBox(0.2f, 3.6f, 0.2f); glPopMatrix(); }
}

void newFC_drawLeftGazebo() {
    float totalW = FCNEW_COLS * FCNEW_TILE + (FCNEW_COLS + 1) * FCNEW_GAP;
    float startX = -totalW / 2.0f; float gx = startX - 3.0f; float gz = 2.0f;
    glColor3f(WOOD_R, WOOD_G, WOOD_B); glPushMatrix(); glTranslatef(gx, 0.2f, gz); newFC_drawBox(4.0f, 0.4f, 4.0f); glPopMatrix();
    glColor3f(1.0f, 1.0f, 1.0f);
    float posts[4][2] = { {-1.5,-1.5}, {-1.5,1.5}, {1.5,-1.5}, {1.5,1.5} };
    for (int i = 0; i < 4; i++) { glPushMatrix(); glTranslatef(gx + posts[i][0], 1.5f, gz + posts[i][1]); newFC_drawBox(0.2f, 2.6f, 0.2f); glPopMatrix(); }
    glColor3f(0.25f, 0.50f, 0.25f);
    glPushMatrix(); glTranslatef(gx, 2.8f, gz); glRotatef(-90, 1, 0, 0); glutSolidCone(3.5f, 2.0f, 4, 1); glPopMatrix();
}

void newFC_drawSurroundingFence() {
    float totalW = 8 * 2.0f + 9 * 0.25f; float totalH = 6 * 2.0f + 7 * 0.25f;
    float frontZ = -totalH / 2.0f - 1.8f; float sideX_Left = -totalW / 2.0f - 1.0f; float sideX_Right = totalW / 2.0f + 1.0f; float backZ = totalH / 2.0f;
    glColor3f(0.55f, 0.32f, 0.10f);
    for (int s = -1; s <= 1; s += 2) {
        float startX = (s == -1) ? -2.5f : 2.5f; float endX = (s == -1) ? sideX_Left : sideX_Right;
        glPushMatrix(); glTranslatef((startX + endX) / 2.0f, 0.6f, frontZ); newFC_drawBox(fabs(endX - startX), 1.2f, 0.2f); glPopMatrix();
    }
    float depth = backZ - frontZ;
    glPushMatrix(); glTranslatef(sideX_Left, 0.6f, (frontZ + backZ) / 2.0f); newFC_drawBox(0.2f, 1.2f, depth); glPopMatrix();
    glPushMatrix(); glTranslatef(sideX_Right, 0.6f, (frontZ + backZ) / 2.0f); newFC_drawBox(0.2f, 1.2f, depth); glPopMatrix();
}

void Gedung4_NewFC() {
    glPushMatrix();
    glRotatef(180, 0, 1, 0);
    glScalef(8.0f, 8.0f, 8.0f);
    newFC_drawCourtyard(); newFC_drawEntrancePath(); newFC_drawMiddleTables(); newFC_drawBackShops();
    newFC_drawSideBuildingRight(); newFC_drawLeftGazebo(); newFC_drawSurroundingFence();
    glPopMatrix();
}
// =========================================================================
// 8. GEDUNG 3: DANAU UNESA (POSISI KANAN)
// =========================================================================

// --- DATA DANAU BARU (STAGE 14) ---
const int LAKE_PTS_D = 8;
struct Vec2D { float x, z; };
Vec2D lakeInD[LAKE_PTS_D] = { {-55,-10}, {-25,-16}, {10,-18}, {40,-15}, {70,0}, {75,40}, {60,60}, {-50,60} };
Vec2D lakeOutD[LAKE_PTS_D] = { {-60,-20}, {-30,-26}, {15,-28}, {50,-25}, {80,-5}, {85,45}, {65,70}, {-55,70} };

// --- HELPER KHUSUS DANAU ---
void setDanauMat(float r, float g, float b, float shininess) {
    GLfloat ambient[] = { r * 0.25f, g * 0.25f, b * 0.25f, 1.0f };
    GLfloat diffuse[] = { r, g, b, 1.0f };
    GLfloat specular[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
    glColor3f(r, g, b);
}

void drawDanauFenceSegment(float len) {
    setDanauMat(0.90f, 0.78f, 0.96f, 18.0f);
    glPushMatrix(); glTranslatef(-len / 2, 0.45f, 0); glScalef(0.18f, 0.9f, 0.18f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(len / 2, 0.45f, 0); glScalef(0.18f, 0.9f, 0.18f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0.45f, 0); glScalef(len, 0.1f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(0, 0.75f, 0); glScalef(len, 0.1f, 0.1f); glutSolidCube(1.0f); glPopMatrix();
}

void drawDanauLampPost(float x, float z, float baseY) {
    setDanauMat(0.18f, 0.18f, 0.22f, 10.0f);
    glPushMatrix(); glTranslatef(x, baseY + 2.5f, z); glScalef(0.2f, 5.0f, 0.2f); glutSolidCube(1.0f); glPopMatrix();
    setDanauMat(0.98f, 0.98f, 0.90f, 40.0f);
    glPushMatrix(); glTranslatef(x, baseY + 5.4f, z); glutSolidSphere(0.35f, 16, 16); glPopMatrix();
}

void drawCylPlatform(float cx, float cz, float baseY, float r, float h, float cr, float cg, float cb) {
    setDanauMat(cr, cg, cb, 10.0f);
    int seg = 32;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= seg; ++i) {
        float th = 2.0f * 3.14159f * i / seg;
        float x = cx + r * cos(th); float z = cz + r * sin(th);
        glNormal3f(cos(th), 0, sin(th));
        glVertex3f(x, baseY, z); glVertex3f(x, baseY + h, z);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN); glNormal3f(0, 1, 0); glVertex3f(cx, baseY + h, cz);
    for (int i = 0; i <= seg; ++i) {
        float th = 2.0f * 3.14159f * i / seg;
        glVertex3f(cx + r * cos(th), baseY + h, cz + r * sin(th));
    }
    glEnd();
}

void drawDanauTree(float x, float z, float baseY, int type) {
    float scale = (type == 0) ? 1.0f : 0.9f;
    float trunkH = (type == 0) ? 3.5f : 1.0f;
    setDanauMat(0.45f, 0.28f, 0.12f, 8.0f);
    glPushMatrix(); glTranslatef(x, baseY, z); glRotatef(-90, 1, 0, 0);
    gluCylinder(gQuadric, 0.35f * scale, 0.3f * scale, trunkH, 16, 1); glPopMatrix();
    if (type == 0) {
        setDanauMat(0.5f, 0.8f, 0.3f, 10.0f);
        glPushMatrix(); glTranslatef(x, baseY + trunkH + 0.5f, z); glScalef(2.0f, 1.6f, 2.0f); glutSolidSphere(1.0f, 24, 24); glPopMatrix();
    }
    else {
        setDanauMat(0.2f, 0.55f, 0.25f, 10.0f);
        glPushMatrix(); glTranslatef(x, baseY + trunkH, z); glRotatef(-90, 1, 0, 0); glutSolidCone(0.9f * scale, 2.2f * scale, 16, 4); glPopMatrix();
    }
}

void drawDanauBench(float x, float z, float angle) {
    setDanauMat(0.96f, 0.96f, 0.96f, 20.0f);
    glPushMatrix(); glTranslatef(x, 0.5f, z); glRotatef(angle, 0, 1, 0);
    glScalef(3.5f, 0.15f, 0.6f); glutSolidCube(1.0f);
    glPopMatrix();
    glPushMatrix(); glTranslatef(x, 0.25f, z); glRotatef(angle, 0, 1, 0);
    glTranslatef(-1.5f, 0, 0); glScalef(0.2f, 0.5f, 0.5f); glutSolidCube(1.0f);
    glTranslatef(15.0f, 0, 0); glutSolidCube(1.0f);
    glPopMatrix();
}

void Gedung3_Danau() {
    float waterY = 0.03f;
    setDanauMat(0.0f, 0.60f, 0.95f, 90.0f);
    glBegin(GL_POLYGON); glNormal3f(0, 1, 0); for (int i = 0; i < LAKE_PTS_D; ++i) glVertex3f(lakeInD[i].x, waterY, lakeInD[i].z); glEnd();

    float topY = waterY + 0.20f;
    setDanauMat(0.94f, 0.91f, 0.86f, 8.0f);
    glBegin(GL_QUAD_STRIP); glNormal3f(0, 1, 0);
    for (int i = 0; i <= LAKE_PTS_D; ++i) {
        int idx = i % LAKE_PTS_D;
        glVertex3f(lakeInD[idx].x, topY, lakeInD[idx].z);
        glVertex3f(lakeOutD[idx].x, topY, lakeOutD[idx].z);
    }
    glEnd();

    for (int i = 0; i < LAKE_PTS_D; ++i) {
        int j = (i + 1) % LAKE_PTS_D;
        float t = 0.4f;
        float x0 = lakeInD[i].x * (1 - t) + lakeOutD[i].x * t; float z0 = lakeInD[i].z * (1 - t) + lakeOutD[i].z * t;
        float x1 = lakeInD[j].x * (1 - t) + lakeOutD[j].x * t; float z1 = lakeInD[j].z * (1 - t) + lakeOutD[j].z * t;
        float dx = x1 - x0, dz = z1 - z0;
        float len = sqrt(dx * dx + dz * dz);
        float ang = atan2(dz, dx) * 180 / 3.14159f;
        int segs = (int)(len / 5.0f); if (segs < 1) segs = 1;
        for (int k = 0; k < segs; k++) {
            float t1 = (float)k / segs;
            float px = x0 + dx * t1 + dx / segs / 2;
            float pz = z0 + dz * t1 + dz / segs / 2;
            glPushMatrix(); glTranslatef(px, topY + 0.05f, pz); glRotatef(ang, 0, 1, 0); drawDanauFenceSegment(len / segs); glPopMatrix();
        }
        drawDanauLampPost((x0 + x1) / 2, (z0 + z1) / 2, topY + 0.05f);
    }

    float cx = 10.0f, cz = 10.0f; float islandTopY = 0.3f;
    setDanauMat(0.32f, 0.72f, 0.28f, 8.0f);
    drawCylPlatform(cx, cz, 0.0f, 7.0f, islandTopY, 0.32f, 0.72f, 0.28f);
    drawDanauTree(cx, cz, islandTopY, 0);

    float mx = 5.0f, mz = -26.0f, monY = 0.11f;
    drawCylPlatform(mx, mz, monY, 7.0f, 0.3f, 0.82f, 0.74f, 0.6f);
    drawCylPlatform(mx, mz, monY + 0.3f, 5.0f, 0.25f, 0.85f, 0.77f, 0.63f);
    setDanauMat(0.96f, 0.96f, 0.97f, 20.0f);
    glPushMatrix(); glTranslatef(mx, monY + 0.55f + 4.0f, mz); glScalef(2.0f, 8.0f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
    setDanauMat(0.8f, 0.82f, 0.86f, 18.0f);
    glPushMatrix(); glTranslatef(mx, monY + 0.55f + 7.0f, mz + 0.6f); glScalef(1.2f, 1.2f, 0.2f); glutSolidSphere(0.5f, 24, 24); glPopMatrix();

    float parkY = 0.05f;
    setDanauMat(0.34f, 0.76f, 0.28f, 6.0f);
    glBegin(GL_POLYGON); glNormal3f(0, 1, 0);
    glVertex3f(10, parkY, -25); glVertex3f(-10, parkY, -35); glVertex3f(0, parkY, -40);
    glVertex3f(20, parkY, -40); glVertex3f(35, parkY, -32); glVertex3f(30, parkY, -18);
    glEnd();
    drawDanauTree(-5.0f, -30.0f, parkY, 0);
    drawDanauTree(25.0f, -20.0f, parkY, 1);
    drawDanauBench(0.0f, -23.0f, 0.0f);
    drawDanauBench(20.0f, -22.0f, -10.0f);

    float gx = 60.0f, gz = 69.0f, gy = 0.25f;
    setDanauMat(0.78f, 0.73f, 0.65f, 8.0f);
    glPushMatrix(); glTranslatef(gx, gy, gz); glScalef(18.0f, 0.5f, 8.0f); glutSolidCube(1.0f); glPopMatrix();
    setDanauMat(0.98f, 0.98f, 0.99f, 20.0f);
    glPushMatrix(); glTranslatef(gx - 4.0f, gy + 3.5f, gz + 2.0f); glScalef(1.2f, 7.0f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(gx + 4.0f, gy + 3.5f, gz + 2.0f); glScalef(1.2f, 7.0f, 1.0f); glutSolidCube(1.0f); glPopMatrix();
    glPushMatrix(); glTranslatef(gx, gy + 7.0f, gz + 2.0f); glScalef(12.0f, 1.2f, 1.2f); glutSolidCube(1.0f); glPopMatrix();

    for (int i = 0; i <= 5; ++i) {
        float t = (float)i / 5.0f;
        float x = -40.0f + t * 100.0f;
        drawDanauTree(x, 72.0f, 0.0f, i % 2);
    }
    for (float x = -70.0f; x <= 70.0f; x += 20.0f) {
        setDanauMat(0.1f, 0.1f, 0.1f, 10.0f);
        glPushMatrix(); glTranslatef(x, 4.5f, -50.0f); glScalef(0.2f, 9.0f, 0.2f); glutSolidCube(1.0f); glPopMatrix();
        setDanauMat(0.9f, 0.9f, 0.8f, 30.0f);
        glPushMatrix(); glTranslatef(x, 8.5f, -48.0f); glutSolidSphere(0.4f, 16, 16); glPopMatrix();
    }
}

// =========================================================================
// 9. INPUT & DISPLAY SYSTEM
// =========================================================================

void keyDown(unsigned char key, int x, int y) { keyStates[key] = true; if (key == 27) exit(0); }
void keyUp(unsigned char key, int x, int y) { keyStates[key] = false; }
void specialDown(int key, int x, int y) { specialStates[key] = true; }
void specialUp(int key, int x, int y) { specialStates[key] = false; }

void updateMovement() {
    float radYaw = camYaw * PI / 180.0f;
    if (keyStates['w'] || keyStates['W']) { camX += sin(radYaw) * speed; camZ -= cos(radYaw) * speed; }
    if (keyStates['s'] || keyStates['S']) { camX -= sin(radYaw) * speed; camZ += cos(radYaw) * speed; }
    if (keyStates['a'] || keyStates['A']) { camX -= cos(radYaw) * speed; camZ -= sin(radYaw) * speed; }
    if (keyStates['d'] || keyStates['D']) { camX += cos(radYaw) * speed; camZ += sin(radYaw) * speed; }
    if (keyStates['q'] || keyStates['Q']) camY += speed;
    if (keyStates['e'] || keyStates['E']) camY -= speed;

    if (specialStates[GLUT_KEY_LEFT]) camYaw -= 2.0f;
    if (specialStates[GLUT_KEY_RIGHT]) camYaw += 2.0f;
    if (specialStates[GLUT_KEY_UP]) { camPitch += 2.0f; if (camPitch > 89) camPitch = 89; }
    if (specialStates[GLUT_KEY_DOWN]) { camPitch -= 2.0f; if (camPitch < -89) camPitch = -89; }
}

// --- TAMBAHAN FUNGSI MOUSE ---
void mouseMove(int x, int y) {
    if (firstMouse) {
        lastMouseX = (float)x;
        lastMouseY = (float)y;
        firstMouse = false;
    }

    float xoffset = (float)x - lastMouseX;
    float yoffset = lastMouseY - (float)y; // Dibalik karena koordinat Y window dari atas ke bawah
    lastMouseX = (float)x;
    lastMouseY = (float)y;

    float sensitivity = 0.1f; // Atur sensitivitas mouse di sini
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camYaw += xoffset;
    camPitch += yoffset;

    // Batasi agar tidak bisa mendongak sampai terbalik (Gimbal Lock)
    if (camPitch > 89.0f) camPitch = 89.0f;
    if (camPitch < -89.0f) camPitch = -89.0f;

    // Refresh tampilan
    glutPostRedisplay();
}

void timer(int value) {
    updateMovement();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    float radYaw = camYaw * PI / 180.0f;
    float radPitch = camPitch * PI / 180.0f;
    gluLookAt(camX, camY, camZ, camX + sin(radYaw) * cos(radPitch), camY + sin(radPitch), camZ - cos(radYaw) * cos(radPitch), 0.0f, 1.0f, 0.0f);

    GLfloat lightPos[] = { 0.0f, 400.0f, 200.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    float dist = 400.0f;

    drawGlobalEnvironment(dist);

    // 1. HUKUM (KIRI)
    glPushMatrix(); glTranslatef(-dist, 0.0f, 0.0f);
    Gedung1_Hukum();
    glPopMatrix();

    // 2. FOODCOURT (TENGAH)
    glPushMatrix(); glTranslatef(0.0f, 0.0f, 0.0f);
    Gedung2_Foodcourt();
    glPopMatrix();

    // 3. FOODCOURT BARU (ANTARA TENGAH & KANAN) -> Posisi X = +250
    resetMaterial();
    glPushMatrix(); glTranslatef(250.0f, 0.0f, 0.0f);
    Gedung4_NewFC();
    glPopMatrix();

    // 4. DANAU (KANAN) -> X = +500
    glPushMatrix(); glTranslatef(dist + 100.0f, 0.0f, 0.0f);
    Gedung3_Danau();
    glPopMatrix();

    drawCoordinates();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(60, (float)w / h, 1.0f, 3000.0f);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1300, 700);
    glutCreateWindow("UNESA COMPLETE (Final Fix: Danau Full Detail)");

    glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING); glEnable(GL_LIGHT0); glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL); glShadeModel(GL_SMOOTH);
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    gQuadric = gluNewQuadric();
    gluQuadricNormals(gQuadric, GLU_SMOOTH);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutPassiveMotionFunc(mouseMove); // Aktifkan mouse look
    glutSetCursor(GLUT_CURSOR_NONE);  // Sembunyikan kursor mouse biar imersif
    glutKeyboardFunc(keyDown); glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown); glutSpecialUpFunc(specialUp);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}