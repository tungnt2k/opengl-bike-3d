#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#define PI 3.14159f
#define WIN_WIDTH 600
#define WIN_HEIGHT 600
#define CYCLE_LENGTH 3.3f
#define ROD_RADIUS 0.05f
#define NUM_SPOKES 20
#define SPOKE_ANGLE 18
#define RADIUS_WHEEL 1.0f
#define TUBE_WIDTH 0.08f
#define RIGHT_ROD 1.6f
#define RIGHT_ANGLE 47.0f
#define MIDDLE_ROD 1.7f
#define MIDDLE_ANGLE 106.0f
#define BACK_CONNECTOR 0.5f
#define LEFT_ANGLE 50.0f
#define WHEEL_OFFSET 0.11f
#define WHEEL_LEN 1.1f
#define TOP_LEN 1.5f
#define CRANK_ROD 0.7f
#define CRANK_RODS 1.12f
#define CRANK_ANGLE 8.0f
#define HANDLE_ROD 1.2f
#define FRONT_INCLINE 70.0f
#define HANDLE_LIMIT 70.0f

#define INC_STEERING 2.0f
#define INC_SPEED 0.01f

/*****************************************
*    Biến
****************************************/
/*****************************************
*   Cycle - biến của xe đạp
******************************************/
GLfloat pedalAngle, speed, steering;

/*******************************
*   User view - biến để xem
********************************/
GLfloat camx, camy, camz;
GLfloat anglex, angley, anglez;

/****************************
*   biến để dùng chuột
****************************/
int prevx, prevy;
GLenum Mouse;

/**************************
*   Biến vị trí xe
***************************/
GLfloat xpos, zpos, direction;

void ZCylinder(GLfloat radius, GLfloat length);
void XCylinder(GLfloat radius, GLfloat length);

void drawFrame(void);
void gear(GLfloat inner_radius, GLfloat outer_radius,
    GLfloat width, GLint teeth, GLfloat tooth_depth);
void drawChain(void);
void drawPedals(void);
void drawTyre(void);
void drawSeat(void);
void help(void);
void init(void);
void reset(void);
void display(void);
void idle(void);
void updateScene(void);
void landmarks(void);
void special(int key, int x, int y);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void reshape(int w, int h);
void glSetupFuncs(void);
GLfloat Abs(GLfloat);
GLfloat degrees(GLfloat);
GLfloat radians(GLfloat);
GLfloat angleSum(GLfloat, GLfloat);
GLuint texture[1];

GLuint LoadTexture(const char* filename, int width, int height)
{

    GLuint texture;
    unsigned char* data;
    FILE* file;
    file = fopen(filename, "rb");
    if (file == NULL) return 0;
    data = (unsigned char*)malloc(width * height * 3);
    fread(data, width * height * 3, 1, file);
    fclose(file);


    glGenTextures(1, &texture);


    glBindTexture(GL_TEXTURE_2D, texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);



    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

    return texture;
}

void ZCylinder(GLfloat radius, GLfloat length)
{
    GLUquadricObj* cylinder;
    cylinder = gluNewQuadric();
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.04f);
    gluCylinder(cylinder, radius, radius, length, 15, 5);
    glPopMatrix();
}

void XCylinder(GLfloat radius, GLfloat length)
{
    glPushMatrix();
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    ZCylinder(radius, length);
    glPopMatrix();
}

// được gọi bởi hàm idle()
void updateScene()
{
    GLfloat xDelta, zDelta;
    GLfloat rotation;
    GLfloat sin_steering, cos_steering;

    // Nếu bánh k chuyển động thì không làm gì
    if (-INC_SPEED < speed && speed < INC_SPEED)
        return;

    if (speed < 0.0f)
        pedalAngle = speed = 0.0f;

    // ngược lại thì tính toán lại vị trí của bánh xe
    // và độ xoay của mỗi bánh
    // bánh xe đã di chuyển "speed*(time elapsed)".
    // Mặc đinh là 1 "(time elapsed)=1".

    xDelta = speed * cos(radians(direction + steering));
    zDelta = speed * sin(radians(direction + steering));
    xpos += xDelta;
    zpos -= zDelta;
    pedalAngle = degrees(angleSum(radians(pedalAngle), speed / RADIUS_WHEEL));

    // sử dụng hàm sin và cos để tính toán hướng lái
    // tính lại mỗi lần thay đổi

    sin_steering = sin(radians(steering));
    cos_steering = cos(radians(steering));

    // see the assignment 3 "Hint"
    rotation = atan2(speed * sin_steering, CYCLE_LENGTH + speed * cos_steering);
    direction = degrees(angleSum(radians(direction), rotation));
}

//tính tổng góc angleSum(a,b) = (a+b) MOD 2*PI
// a và b là 2 góc (radians)
// cả hai đều nằm giữa 0 và 2*PI
GLfloat angleSum(GLfloat a, GLfloat b)
{
    a += b;
    if (a < 0)
        return a + 2 * PI;
    else if (a > 2 * PI)
        return a - 2 * PI;
    else
        return a;
}

/************************************************
    * Vẽ khung xe kim loại cho xe, yên xe và bánh sau.

************************************************/

void drawFrame() // vẽ khung
{
    //glColor3f(0.0f, 1.0f, 0.15f);
    /******
    Đầu tiên, vẽ tất cả các chi tiết ở tâm khung.
    Bao gồm: Bánh răng lớn, trục xe (đóng vai trò
    như ổ cắm cho bàn đạp). Vẽ 2 thành trục để nối
    với bàn đạp.
    ******/
    glPushMatrix();
    /******************************
    *   Cho phép tôi vẽ bánh răng lớn
    *   Bánh răng và trục xe
    *******************************/
    glPushMatrix();
    /***************************
    *   Màu Bánh răng lớn:
    *   - Xanh lá
    ***************************/
    glColor3f(0.07f, 0.07f, 0.07f);

    /**************************
    *   Bánh răng:
    *   - Nằm ngoài khung
    *   - Là Bánh răng lớn
    *   - GEAR
    ***************************/
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.06f);
    glRotatef(-2 * pedalAngle, 0.0f, 0.0f, 1.0f);
    gear(0.08f, 0.3f, 0.03f, 30, 0.03f);
    glPopMatrix();
    /***************************
    *   Màu sắc của khung giữa hình chữ "V"
    ****************************/
    glColor3f(0.0f, 0.15f, 0.45f);
    /***************************
    *   Trục nối bàn đạp
    ****************************/
    glTranslatef(0.0f, 0.0f, -0.34f);
    ZCylinder(0.08f, 0.60f);
    glPopMatrix();
    /*****************************
    *   Đầu tiên vẽ khung xe nối với bánh trước
    *******************************/
    glRotatef(RIGHT_ANGLE, 0.0f, 0.0f, 1.0f);
    XCylinder(ROD_RADIUS, RIGHT_ROD);



    /*******************************
    *   Vẽ yên xe
    *   Vẽ khung nối chỗ ngồi với yên xe
    *********************************/
    glRotatef(MIDDLE_ANGLE - RIGHT_ANGLE, 0.0f, 0.0f, 1.0f);
    XCylinder(ROD_RADIUS, MIDDLE_ROD);
    /********************************
    *   Vẽ yên xe

    *********************************/

    glTranslatef(MIDDLE_ROD, 0.0f, 0.0f);
    glRotatef(-MIDDLE_ANGLE, 0.0f, 0.0f, 1.0f);
    glScalef(0.3f, ROD_RADIUS, 0.25f);
    drawSeat();


    /**********************
    *   Màu sắc của khung nối của bánh sau với bánh răng lớn.
    ************************/
    glColor3f(0.0f, 0.15f, 0.45f);
    glPopMatrix();

    /*glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(MIDDLE_ROD, 0.0f, 0.0f);
    glRotatef(-MIDDLE_ANGLE, 0.0f, 0.0f, 1.0f);
    glScalef(0.3f, ROD_RADIUS, 0.25f);

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();*/
    /*********************************
    *   Draw the horizontal part of
    *   the frame.
    *********************************/

    /*********************************
    *   Vẽ khung nối của bánh sau với bánh răng lớn
    **********************************/
    glPushMatrix();
    glRotatef(-180.0f, 0.0f, 1.0f, 0.0f);
    XCylinder(ROD_RADIUS, BACK_CONNECTOR);

    /***********************************
    *   Vẽ 2 que khung :
    *   - Nối với bánh xe với bánh răng lớn
    ************************************/
    glPushMatrix();
    glTranslatef(0.5f, 0.0f, WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, RADIUS_WHEEL + TUBE_WIDTH);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5f, 0.0f, -WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, RADIUS_WHEEL + TUBE_WIDTH);
    glPopMatrix();
    glPopMatrix();

    /************************************
    * vẽ khung dưới của xe (nối càng, yên và bánh sau)
    *************************************/
    glPushMatrix();
    glTranslatef(-(BACK_CONNECTOR + RADIUS_WHEEL + TUBE_WIDTH), 0.0f, 0.0f);


    glPushMatrix();
    glRotatef(-2 * pedalAngle, 0.0f, 0.0f, 1.0f);

    // vẽ bánh sau
    drawTyre();

    //   Vẽ chắn bùn
    glPushMatrix();
    glRotatef(2 * pedalAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (GLfloat i = -90; i < 60; i += 2)
    {
        glPushMatrix();
        glRotatef(i, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f, 1.12f, -0.14f);
        glScalef(4.0f, 1.0f, 1.0f);
        ZCylinder(0.01f, 0.2f);
        glPopMatrix();
    }
    glPopMatrix();

    glRotatef(-2 * 10, 0.0f, 0.0f, 1.0f);

    glColor3f(0.07f, 0.07f, 0.07f);

    // vẽ nhông xe
    gear(0.03f, 0.15f, 0.03f, 20, 0.03f);
    glColor3f(0.0f, 0.15f, 0.45f);
    glPopMatrix();
    glRotatef(LEFT_ANGLE, 0.0f, 0.0f, 1.0f);



    /*****************************
    *   Vẽ thanh nối giữa khung nối bánh xe và khung xe
    ******************************/
    glPushMatrix();
    glTranslatef(0.7f, -0.85f, -0.2f);
    ZCylinder(ROD_RADIUS, 0.32);
    glPopMatrix();

    /*****************************
    *   Vẽ chỗ để chân bánh sau
    ******************************/
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.43f);
    ZCylinder(ROD_RADIUS, 0.8);
    glPopMatrix();

    /*****************************
    *   Vẽ thanh nối để khung không bị hở
    ******************************/
    glPushMatrix();
    glTranslatef(WHEEL_LEN + 0.01f, 0.0f, -0.2f);
    ZCylinder(ROD_RADIUS, 0.32);
    glPopMatrix();


    /*****************************
    *   Vẽ gacbaga
    ******************************/
    // trục nối
    glPushMatrix();
    glTranslatef(1.4f, -0.02f, -0.0f);
    glRotatef(-260.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.03, 0.25);
    glPopMatrix();

    ////khung liền
    
    for (GLfloat i = 0; i < 1.0; i += 0.1)
    {
        glPushMatrix();
        glRotatef(40.0f, 0.0f, 0.0f, 1.0f);
        glTranslatef(1.2f, -0.7f + i, -0.16f);
        ZCylinder(0.015, 0.24);
        glPopMatrix();
    }

    //thanh nối gacbaga
    glPushMatrix();
    glRotatef(130.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.75f, -1.2f, WHEEL_OFFSET);
    XCylinder(0.015, 0.95);
    glPopMatrix();

    glPushMatrix();
    glRotatef(130.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-0.75f, -1.2f, -WHEEL_OFFSET);
    XCylinder(0.015, 0.95);
    glPopMatrix();


    //thanh trống gacbaga
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, WHEEL_OFFSET);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.02, 1.19);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -WHEEL_OFFSET);
    glRotatef(45.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.02, 1.19);
    glPopMatrix();

    /************************************
    *   Vẽ 2 que khung chéo nối giữa bánh sau và 1 phần của khung chính
    *************************************/
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, WHEEL_LEN);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, WHEEL_LEN);
    glPopMatrix();

    /*****************************
    *   Vẽ khung đơn nối với tay cầm
    ******************************/


    glTranslatef(WHEEL_LEN, 0.0f, 0.0f);
    XCylinder(ROD_RADIUS, CRANK_ROD);



    /*****************************
    * vẽ thanh ngang khung trên và hướng của khung
    *****************************/
    glTranslatef(CRANK_ROD, 0.0f, 0.0f);
    glRotatef(-LEFT_ANGLE, 0.0f, 0.0f, 1.0f);
    XCylinder(ROD_RADIUS, 1.45);

    glTranslatef(TOP_LEN, 0.0f, 0.0f);
    glRotatef(-FRONT_INCLINE, 0.0f, 0.0f, 1.0f);

    /******************************
    *   Vẽ cái thanh khung nhỏ nối giữa
    tay cầm và thanh khung chính
    ******************************/
    glPushMatrix();
    glTranslatef(-0.1f, 0.0f, 0.0f);
    XCylinder(ROD_RADIUS, 0.45f);
    glPopMatrix();


    /******************************
    *   Xoay đầu xe
    *******************************/
    glPushMatrix();
    glRotatef(-steering, 1.0f, 0.0f, 0.0f);
    /******************************
    *   Nối giữa tay cầm và khung bánh trước
    *******************************/
    glTranslatef(-0.3f, 0.0f, 0.0f);
    glPushMatrix();
    glRotatef(FRONT_INCLINE, 0.0f, 0.0f, 1.0f);

    // vẽ tay lái
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -HANDLE_ROD / 2);
    ZCylinder(ROD_RADIUS, HANDLE_ROD);
    glPopMatrix();


    // vẽ bọc tay lái
    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.0f, 0.0f, -HANDLE_ROD / 2);
    ZCylinder(0.07f, HANDLE_ROD / 4);
    glTranslatef(0.0f, 0.0f, HANDLE_ROD * 3 / 4);
    ZCylinder(0.07f, HANDLE_ROD / 4);
    glColor3f(0.0f, 0.15f, 0.45f);
    glPopMatrix();

    // vẽ tay phanh 

    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.18f, 0.0f, -HANDLE_ROD / 2);
    glRotatef(-20.0f, 0.0f, 1.0f, 0.0f);
    ZCylinder(0.02f, HANDLE_ROD / 4);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.5f, 0.5f, 0.5f);
    glTranslatef(0.06f, 0.0f, HANDLE_ROD / 4 );
    glRotatef(20.0f, 0.0f, 1.0f, 0.0f);
    ZCylinder(0.02f, HANDLE_ROD / 4);
    glColor3f(0.0f, 0.15f, 0.45f);
    glPopMatrix();

    glPopMatrix();

    /*********************************
    *   Sử dụng độ nghiêng trên để vẽ tay cầm.
    *   Có thể sử dụng lại để vẽ bánh xe
    **********************************/
    glPushMatrix();
    /****************************
    *   Vẽ thanh khung chính
    ****************************/
    XCylinder(ROD_RADIUS, CRANK_ROD);

    /******************************
    *   Why not draw the two rods and
    *   the WHEEL?   :)
    *   Yes!So,first go to the
    *   end of the main rod.
    *******************************/
    glTranslatef(CRANK_ROD, 0.0f, 0.0f);
    glRotatef(CRANK_ANGLE, 0.0f, 0.0f, 1.0f);

    /******************************
 *   vẽ thanh nối giữa cổ xe nhỏ với bánh trước
 *******************************/
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.190f);
    ZCylinder(ROD_RADIUS, 0.3);
    glPopMatrix();
    /*******************************
    *  Vẽ hai que khung nối giữa tay cầm với bánh trước
    ********************************/

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, CRANK_RODS);
    glPopMatrix();

    /*******************************
    *  Vẽ khung nối nan hoa cho bánh trước
    ********************************/
    glPushMatrix();
    glTranslatef(WHEEL_LEN + 0.01f, 0.0f, -0.190f);
    ZCylinder(ROD_RADIUS, 0.3);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -WHEEL_OFFSET);
    XCylinder(ROD_RADIUS, CRANK_RODS);
    glPopMatrix();



    /********************************
    *   Hiệu ứng cho bánh trước
    *********************************/
    glTranslatef(CRANK_RODS, 0.0f, 0.0f);
    glRotatef(-2 * pedalAngle, 0.0f, 0.0f, 1.0f);
    drawTyre();

    glPushMatrix();
    glRotatef(2 * pedalAngle, 0.0f, 0.0f, 1.0f);
    glColor3f(1.0f, 1.0f, 1.0f);
    for (GLfloat i = 20; i < 150; i += 2)
    {
        glPushMatrix();
        glRotatef(i, 0.0f, 0.0f, 1.0f);
        glTranslatef(0.0f, 1.12f, -0.14f);
        glScalef(4.0f, 1.0f, 1.0f);
        ZCylinder(0.01f, 0.2f);
        glPopMatrix();
    }
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
    glPopMatrix();

    //   Vẽ chắn bùn
   


}

void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width,
    GLint teeth, GLfloat tooth_depth)
{
    GLint i;
    GLfloat r0, r1, r2;
    GLfloat angle, da;
    GLfloat u, v, len;
    const double pi = 3.14159264;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * pi / teeth / 4.0;

    glShadeModel(GL_FLAT);

    glNormal3f(0.0, 0.0, 1.0);

    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++)
    {
        angle = i * 8.0 * pi / teeth;
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
    glEnd();

    /* mặt trước của nhông lớn nối với bàn đạp */
    glBegin(GL_QUADS);
    da = 2.0 * pi / teeth / 4.0;
    for (i = 0; i < teeth; i++)
    {
        angle = i * 2.0 * pi / teeth;

        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
    }
    glEnd();

    glNormal3f(0.0, 0.0, -1.0);

    /* mặt sau của nhông lớn nối với bàn đạp */
    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++)
    {
        angle = i * 2.0 * pi / teeth;
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
    }
    glEnd();


    glBegin(GL_QUADS);
    da = 2.0 * pi / teeth / 4.0;
    for (i = 0; i < teeth; i++)
    {
        angle = i * 2.0 * pi / teeth;

        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
    }
    glEnd();


    glBegin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++)
    {
        angle = i * 2.0 * pi / teeth;

        glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
        glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
        u = r2 * cos(angle + da) - r1 * cos(angle);
        v = r2 * sin(angle + da) - r1 * sin(angle);
        len = sqrt(u * u + v * v);
        u /= len;
        v /= len;
        glNormal3f(v, -u, 0.0);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
        glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
        glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
        u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
        v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
        glNormal3f(v, -u, 0.0);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
        glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
        glNormal3f(cos(angle), sin(angle), 0.0);
    }

    glVertex3f(r1 * cos(0.0), r1 * sin(0.0), width * 0.5);
    glVertex3f(r1 * cos(0.0), r1 * sin(0.0), -width * 0.5);

    glEnd();

    glShadeModel(GL_SMOOTH);


    glBegin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++)
    {
        angle = i * 2.0 * pi / teeth;
        glNormal3f(-cos(angle), -sin(angle), 0.0);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
        glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
    }
    glEnd();
}

/******************************************
*   Vẽ xích xe
******************************************/
void drawChain()
{
    GLfloat depth;
    static int mode = 0;

    glColor3f(0.5f, 0.5f, 0.5f);
    glEnable(GL_LINE_STIPPLE);
    mode = (mode + 1) % 2;

    if (mode == 0 && speed > 0)
        glLineStipple(1, 0x1c47);
    else if (mode == 1 && speed > 0)
        glLineStipple(1, 0x00FF);

    glBegin(GL_LINES);
    for (depth = 0.06f; depth <= 0.12f; depth += 0.01f)
    {
        glVertex3f(-1.6f, 0.15f, ROD_RADIUS);
        glVertex3f(0.0f, 0.3f, depth);

        glVertex3f(-1.6f, -0.15f, ROD_RADIUS);
        glVertex3f(0.0f, -0.3f, depth);
    }
    glEnd();
    glDisable(GL_LINE_STIPPLE);
}

/************************************
*   Vẽ ghế ngồi
************************************/
void drawSeat()
{

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[0]);

    /*********************************
    *   Vẽ phần trên của ghế ngồi
    **********************************/

    glBegin(GL_POLYGON);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 1.0f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, -0.3f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.3f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.1f, 1.0f, 0.5f);
    glTexCoord2f(0.0f, 0.5f); glVertex3f(-0.5f, 1.0f, 1.0f);
    glTexCoord2f(0.5f, 0.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 1.0f, -1.0f);
    glEnd();

    /**********************************
    *   Vẽ phần dưới của ghế ngồi
    ************************************/
    glBegin(GL_POLYGON);
    glVertex3f(-0.1f, -1.0f, -0.5f);
    glVertex3f(1.0f, -1.0f, -0.3f);
    glVertex3f(1.0f, -1.0f, 0.3f);
    glVertex3f(-0.1f, -1.0f, 0.5f);
    glVertex3f(-0.5f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-0.5f, -1.0f, -1.0f);
    glEnd();

    /**********************
    *   Vẽ ở hai bên
    ***********************/
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.3f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1.0f, 1.0f, 0.3f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1.0f, -1.0f, 0.3f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -0.3f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, 0.3f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1f, 1.0f, 0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.1f, -1.0f, 0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, 0.3f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(1.0f, 1.0f, -0.3f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.1f, 1.0f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.1f, -1.0f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(1.0f, -1.0f, -0.3f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 1.0f, 0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.1f, -1.0f, 0.5f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.1f, 1.0f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.1f, -1.0f, -0.5f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f);  glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, 1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -1.0f, 1.0f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 1.0f, -1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -1.0f, -1.0f);

    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, 1.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, 1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, 1.0f);

    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}


/************************************
*   Vẽ bàn đạp
************************************/
void drawPedals()
{
    glColor3f(0.5f, 0.5f, 0.5f);
    /***************************
    *   Vẽ bàn đạp cách đều khung
    ****************************/

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.25f);
    glRotatef(-pedalAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.25f, 0.0f, 0.0f);
    /*************************
    *  Vẽ cần đạp
    *************************/
    glPushMatrix();
    glScalef(0.5f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /************************
    *  Vẽ đệm bàn đạp
    ************************/
    glPushMatrix();
    glTranslatef(0.25f, 0.0f, 0.15f);
    glRotatef(pedalAngle, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.02f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();

    /*******************************
    *   Vẽ chiều còn lại
    *******************************/
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.25f);
    glRotatef(180.0f - pedalAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(0.25f, 0.0f, 0.0f);

    /***************************
    *   Vẽ thêm cần đạp
    ****************************/
    glPushMatrix();
    glScalef(0.5f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    /****************************
    *   Vẽ phần còn lại của bàn đạp
    *****************************/
    glPushMatrix();
    glTranslatef(0.25f, 0.0f, -0.15f);
    glRotatef(pedalAngle - 180.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.2f, 0.02f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();

    glColor3f(1.0f, 0.0f, 0.0f);
}

/************************************
*   Vẽ bánh xe
************************************/
void drawTyre(void)
{
    int i;
    //   Vẽ lốp bánh xe
    glColor3f(1.0f, 1.0f, 1.0f);
    glutSolidTorus(0.06f, 0.92f, 4, 30);
    //   Trục bánh xe
    //   Chiều dài  0.12f
    glColor3f(0.07f, 0.07f, 0.07f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.06f);
    ZCylinder(0.02f, 0.12f);
    glPopMatrix();
    glutSolidTorus(0.02f, 0.02f, 3, 20);

    //   Vẽ nan hoa
    glColor3f(1.0f, 1.0f, 1.0f);
    for (i = 0; i < NUM_SPOKES; ++i)
    {
        glPushMatrix();
        glRotatef(i * SPOKE_ANGLE, 0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.02f, 0.0f);
        glVertex3f(0.0f, 0.86f, 0.0f);
        glEnd();
        glPopMatrix();
    }



    //   Vẽ lốp xe
    glColor3f(0.0f, 0.0f, 0.0f);
    glutSolidTorus(TUBE_WIDTH, RADIUS_WHEEL, 10, 30);
    glColor3f(1.0f, 0.0f, 0.0f);
}


void init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 100.0 };
    GLfloat light_directional[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_positional[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0 };

    reset();

    glShadeModel(GL_SMOOTH);
    // chiếu sáng
    glLightfv(GL_LIGHT0, GL_POSITION, light_directional);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_diffuse);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_positional);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glColorMaterial(GL_FRONT, GL_DIFFUSE);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glDepthFunc(GL_LESS);

}

/************************************
*   Vẽ mặt phẳng lưới
************************************/
void landmarks(void)
{
    glPushMatrix();
    //    //tạo liên kết texture đến dạng hình của chúng 
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[2]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-1000.0f, -1.08f, -1000.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(1000.0f, -1.08f, -1000.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(1000.0f, -1.08f, 1000.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-1000.0f, -1.08f, 1000.0f);

    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();


    // Vẽ biển báo
   




    for (int i = -10; i < 10; i++) {
        glPushMatrix();
        //tạo liên kết texture đến dạng hình của chúng 
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, -1.07f, -10.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(i * 20 + 10.0f, -1.07f, -10.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(i * 20 + 10.0f, -1.07f, 10.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-10.0f, -1.07f, 10.0f);

        glEnd();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        glPushMatrix();
        //tạo liên kết texture đến dạng hình của chúng 
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texture[3]);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-10.0f, -1.07f, -10.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(30.0f, -1.07f, -10.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(30.0f, -1.07f, 10.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-10.0f, -1.07f, 10.0f);

        glEnd();
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

}

void square1() {
    glPushMatrix();
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.014, 0.0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1.014, 1.0); glVertex3f(1, 1, -1);
    glTexCoord2f(2.014, 1.0); glVertex3f(1, -1, -1);
    glTexCoord2f(2.014, 0.0); glVertex3f(-1, -1, -1);
    glEnd();


    glPopMatrix();
}

void square2() {
    glPushMatrix();
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[5]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.014, 0.0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1.014, 1.0); glVertex3f(1, 1, -1);
    glTexCoord2f(2.014, 1.0); glVertex3f(1, -1, -1);
    glTexCoord2f(2.014, 0.0); glVertex3f(-1, -1, -1);
    glEnd();


    glPopMatrix();
}

void square3() {
    glPushMatrix();
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[6]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.014, 0.0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1.014, 1.0); glVertex3f(1, 1, -1);
    glTexCoord2f(2.014, 1.0); glVertex3f(1, -1, -1);
    glTexCoord2f(2.014, 0.0); glVertex3f(-1, -1, -1);
    glEnd();


    glPopMatrix();
}

void square4() {
    glPushMatrix();
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[7]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.014, 0.0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1.014, 1.0); glVertex3f(1, 1, -1);
    glTexCoord2f(2.014, 1.0); glVertex3f(1, -1, -1);
    glTexCoord2f(2.014, 0.0); glVertex3f(-1, -1, -1);
    glEnd();


    glPopMatrix();
}

void square5() {
    glPushMatrix();
    //tạo liên kết texture đến dạng hình của chúng 
    glBindTexture(GL_TEXTURE_2D, texture[4]);
    glBegin(GL_QUADS);
    glTexCoord2f(1.014, 0.0); glVertex3f(-1, 1, -1);
    glTexCoord2f(1.014, 1.0); glVertex3f(1, 1, -1);
    glTexCoord2f(2.014, 1.0); glVertex3f(1, -1, -1);
    glTexCoord2f(2.014, 0.0); glVertex3f(-1, -1, -1);
    glEnd();


    glPopMatrix();
}


void vinmart(void)
{

    //Trần
    glPushMatrix();
    glColor3f(0.157, 0.321, 0.388);
    glTranslatef(0.0f, 7.4f, -35.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.2, 40, 20);
    glutSolidCube(0.5);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.863, 0.690, 0.533);
    glTranslatef(11.2f, -0.45f, -23.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    glScalef(2, 2, 2);
    glutSolidCube(0.5);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.863, 0.690, 0.533);
    glTranslatef(12.5f, -0.3f, -21.8f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(70.0f, 1.0f, 0.0f, 0.0f);
    glScalef(2.6, 2.6, 2.6);
    glutSolidCube(0.5);
    glPopMatrix();

    //điều_hòa
    glPushMatrix();
    glColor3f(0.376, 0.368, 0.372);
    glTranslatef(9.8f, 6.5f, -32.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(2, 2, 4);
    glutSolidCube(0.5);
    glPopMatrix();

    // nhà
    glPushMatrix();
    glColor3f(0.2352, 0.3176, 0.5254);
    glTranslatef(0.0f, 3.8f, -35.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(18, 38, 20);
    glutSolidCube(0.5);
    glPopMatrix();

    // Dán tường
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(-2.6f, 3.5f, -35.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(5, 5, 7);
    square3();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(16.8f, 3.5f, -35.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(5, 5, 7);
    square3();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(0.0f, 3.5f, -33.5f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(5, 10, 7);
    square3();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(0.0f, 4.0f, -22.9f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(5, 10, 7);
    square5();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    //Biển Quảng Cáo
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(0.0f, 7.8f, -27.86f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.2, 5.02, 2);
    square2();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    //Đường ống nước
    glPushMatrix();
    glColor3f(0.658, 0.588, 0.643);
    glTranslatef(9.8f, -1.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.12, 8.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 1.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 3.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 5.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();
    //bậc
    glPushMatrix();
    glColor3f(0.157, 0.321, 0.388);
    glTranslatef(0.0f, -0.6f, -33.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.5, 40, 30);
    glutSolidCube(0.5);
    glPopMatrix();
    //sàn
    glPushMatrix();
    glColor3f(0.376, 0.368, 0.372);
    glTranslatef(0.0f, -1.0f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.1, 60, 70);
    glutSolidCube(0.5);
    glPopMatrix();

    // Hàng rào
    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(14.8f, 0.5f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(14.8f, -0.2f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(-14.8f, 0.5f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(-14.8f, -0.2f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();


    // vẽ cột đứng
    // bên trái
    for (GLfloat i = -17.0; i < 18; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(-14.8f, -1.0f, -28.0f + i);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }

    // bên phải
    for (GLfloat i = -17.0; i < 18; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(14.8f, -1.0f, -28.0f + i);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }

    // đằng sau
    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(0.0f, 0.5f, -45.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.0f, 1.0f, 150.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(0.0f, -0.2f, -45.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.0f, 1.0f, 150.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    for (GLfloat i = 1; i < 30; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(-14.8f + i, -1.0f, -45.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }


}

void circleK(void)
{

    //mái_che
    glPushMatrix();
    glColor3f(0.168, 0.407, 0.501);
    glTranslatef(0.0f, 4.8f, -29.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.375, 20, 4);
    glutSolidCube(0.5);
    glPopMatrix();

    //Trần
    glPushMatrix();
    glColor3f(0.157, 0.321, 0.388);
    glTranslatef(0.0f, 7.4f, -35.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.2, 40, 20);
    glutSolidCube(0.5);
    glPopMatrix();

    //Thùng cát-tông
    glPushMatrix();
    glColor3f(0.863, 0.690, 0.533);
    glTranslatef(11.5f, -0.20f, -25.20f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
    glScalef(3, 3, 3);
    glutSolidCube(0.5);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.863, 0.690, 0.533);
    glTranslatef(11.2f, -0.45f, -23.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
    glScalef(2, 2, 2);
    glutSolidCube(0.5);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.863, 0.690, 0.533);
    glTranslatef(12.5f, -0.3f, -21.8f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glRotatef(70.0f, 1.0f, 0.0f, 0.0f);
    glScalef(2.6, 2.6, 2.6);
    glutSolidCube(0.5);
    glPopMatrix();

    //điều_hòa
    glPushMatrix();
    glColor3f(0.376, 0.368, 0.372);
    glTranslatef(9.8f, 6.5f, -32.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(2, 2, 4);
    glutSolidCube(0.5);
    glPopMatrix();

    // nhà
    glPushMatrix();
    glColor3f(0.6901, 0.3568, 0.2313);
    glTranslatef(0.0f, 3.8f, -35.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(18, 38, 20);
    glutSolidCube(0.5);
    glPopMatrix();

    //cửa
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTranslatef(0.0f, 4.0f, -22.9f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(5, 10, 7);
    square4();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();


    //Đường ống nước
    glPushMatrix();
    glColor3f(0.658, 0.588, 0.643);
    glTranslatef(9.8f, -1.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.12, 8.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 1.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 3.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(0.353, 0.353, 0.439);
    glTranslatef(9.8f, 5.0f, -38.f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(0.15, 0.20f);
    glPopMatrix();
    //bậc
    glPushMatrix();
    glColor3f(0.157, 0.321, 0.388);
    glTranslatef(0.0f, -0.6f, -33.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.5, 40, 30);
    glutSolidCube(0.5);
    glPopMatrix();
    //sàn
    glPushMatrix();
    glColor3f(0.376, 0.368, 0.372);
    glTranslatef(0.0f, -1.0f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(0.1, 60, 70);
    glutSolidCube(0.5);
    glPopMatrix();

    // Hàng rào
    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(14.8f, 0.5f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(14.8f, -0.2f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(-14.8f, 0.5f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(-14.8f, -0.2f, -28.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    glScalef(1.0f, 1.0f, 175.0f);
    glutSolidCube(0.2f);
    glPopMatrix();


    // vẽ cột đứng
    // bên trái
    for (GLfloat i = -17.0; i < 18; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(-14.8f, -1.0f, -28.0f + i);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }

    // bên phải
    for (GLfloat i = -17.0; i < 18; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(14.8f, -1.0f, -28.0f + i);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }

    // đằng sau
    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(0.0f, 0.5f, -45.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.0f, 1.0f, 150.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(0.0f, -0.2f, -45.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.0f, 1.0f, 150.0f);
    glutSolidCube(0.2f);
    glPopMatrix();

    for (GLfloat i = 1; i < 30; i++)
    {
        glPushMatrix();
        glColor3f(1.0f, 1.0, 1.0f);
        glTranslatef(-14.8f + i, -1.0f, -45.0f);
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glScalef(1.0f, 20.0f, 1.0f);
        glutSolidCube(0.2f);
        glPopMatrix();
    }

}

void drawTrafficSigns(void)
{
    glPushMatrix();
    glTranslatef(0.0f, -1.0f, 11.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(ROD_RADIUS, 5.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, -1.0f, 12.0f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
    XCylinder(ROD_RADIUS, 5.0f);
    glPopMatrix();


    //hình hộp
    glPushMatrix();
    glTranslatef(1.0f, 4.0f, 11.5f);
    glRotatef(90.0f, 0.0f, 0.0f, 1.0f);

    glBegin(GL_POLYGON);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(-1.0f, 1.2f, -1.0f);
    glVertex3f(1.0f, 1.2f, -1.0f);
    glVertex3f(1.0f, 1.2f, 1.0f);
    glVertex3f(-1.0f, 1.2f, 1.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(-1.0f, 1.2f, -1.0f);
    glVertex3f(1.0f, 1.2f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(1.0f, 1.2f, 1.0f);
    glVertex3f(1.0f, 1.2f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(1.0f, 1.2f, 1.0f);
    glVertex3f(-1.0f, 1.2f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex3f(-1.0f, 1.2f, 1.0f);
    glVertex3f(-1.0f, 1.2f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glEnd();

    glPopMatrix();
}

void treeR(void)
{
    // Thân cột
    glPushMatrix();
    glColor3f(1.0f, 1.0, 1.0f);
    glTranslatef(0.0f, -1.0f, 12.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.0f, 1.0f, 6.0f);
    glutSolidCube(0.4f);
    glPopMatrix();

    // Đỉnh cột
    glPushMatrix();
    glColor3f(1.0f, 0.0, 0.0f);
    glTranslatef(0.0f, 0.4f, 12.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCube(0.4f);
    glPopMatrix();


}



void MileStone(void)
{
    glPushMatrix();
    treeR();
    glPopMatrix();
    GLfloat i;
    for (i = 5.0; i < 100.0; i += 5.0)
    {
        glPushMatrix();
        glTranslatef(i, 0.0f, 0.0f);
        treeR();
        glPopMatrix();

        glPushMatrix();
        glTranslatef(-i, 0.0f, 0.0f);
        treeR();
        glPopMatrix();
    }

}


void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //glEnable(GL_NORMALIZE);

    glPushMatrix();
    /*******************************
     *    Chuẩn bị các góc quay
     *******************************/
    glRotatef(angley, 1.0f, 0.0f, 0.0f);
    glRotatef(anglex, 0.0f, 1.0f, 0.0f);
    glRotatef(anglez, 0.0f, 0.0f, 1.0f);

    /***********************
     *    Bắt đầu render để hiển thị xe đạp
     **********************/

     //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

     /*glTranslatef(MIDDLE_ROD, 0.0f, 0.0f);
     glRotatef(-MIDDLE_ANGLE, 0.0f, 0.0f, 1.0f);
     glScalef(0.3f, ROD_RADIUS, 0.25f);*/
    landmarks();

    circleK();
    
    glPushMatrix();
    glTranslatef(50.0f, 0.0f, 0.0f);
    vinmart();
    glPopMatrix();


    drawTrafficSigns();


    MileStone();


    /****************************
    *   Di chuyển xe đạp
    ****************************/
    glPushMatrix();
    glTranslatef(xpos, 0.0f, zpos);
    glRotatef(direction, 0.0f, 1.0f, 0.0f);

    drawFrame();
    drawChain();
    drawPedals();
    glPopMatrix();


    // Đổ bóng
    glPushMatrix();

    glTranslatef(xpos, 0.0f, zpos);
    glRotatef(direction, 0.0f, 1.0f, 0.0f);

    glDisable(GL_LIGHTING);
    glTranslatef(0.0f, -1.0f, 0.0f);
    glRotatef(-90, 1, 0, 0);
    glScalef(1, 1, 0);
    glColor3f(0.0, 0.0, 0.0);
    drawFrame();
    drawChain();
    drawPedals();

    glEnable(GL_LIGHTING);
    glPopMatrix();

    glPopMatrix();


    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camx, camy, camz, camx, 0.0, 0.0, 0.0, 1.0, 0.0);

    glutSwapBuffers();    

}

/************************
*   Tính trị tuyệt đối
************************/
GLfloat Abs(GLfloat a)
{
    if (a < 0.0f)
        return -a;
    else
        return a;
}

/************************
*   Chuyển góc về độ
************************/
GLfloat degrees(GLfloat a)
{
    return a * 180.0f / PI;
}

/************************
*   Chuyển góc về radian
************************/
GLfloat radians(GLfloat a)
{
    return a * PI / 180.0f;
}

/*************************
*   Tạo ra vòng lặp liên tục
***************************/
void idle(void)
{
    updateScene();
    glutPostRedisplay();
}

void special(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_UP:
        camz -= 0.1f;
        break;
    case GLUT_KEY_DOWN:
        camz += 0.1f;
        break;
    case GLUT_KEY_LEFT:
        camx -= 0.1f;
        break;
    case GLUT_KEY_RIGHT:
        camx += 0.1f;
        break;
    }
    glutPostRedisplay();
}

/*****************************
*   Reset màn hình
*****************************/
void reset()
{
    anglex = angley = anglez = 0.0f;
    pedalAngle = steering = 0.0f;
    Mouse = GLUT_UP;
    pedalAngle = speed = steering = 0.0f;
    camx = camy = 0.0f;
    camz = 5.0f;
    xpos = zpos = 0.0f;
    direction = 0.0f;
}

/********************************
*   Handle sự kiện nhập từ bàn phím
*********************************/
void keyboard(unsigned char key, int x, int y)
{
    GLfloat r = 0.0f;

    switch (key)
    {
    case 'r':
    case 'R':
        reset();
        break;
    case 'a':
        if (steering < HANDLE_LIMIT)
            steering += INC_STEERING;
        break;
    case 'd':
        if (steering > -HANDLE_LIMIT)
            steering -= INC_STEERING;
        break;
    case '+':
        speed += INC_SPEED;
        break;
    case '-':
        speed -= INC_SPEED;
        break;
    case 27:
        exit(1);
    }

    pedalAngle += speed;
    if (speed < 0.0f)
        speed = 0.0f;
    if (pedalAngle < 0.0f)
        pedalAngle = 0.0f;
    if (pedalAngle >= 360.0f)
        pedalAngle -= 360.0f;

    glutPostRedisplay();
}

/********************************
*   Handle sự kiện nhập từ chuột
*********************************/
void mouse(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            Mouse = GLUT_DOWN;
            prevx = x;
            prevy = y;
        }
        if (state == GLUT_UP)
        {
            Mouse = GLUT_UP;
        }
        break;
    case GLUT_RIGHT_BUTTON:

        break;
    }
    glutPostRedisplay();
}

void motion(int x, int y)
{
    if (Mouse == GLUT_DOWN)
    {
        int deltax, deltay;
        deltax = prevx - x;
        deltay = prevy - y;
        anglex += 0.1 * deltax;
        angley += 0.1 * deltay;


       /* if (deltax != 0 && deltay != 0)
            anglez += 0.1 * sqrt(deltax * deltax + deltay * deltay);*/

        if (anglex < 0)
            anglex += 360.0;
        if (angley < 0)
            angley += 360.0;
        if (anglez < 0)
            anglez += 360.0;

        if (anglex > 360.0)
            anglex -= 360.0;
        if (angley > 360.0)
            angley -= 360.0;
        if (anglez > 360.0)
            anglez -= 360.0;
    }
    else
    {
        Mouse = GLUT_UP;
    }
    prevx = x;
    prevy = y;
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)w / (GLfloat)h, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(camx, camy, camz, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    
}

void glSetupFuncs(void)
{
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    glutSpecialFunc(special);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutSetCursor(GLUT_CURSOR_CROSSHAIR);
}

void help(void)
{
    printf("Mo hinh xe dap 3D\n");
    printf("Mon do hoa may tinh\n");
    printf("Bai tap nhom\n\n");
    printf("Nhan '+' de tang toc do\n");
    printf("Nhan '-' de giam toc do\n");
    printf("Nhan 'a' de re sang ben trai\n");
    printf("Nhan 'd' de re sang ben phai\n");
    printf("Nhan 'r' or 'R' de quay ve vi tri ban dau\n");
    printf("Dung phim dieu huong de di chuyen camera\n");
    printf("Dung ban phim de di chuyen man hinh\n");
}

int main(int argc, char* argv[])
{

    help();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Mo hinh xe dap 3D");
    init();
    texture[0] = LoadTexture("rawnri.bmp", 648, 1151);
    texture[2] = LoadTexture("sand.bmp", 1340, 1000);
    texture[3] = LoadTexture("road.bmp", 256, 128);
    texture[4] = LoadTexture("front-of-vinmart.bmp", 748, 412);
    texture[5] = LoadTexture("vinmart.bmp", 1200, 675);
    texture[6] = LoadTexture("brick-wall.bmp", 640, 640);
    texture[7] = LoadTexture("circle_k.bmp", 960, 541);

    glSetupFuncs();
    glutMainLoop();
    return 0;
}
