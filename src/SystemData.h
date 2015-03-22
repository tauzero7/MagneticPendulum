/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file SystemData.h
*/

#ifndef  MPSIM_SYSTEM_DATA_H
#define  MPSIM_SYSTEM_DATA_H

#include <vector>
#include <qtdefs.h>
#include <QColor>
#include <QTimer>
#include <QScriptEngine>

#define MAGNET_COLOR_ID_OFFSET  1000
#define BOB_COLOR_ID           15000
#define TIMER_INTERVAL            10
#define TIMER_SCALING           0.1f

class OpenGL2d;

#include "glm.hpp"

typedef struct magnetProps_t {
    glm::vec3 pos;
    float alpha;
    glm::vec4 color;
    glm::vec3 idCol;
} magnetProps;


class SystemData : public QObject
{
    Q_OBJECT

public:
    SystemData();
    ~SystemData();


public:
    void   AddObjectsToScriptEngine ( QScriptEngine* engine );   //!< Add this object to the script engine.
    void   setOpenGLPtr             ( OpenGL2d* ogl );

    void   CalcTrajectory(double initX, double initY);
    void   UpdateTrajectory(unsigned int *vbo);
    void   LoadParams( QString filename );
    void   SaveParams( QString filename );
    bool   CalcNextPos();

    glm::vec3    idToColor( unsigned int id );
    unsigned int colorToId( unsigned char buf[3] );

public slots:
    void   ResetParams();
    void   ResetAnim();
    void   SetTimerInterval( int val );    //!< Set interval of qt timer; if val=0 the timeout is fired as fast as possible.

signals:
    void   dataRead();


private:
    void  calcRHS( double *y, double *rhs );
    void  rkck( double *y, double *dydx, int n, double h, double *yout, double *yerr );
    void  rkqs( double *y, double *dydx, int n, double *t, double htry, double eps, double *yscal, double &hdid, double &hnext );

private:
    OpenGL2d*   mOpenGL2d;

public:
    double  m_pendulumLength;
    double  m_pendulumHeight;
    double  m_gravity;
    double  m_damping;
    double  m_kappa;
    double  m_magFactor;
    double  m_maxTheta;
    double  m_rmax, m_rmaxX, m_rmaxY;

    QList<magnetProps>  m_magnets;
    float   m_magnetSize;
    float*  m_trajectory;
    std::vector<float>  m_trajTime;
    std::vector<float>::iterator m_TrajTimeItr;

    int     m_maxNumPoints;
    int     m_numPoints;
    int     m_lineWidth;
    QColor  m_lineColor;

    bool    m_play;
    QTimer* m_timer;
    int     m_numSteps;
    double  m_tScale;

    QTimer* m_animateTimer;
    glm::vec2 m_currAnimPos;
    int     m_currIndex;
    float   m_currAnimTime;

    QColor bobColor;
    QColor ambientColor;
    QColor diffuseColor;
    QColor specularColor;
    float  k_ambient, k_diffuse, k_specular, k_exp;
};

#endif // MPSIM_SYSTEM_DATA_H
