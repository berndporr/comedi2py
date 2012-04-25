/**
 * comedi2py.h
 * (c) 2012, Bernd Porr, no warranty, GNU-public license
 * BerndPorr@f2s.com
 **/
class Comedi2py;
#ifndef COMEDI2PY_H
#define COMEDI2PY_H

#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QPushButton>
#include <QCheckBox>
#include <QLayout> 
#include <QTextEdit>
#include <QGroupBox>
#include <QLabel>

#include "comediasync.h"
#include <Python.h>

///////////////////////
/// python

#define PY_START_FUNCTION_NAME "comedistart"
#define PY_DATA_FUNCTION_NAME "comedidata"
#define PY_STOP_FUNCTION_NAME "comedistop"

class Comedi2py : public QWidget
{

    Q_OBJECT
public:
/**
 * Constructor
 **/
	Comedi2py( QWidget *parent, 
		      int channels,
		      int num_of_devices,
		      int requested_sampling_rate,
		      const char* fname
		);
	
	/**
	 * Destructor
	 **/
	~Comedi2py();
	
    /**
     * The widget which contains the graphical plots of the AD-data
     **/
    ComediAsync* comediAsync;

    /**
     * Text-field: elapsed time
     **/
    QTextEdit* timeInfoTextEdit;

 private:
    /**
     * Button which controls recording
     **/
    QCheckBox *recPushButton;

/**
 * Text-field: time between samples
 **/
    QTextEdit   *tbInfoTextEdit;

private slots:
/**
 * Button to increase the time-base has been pressed
 **/
    void incTbEvent();

/**
 * Button to decrease the time-base has been pressed
 **/
    void decTbEvent();
    
private:
/**
 * Called if a change in the time-base has occurred
 **/
    void changeTB();

/**
 * returns the timebase
 **/
public:
    int getTB() {return tb_us;};

private:
/**
 * Time between two samples in ms
 **/
    int tb_us;

/**
 * filename of the python file
 **/
    const char* filename;

///////////////////////////////////////////////
//Python stuff

private:
    PyObject *pName, *pModule, *pDict;
    PyObject *pStartFunc,*pStopFunc,*pDataFunc;
    PyObject *pValue;
    PyObject **pList;
    PyObject *pArgs,*pStartArgs;
    PyObject *pValueSamplingrate;

public:
    void initPython();
    void runPython(float **buffer);
    void closePython();
};


#endif
