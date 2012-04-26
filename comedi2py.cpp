/********************************************************************
 * comedi2py.cpp 
 * License: GNU, GPL
 * (c) 2004-2012, Bernd Porr
 * No Warranty
 ********************************************************************/

#include <sys/ioctl.h>
#include <math.h>

#include <QTimer>
#include <QPainter>
#include <QApplication>
#include <QButtonGroup>
#include <QGroupBox>
#include <QFileDialog>
#include <QSizePolicy>
#include <QTextEdit>
#include <QMainWindow>
#include <QSettings>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <qtextedit.h>
#include <qfont.h>


#include <iostream>
#include <fstream>

#include "comediasync.h"
#include "comedi2py.h"

// for the layout
#define MAXROWS 8

Comedi2py::Comedi2py( QWidget *parent, 
		      int nchannels,
		      int num_of_devices,
		      int requrested_sampling_rate,
		      int py_sampling_rate,
		      const char *fname
	)
    : QWidget( parent ) {

	filename = fname;

        comediAsync=new ComediAsync(this,
				    nchannels,
				    num_of_devices,
				    requrested_sampling_rate
		);

	int factual = comediAsync->getDAQSamplingRate();

	int decimation = factual/py_sampling_rate;

	comediAsync->setDecimation(decimation);

	// fonts
	QFont voltageFont("Courier",10);
	QFontMetrics voltageMetrics(voltageFont);

	// this the main layout which contains two sub-windows:
	// the control window and the oscilloscope window
	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->setSpacing(0);
	setLayout(mainLayout);

	// this is the vertical layout for all the controls
	QHBoxLayout *controlLayout = new QHBoxLayout(0);
	// the corresponding box which contains all the controls
	QGroupBox *controlBox = new QGroupBox ();

	// this is the vertical layout for all the controls
	QHBoxLayout *scopeLayout = new QHBoxLayout(0);
	// the corresponding box which contains all the controls
	QGroupBox *scopeGroup = new QGroupBox ();

	// group for the record stuff
        QGroupBox* recGroupBox = new QGroupBox("python:");
        QHBoxLayout *recLayout = new QHBoxLayout();

        recPushButton = new QCheckBox("&data acquisition");
        recPushButton->setEnabled( true );
        recLayout->addWidget(recPushButton);

        recGroupBox->setLayout(recLayout);
        controlLayout->addWidget(recGroupBox);

       // group for the time base
        QGroupBox *tbgrp = new QGroupBox("Sampling rate");
        QHBoxLayout *tbLayout = new QHBoxLayout;
        QFont tbFont("Courier",12);
        tbFont.setBold(TRUE);
        QFontMetrics tbMetrics(tbFont);

	tbInfoTextEdit = new QTextEdit(tbgrp);
	tbInfoTextEdit->setFont (tbFont);
	QFontMetrics metricsTb(tbFont);
	tbInfoTextEdit->setMaximumSize (3*metricsTb.width("99999 sec")/2,
					3*metricsTb.height()/2);
	tbInfoTextEdit->setReadOnly(TRUE);
	tbInfoTextEdit->setLineWidth(1);
	tbInfoTextEdit->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

        QString s;
	float cbsr = comediAsync->getCallbackSamplingRate();
        if (cbsr<1000) {
                s.sprintf( "%5.0f Hz", cbsr);
        } else if (cbsr<1000000) {
                s.sprintf( "%5.0f kHz", cbsr/1000.0);
        } else {
                s.sprintf( "%5.0f Mhz", cbsr/1000000.0);
        }               
        tbInfoTextEdit->setText(s);

	tbLayout->addWidget(tbInfoTextEdit);

	tbgrp->setLayout(tbLayout);
	controlLayout->addWidget(tbgrp);
	controlBox->setLayout(controlLayout);

	comediAsync->setMinimumWidth ( 300 );
	comediAsync->setMinimumHeight ( 200 );

	scopeLayout->addWidget(comediAsync);
	scopeGroup->setLayout(scopeLayout);

	controlBox->setSizePolicy ( QSizePolicy(QSizePolicy::Fixed,
						QSizePolicy::Fixed ) );

	mainLayout->addWidget(controlBox);
	mainLayout->addWidget(scopeGroup);

	initPython();

	comediAsync->startDAQ();
}

Comedi2py::~Comedi2py() {
	delete comediAsync;
	closePython();
}


void Comedi2py::initPython() {
	Py_Initialize();

	// make the interpreter look in the working dir
	PyRun_SimpleString("import sys");
	char *workingdir = get_current_dir_name();
	char tmp[1024];
	sprintf(tmp,"sys.path.append(\"%s\")",workingdir);
	PyRun_SimpleString(tmp);
	free(workingdir);

	// load the module
	pName = PyString_FromString(filename);
	pModule = PyImport_Import(pName);
	Py_DECREF(pName);

	if (pModule == NULL) {
		fprintf(stderr,"Python module not loadable:\n");
		PyErr_Print();
		exit(1);
	}

	pStartFunc = PyObject_GetAttrString(pModule,PY_START_FUNCTION_NAME);
	if (!pStartFunc) {
		fprintf(stderr,"function "PY_START_FUNCTION_NAME" not defined\n");
		exit(1);
	}
	if (!PyCallable_Check(pStartFunc)) {
		fprintf(stderr,"function "PY_START_FUNCTION_NAME" not callable\n");
	}

	pDataFunc = PyObject_GetAttrString(pModule,PY_DATA_FUNCTION_NAME);
	if (!pDataFunc) {
		fprintf(stderr,"function "PY_DATA_FUNCTION_NAME" not defined\n");
		exit(1);
	}
	if (!PyCallable_Check(pDataFunc)) {
		fprintf(stderr,"function "PY_DATA_FUNCTION_NAME" not callable\n");
	}

	pStopFunc = PyObject_GetAttrString(pModule,PY_STOP_FUNCTION_NAME);
	if (!pStopFunc) {
		fprintf(stderr,"function "PY_STOP_FUNCTION_NAME" not defined\n");
		exit(1);
	}
	if (!PyCallable_Check(pStopFunc)) {
		fprintf(stderr,"function "PY_STOP_FUNCTION_NAME" not callable\n");
	}

	float r = comediAsync-> getCallbackSamplingRate();
	pValueSamplingrate = PyFloat_FromDouble(r);
	pStartArgs = PyTuple_New(1);
	PyTuple_SetItem(pStartArgs, 0, pValueSamplingrate);
	PyObject_CallObject(pStartFunc, pStartArgs);
	PyErr_Print();

	int ndev = comediAsync->getNcomediDevices();
	int n = comediAsync->getNchannels();
	pArgs = PyTuple_New(ndev);
	pList = new PyObject*[ndev];
	for (int d = 0; d < ndev; d++) {
		pList[d] = PyList_New(n);
		for (int i = 0; i < n; ++i) {
			pValue = PyFloat_FromDouble(0);
			if (!pValue) {
				Py_DECREF(pArgs);
				Py_DECREF(pModule);
				fprintf(stderr, "Cannot convert argument\n");
				exit(1);
			}
			PyList_SetItem(pList[d], i, pValue);
		}
		PyTuple_SetItem(pArgs, d, pList[d]);
	}
}

void Comedi2py::runPython(float **buffer) {
	if (recPushButton->checkState()==Qt::Checked) 
	{
		int ndev = comediAsync->getNcomediDevices();
		int n = comediAsync->getNchannels();
		for (int d = 0; d < ndev; d++) {
			pList[d] = PyList_New(n);
			for (int i = 0; i < n; ++i) {
				pValue = PyFloat_FromDouble(buffer[d][i]);
				if (!pValue) {
					Py_DECREF(pArgs);
					Py_DECREF(pModule);
					fprintf(stderr, "Cannot convert argument\n");
					exit(1);
				}
				PyList_SetItem(pList[d], i, pValue);
			}
			PyTuple_SetItem(pArgs, d, pList[d]);
		}
	pValue = PyObject_CallObject(pDataFunc, pArgs);
	PyErr_Print();
	}
}

void Comedi2py::closePython() {
	pStopArgs = PyTuple_New(0);
	PyObject_CallObject(pStopFunc, pStopArgs);
	if (pStartFunc) Py_XDECREF(pStartFunc);
	if (pDataFunc) Py_XDECREF(pDataFunc);
	if (pStopFunc) Py_XDECREF(pStopFunc);
        if (pModule) Py_DECREF(pModule);
}

// options
#define OPTIONS_GETOPT "r:d:c:hs:"


///////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv )
{
	int c;
	int num_of_channels = 16;
	int num_of_devices = 1;
	const char *filename = NULL;
	int daq_sampling_rate = 1000;
	int py_sampling_rate = 10;

	QApplication a( argc, argv );		// create application object

	// we are just interested in the module name
	// skipping options and arriving at the module name
	while (-1 != (c = getopt(argc, argv,OPTIONS_GETOPT)));

	if (optind < argc) {
		filename = argv[optind];
	} else {
		fprintf(stderr,"We need at least the python module to run.\n");
		exit(1);
	}
	// just to be sure we really have a module name
	assert(filename!=NULL);

	QSettings settings(QSettings::IniFormat, 
			   QSettings::UserScope,
			   "USB-DUX",
			   "comedi2py");

	settings.beginGroup(filename);
	num_of_channels = settings.value("num_of_channels",16).toInt();
	num_of_devices = settings.value("num_of_devices",1).toInt();
	daq_sampling_rate = settings.value("daq_sampling_rate",1000).toInt();
	py_sampling_rate = settings.value("py_sampling_rate",10).toInt();
	settings.endGroup();
	
	// rewinding to get the options
	optind = 1;
	while (-1 != (c = getopt(argc, argv,OPTIONS_GETOPT))) {
		switch (c) {
		case 'c':
			num_of_channels = strtoul(optarg,NULL,0);
			break;
		case 'd':
			num_of_devices = atoi(optarg);
			break;
		case 'r':
			daq_sampling_rate = atoi(optarg);
			break;
		case 's':
			py_sampling_rate = atoi(optarg);
			break;
		case 'h':
		default:
		printf("%s usage:\n"
                       "   -c <number of channels>\n"
		       "   -d <max number of comedi devices>\n"
                       "   -r <sampling rate of the DAQ card> \n"
		       "   -s <sampling rate of the python callback \n"
		       "<python module>\n",argv[0]);
		exit(1);
		}
	}

	settings.beginGroup(filename);
	settings.setValue("num_of_channels",num_of_channels);
	settings.setValue("num_of_devices",num_of_devices);
	settings.setValue("daq_sampling_rate",daq_sampling_rate);
	settings.setValue("py_sampling_rate",py_sampling_rate);
	settings.endGroup();

	Comedi2py comedi2py(0,
			    num_of_channels,
			    num_of_devices,
			    daq_sampling_rate,
			    py_sampling_rate,
			    filename
		);

	comedi2py.show();			// show widget
	return a.exec();			// run event loop
}
