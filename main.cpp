/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file main.cpp
*/

#include <iostream>
#include <QApplication>
#include <QTextStream>

#if QT_VERSION < 0x050000
#include <QCleanlooksStyle>
#endif

#include "src/MainWindow.h"


int main( int argc, char *argv[] )
{
    QApplication app(argc, argv);
#if QT_VERSION >= 0x050100
    //QApplication::setStyle( QStyleFactory::create("Fusion") );
#else
    QStyle *style = new QCleanlooksStyle;
    app.setStyle(style);
#endif

    QString cssStyle;
    QFile cssFile(":/viewer.css");
    if (cssFile.open(QFile::ReadOnly)) {
        QTextStream styleIn(&cssFile);
        while( !styleIn.atEnd()) {
            QString line = styleIn.readLine();
            if (!line.startsWith("#"))
                cssStyle += line;
        }
        cssFile.close();
    }
    else {
        fprintf(stderr,"Cannot read stylesheet!\n");
    }
    app.setStyleSheet(cssStyle);

    QLocale::setDefault(QLocale::C);
    QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));

    MainWindow w;
    w.show();
    w.Update();
    return app.exec();
}

