/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file SystemView.h
*/

#ifndef MPSIM_SYSTEM_VIEW_H
#define MPSIM_SYSTEM_VIEW_H

#include "DoubleEdit.h"
#include "OpenGL2d.h"
#include "OpenGL3d.h"
#include "SystemData.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSortFilterProxyModel>

/**
 * @brief The SystemView class
 */
class SystemView : public QDockWidget
{
    Q_OBJECT

   // Q_PROPERTY( double   maxArea   READ GetMaxArea      WRITE  SetMaxArea )
    //Q_PROPERTY( double   minAngle  READ GetMinAngle     WRITE  SetMinAngle )


public:
    /** Standard constructor.
     * @param sd pointer to system data widget.
     * @param ogl pointer to main opengl widget.
     * @param parent  pointer to parent widget.
     */
    SystemView ( SystemData* sd, OpenGL2d* ogl, OpenGL3d* ogl3, QWidget* parent = 0 );

    /** Standard destructor.
     */
    virtual ~SystemView ();

    void AddObjectsToScriptEngine( QScriptEngine* engine );
    void SetView( int view );

public slots:
    void  SetTimer(bool);
    void  SingleTimeStep();
    void  Reset();
    void  Play();    
    void  UpdateMagnet(int);
    void  SetAllParams();

    void  AnimPlay();
    void  SetAnimTimer(bool);
    void  AnimReset();
    void  SingleAnimTimeStep();

protected slots:
    void updateParams();
    void changeMagnet(int);
    void setMagnetParams();
    void setMagnetColor();
    void setTScale();
    void setLineWidth(int);
    void setLineColor();
    void setMaxNumPoints(int);

signals:
    void emitReset();
    void updateView();

protected:
    void init();
    void initElements();
    void initGUI();
    void initActions();
    void initConnect();

    virtual QSize  sizeHint () const;
 
    void setBlockMagnetSignals( bool block );

private:
    SystemData*   mData;
    OpenGL2d*     mOpenGL2d;
    OpenGL3d*     mOpenGL3d;

    QLabel*       lab_pendulumHeight;
    DoubleEdit*   led_pendulumHeight;
    QLabel*       lab_pendulumLength;
    DoubleEdit*   led_pendulumLength;

    QLabel*       lab_gravity;
    DoubleEdit*   led_gravity;
    QLabel*       lab_damping;
    DoubleEdit*   led_damping;
    QLabel*       lab_kappa;
    DoubleEdit*   led_kappa;
    QLabel*       lab_magFactor;
    DoubleEdit*   led_magFactor;
    QLabel*       lab_maxTheta;
    DoubleEdit*   led_maxTheta;

    QLabel*       lab_trajWidth;
    QSpinBox*     spb_trajWidth;
    QLabel*       lab_trajColor;
    QPushButton*  pub_trajColor;
    QLabel*       lab_trajMaxNumPoints;
    QSpinBox*     spb_trajMaxNumPoints;

    QLabel*       lab_currMagnet;
    QSpinBox*     spb_currMagnet;
    QLabel*       lab_posX;
    DoubleEdit*   led_posX;
    QLabel*       lab_posY;
    DoubleEdit*   led_posY;
    QLabel*       lab_alpha;
    DoubleEdit*   led_alpha;
    QLabel*       lab_color;
    QPushButton*  pub_color;

    QPushButton*  pub_reset;
    QPushButton*  pub_play;
    QPushButton*  pub_step;
    QLabel*       lab_timeScale;
    DoubleEdit*   led_timeScale;

    QPushButton*  pub_anim_reset;
    QPushButton*  pub_anim_play;
    QPushButton*  pub_anim_step;

    QGroupBox* grb_control;
    QGroupBox* grb_anim;
};

#endif // MPSIM_SYSTEM_VIEW_H
