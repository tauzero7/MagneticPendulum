/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @file DoubleEdit.cpp
*/

#include <iostream>
#include <limits>

#include "DoubleEdit.h"

#include <QDoubleValidator>
#include <QKeyEvent>

/*! Standard constructor.
 *  \param prec  : precision.
 *  \param value : initial value.
 *  \param step  : stepsize.
 *  \param parent : parent widget.
 */
DoubleEdit :: DoubleEdit (int prec, double val, double stp, bool needsApproval, QWidget* parent )
    : QLineEdit(parent)
{
    mPrecision = prec;
    mStep      = stp;
    setValue( val );
    setAlignment( Qt::AlignRight );

    mMin = (std::numeric_limits<double>::min)();
    mMax = (std::numeric_limits<double>::max)();

    setValidator( new QDoubleValidator(this) );
    mNeedsApproval = needsApproval;
    mApprovalColor = Qt::blue;
}

/*! Standard destructor.
 */
DoubleEdit :: ~DoubleEdit() {
}


// *********************************** public methods ******************************
void DoubleEdit::setValue ( double value ) {
    setText( QString::number(value,'f',mPrecision) );
}

double DoubleEdit::getValue() {
    /*
  double val = atof(text().toStdString().c_str());
  fprintf(stderr,"%15.12f  %15.12f\n",val,text().toDouble());
*/
    return text().toDouble();
}

void DoubleEdit::setStep( double step ) {
    mStep = step;
}

double DoubleEdit::getStep() {
    return mStep;
}

void DoubleEdit::setValueAndStep(double val, double stp ) {
    setValue(val);
    setStep(stp);
}

void DoubleEdit::getValueAndStep( double &val, double &stp ) {
    val = getValue();
    stp = getStep();
}

void DoubleEdit::setRange( double min, double max ) {
    mMin = min;
    mMax = max;
}

void DoubleEdit::getRange( double &min, double &max ) {
    min = mMin;
    max = mMax;
}

void DoubleEdit::setApprovalColor( QColor color ) {
    mApprovalColor = color;
}

QColor  DoubleEdit::getApprovalColor() {
    return mApprovalColor;
}

// *********************************** public slots ********************************

void DoubleEdit::slot_setValue() {
    QObject* obj = sender();
    DoubleEdit* led = reinterpret_cast<DoubleEdit*>(obj);
    double value = led->getValue();
    setValue(value);
}

void DoubleEdit::slot_setStep() {
    QObject* obj = sender();
    DoubleEdit* led = reinterpret_cast<DoubleEdit*>(obj);
    double value = led->getValue();
    setStep(value);
}

// ********************************* protected methods *****************************

void DoubleEdit::keyPressEvent( QKeyEvent* event ) {
    double val = text().toDouble();

    if (mNeedsApproval) {
        QPalette pal = palette();
        if ((event->key() >= 0x30 && event->key() <= 0x39) ||
                event->key() == Qt::Key_Up ||
                event->key() == Qt::Key_Down ||
                event->key() == Qt::Key_PageUp ||
                event->key() == Qt::Key_PageDown ||
                event->key() == Qt::Key_Backspace ||
                event->key() == Qt::Key_Insert ||
                event->key() == Qt::Key_Delete) {
            pal.setColor( QPalette::Text, QColor(Qt::blue));
        } else {
            pal.setColor( QPalette::Text, QColor(Qt::black));
        }
        setPalette(pal);
    }

    if (event->key()==Qt::Key_Up) {
        val+=mStep;
    } else if (event->key()==Qt::Key_Down) {
        val-=mStep;
    } else if (event->key()==Qt::Key_PageUp) {
        val+=mStep*10.0;
    } else if (event->key()==Qt::Key_PageDown) {
        val-=mStep*10.0;
    } else {
        QLineEdit::keyPressEvent(event);
        return;
    }

    if (val>mMax) {
        val=mMax;
    }
    if (val<mMin) {
        val=mMin;
    }
    setText( QString("%1").arg(val,0,'f',mPrecision) );
    emit editingFinished();
}

void  DoubleEdit::wheelEvent( QWheelEvent* event ) {
    double val = text().toDouble();

    if (mNeedsApproval) {
        QPalette pal = palette();
        pal.setColor( QPalette::Text, QColor(Qt::blue));
        setPalette(pal);
    }

    if (event->delta()>0) {
        val += mStep;
    } else {
        val -= mStep;
    }

    if (val>mMax) {
        val=mMax;
    }
    if (val<mMin) {
        val=mMin;
    }

    setText( QString("%1").arg(val,0,'f',mPrecision) );
    event->accept();

    setFocus(Qt::OtherFocusReason);
    //emit returnPressed();
    emit editingFinished();
}
