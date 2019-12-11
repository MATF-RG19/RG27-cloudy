#include<iostream>
#include<cmath>
#include<vector>
#include<math.h>
#include<GL/glut.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<math.h>

/*Definisanje makroa za PI, tajmere i maksimalan broj karaktera*/
#define PI 3.141592653589
#define DEG2RAD(deg) (deg * PI / 180)
#define TIMER_ID 0
#define TIMER_ID1 1
#define TIMER_INTERVAL 20
#define MAKSIMALAN_BROJ_KARAKTERA 256


/*Deklaracija callback funkcija*/
static void on_keyboard(unsigned char key, int x, int y);
static void on_reshape(int width, int height);
static void on_display(void);
static void on_mouse(int button, int state, int x, int y);
static void on_motion(int x, int y);

/*Funkcije za pomeranje objekata, oblaka i metaka*/
static void pomeranjeOblaka(int value);
static void pomeranjeMetaka(int value);

/*Funkcija za inicijalizaciju*/
void pocetneVrednosti();

/*Funkcije za crtanje objekata na sceni*/
void nacrtajPostolje(void);
void nacrtajElipsu(double centerX, double centerY, double radiusX, double radiusY);
void nacrtajOblak();
void nacrtajTop();
void nacrtajMuniciju();

/*Funkcija za dodavanje teksta*/
void dodajTekst();

/*Promenljive za visinu i sirinu ekrana*/
static int window_width, window_height;

/*Niz u kojem se cuvaju koordinate po x i y oblaka*/
static std::vector<double> xKoordinateOblaka(1000), yKoordinateOblaka(1000);
/*Niz u kojem se cuvaju brzine po x i y oblaka*/
static std::vector<double> brzinaOblakaPoX(1000), brzinaOblakaPoY(1000);
/*Niz u kojem se cuva boja oblaka*/
static std::vector<int> bojaOblaka(1000);
/*Niz u kojem se cuvaju koordinate po x i y municije*/
static std::vector<double> yKoordinataMunicije(1000), zKoordinataMunicije(1000);
/*Brojac koji govori koliko je metkova pogodjeno i drugi brojac za pogodjene oblake*/
static int brojacMetkova;
static int brojPogodjenih;

static double pocetnaXMunicije, pocetnaYMunicije, pocetnaZMunicije;
static int kretanjeOblaka;
static int mouse_x, mouse_y; /* Koordinate misa. */
static float matrix[16];/*kumulativna matrica rotacije*/
/*Koordinata dela topa i rotacija topovske cevi*/
static double pomerajPoX, rotacijaUgla;
static int pritisnutoPucanje, drugiPut;
static bool prikazi_municiju;
static int izgubio;

int main(int argc, char** argv){
    
    /* Inicijalizuje se GLUT. */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);

    /* Kreira se prozor. */
    glutInitWindowSize(700, 600);
    glutInitWindowPosition(0, 0);
    glutCreateWindow(argv[0]);
    
    pocetneVrednosti();
    
    glutKeyboardFunc(on_keyboard);
    glutMouseFunc(on_mouse);
    glutMotionFunc(on_motion);
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(on_display);

    /* Obavlja se OpenGL inicijalizacija. */
    glClearColor(0.75, 0.85, 0.85, 0);
    glEnable(GL_DEPTH_TEST);
    
    /* Program ulazi u glavnu petlju. */
    glutMainLoop();

    
}
/*Postavljanje pocetnih vrednosti*/
void pocetneVrednosti(){
    pomerajPoX = 0.0;
    rotacijaUgla = 45;
    kretanjeOblaka = 0;
    pritisnutoPucanje = 0;
    drugiPut = 0;
    brojacMetkova = 0;
    brojPogodjenih = 0;
    double pocetnaBrzinaPoX = 0.03;
    double pocetnaBrzinaPoY = 0.003;
    prikazi_municiju = false;
    izgubio = 0;
    pocetnaYMunicije = -0.5;
    pocetnaZMunicije = 1.1;
    /*Postavaljanje pocetnih koordinata i boja oblaka na slucajan nacin*/
    srand(time(NULL));
    for(int i = 0; i<100; i++){
        
        yKoordinateOblaka.at(i) = 0.7 + i;
        double xKoordinata = rand()/(float)RAND_MAX;
    
        double randomBroj = rand()/(float)RAND_MAX;
        if(randomBroj < 0.5){
            xKoordinateOblaka.at(i) = -1*xKoordinata;
        } else {
            xKoordinateOblaka.at(i) = xKoordinata;
        }
        double smerKretanja = rand()/(float)RAND_MAX;
        if(smerKretanja < 0.5){
            brzinaOblakaPoX.at(i) = pocetnaBrzinaPoX;
        } else {
            brzinaOblakaPoX.at(i) = -pocetnaBrzinaPoX;
        }
        brzinaOblakaPoY.at(i) = pocetnaBrzinaPoY;
        if(i!=0 && i%3 == 0){
            pocetnaBrzinaPoX += 0.01;
            pocetnaBrzinaPoY += 0.001;
        }
        double indikatorZaBoju = rand()/(float)RAND_MAX;
        if(indikatorZaBoju < 0.5)
            bojaOblaka.at(i) = 1;
        else
            bojaOblaka.at(i) = 0;
        
        /*Postavaljanje pocetnih koordinata za municiju*/
        yKoordinataMunicije.at(i) = pocetnaYMunicije;
        zKoordinataMunicije.at(i) = pocetnaZMunicije;
    
    }
    
    /*inicijalizacija matrice rotacije*/
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
}

static void on_mouse(int button, int state, int x, int y){
    /*cuvaju se pozicije misa */
    mouse_x = x;
    mouse_y = y; 
}

static void on_motion(int x, int y){
    /*promene pozicije misa, izracunavanje te promene
     i cuvanje novih pozicija*/
    int deltaX, deltaY;
    
    deltaX = x - mouse_x;
    deltaY = y - mouse_y;
    mouse_x = x;
    mouse_y = y;
    
    /*izracunavanje nove matrice rotacije */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
        glLoadIdentity();
        /*Podesavanje parametara topa tako da prate pozicije misa*/
        if(pomerajPoX <= 1.8 && pomerajPoX >=-1.8)
            pomerajPoX += deltaX * 0.005;
        else if(pomerajPoX > 1.8)
            pomerajPoX = 1.8;
        else
            pomerajPoX = -1.8;
        if(rotacijaUgla <= 90 && rotacijaUgla >= 30)
            rotacijaUgla += deltaY;
        else if(rotacijaUgla > 90)
            rotacijaUgla = 90;
        else 
            rotacijaUgla = 30;
        
        glRotatef(pomerajPoX, 1, 0, 0);
        glMultMatrixf(matrix);        
    glPopMatrix();
    glutPostRedisplay();
}

static void on_reshape(int width, int height){
    window_width = width;
    window_height = height;
}

static void on_keyboard(unsigned char key, int x, int y){
    
    switch (key) {
    case 27:
        /*Izlazak iz programa*/
        exit(0);
        break;        
    case 's':
    case 'S':
        /*Pokretanje*/
        if (!kretanjeOblaka) {
            glutTimerFunc(TIMER_INTERVAL, pomeranjeOblaka, TIMER_ID);
            kretanjeOblaka = 1;
        }
        break;
    case 'p':
    case 'P':
        /*Pucanje*/
        if(kretanjeOblaka){
            if(brojacMetkova<1){
                pocetnaXMunicije = pomerajPoX;
            
                brojacMetkova += 1;
        
                pritisnutoPucanje = 1;
                prikazi_municiju=true;
                glutTimerFunc(TIMER_INTERVAL, pomeranjeMetaka, TIMER_ID1);
            }
        }
        break;
    case 'r':
    case 'R':
        /*Reset*/
        pocetneVrednosti();
        glutPostRedisplay();
        break;
    }
}


static void on_display(void){
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /* Podesava se vidna tacka. */
    glViewport(0, 0, window_width, window_height);

    /* Podesava se projekcija. */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30, (float) window_width / window_height, 1, 100);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 7, 
              0, 0, 0, 
              0, 1, 0);
    
    glEnable(GL_DEPTH_TEST);
    /*Dodavanje objekata na scenu*/
    nacrtajOblak();
    nacrtajPostolje();
    
    glMultMatrixf(matrix);
    nacrtajTop();
    nacrtajMuniciju();
    
    /*Dodavanje teksta*/
    dodajTekst();
    glutSwapBuffers();
}

/*Funkcija pomocu koje se oblak krece levo desno
 * kada udari u ivicu ekrana, menja smer kretanja.
 * Ukoliko se desi da crni oblak dodje do postolja, izgubili ste */
static void pomeranjeOblaka(int value){

    if (value != TIMER_ID)
        return;
    unsigned int i = 0;
    if(!izgubio){
        for(i = 0; i<100; i++){
            xKoordinateOblaka.at(i) += brzinaOblakaPoX.at(i);
            yKoordinateOblaka.at(i) -= brzinaOblakaPoY.at(i);
            if(xKoordinateOblaka.at(i) + 0.2 >= 2 || xKoordinateOblaka.at(i) - 0.2 <= -2)
                brzinaOblakaPoX.at(i) *= -1;
            if(yKoordinateOblaka.at(i) < - 1.1 && !bojaOblaka.at(i)){
                izgubio = 1;            
            }
        }
    }
    else{
        for(i=0; i<100; i++){
            if(yKoordinateOblaka.at(i) >= -4){
                yKoordinateOblaka.at(i) -= 0.3;
            }
        }
        if(yKoordinateOblaka.at(99) < -4){
            pocetneVrednosti();
        }
    }
    glutPostRedisplay();
    
    if (kretanjeOblaka) {
        glutTimerFunc(TIMER_INTERVAL, pomeranjeOblaka, TIMER_ID);
    }
}

/*Postavljane vrednosti za kretanje metaka, detekcija sudara metka sa oblakom
  Metkovi su u obliku loptice i pojavljuju se kad pritisnemo dugme P*/
static void pomeranjeMetaka(int value){

    if (value != TIMER_ID1)
        return;
   
    
    for(int i = 0; i<brojacMetkova; i++){
        
        if(yKoordinataMunicije.at(i)<2 ){
            yKoordinataMunicije.at(i) += 0.06/brojacMetkova ;
            zKoordinataMunicije.at(i) -= 0.05/brojacMetkova ;
            for(int j = 0; j<100; j++){
                if(yKoordinataMunicije.at(i) <= yKoordinateOblaka.at(j) + 0.15 &&
                   yKoordinataMunicije.at(i) >= yKoordinateOblaka.at(j) - 0.15 &&
                   pocetnaXMunicije <= xKoordinateOblaka.at(j) + 0.25 &&
                   pocetnaXMunicije >= xKoordinateOblaka.at(j) - 0.25 &&
                   zKoordinataMunicije.at(i) <= 0.5){
                    if(!bojaOblaka.at(j)){
                        brojPogodjenih += 1;
                        
                    }
                    else {
                        brojPogodjenih -= 1;
                    }
                     
                    yKoordinateOblaka.at(j) = 1000;
                }
            }
        }
        
        else{
            glutTimerFunc(TIMER_INTERVAL, pomeranjeMetaka, TIMER_ID);

            yKoordinataMunicije.at(i) = pocetnaYMunicije;
            zKoordinataMunicije.at(i) = pocetnaZMunicije;
            prikazi_municiju=false;
            nacrtajMuniciju();
            brojacMetkova--;

        }

    } 
    glutPostRedisplay();

    if (brojacMetkova>0)  {
        glutTimerFunc(TIMER_INTERVAL, pomeranjeMetaka, TIMER_ID1);
    }
}

/*Postolje na kojem se nalazi top pomocu kojeg gadjamo oblake*/
void nacrtajPostolje(){
    glDisable(GL_LIGHTING);
    GLfloat pozicija_osvetljenja[] = { 1, 10, 8, 1 };

    GLfloat ambijentalno_osvetljenje[] = { 0.35, 0.35, 0.35, 1 };
    GLfloat difuzno_osvetljenje[] = { 0.75, 0.75, 0.75, 1 };
    GLfloat spekularno_osvetljenje[] = { 0.9, 0.9, 0.9, 1 };
    
    
    GLfloat ambijentalni_materijal[] = { 0.0, 1.0, 0.0, 1 };
    GLfloat difuzni_materijal[] = { 0.0, 0.6, 0.0, 1 };
    GLfloat spekularni_materijal[] = { 1, 1, 1, 1 };
    GLfloat shininess = 40;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pozicija_osvetljenja);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambijentalno_osvetljenje);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, difuzno_osvetljenje);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spekularno_osvetljenje);

    /* Podesavaju se parametri materijala. */
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambijentalni_materijal);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difuzni_materijal);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spekularni_materijal);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);

    glPushMatrix();
            glTranslatef(0, -1.4, 1);
            glScalef(3.5, 0.5, 1);
            glutSolidCube(1);
    glPopMatrix();
    
}

/*Crtanje elipsi koje cine oblake*/
void nacrtajElipsu(double centerX, double centerY, double radiusX, double radiusY){
    const float DEG2RAD = 3.14159 / 180;
    glBegin(GL_TRIANGLE_FAN);
        glVertex2d (centerX, centerY);
        for (int i = -1; i < 360; i++) {
            float degInRad = i * DEG2RAD;
            glVertex2d (centerX + cos (degInRad) * radiusX*0.3, 
                            centerY + sin (degInRad) * radiusY*0.3);
        }
        
    glEnd();
}

/*Crtanje oblaka koji se sastoji od 5 elipsi*/
void nacrtajOblak(){
    
    glDisable(GL_LIGHTING);
    
    for(int i = 0; i<100; i++){
        double x = xKoordinateOblaka.at(i);
        double y = yKoordinateOblaka.at(i);
        glColor3f(bojaOblaka[i], bojaOblaka[i], bojaOblaka[i]);
        nacrtajElipsu(x, y, 1.2, 0.7);
        glColor3f(bojaOblaka[i], bojaOblaka[i], bojaOblaka[i]);
        nacrtajElipsu(x+0.23, y+0.12, 0.35, 0.35);
        glColor3f(bojaOblaka[i], bojaOblaka[i], bojaOblaka[i]);
        nacrtajElipsu(x+0.23, y-0.12, 0.35, 0.35);
        glColor3f(bojaOblaka[i], bojaOblaka[i], bojaOblaka[i]);
        nacrtajElipsu(x-0.23, y-0.12, 0.35, 0.35);
        glColor3f(bojaOblaka[i], bojaOblaka[i], bojaOblaka[i]);
        nacrtajElipsu(x-0.23, y+0.12, 0.35, 0.35);
        
    }
}

/*Crtanje topa, koji se sastoji od 3 cilindra.
 * Jednog za cev odakle se puca i dva za tockove*/
void nacrtajTop(){
        
    GLfloat pozicija_osvetljenja[] = { 1, 10, 8, 1 };

    GLfloat ambijentalno_osvetljenje[] = { 0.35, 0.35, 0.35, 1 };
    GLfloat difuzno_osvetljenje[] = { 0.75, 0.75, 0.75, 1 };
    GLfloat spekularno_osvetljenje[] = { 0.9, 0.9, 0.9, 1 };
    
    
    GLfloat ambijentalni_materijal[] = { 0.5, 0.5, 0.5, 1 };
    GLfloat difuzni_materijal[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat spekularni_materijal[] = { 1, 1, 1, 1 };
    GLfloat shininess = 40;

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, pozicija_osvetljenja);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambijentalno_osvetljenje);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, difuzno_osvetljenje);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spekularno_osvetljenje);

    /* Podesavaju se parametri materijala. */
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambijentalni_materijal);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, difuzni_materijal);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spekularni_materijal);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    glPushMatrix();
        GLUquadricObj *quadratic;
        quadratic = gluNewQuadric();
        glTranslatef(pomerajPoX, -0.5, 1.15);
        
        glRotatef(rotacijaUgla, 1, 0, 0);
        
        
        gluCylinder(quadratic, 0.1f, 0.1f, 0.7f, 72, 72);
    glPopMatrix();        
    
    glPushMatrix();
        GLUquadricObj *quadratic1;
        quadratic1 = gluNewQuadric();
        glRotatef(90, 0, 1, 0);
        glTranslatef(-1.4, -0.95, pomerajPoX+0.1);
        
        gluCylinder(quadratic1, 0.2f, 0.2f, 0.1f, 72, 72);

        glTranslatef(0, 0, -0.3);
        gluCylinder(quadratic1, 0.2f, 0.2f, 0.1f, 72, 72);
    glPopMatrix();
}

/*Crtanje municije koja ce biti vidljiva tek kada se ispali metak*/
void nacrtajMuniciju(){
    
    if(prikazi_municiju==true){
        glDisable(GL_LIGHTING);
        GLfloat pozicija_osvetljenja[] = { 1, 10, 8, 1 };

        GLfloat ambijentalno_osvetljenje[] = { 0.35, 0.35, 0.35, 1 };
        GLfloat difuzno_osvetljenje[] = { 0.75, 0.75, 0.75, 1 };
        GLfloat spekularno_osvetljenje[] = { 0.9, 0.9, 0.9, 1 };
        
        
        GLfloat ambijentalni_materijal[] = { 0, 0, 0, 1 };
        GLfloat difuzni_materijal[] = { 0.1, 0.1, 0.1, 1 };
        GLfloat spekularni_materijal[] = { 1, 1, 1, 1 };
        GLfloat shininess = 40;

        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glLightfv(GL_LIGHT0, GL_POSITION, pozicija_osvetljenja);
        glLightfv(GL_LIGHT0, GL_AMBIENT, ambijentalno_osvetljenje);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, difuzno_osvetljenje);
        glLightfv(GL_LIGHT0, GL_SPECULAR, spekularno_osvetljenje);

        /* Podesavaju se parametri materijala. */
        glMaterialfv(GL_FRONT, GL_AMBIENT, ambijentalni_materijal);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, difuzni_materijal);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spekularni_materijal);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
        
        
        for(int i = 0; i<brojacMetkova; i++){
            glPushMatrix();
            glTranslatef(pocetnaXMunicije, yKoordinataMunicije.at(i),                      
                        zKoordinataMunicije.at(i));
            glutSolidSphere(0.12, 500, 500);
            glPopMatrix();
        }
    }
}

/*Funkcija sa dodavanje teksta na scenu*/
void dodajTekst(void){
    glDisable(GL_LIGHTING);
    

    char tekstZaBrojPogodjenih[MAKSIMALAN_BROJ_KARAKTERA], *p1;
    sprintf(tekstZaBrojPogodjenih, "Broj pogodjenih: ");
    
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(-2.2, 1, 0);
        glRasterPos3f(0.2, 0.7, 0);
        for(p1 = tekstZaBrojPogodjenih; *p1!= '\0'; p1++){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p1);
        }
    glPopMatrix();
    
    char brPogodaka[MAKSIMALAN_BROJ_KARAKTERA], *p2;
    sprintf(brPogodaka, "%d", brojPogodjenih);
    
    glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(-1.2, 1, 0);
        glRasterPos3f(0.2, 0.7, 0);
        for(p2 = brPogodaka; *p2!= '\0'; p2++){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p2);
        }
    glPopMatrix();
        
    char tekstZaKraj[MAKSIMALAN_BROJ_KARAKTERA], *p3;
    sprintf(tekstZaKraj, "IZGUBILI STE!");
    
    if(izgubio){
        glPushMatrix();
        glColor3f(1, 1, 1);
        glTranslatef(-0.7, 0, 0);
        glRasterPos3f(0.2, 0.7, 0);
        for(p3 = tekstZaKraj; *p3!= '\0'; p3++){
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p3);
        }
        glPopMatrix();
    }
    
}



