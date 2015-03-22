/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file SystemView.cpp
*/

#include "SystemView.h"

#include <QAction>
#include <QColorDialog>
#include <QFileSystemModel>
#include <QGridLayout>


SystemView :: SystemView(SystemData* sd, OpenGL2d* ogl, OpenGL3d *ogl3, QWidget *parent )
    : QDockWidget(parent)
{
    mData = sd;
    mOpenGL2d = ogl;
    mOpenGL3d = ogl3;
    init();
}

SystemView::~SystemView() {
}

void SystemView::AddObjectsToScriptEngine( QScriptEngine* engine ) {
    QScriptValue sv = engine->newQObject(this);
    engine->globalObject().setProperty("Ctrl",sv);
}

#ifdef HAVE_COMP_SHADER
void SystemView::SetView( int view ) {
    if (view==0) {        
        grb_control->setEnabled(true);
        grb_anim->setEnabled(false);
    } else {
        grb_control->setEnabled(false);
        grb_anim->setEnabled(true);
    }
#else
void SystemView::SetView( int ) {
#endif // HAVE_COMP_SHADER
}

void SystemView::SetTimer( bool status ) {
    if (status) {
        mData->m_timer->setSingleShot(false);
        //        mData->m_time.restart();
        mData->m_timer->start();
        pub_play->setIcon(QIcon(":/pause.png"));
        pub_step->setEnabled(false);
    } else {
        mData->m_timer->stop();
        pub_play->setIcon(QIcon(":/play.png"));
        pub_step->setEnabled(true);
    }
    pub_play->setChecked(status);
}


void SystemView::SingleTimeStep() {
    mData->m_timer->setSingleShot(true);
    mData->m_timer->start();
    mOpenGL2d->updateGL();
}


void SystemView::Reset() {
    mData->m_timer->setSingleShot(false);
    //mOpenGL2d->ResetParticleSimulation();
    mData->m_numPoints = 0;
    mOpenGL2d->updateGL();
    emit emitReset();
}


void SystemView::Play() {    
    if (mData->m_timer->isActive()) {
        SetTimer(false);
    } else {
        SetTimer(true);
    }
}

void SystemView::AnimPlay() {
    if (mData->m_animateTimer->isActive()) {
        SetAnimTimer(false);
    } else {
        SetAnimTimer(true);
    }
}

void SystemView::SetAnimTimer(bool status ) {
    if (status) {
        mData->m_animateTimer->setSingleShot(false);
        mData->m_animateTimer->start();
        pub_anim_play->setIcon(QIcon(":/pause.png"));
        pub_anim_step->setEnabled(false);
    } else {
        mData->m_animateTimer->stop();
        pub_anim_play->setIcon(QIcon(":/play.png"));
        pub_anim_step->setEnabled(true);
    }
    pub_anim_play->setChecked(status);
}

void SystemView::AnimReset() {
    mData->m_animateTimer->setSingleShot(false);
    mData->ResetAnim();
    mOpenGL3d->updateGL();
    //emit updateView();
}

void SystemView::SingleAnimTimeStep() {
    mData->CalcNextPos();
    mOpenGL3d->updateGL();
}

void SystemView::UpdateMagnet(int m) {
    if (m==spb_currMagnet->value()) {
        led_posX->setValue(mData->m_magnets[m].pos.x);
        led_posY->setValue(mData->m_magnets[m].pos.y);
    }
}

void SystemView::updateParams() {
    mData->m_pendulumLength = led_pendulumLength->getValue();
    mData->m_pendulumHeight = led_pendulumHeight->getValue();
    mData->m_gravity = led_gravity->getValue();
    mData->m_damping = led_damping->getValue();
    mData->m_kappa   = led_kappa->getValue();
    mData->m_magFactor = led_magFactor->getValue();
    mData->m_maxTheta = led_maxTheta->getValue();
    mOpenGL2d->ResetParticleSimulation();
    mData->m_numPoints = 0;
    mOpenGL2d->updateGL();
}

void SystemView::SetAllParams() {
    led_pendulumHeight->setValue( mData->m_pendulumHeight );
    led_pendulumLength->setValue( mData->m_pendulumLength );
    led_gravity->setValue( mData->m_gravity );
    led_damping->setValue( mData->m_damping );
    led_kappa->setValue( mData->m_kappa );
    led_magFactor->setValue( mData->m_magFactor );
    led_maxTheta->setValue( mData->m_maxTheta );
    mOpenGL2d->ResetParticleSimulation();
    mData->m_numPoints = 0;
    mOpenGL2d->updateGL();

    spb_currMagnet->setRange(0,mData->m_magnets.size()-1);
    int id = spb_currMagnet->value();
    if (id>=mData->m_magnets.size()) {
        spb_currMagnet->setValue(0);
        id = 0;
    }
    changeMagnet(id);
}

void SystemView::changeMagnet( int id ) {
    led_posX->setValue( mData->m_magnets[id].pos.x );
    led_posY->setValue( mData->m_magnets[id].pos.y );
    led_alpha->setValue( mData->m_magnets[id].alpha );
    glm::vec4 col = mData->m_magnets[id].color * 255;
    pub_color->setPalette( QPalette(QColor(col.r,col.g,col.b)) );
}

void SystemView::setMagnetParams() {
    int id = spb_currMagnet->value();
    if (id<0 || id>=mData->m_magnets.size()) {
        return;
    }
    mData->m_magnets[id].pos = glm::vec3( led_posX->getValue(), led_posY->getValue(), mData->m_magnets[id].pos.z );
    mData->m_magnets[id].alpha = led_alpha->getValue();
    mData->m_numPoints = 0;
    mOpenGL2d->updateGL();
    emit updateView();
}

void SystemView::setMagnetColor() {
    int id = spb_currMagnet->value();
    if (id<0 || id>=mData->m_magnets.size()) {
        return;
    }
    glm::vec4 c = mData->m_magnets[id].color * 255;
    QColor col = QColorDialog::getColor(QColor(c.r,c.g,c.b));
    if (col.isValid()) {
        mData->m_magnets[id].color = glm::vec4( col.redF(), col.greenF(), col.blueF(), 1.0f );
        pub_color->setPalette( QPalette(col) );
    }
}

void SystemView::setBlockMagnetSignals( bool block ) {
    led_posX->blockSignals(block);
    led_posY->blockSignals(block);
    led_alpha->blockSignals(block);
    pub_color->blockSignals(block);
}

void SystemView::setTScale() {
    mData->m_tScale = led_timeScale->getValue();
    mOpenGL2d->updateGL();
}

void SystemView::setLineWidth(int lw) {
    mData->m_lineWidth = lw;
    mOpenGL2d->updateGL();
}

void SystemView::setLineColor() {
    QColor col = QColorDialog::getColor(mData->m_lineColor);
    if (col.isValid()) {
        mData->m_lineColor = col;
        pub_trajColor->setPalette( QPalette(col) );
        mOpenGL2d->updateGL();
    }
}

void SystemView::setMaxNumPoints(int num) {
    SetAnimTimer(false);
    mOpenGL2d->SetMaxNumPoints(num);
    mOpenGL2d->updateGL();
}


void SystemView::init() {
    initElements();
    initGUI();
    initActions();
    initConnect();
}


void SystemView::initElements() {
    lab_pendulumHeight = new QLabel("Height");
    led_pendulumHeight = new DoubleEdit( 2, mData->m_pendulumHeight, 0.01 );
    led_pendulumHeight->setMinimumWidth(60);
    lab_pendulumLength = new QLabel("Length");
    led_pendulumLength = new DoubleEdit( 2, mData->m_pendulumLength, 0.01);

    lab_gravity = new QLabel("gravity");
    led_gravity = new DoubleEdit(2, mData->m_gravity, 0.01, true );
    lab_damping = new QLabel("damping");
    led_damping = new DoubleEdit(3, mData->m_damping, 0.001, true);
    lab_kappa   = new QLabel("kappa");
    led_kappa   = new DoubleEdit(3, mData->m_kappa, 0.001, true);
    lab_magFactor = new QLabel("magFactor");
    led_magFactor = new DoubleEdit(3, mData->m_magFactor, 0.001, true );
    lab_maxTheta = new QLabel("maxTheta");
    led_maxTheta = new DoubleEdit(2, mData->m_maxTheta, 0.01, true);

    lab_trajWidth = new QLabel("width");
    spb_trajWidth = new QSpinBox();
    spb_trajWidth->setRange(1,5);
    spb_trajWidth->setValue( mData->m_lineWidth );
    lab_trajColor = new QLabel("color");
    pub_trajColor = new QPushButton();
    pub_trajColor->setPalette( QPalette( mData->m_lineColor) );
    lab_trajMaxNumPoints = new QLabel("#steps");
    spb_trajMaxNumPoints = new QSpinBox();
    spb_trajMaxNumPoints->setRange(1,10000);
    spb_trajMaxNumPoints->setValue( mData->m_maxNumPoints );

    lab_currMagnet = new QLabel("ID");
    spb_currMagnet = new QSpinBox();
    spb_currMagnet->setMinimum(0);
    spb_currMagnet->setMaximum(mData->m_magnets.size()-1);

    lab_posX = new QLabel("posX");
    led_posX = new DoubleEdit(3,0.0,0.001, true);
    led_posX->setRange(-1.0,1.0);

    lab_posY = new QLabel("posY");
    led_posY = new DoubleEdit(3,0.0,0.001, true);
    led_posY->setRange(-1.0,1.0);

    lab_alpha = new QLabel("alpha");
    led_alpha = new DoubleEdit(3,1.0,0.001, true);
    lab_color = new QLabel("color");
    pub_color = new QPushButton();
    changeMagnet(0);

    pub_reset = new QPushButton(QIcon(":/back.png"),"");
    pub_reset->setMaximumWidth(30);
    pub_play  = new QPushButton(QIcon(":/play.png"),"");
    pub_play->setCheckable(true);
#ifdef SHOW_TOOLTIPS
    pub_play->setToolTip("Toggle play with 'Ctrl+p'");
#endif
    pub_step  = new QPushButton(QIcon(":/step.png"),"");
    pub_step->setAutoRepeat(true);
    pub_step->setAutoRepeatInterval(10);
    lab_timeScale = new QLabel("TScale");
    led_timeScale = new DoubleEdit(2,mData->m_tScale,0.01);
    led_timeScale->setRange(0.0,100.0);

    pub_anim_reset = new QPushButton(QIcon(":/back.png"),"");
    pub_anim_play  = new QPushButton(QIcon(":/play.png"),"");
    pub_anim_play->setCheckable(true);
#ifdef SHOW_TOOLTIPS
    pub_anim_play->setToolTip("Toggle animate with 'Ctrl+a'");
#endif
    pub_anim_step  = new QPushButton(QIcon(":/step.png"),"");
    pub_anim_step->setAutoRepeat(true);
    pub_anim_step->setAutoRepeatInterval(10);
}


void SystemView::initGUI() {
    QGroupBox* grb_pend = new QGroupBox("Pendulum");
    QGridLayout* layout_pend = new QGridLayout();
#ifdef EXPERT_MODE
    layout_pend->addWidget( lab_pendulumHeight, 0, 0 );
    layout_pend->addWidget( led_pendulumHeight, 0, 1 );
    layout_pend->addWidget( lab_pendulumLength, 1, 0 );
    layout_pend->addWidget( led_pendulumLength, 1, 1 );
    layout_pend->addWidget( lab_gravity, 2, 0 );
    layout_pend->addWidget( led_gravity, 2, 1 );
    layout_pend->addWidget( lab_damping, 3, 0 );
    layout_pend->addWidget( led_damping, 3, 1 );
    layout_pend->addWidget( lab_kappa, 4, 0 );
    layout_pend->addWidget( led_kappa, 4, 1 );
    layout_pend->addWidget( lab_magFactor, 5, 0 );
    layout_pend->addWidget( led_magFactor, 5, 1 );
    layout_pend->addWidget( lab_maxTheta, 6, 0 );
    layout_pend->addWidget( led_maxTheta, 6, 1 );
    layout_pend->setRowStretch(7,5);
#else
    layout_pend->addWidget( lab_damping, 0, 0 );
    layout_pend->addWidget( led_damping, 0, 1 );
    layout_pend->addWidget( lab_maxTheta, 1, 0 );
    layout_pend->addWidget( led_maxTheta, 1, 1 );
    layout_pend->setRowStretch(2,5);
#endif // EXPERT_MODE
    grb_pend->setLayout(layout_pend);

    QGroupBox* grb_traj = new QGroupBox("Trajectory");
    QGridLayout* layout_traj = new QGridLayout();
    layout_traj->addWidget( lab_trajWidth, 0, 0 );
    layout_traj->addWidget( spb_trajWidth, 0, 1 );
    layout_traj->addWidget( lab_trajColor, 1, 0 );
    layout_traj->addWidget( pub_trajColor, 1, 1 );
    layout_traj->addWidget( lab_trajMaxNumPoints, 2, 0 );
    layout_traj->addWidget( spb_trajMaxNumPoints, 2, 1 );
    layout_traj->setRowStretch(3,5);
    grb_traj->setLayout(layout_traj);

    QGroupBox* grb_magnets = new QGroupBox("Magnets");
    QGridLayout* layout_magnets = new QGridLayout();
    layout_magnets->addWidget( lab_currMagnet, 0, 0 );
    layout_magnets->addWidget( spb_currMagnet, 0, 1 );
    layout_magnets->addWidget( lab_posX, 1, 0 );
    layout_magnets->addWidget( led_posX, 1, 1 );
    layout_magnets->addWidget( lab_posY, 2, 0 );
    layout_magnets->addWidget( led_posY, 2, 1 );
    //layout_magnets->addWidget( led_posY, 1, 2 );
    layout_magnets->addWidget( lab_alpha, 3, 0 );
    layout_magnets->addWidget( led_alpha, 3, 1 );
    layout_magnets->addWidget( lab_color, 4, 0 );
    layout_magnets->addWidget( pub_color, 4, 1 );
    layout_magnets->setRowStretch(5,5);
    grb_magnets->setLayout(layout_magnets);

    grb_anim = new QGroupBox("Animate");
    QHBoxLayout* layout_anim = new QHBoxLayout();
    layout_anim->addWidget( pub_anim_reset );
    layout_anim->addWidget( pub_anim_play );
    layout_anim->addWidget( pub_anim_step );
    grb_anim->setLayout(layout_anim);

#ifdef HAVE_COMP_SHADER
    grb_control = new QGroupBox("Control");
    QGridLayout* layout_control = new QGridLayout();
    layout_control->addWidget( pub_reset, 0, 0 );
    layout_control->addWidget( pub_play, 0, 1 );
    layout_control->addWidget( pub_step, 0, 2 );
    QHBoxLayout* layout_scale = new QHBoxLayout();
    layout_scale->addWidget( lab_timeScale );
    layout_scale->addWidget( led_timeScale );
    layout_control->addLayout( layout_scale, 1, 0, 1, 3 );
    layout_control->setRowStretch(2,5);
    grb_control->setLayout(layout_control);
    grb_control->setEnabled(true);
    grb_anim->setEnabled(false);
#endif // HAVE_COMP_SHADER


    QGridLayout* layout_complete = new QGridLayout();
    layout_complete->addWidget( grb_pend, 0, 0 );
    layout_complete->addWidget( grb_magnets, 1, 0 );
    layout_complete->addWidget( grb_traj, 2, 0 );
#ifdef HAVE_COMP_SHADER      
    layout_complete->addWidget( grb_control, 3, 0 );
    layout_complete->addWidget( grb_anim, 4, 0 );
#else
    layout_complete->addWidget( grb_anim, 3, 0 );
#endif 

    QWidget* centralWidget = new QWidget();
    centralWidget->setLayout( layout_complete );
    centralWidget->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    setWidget(centralWidget);

    setWindowTitle("Parameters");
    setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetClosable);
}


void SystemView :: initActions() {
}


void SystemView::initConnect() {
    connect( led_pendulumHeight, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_pendulumLength, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_gravity, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_damping, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_kappa, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_magFactor, SIGNAL(returnPressed()), this, SLOT(updateParams()) );
    connect( led_maxTheta, SIGNAL(returnPressed()), this, SLOT(updateParams()) );

    connect( spb_trajWidth, SIGNAL(valueChanged(int)), this, SLOT(setLineWidth(int)) );
    connect( pub_trajColor, SIGNAL(pressed()), this, SLOT(setLineColor()) );
    connect( spb_trajMaxNumPoints, SIGNAL(valueChanged(int)), this, SLOT(setMaxNumPoints(int)) );

    connect( spb_currMagnet, SIGNAL(valueChanged(int)), this, SLOT(changeMagnet(int)) );
    connect( led_posX, SIGNAL(returnPressed()), this, SLOT(setMagnetParams()) );
    connect( led_posY, SIGNAL(returnPressed()), this, SLOT(setMagnetParams()) );
    connect( led_alpha, SIGNAL(returnPressed()), this, SLOT(setMagnetParams()) );
    connect( pub_color, SIGNAL(pressed()), this, SLOT(setMagnetColor()) );

#ifdef HAVE_COMP_SHADER  
    connect( pub_play, SIGNAL(toggled(bool)), this,      SLOT(SetTimer(bool)) );
    connect( pub_step, SIGNAL(pressed()),     this,      SLOT(SingleTimeStep()) );
    connect( pub_reset, SIGNAL(pressed()),    this,      SLOT(Reset()) );
    connect( led_timeScale, SIGNAL(editingFinished()), this, SLOT(setTScale()) );
#endif // HAVE_COMP_SHADER      
    connect( pub_anim_play,  SIGNAL(toggled(bool)), this, SLOT(SetAnimTimer(bool)) );
    connect( pub_anim_reset, SIGNAL(pressed()), this, SLOT(AnimReset()) );
    connect( pub_anim_step,  SIGNAL(pressed()), this, SLOT(SingleAnimTimeStep()) );
}


QSize SystemView::sizeHint() const {
    return QSize(100,50);
}

