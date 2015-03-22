/**
    Copyright (c) 2014, Universitaet Stuttgart, VISUS, SFB 716, Thomas Mueller

    MPSim is licensed under a Creative Commons
    Attribution-ShareAlike 3.0 Unported License.

    http://creativecommons.org/licenses/by-sa/3.0/deed.en_US

    @brief Header file for DoubleEdit.
    @file DoubleEdit.h
*/

#ifndef DOUBLE_EDIT_H
#define DOUBLE_EDIT_H

#include <qtdefs.h>
#include <QColor>
#include <QLineEdit>

/**
 * @brief Specialization of QLineEdit for double values only
 */
class DoubleEdit : public QLineEdit
{
    Q_OBJECT

public:
    DoubleEdit ( int prec, double val = 0.0, double stp = 0.01, bool needsApproval = false, QWidget* parent = 0 );
    ~DoubleEdit ( );

    // --------- public methods -----------
public:
    /** Set value.
     * @param value new value.
     */
    void    setValue( double value );

    /** Get value.
     * @return  value.
     */
    double  getValue();

    /** Set stepsize.
     * @param step  new stepsize.
     */
    void    setStep( double step );

    /** Get stepsize.
     * @return  stepsize.
     */
    double  getStep();

    /** Set value and stepsize.
     * @param value  new value.
     * @param step   new step.
     */
    void    setValueAndStep ( double val, double stp );

    /** Get value and stepsize.
     * @param value  reference to value.
     * @param step   reference to step.
     */
    void    getValueAndStep ( double &val, double &stp );

    /** Set range for input.
     * @param min  range minimum.
     * @param max  range maximum.
     */
    void    setRange ( double min, double max );

    /** Get range for input.
     * @param min : reference to range minimum.
     * @param max : reference to range maximum.
     */
    void    getRange ( double &min, double &max );

    void    setApprovalColor( QColor color );
    QColor  getApprovalColor();

    // --------- public slots -----------
public slots:
    void    slot_setValue ( );
    void    slot_setStep  ( );


    // --------- protected methods -----------
protected:
    /** Handle key press event.
     *  @param event  key event.
     */
    virtual void  keyPressEvent       ( QKeyEvent* event );

    /** Wheel event handling.
     * @param event  pointer to QWheelEvent.
     */
    virtual void  wheelEvent          ( QWheelEvent* event );
    
    // ------ protected attributes --------
protected:
    int    mPrecision;
    double mStep;
    double mMin;
    double mMax;
    bool   mNeedsApproval;
    QColor mApprovalColor;
};

#endif // DOUBLE_EDIT_H
