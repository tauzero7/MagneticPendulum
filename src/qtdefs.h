/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for MPsim definitions.
    @file qtdefs.h
*/

#ifndef QTMPSIM_DEFS_H
#define QTMPSIM_DEFS_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QList>

// -----------------------------------
//   global definitions
// -----------------------------------
#ifndef DEG_TO_RAD
#define DEG_TO_RAD  0.017453292519943295770
#define RAD_TO_DEG  57.295779513082320875
#endif

#ifndef DBL_MAX
#define DBL_MAX 1.844674407370955616e19
#endif

#define  DEF_SIGN(x)   ((x)>=0?1:-1)
#define  DEF_SQR(x)    ((x)*(x))


// -----------------------------------
//   GUI definitions
// -----------------------------------

#ifdef  USE_BIG_SCREEN
#define    DEF_GUI_WIDTH     1550
#define    DEF_GUI_HEIGHT    1150
#define    DEF_DRAW_WIDTH    1100
#define    DEF_DRAW_HEIGHT    700
#define    DEF_TEXT_WIDTH     350
#define    DEF_WIDGET_HEIGHT   20
#define    DEF_DWIDGET_WIDTH   60
#define    DEF_MAX_PER_LINE     8
#define    DEF_MAX_LINES       30
#else
#define    DEF_GUI_WIDTH     1270
#define    DEF_GUI_HEIGHT     700
#define    DEF_OGL_WIDTH      900
#define    DEF_OGL_HEIGHT     500
#define    DEF_TEXT_WIDTH     280
#define    DEF_WIDGET_HEIGHT   20
#define    DEF_DWIDGET_WIDTH   60
#define    DEF_MAX_PER_LINE     8
#define    DEF_MAX_LINES       26
#endif

#define    DEF_TBLVIEW_ROW_HEIGHT  20
#define    DEF_DOUBLE_EDIT_COLOR   10,10,100

const float quadVerts[] = { 0.0f,0.0f, 1.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f };

#endif // QTMPSIM_DEFS_H
