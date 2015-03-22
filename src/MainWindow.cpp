/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file MainWindow.cpp
*/

#include "MainWindow.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QGridLayout>
#include <QMenuBar>
#include <QMessageBox>
#include <QResizeEvent>
#include <QStatusBar>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    init();
    setWindowTitle("MagneticPendulum Simulation");
}


MainWindow::~MainWindow() {
    delete mSysData;
    delete mOpenGL2d;
}

/**
 * This property holds the recommended minimum size for the widget.
 */
QSize MainWindow::minimumSizeHint() const {
    return QSize( DEF_GUI_WIDTH, DEF_GUI_HEIGHT );
}

/** This property holds the recommended size for the widget.
*/
QSize MainWindow::sizeHint() const {
    return QSize( DEF_GUI_WIDTH, DEF_GUI_HEIGHT );
}

void MainWindow::Update() {
    mOpenGL2d->ResetParticleSimulation();
}

void MainWindow::quit() {
    close();
}

void MainWindow::loadParams() {
    QDir cdir = QDir::current();
    cdir.cd("config");
    QString filename = QFileDialog::getOpenFileName(this,"Load Parameters",cdir.absolutePath(),"*.par");
    if (!filename.isEmpty()) {
        if (!filename.endsWith(".par")) {
            filename.append(".par");
        }
        mSysData->LoadParams(filename);
        mSysView->UpdateMagnet(0);
    }
}

void MainWindow::saveParams() {
    QDir cdir = QDir::current();
    cdir.cd("config");
    QString filename = QFileDialog::getSaveFileName(this,"Save Parameters",cdir.absolutePath(),"*.par");
    if (!filename.isEmpty()) {
        if (!filename.endsWith(".par")) {
            filename.append(".par");
        }
        mSysData->SaveParams(filename);
    }
}

void MainWindow::particleStep() {
    mOpenGL2d->particleStep();
    lcd_numSteps->display( mSysData->m_numSteps );
}

void MainWindow::particleReset() {
    mOpenGL2d->ResetParticleSimulation();
    mSysData->m_numSteps = 0;
    lcd_numSteps->display( mSysData->m_numSteps );
}

void MainWindow::selectTab() {
    QObject* obj = sender();
    QString objName = obj->objectName();
    if (objName.compare("act_view2d")==0) {
        tab_central->setCurrentIndex(0);
        mSysView->SetView(0);
        mActionAnimate->setEnabled(false);
        mActionPlay->setEnabled(true);
    } else {
        tab_central->setCurrentIndex(1);
        mSysView->SetView(1);
        mActionAnimate->setEnabled(true);
        mActionPlay->setEnabled(false);
    }
    mSysView->SetTimer(false);
    mSysView->SetAnimTimer(false);
}

void MainWindow::animateStep() {
    if (!mSysData->CalcNextPos()) {
        mSysView->SetAnimTimer(false);
    }
    mOpenGL3d->updateGL();
}

void MainWindow::about() {
    QMessageBox::about( this, tr("MPSim") ,
                        tr("MagneticPendulum Simulation\n\nCopyright (c) 2014, Thomas Mueller\nVisualization Research Center and SFB 716\nUniversity of Stuttgart, Germany\n\nEmail: Thomas.Mueller@vis.uni-stuttgart.de") );
}

void MainWindow::play() {
    if (mSysData->m_timer->isActive()) {
        mSysView->SetTimer(false);
    }
    else {
        mSysView->SetTimer(true);
    }
}

void MainWindow::animate() {
    if (mSysData->m_animateTimer->isActive()) {
        mSysView->SetAnimTimer(false);
    } else {
        mSysView->SetAnimTimer(true);
    }
}

void MainWindow::showParamWin() {
    if (mSysView->isVisible()) {
        mSysView->hide();
    } else {
        mSysView->show();
    }
}

void MainWindow::grabWindow() {
    QDir cdir = QDir::current();
    QString filename = QFileDialog::getSaveFileName(this,"Save window",cdir.absolutePath(),"*.png");
    if (!filename.isEmpty()) {
        if (!filename.endsWith(".png")) {
            filename.append(".png");
        }
        if (tab_central->currentIndex()==0) {
            mOpenGL2d->GrabWindow(filename);
        } else {
            mOpenGL3d->GrabWindow(filename);
        }
    }
}

void MainWindow::init() {
    initElements();
    initActions();
    initConnects();
    initScripting();
    initGUI();
    initMenus();
}


void MainWindow::initElements() {

    QGLFormat format;
#if QT_VERSION >= 0x050000
    format.setVersion(4,3);
#endif
    format.setProfile(QGLFormat::CoreProfile);
    format.setAlpha(true);
    //fprintf(stderr,"QGLFormat version: %d.%d\n",format.majorVersion(),format.minorVersion());

    mSysData  = new SystemData();
    mOpenGL2d = new OpenGL2d(format,mSysData,this);
    mSysData->setOpenGLPtr(mOpenGL2d);

    mOpenGL3d = new OpenGL3d(format,mSysData,this);

    mSysData->m_timer = new QTimer();
    mSysData->m_timer->setInterval(1);
    connect( mSysData->m_timer, SIGNAL(timeout()), this, SLOT(particleStep()) );

    mSysData->m_animateTimer = new QTimer();
    mSysData->m_animateTimer->setInterval(TIMER_INTERVAL);
    connect( mSysData->m_animateTimer, SIGNAL(timeout()), this, SLOT(animateStep()) );

    mSysView = new SystemView(mSysData,mOpenGL2d,mOpenGL3d);

    lab_numSteps = new QLabel("# steps:");
    lcd_numSteps = new QLCDNumber();
    lcd_numSteps->setEnabled(false);
    lcd_numSteps->display(0);
    lcd_numSteps->setSegmentStyle(QLCDNumber::Flat);

    mStatusBar = new QStatusBar(this);
    mStatusBar->addPermanentWidget(lab_numSteps);
    mStatusBar->addPermanentWidget(lcd_numSteps);
}


void MainWindow::initConnects() {
    connect( mSysView,  SIGNAL(emitReset()),  this,  SLOT(particleReset()) );
    connect( mSysView,  SIGNAL(updateView()), mOpenGL3d, SLOT(updateGL()) );
    connect( mOpenGL2d, SIGNAL(magnetMoved(int)), mSysView, SLOT(UpdateMagnet(int)) );
    connect( mOpenGL3d, SIGNAL(magnetMoved(int)), mSysView, SLOT(UpdateMagnet(int)) );
    connect( mSysData,  SIGNAL(dataRead()), mSysView, SLOT(SetAllParams()) );    
    connect( mOpenGL3d, SIGNAL(updateTraj()), mOpenGL2d, SLOT(UpdateTraj()) );
}


void MainWindow::initMenus() {
    // ------------------
    //  File menu
    // ------------------
    mFileMenu = menuBar()->addMenu("&File");
    mFileMenu->addAction(QIcon(":/open.png"),"Load params",this,SLOT(loadParams()),Qt::CTRL|Qt::Key_L)->setIconVisibleInMenu(true);
    mFileMenu->addAction(QIcon(":/save.png"),"Save params",this,SLOT(saveParams()),Qt::CTRL|Qt::Key_S)->setIconVisibleInMenu(true);
    mFileMenu->addSeparator();
    mFileMenu->addAction(mActionShowParamsWin);
    mFileMenu->addAction(mActionGrabWindow);
    mFileMenu->addSeparator();
    mFileMenu->addAction(QIcon(":/exit.png"),"Quit",this,SLOT(close()),Qt::CTRL|Qt::Key_Q)->setIconVisibleInMenu(true);

    // ------------------
    //  Help menu
    // ------------------
    mHelpMenu = menuBar()->addMenu("&Help");
    mHelpMenu->addAction(QIcon(":/about.png"),"&About",this,SLOT(about()));
}


void MainWindow::initGUI() {
    tab_central = new QTabWidget();
    tab_central->addTab(mOpenGL2d,"View2D");
    tab_central->addTab(mOpenGL3d,"View3D");

    addDockWidget(Qt::RightDockWidgetArea, mSysView);

    setCentralWidget(tab_central);
    setStatusBar(mStatusBar);        
}


void MainWindow :: initActions() {
    mActionPlay = new QAction("Play",this);
    mActionReset = new QAction("Reset",this);

#ifdef HAVE_COMP_SHADER
    mActionPlay->setShortcut(Qt::CTRL|Qt::Key_P);
    addAction( mActionPlay );
    connect  ( mActionPlay, SIGNAL(triggered()), this, SLOT(play()) );    

    mActionReset->setShortcut(Qt::CTRL|Qt::Key_R);
    addAction( mActionReset );
    connect  ( mActionReset, SIGNAL(triggered()), this, SLOT(particleReset()) );
#endif // HAVE_COMP_SHADER

    mActionAnimate = new QAction("Animate",this);
    mActionAnimate->setShortcut(Qt::CTRL|Qt::Key_A);
    addAction( mActionAnimate );
    connect  ( mActionAnimate, SIGNAL(triggered()), this, SLOT(animate()) );

#ifdef HAVE_COMP_SHADER
    mActionPlay->setEnabled(true);
    mActionAnimate->setEnabled(false);
#else
    mActionAnimate->setEnabled(true);
#endif

    mActionShowParamsWin = new QAction("Show Param Win",this);
    mActionShowParamsWin->setShortcut(Qt::CTRL|Qt::Key_W);
    addAction( mActionShowParamsWin );
    connect  ( mActionShowParamsWin, SIGNAL(triggered()), this, SLOT(showParamWin()) );

    mActionGrabWindow = new QAction("Grab window",this);
    mActionGrabWindow->setShortcut(Qt::CTRL|Qt::Key_G);
    addAction( mActionGrabWindow );
    connect  ( mActionGrabWindow, SIGNAL(triggered()), this, SLOT(grabWindow()) );

    mActionSelectOGL2Tab = new QAction("View2D",this);
    mActionSelectOGL2Tab->setObjectName("act_view2d");
    mActionSelectOGL2Tab->setShortcut(Qt::CTRL|Qt::Key_1);
    addAction( mActionSelectOGL2Tab );
    connect  ( mActionSelectOGL2Tab, SIGNAL(triggered()), this, SLOT(selectTab()) );

    mActionSelectOGL3Tab = new QAction("View3D",this);
    mActionSelectOGL3Tab->setObjectName("act_view3d");
    mActionSelectOGL3Tab->setShortcut(Qt::CTRL|Qt::Key_2);
    addAction( mActionSelectOGL3Tab );
    connect  ( mActionSelectOGL3Tab, SIGNAL(triggered()), this, SLOT(selectTab()) );
}


void MainWindow::initScripting() {
    //QScriptValue bv = mScriptEngine->newQObject(this);
    //mScriptEngine->globalObject().setProperty("Main",bv);
}



void MainWindow::closeEvent( QCloseEvent * event ) {
    //fprintf(stderr,"CloseAll\n");
    QApplication::closeAllWindows();
    event->accept();
}
