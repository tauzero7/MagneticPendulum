/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for graphical user interface of MPSim.
    @file MainWindow.h
*/

#ifndef MPSIM_MAINWINDOW_H
#define MPSIM_MAINWINDOW_H

#include <OpenGL2d.h>
#include <OpenGL3d.h>
#include <SystemData.h>
#include <SystemView.h>

#include <qtdefs.h>
#include <QtScript>
#include <QLCDNumber>
#include <QMainWindow>
#include <QDockWidget>
#include <QList>
#include <QTabWidget>

/**
 * @brief The MainWindow class
 *
 *  This is the main window of the graphical user interface. It initialized the main
 *  UI elements, the menu bar, and the scripting stuff.
 *
 *  All public slots can be used directly within the script editor. But note that
 *  only public slots starting with a captial should be used. Methods with the
 *  prefix 'Q_INVOKABLE' are also available as scripting functions. Here, the 'Help()'
 *  function is most important.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:    
    virtual QSize minimumSizeHint() const;
    virtual QSize sizeHint() const;

    void Update();

protected:
    void  init();           //!< Initialize view.
    void  initElements();   //!< Initialize view elements.
    void  initGUI();        //!< Construct graphical user interface.
    void  initActions();    //!< Initialize actions.
    void  initConnects();   //!< Connect signals and slots.
    void  initMenus();      //!< Initialize menu bars.
    void  initScripting();  //!< Initialize scripting.

    virtual void closeEvent(QCloseEvent *event);

public slots:
    void about();   //!< Show about message.
    void play();    //!< Play.
    void quit();    //!< Quit program.
    void loadParams();
    void saveParams();
    void animate();
    void showParamWin();
    void grabWindow();

protected slots:
    void  particleStep();
    void  particleReset();
    void  selectTab();
    void  animateStep();

private:    
    SystemData*  mSysData;
    SystemView*  mSysView;
    OpenGL2d*    mOpenGL2d;
    OpenGL3d*    mOpenGL3d;

    QTabWidget*   tab_central;
    QStatusBar*   mStatusBar;
    QLabel*       lab_status_numParticles;
    QLCDNumber*   lcd_status_numParticles;
    QLabel*       lab_numSteps;
    QLCDNumber*   lcd_numSteps;


    // ---- File Menu ----
    QMenu*       mFileMenu;
    QAction*     mActionLoadParams;
    QAction*     mActionSaveParams;
    QAction*     mActionShowParamsWin;
    QAction*     mActionGrabWindow;
    QAction*     mActionSelectOGL2Tab;
    QAction*     mActionSelectOGL3Tab;

    // ---- Help menu ----
    QMenu*       mHelpMenu;
    QAction*     mActionAbout;
    QAction*     mActionPlay;
    QAction*     mActionReset;
    QAction*     mActionAnimate;
};

#endif // MPSIM_MAINWINDOW_H
