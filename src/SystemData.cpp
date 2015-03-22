/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file SystemData.cpp
*/

#include "SystemData.h"
#include "OpenGL2d.h"


#include <QCoreApplication>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>

#define DEF_MAX(x,y)  ((x)>(y)?(x):(y))
#define DEF_MIN(x,y)  ((x)<(y)?(x):(y))

#define  SAFETY 0.9
#define  PGROW  -0.2
#define  PSHRNK -0.25
#define  ERRCON 1.89e-4
#define  TINY   1.0e-30

double
b21 = 0.2, b31 = 3.0/40.0, b32 = 9.0/40.0, b41 = 0.3, b42 = -0.9, b43 = 1.2,
b51 = -11.0/54.0, b52 = 2.5, b53 = -70.0/27.0, b54 = 35.0/27.0,
b61 = 1631.0/55296.0, b62 = 175.0/512.0, b63 = 575.0/13824.0,
b64 = 44275.0/110592.0, b65 = 253.0/4096.0,
c1 = 37.0/378.0, c3 = 250.0/621.0, c4=125.0/594.0, c6 =512.0/1771.0,
dc5 = -277.0/14336.0;

double dc1 = c1-2825.0/27648.0, dc3 = c3-18575.0/48384.0, dc4 = c4-13525.0/55296.0,
dc6 = c6-0.25;



SystemData::SystemData() :
    mOpenGL2d(NULL),
    m_trajectory(NULL),
    m_timer(NULL)
{
    ResetParams();

    magnetProps mp1 = { glm::vec3(-0.03,-0.03,0.0), 1.0, glm::vec4(1.0,0.0,0.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 0) };
    magnetProps mp2 = { glm::vec3( 0.03,-0.03,0.0), 1.0, glm::vec4(0.0,1.0,0.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 1) };
    magnetProps mp3 = { glm::vec3( 0.0, 0.03*sqrt(2.0),0.0), 1.0, glm::vec4(0.0,0.0,1.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 2) };
    //magnetProps mp4 = { glm::vec3( 0.0, -0.03*sqrt(2.0),0.0), 1.0, glm::vec4(0.0,1.0,1.0,1.0) };
    m_magnets.push_back(mp1);
    m_magnets.push_back(mp2);
    m_magnets.push_back(mp3);
    // m_magnets.push_back(mp4);
    m_magnetSize = 0.005f;

    bobColor = QColor(255,240,160);

    ambientColor = Qt::white;
    diffuseColor = Qt::white;
    specularColor = Qt::white;
    k_ambient = 0.2f;
    k_diffuse = 0.8f;
    k_specular = 0.0f;
    k_exp = 120.0f;
}


SystemData::~SystemData() {
    m_timer->stop();
    delete m_timer;
    mOpenGL2d = NULL;
    delete [] m_trajectory;
    m_trajTime.clear();
}


/**
*  @param engine  main script engine.
*/
void SystemData :: AddObjectsToScriptEngine( QScriptEngine* engine ) {
    QScriptValue sys = engine->newQObject(this);
    engine->globalObject().setProperty("System",sys);
}


void SystemData::setOpenGLPtr( OpenGL2d* ogl ) {
    mOpenGL2d = ogl;
}


/**
 *   If set to zero, the timeout is shot as fast as possible.
 *   But that could yield to some bucking.
 * @param val  Timer interval in milliseconds.
 */
void SystemData::SetTimerInterval( int val ) {
    m_timer->setInterval(val);
}

void SystemData::ResetParams() {
    m_pendulumHeight = 2.02;
    m_pendulumLength = 2.0;
    m_gravity = 9.81;
    m_damping = 1.0;
    m_kappa = 1.0;
    m_magFactor = 0.01;
    m_maxTheta = 5.0;

    m_rmax = 1.0;
    m_rmaxX = 1.0;
    m_rmaxY = 1.0;

    m_numPoints = 0;
    m_maxNumPoints = 1500;
    m_lineWidth = 2;
#ifdef HAVE_COMP_SHADER      
    m_lineColor = Qt::black;
#else
    m_lineColor = Qt::white;
#endif    

    m_tScale = 1.0;

    m_trajectory = new float[m_maxNumPoints*4];
    m_currAnimPos = glm::vec2(0,0);
    m_currIndex = 0;
    m_currAnimTime = 0.0f;
}

void SystemData::ResetAnim() {
    if (m_trajectory!=NULL) {
        m_currAnimPos = glm::vec2(m_trajectory[0],m_trajectory[1]);
    } else {
        m_currAnimPos = glm::vec2(0);
    }
    m_currAnimTime = 0.0f;
    m_currIndex = 0;
}

void SystemData::calcRHS( double *y, double *rhs ) {
    double l  = m_pendulumLength;
    double z0 = m_pendulumHeight;
    double g  = m_gravity;
    double gamma = m_damping;
    double mf = m_magFactor;
    double kappa = m_kappa;

#ifdef USE_SPHERICAL
    double theta = y[0];
    double phi   = y[1];
    double Dth   = y[2];
    double Dph   = y[3];

    double sth = sin(theta);
    double cth = cos(theta);
    double sph = sin(phi);
    double cph = cos(phi);

    rhs[0] = y[2];
    rhs[1] = y[3];
    rhs[2] = Dph*Dph*sth*cth - g/l*sth - gamma/l*Dth;
    rhs[3] = -2.0*Dth*Dph*cth/sth - gamma/l*Dph;
    //return;

    double alpha,rx,ry,rz,numer;
    double M1 = 0.0;
    double M2 = 0.0;
    for(unsigned int i=0; i<m_magnets.size(); i++) {
        alpha = m_magnets[i].alpha*mf;
        rx = l*sth*cph - m_magnets[i].pos.x;
        ry = l*sth*sph - m_magnets[i].pos.y;
        rz = z0 - l*cth - m_magnets[i].pos.z;
        numer = pow(sqrt(rx*rx + ry*ry + rz*rz),-2.0-kappa);

        M1 += kappa*alpha/l*(rx*cth*cph + ry*cth*sph + rz*sth)*numer;
        M2 += kappa*alpha/(l*sth)*(-rx*sph + ry*cph)*numer;
    }
    rhs[2] -= M1;
    rhs[3] -= M2;

#else

    double xx = y[0];
    double yy = y[1];
    double dx = y[2];
    double dy = y[3];

    rhs[0] = y[2];
    rhs[1] = y[3];
    rhs[2] = -gamma*dx - g/l*xx;
    rhs[3] = -gamma*dy - g/l*yy;

    double alpha,numer,rx,ry,rz;
    double M1 = 0.0;
    double M2 = 0.0;
    for(int i=0; i<m_magnets.size(); i++) {
        alpha = m_magnets[i].alpha*mf;
        rx = xx - m_magnets[i].pos.x;
        ry = yy - m_magnets[i].pos.y;
        rz = z0-l - m_magnets[i].pos.z;
        numer = pow(sqrt(rx*rx + ry*ry + rz*rz),-2.0-kappa);

        M1 += kappa*alpha*rx*numer;
        M2 += kappa*alpha*ry*numer;
    }
    rhs[2] -= M1;
    rhs[3] -= M2;
#endif
}

/**
 *  Runge-Kutta Cash-Karp step
 */
void SystemData::rkck( double *y, double *dydx, int n, double h,
                       double *yout, double *yerr )
{
    register int i;
    double *ak2 = new double[n];
    double *ak3 = new double[n];
    double *ak4 = new double[n];
    double *ak5 = new double[n];
    double *ak6 = new double[n];
    double *ytemp = new double[n];

    for(i=0; i<n; i++) {
        ytemp[i] = y[i] + h * b21 * dydx[i];
    }

    calcRHS( ytemp, ak2);
    for(i=0; i<n; i++) {
        ytemp[i] = y[i] + h * (b31*dydx[i] + b32*ak2[i]);
    }

    calcRHS( ytemp, ak3);
    for(i=0; i<n; i++) {
        ytemp[i] = y[i] + h * (b41*dydx[i] + b42*ak2[i] + b43*ak3[i]);
    }

    calcRHS( ytemp, ak4);
    for(i=0; i<n; i++) {
        ytemp[i] = y[i] + h * (b51*dydx[i] + b52*ak2[i] + b53*ak3[i] + b54*ak4[i]);
    }

    calcRHS( ytemp, ak5);
    for(i=0; i<n; i++) {
        ytemp[i] = y[i] + h * (b61*dydx[i] + b62*ak2[i] + b63*ak3[i] + b64*ak4[i] + b65*ak5[i]);
    }

    calcRHS( ytemp, ak6);
    for(i=0; i<n; i++) {
        yout[i] = y[i] + h * (c1*dydx[i] + c3*ak3[i] + c4*ak4[i] + c6*ak6[i]);
        yerr[i] = h * (dc1*dydx[i] + dc3*ak3[i] + dc4*ak4[i] + dc5*ak5[i] + dc6*ak6[i]);
    }

    delete [] ytemp;
    delete [] ak6;
    delete [] ak5;
    delete [] ak4;
    delete [] ak3;
    delete [] ak2;
}


/**
 *  stepper function
 */
void SystemData::rkqs( double *y, double *dydx, int n, double *t,
                       double htry, double eps, double *yscal, double &hdid, double &hnext )
{
    register int i;
    double errmax, h, htemp, *yerr, *ytemp;

    yerr  = new double[n];
    ytemp = new double[n];

    h = htry;
    for(;;) {
        rkck( y, dydx, n, h, ytemp, yerr );

        errmax = 0.0;
        for(i=0; i<n; i++) {
            errmax = DEF_MAX( errmax, fabs(yerr[i]/yscal[i]) );
        }
        errmax /= eps;

        if (errmax <= 1.0) {  // Step succeeded. Compute size of next step.
            break;
        }

        htemp = SAFETY * h * pow(errmax, PSHRNK);
        h = (h>=0.0 ? DEF_MAX(htemp,0.1*h) : DEF_MIN(htemp,0.1*h));
        if (h<1e-8) {
            break;
        }
    }

    if (errmax > ERRCON) {
        hnext = SAFETY * h * pow(errmax,PGROW);
    } else {
        hnext = 5.0*h;
    }

    *t += (hdid=h);
    for(i=0; i<n; i++) {
        y[i] = ytemp[i];
    }

    delete [] yerr;
    delete [] ytemp;
}


void SystemData::CalcTrajectory(double initX, double initY) {
    double y[4], yscal[4], dydx[4];
    double h = 0.005;
    y[0] = initX;
    y[1] = initY;
    y[2] = 0.0;
    y[3] = 0.0;

    if (m_trajectory==NULL) {
        m_trajectory = new float[m_maxNumPoints*4];
    }
    m_trajTime.clear();

    int N = m_maxNumPoints;
    float* fptr = m_trajectory;

    double t = 0.0;
    double hdid;
    double hnext = h;

    m_numPoints = 0;
    register int nstp,i;

    for(nstp=0; nstp<N; nstp++) {
#ifdef USE_SPHERICAL
        *(fptr++) = static_cast<float>(l*sin(y[0])*cos(y[1]));
        *(fptr++) = static_cast<float>(l*sin(y[0])*sin(y[1]));
#else
        *(fptr++) = static_cast<float>(y[0]);
        *(fptr++) = static_cast<float>(y[1]);
#endif
        *(fptr++) = static_cast<float>(y[2]);
        *(fptr++) = static_cast<float>(y[3]);
        m_trajTime.push_back(t);

        //fprintf(stdout,"%f %f\n",pos[nstp*4+0],pos[nstp*4+1]);

        calcRHS(y,dydx);
        for(i=0; i<4; i++) {
            yscal[i] = fabs(y[i]) + fabs(dydx[i]*h) + TINY;
        }
        rkqs(y,dydx,4,&t,h,1e-8,yscal,hdid,hnext);
        t += hdid;

        m_numPoints = m_numPoints+1;
        if (fabs(hnext)<1e-8) {
            break;
        }
        h = hnext;
    }
    //std::cerr << m_numPoints << " " << (*vbo) << std::endl;
    if (m_numPoints>1) {
        m_currAnimTime = m_trajTime.at(0);
    }
}

void SystemData::UpdateTrajectory(unsigned int *vbo) {
    glBindBuffer( GL_ARRAY_BUFFER, *vbo );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(float)*m_numPoints*4,m_trajectory);
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

void SystemData::LoadParams( QString filename ) {
    m_magnets.clear();

    QFile fin(filename);
    if (fin.open(QFile::ReadOnly)) {
        QTextStream ts(&fin);
        while (!ts.atEnd()) {
            QString line = ts.readLine();
            if (!line.startsWith("#") && !line.isEmpty()) {
                QStringList sepLine = line.split(QRegExp("\\s+"));
                if (sepLine.size()>1) {
                    if (sepLine[0].compare("pendulumHeight")==0) {
                        m_pendulumHeight = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("pendulumLength")==0) {
                        m_pendulumLength = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("gravity")==0) {
                        m_gravity = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("damping")==0) {
                        m_kappa = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("magFactor")==0) {
                        m_magFactor = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("maxTheta")==0) {
                        m_maxTheta = sepLine[1].toDouble();
                    }
                    else if (sepLine[0].compare("magnet")==0 && sepLine.size()>6) {
                        magnetProps mp = { glm::vec3( sepLine[1].toFloat(), sepLine[2].toFloat(),0.0),
                                           sepLine[6].toFloat(),
                                           glm::vec4( sepLine[3].toFloat(), sepLine[4].toFloat(), sepLine[5].toFloat(),1.0),
                                           idToColor(m_magnets.size()+MAGNET_COLOR_ID_OFFSET)};
                        m_magnets.push_back(mp);
                    }
                }
            }
        }
        fin.close();

        if (m_magnets.size()<1) {
            magnetProps mp1 = { glm::vec3(-0.03,-0.03,0.0), 1.0, glm::vec4(1.0,0.0,0.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 0) };
            magnetProps mp2 = { glm::vec3( 0.03,-0.03,0.0), 1.0, glm::vec4(0.0,1.0,0.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 1) };
            magnetProps mp3 = { glm::vec3( 0.0, 0.03*sqrt(2.0),0.0), 1.0, glm::vec4(0.0,0.0,1.0,1.0), idToColor(MAGNET_COLOR_ID_OFFSET + 2) };
            m_magnets.push_back(mp1);
            m_magnets.push_back(mp2);
            m_magnets.push_back(mp3);
        }
        emit dataRead();
    } else {
        fprintf(stderr,"Cannot read parameter file %s\n",filename.toStdString().c_str());
    }
}

void SystemData::SaveParams( QString filename ) {
    QFile fout(filename);
    if (fout.open(QFile::WriteOnly)) {
        QTextStream ts(&fout);
        ts << "pendulumHeight " << m_pendulumHeight << endl;
        ts << "pendulumLength " << m_pendulumLength << endl;
        ts << "gravity " << m_gravity << endl;
        ts << "damping " << m_kappa << endl;
        ts << "magFactor " << m_magFactor << endl;
        ts << "maxTheta " << m_maxTheta << endl;
        ts << endl;
        for(int m=0; m<m_magnets.size(); m++) {
            ts << "magnet " << m_magnets[m].pos.x << " " << m_magnets[m].pos.y << " "
               << m_magnets[m].color.x << " " << m_magnets[m].color.y << " " << m_magnets[m].color.z << " "
               << m_magnets[m].alpha << endl;
        }
        fout.close();
    } else {
        fprintf(stderr,"Cannot save parameter file %s\n",filename.toStdString().c_str());
    }
}

bool SystemData::CalcNextPos() {
    if (m_numPoints<=1) {
        m_currAnimPos = glm::vec2(0,0);
        m_currIndex = 0;
        m_currAnimTime = 0.0f;
    }

    if (m_currIndex < m_numPoints-1) {
        m_currAnimTime += m_animateTimer->interval() * 0.001f * TIMER_INTERVAL * TIMER_SCALING;
        int i = m_currIndex;
        while (i>=0 && i<(int)m_trajTime.size()-1) {
            if (m_currAnimTime>=m_trajTime[i] && m_currAnimTime < m_trajTime[i+1]) {
                m_currIndex = i;
                float t = (m_currAnimTime - m_trajTime[i])/(m_trajTime[i+1] - m_trajTime[i]);
                glm::vec2 p1 = glm::vec2(m_trajectory[4*i+0],m_trajectory[4*i+1]);
                glm::vec2 p2 = glm::vec2(m_trajectory[4*(i+1)+0],m_trajectory[4*(i+1)+1]);
                m_currAnimPos = p1 + t*(p2-p1);
                //fprintf(stderr,"%f  %f %f\n",m_currAnimTime,m_currAnimPos.x,m_currAnimPos.y);
                return true;
            }
            i++;
        }
    }
    return false;
}


glm::vec3 SystemData::idToColor( unsigned int id ) {
    glm::vec3 col = glm::vec3(0.0f);
    glm::ivec3 color = glm::ivec3(0);
    unsigned int num = id;
    color.r = num % 256;
    num = num >> 8;
    color.g = num % 256;
    num = num >> 8;
    color.b = num % 256;
    num = num >> 8;
    col = glm::vec3(color)/255.0f;
    return col;
}

/**
 *  Convert color to object ID.
 * @param buf  color defined as 3-array [red,green,blue]
 * @return object ID
 */
unsigned int SystemData::colorToId( unsigned char *buf ) {
    unsigned int num = 0;
    int b1 = (int)buf[0];
    int b2 = (int)buf[1];
    int b3 = (int)buf[2];
    num = (b3<<16) + (b2<<8) + b1;
    return num;
}
