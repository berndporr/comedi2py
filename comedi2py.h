/**
 * comedi2py.h
 * (c) 2012, Bernd Porr, no warranty, GNU-public license
 * BerndPorr@f2s.com
 * www.berndporr.me.uk
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

// the 3 python functions are called by comedi2py
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
		   int nchannels,
		   int num_of_devices,
		   int requrested_sampling_rate,
		   int py_sampling_rate,
		   const char *fname
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

private:
        /**
	 * Sampling rate seen by the python callback functions
	 **/
	int pySamplingRate;

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
	 PyObject *pValueComedidev;
	 PyObject **pList;
	 PyObject *pArgs,*pStartArgs,*pStopArgs;
	 PyObject *pValueSamplingrate;

public:
	 void initPython();
	 void runPython(float **buffer);
	 void closePython();
};


#endif
