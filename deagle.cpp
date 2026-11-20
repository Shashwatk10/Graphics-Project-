/*
 * COLOURED SOLID DESERT EAGLE PISTOL - 3D MODEL WITH ALL TRANSFORMATIONS
 * (Turbo C++ 3.0)
 * ----------------------------------------------------------------------
 * Model      : chrome slide with top rib and sights, wedge barrel with
 *              muzzle ring, steel frame with trigger guard, red grip,
 *              brass trigger / safety, slide serrations, ejection port
 * Rendering  : filled polygons, painter's algorithm (far to near),
 *              flat shading with a fixed light, perspective projection,
 *              page flipping (no flicker)
 * Transforms : Translation, Scaling, Rotation (X,Y,Z), Reflection, Shearing
 *
 * CONTROLS
 *   Translation : A / D = -X / +X    W / S = +Y / -Y    Q / E = -Z / +Z
 *   Scaling     : + / -  uniform     N / M = shrink / grow only in X
 *   Rotation    : 1 / 2 = about X    3 / 4 = about Y    5 / 6 = about Z
 *   Reflection  : X = YZ plane       Y = XZ plane       Z = XY plane
 *   Shearing    : H / J = shear X by Y (+/-)   K / L = shear Y by X (+/-)
 *   F = toggle solid / hidden-line wireframe
 *   O = auto demo of all transformations    R = reset    ESC = exit
 */

#include <graphics.h>
#include <conio.h>
#include <dos.h>
#include <math.h>
#include <stdio.h>
#include <ctype.h>

#define PI    3.14159265358979
#define MAXV  400
#define MAXF  400

/* ------------------------- 4x4 matrix utilities ------------------------- */
struct Mat4 { double m[4][4]; };

Mat4 identity()
{
    Mat4 r;
    int i, j;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            r.m[i][j] = (i == j) ? 1.0 : 0.0;
    return r;
}

Mat4 mul(Mat4 a, Mat4 b)
{
    Mat4 r;
    int i, j, k;
    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++) {
            r.m[i][j] = 0;
            for (k = 0; k < 4; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
        }
    return r;
}

Mat4 translateM(double tx, double ty, double tz)
{
    Mat4 r = identity();
    r.m[0][3] = tx; r.m[1][3] = ty; r.m[2][3] = tz;
    return r;
}

Mat4 scaleM(double sx, double sy, double sz)
{
    Mat4 r = identity();
    r.m[0][0] = sx; r.m[1][1] = sy; r.m[2][2] = sz;
    return r;
}

Mat4 rotateXM(double deg)
{
    double a = deg * PI / 180.0, c = cos(a), s = sin(a);
    Mat4 r = identity();
    r.m[1][1] = c; r.m[1][2] = -s;
    r.m[2][1] = s; r.m[2][2] = c;
    return r;
}

Mat4 rotateYM(double deg)
{
    double a = deg * PI / 180.0, c = cos(a), s = sin(a);
    Mat4 r = identity();
    r.m[0][0] = c;  r.m[0][2] = s;
    r.m[2][0] = -s; r.m[2][2] = c;
    return r;
}

Mat4 rotateZM(double deg)
{
    double a = deg * PI / 180.0, c = cos(a), s = sin(a);
    Mat4 r = identity();
    r.m[0][0] = c; r.m[0][1] = -s;
    r.m[1][0] = s; r.m[1][1] = c;
    return r;
}

/* axis 0 = YZ plane (x -> -x), 1 = XZ plane (y -> -y), 2 = XY plane (z -> -z) */
Mat4 reflectM(int axis)
{
    Mat4 r = identity();
    r.m[axis][axis] = -1;
    return r;
}

/* x' = x + shx*y      y' = y + shy*x */
Mat4 shearM(double shx, double shy)
{
    Mat4 r = identity();
    r.m[0][1] = shx;
    r.m[1][0] = shy;
    return r;
}

/* ------------------------------- materials ------------------------------ */
/* each material has 3 shades: dark, medium, bright (16-colour palette) */
#define M_SLIDE  0
#define M_FRAME  1
#define M_GRIP   2
#define M_BRASS  3
#define M_DARK   4

int mat[5][3] = {
    { DARKGRAY,  LIGHTGRAY, WHITE      },    /* chrome slide / barrel */
    { DARKGRAY,  DARKGRAY,  LIGHTGRAY  },    /* steel frame           */
    { RED,       LIGHTRED,  LIGHTRED   },    /* grip panels           */
    { BROWN,     YELLOW,    YELLOW     },    /* trigger, safety       */
    { BLACK,     BLACK,     DARKGRAY   }     /* ports, bore, slots    */
};

/* ------------------------------ 3D model -------------------------------- */
float VX[MAXV], VY[MAXV], VZ[MAXV];     /* model vertices (centred at 0) */
int   nV = 0;

int FA[MAXF][4];                        /* face vertex indices           */
int FN[MAXF];                           /* 3 = triangle, 4 = quad        */
int FM[MAXF];                           /* face material                 */
int nF = 0;

int addV(double x, double y, double z)
{
    VX[nV] = (float)x; VY[nV] = (float)y; VZ[nV] = (float)z;
    return nV++;
}

void addFace(int a, int b, int c, int d, int m)
{
    FA[nF][0] = a; FA[nF][1] = b; FA[nF][2] = c;
    if (d < 0) { FN[nF] = 3; FA[nF][3] = 0; }
    else       { FN[nF] = 4; FA[nF][3] = d; }
    FM[nF] = m;
    nF++;
}

/* axis-aligned box: centre (cx,cy,cz), half sizes (sx,sy,sz) */
void addBox(double cx, double cy, double cz,
            double sx, double sy, double sz, int m)
{
    int i, b = nV;
    for (i = 0; i < 8; i++)
        addV(cx + ((i & 1) ? sx : -sx),
             cy + ((i & 2) ? sy : -sy),
             cz + ((i & 4) ? sz : -sz));
    addFace(b + 0, b + 2, b + 6, b + 4, m);
    addFace(b + 1, b + 3, b + 7, b + 5, m);
    addFace(b + 0, b + 1, b + 5, b + 4, m);
    addFace(b + 2, b + 3, b + 7, b + 6, m);
    addFace(b + 0, b + 1, b + 3, b + 2, m);
    addFace(b + 4, b + 5, b + 7, b + 6, m);
}

/* slanted block: a quad in the XY plane (top edge x0t..x1t at height yt,
   bottom edge x0b..x1b at height yb) extruded from -hz to +hz in Z.
   Used for the sloping pistol grip.                                     */
void addSkew(double x0t, double x1t, double yt,
             double x0b, double x1b, double yb, double hz, int m)
{
    int b = nV;
    addV(x0t, yt, -hz); addV(x1t, yt, -hz);      /* b+0, b+1 */
    addV(x1b, yb, -hz); addV(x0b, yb, -hz);      /* b+2, b+3 */
    addV(x0t, yt,  hz); addV(x1t, yt,  hz);      /* b+4, b+5 */
    addV(x1b, yb,  hz); addV(x0b, yb,  hz);      /* b+6, b+7 */
    addFace(b + 0, b + 1, b + 2, b + 3, m);      /* back side  */
    addFace(b + 4, b + 5, b + 6, b + 7, m);      /* front side */
    addFace(b + 0, b + 1, b + 5, b + 4, m);      /* top        */
    addFace(b + 1, b + 2, b + 6, b + 5, m);      /* front edge */
    addFace(b + 2, b + 3, b + 7, b + 6, m);      /* bottom     */
    addFace(b + 3, b + 0, b + 4, b + 7, m);      /* rear edge  */
}

void buildGun()
{
    int i;
    double x;

    nV = 0; nF = 0;

    /* the barrel points toward +X, the grip hangs toward -Y */

    /* ---- slide (chrome) ---- */
    addBox(0.15, 0.40, 0, 1.85, 0.25, 0.27, M_SLIDE);      /* x -1.70..2.00 */

    /* ---- barrel: wedge-shaped, extends past the slide ---- */
    addBox(2.65, 0.40, 0, 0.65, 0.25, 0.20, M_SLIDE);      /* x  2.00..3.30 */

    /* ---- muzzle ring and dark bore ---- */
    addBox(3.325, 0.40, 0, 0.075, 0.27, 0.24, M_SLIDE);
    addBox(3.40, 0.40, 0, 0.03, 0.10, 0.08, M_DARK);

    /* ---- top rib with vent slots ---- */
    addBox(1.10, 0.69, 0, 2.10, 0.04, 0.07, M_SLIDE);
    for (i = 0; i < 3; i++)
        addBox(2.20 + 0.30 * i, 0.735, 0, 0.08, 0.01, 0.05, M_DARK);

    /* ---- sights ---- */
    addBox(-1.55, 0.72, 0, 0.12, 0.06, 0.20, M_FRAME);     /* rear  */
    addBox( 3.10, 0.78, 0, 0.05, 0.07, 0.04, M_FRAME);     /* front */

    /* ---- slide serrations (both sides) ---- */
    for (i = 0; i < 4; i++) {
        x = -1.55 + 0.14 * i;
        addBox(x, 0.40,  0.275, 0.025, 0.20, 0.01, M_DARK);
        addBox(x, 0.40, -0.275, 0.025, 0.20, 0.01, M_DARK);
    }

    /* ---- ejection port (both sides) ---- */
    addBox(0.40, 0.42,  0.275, 0.50, 0.12, 0.012, M_DARK);
    addBox(0.40, 0.42, -0.275, 0.50, 0.12, 0.012, M_DARK);

    /* ---- hammer ---- */
    addBox(-1.78, 0.30, 0, 0.08, 0.15, 0.04, M_FRAME);

    /* ---- frame with long dust cover ---- */
    addBox(0.40, 0.015, 0, 2.10, 0.135, 0.25, M_FRAME);    /* x -1.70..2.50 */

    /* ---- grip: frame strap + red side panels + base plate ---- */
    addSkew(-1.70, -0.35, -0.12, -2.05, -0.70, -2.50, 0.25, M_FRAME);
    addSkew(-1.60, -0.45, -0.20, -1.98, -0.78, -2.40, 0.29, M_GRIP);
    addSkew(-2.04, -0.685, -2.40, -2.10, -0.65, -2.58, 0.27, M_FRAME);

    /* ---- trigger guard (front bar + bottom bar) and trigger ---- */
    addBox(0.62, -0.45, 0, 0.07, 0.33, 0.10, M_FRAME);
    addBox(0.12, -0.75, 0, 0.57, 0.05, 0.10, M_FRAME);
    addBox(0.05, -0.35, 0, 0.04, 0.20, 0.04, M_BRASS);

    /* ---- safety levers (both sides) ---- */
    addBox(-1.20, 0.00,  0.28, 0.10, 0.05, 0.02, M_BRASS);
    addBox(-1.20, 0.00, -0.28, 0.10, 0.05, 0.02, M_BRASS);

    /* centre the whole pistol on the origin
       (length runs -2.10 .. 3.43, height -2.58 .. 0.78) */
    for (i = 0; i < nV; i++) {
        VX[i] -= 0.65f;
        VY[i] += 0.90f;
    }
}

/* ------------------------ transformation state -------------------------- */
Mat4 M;                    /* cumulative model matrix */
int  demo = 0;
int  demoFrame = 0;
int  solid = 1;
char status[60] = "Ready";

void resetModel()
{
    /* turn the pistol a little and tilt it so the 3D form is visible */
    M = mul(rotateXM(20), rotateYM(-30));
    sprintf(status, "Reset");
}

/* translation / reflection act in WORLD coordinates */
void applyWorld(Mat4 op)
{
    M = mul(op, M);
}

/* scaling / rotation / shear act about the pistol's OWN centre */
void applyAboutCentre(Mat4 op)
{
    double cx = M.m[0][3], cy = M.m[1][3], cz = M.m[2][3];
    M = mul(translateM(cx, cy, cz),
        mul(op, mul(translateM(-cx, -cy, -cz), M)));
}

/* ------------------------------ rendering ------------------------------- */
#define SW     640
#define SH     350
#define CAM    14.0
#define SCALE  42.0

float TX[MAXV], TY[MAXV], TZ[MAXV];     /* transformed vertices */
int   SX[MAXV], SY[MAXV], OK[MAXV];     /* screen coords        */
float FD[MAXF];                         /* face depth           */
int   order[MAXF];

/* light direction (unit length) in view space */
double LX = -0.35, LY = 0.60, LZ = 0.72;

void transformAll()
{
    int i;
    double x, y, z, d, w, fx, fy;
    for (i = 0; i < nV; i++) {
        x = M.m[0][0]*VX[i] + M.m[0][1]*VY[i] + M.m[0][2]*VZ[i] + M.m[0][3];
        y = M.m[1][0]*VX[i] + M.m[1][1]*VY[i] + M.m[1][2]*VZ[i] + M.m[1][3];
        z = M.m[2][0]*VX[i] + M.m[2][1]*VY[i] + M.m[2][2]*VZ[i] + M.m[2][3];
        TX[i] = (float)x; TY[i] = (float)y; TZ[i] = (float)z;

        d = CAM - z;
        OK[i] = 0;
        if (d < 1.0) continue;                 /* behind camera */
        w  = CAM / d;                          /* perspective   */
        fx = SW / 2 + x * SCALE * w;
        fy = SH / 2 - y * SCALE * w;
        if (fx < -3000 || fx > 3000 || fy < -3000 || fy > 3000) continue;
        SX[i] = (int)fx;
        SY[i] = (int)fy;
        OK[i] = 1;
    }
}

int shadeLevel(int f)
{
    int a = FA[f][0], b = FA[f][1], c = FA[f][2];
    double ex = TX[b] - TX[a], ey = TY[b] - TY[a], ez = TZ[b] - TZ[a];
    double fx = TX[c] - TX[a], fy = TY[c] - TY[a], fz = TZ[c] - TZ[a];
    double nx = ey * fz - ez * fy;
    double ny = ez * fx - ex * fz;
    double nz = ex * fy - ey * fx;
    double len = sqrt(nx*nx + ny*ny + nz*nz);
    double k;

    if (len < 1e-9) return 1;
    if (nz < 0) { nx = -nx; ny = -ny; nz = -nz; }   /* face the viewer */
    k = (nx * LX + ny * LY + nz * LZ) / len;
    if (k < 0.25) return 0;
    if (k < 0.60) return 1;
    return 2;
}

void drawFace(int f)
{
    int poly[8];
    int k, n = FN[f], v, col;

    for (k = 0; k < n; k++) {
        v = FA[f][k];
        poly[2 * k]     = SX[v];
        poly[2 * k + 1] = SY[v];
    }

    if (solid) {
        col = mat[FM[f]][shadeLevel(f)];
        setfillstyle(SOLID_FILL, col);
        setcolor(col);
    } else {                                   /* hidden-line wireframe */
        setfillstyle(SOLID_FILL, BLACK);
        setcolor(mat[FM[f]][2]);
    }
    fillpoly(n, poly);
}

void drawHelp()
{
    setcolor(WHITE);
    outtextxy(5, 4,  "MOVE: A/D W/S Q/E  SCALE: +/- N/M  REFLECT: X/Y/Z");
    outtextxy(5, 16, "ROTATE: 1/2(X) 3/4(Y) 5/6(Z)  SHEAR: H/J K/L");
    outtextxy(5, 28, "F: solid/wire   O: demo   R: reset   ESC: exit");
}

void drawStatus()
{
    char buf[90];
    setcolor(YELLOW);
    sprintf(buf, "Operation: %s", status);
    outtextxy(5, SH - 12, buf);
}

void render()
{
    static int page = 0;
    int i, j, cnt = 0, f, key, v, ok;
    double kd;

    setactivepage(page);
    cleardevice();

    transformAll();

    /* collect drawable faces and their average depth */
    for (f = 0; f < nF; f++) {
        ok = 1;
        kd = 0;
        for (i = 0; i < FN[f]; i++) {
            v = FA[f][i];
            if (!OK[v]) { ok = 0; break; }
            kd += CAM - TZ[v];
        }
        if (ok) {
            FD[f] = (float)(kd / FN[f]);
            order[cnt++] = f;
        }
    }

    /* painter's algorithm: sort far -> near (insertion sort) */
    for (i = 1; i < cnt; i++) {
        key = order[i];
        j = i - 1;
        while (j >= 0 && FD[order[j]] < FD[key]) {
            order[j + 1] = order[j];
            j--;
        }
        order[j + 1] = key;
    }

    for (i = 0; i < cnt; i++)
        drawFace(order[i]);

    drawHelp();
    drawStatus();

    setvisualpage(page);
    page = 1 - page;
}

/* ---------------------------- auto demo mode ---------------------------- */
void runDemoStep()
{
    int PH = 120;                          /* frames per phase */
    int phase = (demoFrame / PH) % 9;
    int f = demoFrame % PH;
    int first = (f < PH / 2);

    switch (phase) {
    case 0: sprintf(status, "DEMO: Translation");
        applyWorld(translateM(first ? 0.06 : -0.06, first ? 0.02 : -0.02, 0));
        break;
    case 1: sprintf(status, "DEMO: Scaling");
        if (first) applyAboutCentre(scaleM(1.01, 1.01, 1.01));
        else       applyAboutCentre(scaleM(1/1.01, 1/1.01, 1/1.01));
        break;
    case 2: sprintf(status, "DEMO: Rotation about X axis");
        applyAboutCentre(rotateXM(2)); break;
    case 3: sprintf(status, "DEMO: Rotation about Y axis");
        applyAboutCentre(rotateYM(2)); break;
    case 4: sprintf(status, "DEMO: Rotation about Z axis");
        applyAboutCentre(rotateZM(2)); break;
    case 5: sprintf(status, "DEMO: Reflection about XZ plane");
        if (f == 0 || f == PH / 2) applyWorld(reflectM(1)); break;
    case 6: sprintf(status, "DEMO: Reflection about YZ plane");
        if (f == 0 || f == PH / 2) applyWorld(reflectM(0)); break;
    case 7: sprintf(status, "DEMO: Shearing in X");
        applyAboutCentre(shearM(first ? 0.01 : -0.01, 0)); break;
    case 8: sprintf(status, "DEMO: Shearing in Y");
        applyAboutCentre(shearM(0, first ? 0.01 : -0.01)); break;
    }
    demoFrame++;
    if (demoFrame % (PH * 9) == 0) resetModel();
}

/* -------------------------------- main ---------------------------------- */
int main()
{
    int gd = VGA, gm = VGAMED;    /* 640x350, 16 colours, 2 video pages */
    int c;
    double STEP = 0.2, ANG = 5.0;

    /* change the path if your BGI folder is elsewhere */
    initgraph(&gd, &gm, "C:\\TC\\BGI");
    if (graphresult() != grOk) {
        printf("Graphics error. Check the BGI path in initgraph().");
        getch();
        return 1;
    }

    buildGun();
    resetModel();

    while (1) {
        if (kbhit()) {
            c = tolower(getch());

            if (c == 27) break;                          /* ESC */
            else if (c == 'o') { demo = !demo; demoFrame = 0; if (demo) resetModel(); }
            else if (c == 'r') { demo = 0; resetModel(); }
            else if (c == 'f') { solid = !solid; sprintf(status, solid ? "Solid view" : "Wireframe view"); }
            else if (!demo) {
                switch (c) {
                /* translation */
                case 'a': applyWorld(translateM(-STEP, 0, 0)); sprintf(status, "Translate -X"); break;
                case 'd': applyWorld(translateM( STEP, 0, 0)); sprintf(status, "Translate +X"); break;
                case 'w': applyWorld(translateM(0,  STEP, 0)); sprintf(status, "Translate +Y"); break;
                case 's': applyWorld(translateM(0, -STEP, 0)); sprintf(status, "Translate -Y"); break;
                case 'q': applyWorld(translateM(0, 0, -STEP)); sprintf(status, "Translate -Z (away)"); break;
                case 'e': applyWorld(translateM(0, 0,  STEP)); sprintf(status, "Translate +Z (closer)"); break;
                /* scaling */
                case '+': case '=':
                    applyAboutCentre(scaleM(1.1, 1.1, 1.1));
                    sprintf(status, "Scale up (uniform)"); break;
                case '-': case '_':
                    applyAboutCentre(scaleM(1/1.1, 1/1.1, 1/1.1));
                    sprintf(status, "Scale down (uniform)"); break;
                case 'n': applyAboutCentre(scaleM(1/1.1, 1, 1)); sprintf(status, "Scale X down (non-uniform)"); break;
                case 'm': applyAboutCentre(scaleM(1.1, 1, 1));   sprintf(status, "Scale X up (non-uniform)"); break;
                /* rotation */
                case '1': applyAboutCentre(rotateXM( ANG)); sprintf(status, "Rotate about X (+5 deg)"); break;
                case '2': applyAboutCentre(rotateXM(-ANG)); sprintf(status, "Rotate about X (-5 deg)"); break;
                case '3': applyAboutCentre(rotateYM( ANG)); sprintf(status, "Rotate about Y (+5 deg)"); break;
                case '4': applyAboutCentre(rotateYM(-ANG)); sprintf(status, "Rotate about Y (-5 deg)"); break;
                case '5': applyAboutCentre(rotateZM( ANG)); sprintf(status, "Rotate about Z (+5 deg)"); break;
                case '6': applyAboutCentre(rotateZM(-ANG)); sprintf(status, "Rotate about Z (-5 deg)"); break;
                /* reflection */
                case 'x': applyWorld(reflectM(0)); sprintf(status, "Reflect about YZ plane"); break;
                case 'y': applyWorld(reflectM(1)); sprintf(status, "Reflect about XZ plane"); break;
                case 'z': applyWorld(reflectM(2)); sprintf(status, "Reflect about XY plane"); break;
                /* shearing */
                case 'h': applyAboutCentre(shearM( 0.1, 0)); sprintf(status, "Shear X by Y (+0.1)"); break;
                case 'j': applyAboutCentre(shearM(-0.1, 0)); sprintf(status, "Shear X by Y (-0.1)"); break;
                case 'k': applyAboutCentre(shearM(0,  0.1)); sprintf(status, "Shear Y by X (+0.1)"); break;
                case 'l': applyAboutCentre(shearM(0, -0.1)); sprintf(status, "Shear Y by X (-0.1)"); break;
                }
            }
        }
        if (demo) runDemoStep();
        render();
        delay(20);
    }

    closegraph();
    return 0;
}
