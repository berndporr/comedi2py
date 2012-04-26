/**
 * comediasync.h
 * (c) 2012 Bernd Porr, no warranty, GNU-public license
 * this encapsulates all the comedi async acquisition
 * It samples at the requested sampling rate and then
 * averages by the decimation amount. For example if
 * the sampling rate is 1kHz and the decimation is
 * 100 then the rate python is called is 10Hz.
 * The callback function is "runPython(float** allChannels)"
 * which in turn calls the python function "comedidata(allChannels)".
 **/
class ComediAsync;
#ifndef COMEDIASYNC_H
#define COMEDIASYNC_H

#include <QWidget>
#include <QTextEdit>

#include <comedilib.h>
#include <fcntl.h>

#include "comedi2py.h"

class ComediAsync : public QTextEdit
{
    Q_OBJECT
public:
/**
 * Constructor:
 **/
    ComediAsync( Comedi2py* comedi2pyTmp,
		 int channels = 0,
		 int maxComediDevices = 1,
		 int req_sampling_rate = 1000
	    );
/**
 * Destructor: close the file if necessary
 **/
    ~ComediAsync();


/**
 * Is called by the timer of the window. This causes the drawing and saving
 * of all not yet displayed/saved data.
 **/
    void	timerEvent( QTimerEvent * );

private slots:

private:
    /**
     * file descriptor for /dev/comedi0
     **/
    comedi_t **dev;

 private:
    unsigned int** chanlist;
    
 private:
    int subdevice;

 private:
    comedi_cmd** cmd;

    /**
     * elapsed msec
     **/
    long int nsamples;

private:
    /**
     * pointer to the parent widget which contains all the controls
     **/
    Comedi2py* comedi2py;
  
private:
    /**
     * buffer which adds up the data for averaging
     **/
    float**   adAvgBuffer;

private:
    /**
     * init value for the averaging counter
     **/
    int         tb_init;

private:
    /**
     * counter for the tb. If zero the average is
     * taken from adAvgBuffer and saved into actualAD.
     **/
    int         tb_counter;

private:
    /**
     * the number of channels actually used per comedi device
     **/
    int channels_in_use;

public:
    int getNchannels() {return channels_in_use;};


private:
    /**
     * the max value of the A/D converter
     **/
    lsampl_t* maxdata;

    /**
     * physical range
     **/
    comedi_range** crange;

public:
    float getMinRange(int nDev) {
	    return crange[nDev]->min;
    }

public:
    float getMaxRange(int nDev) {
	    return crange[nDev]->max;
    }

/**
 * Number of detected comedi devices
 **/
    int nComediDevices;

/**
 * The actual sampling rate
 **/
    int sampling_rate;

public:
/**
 * Start the DAQ board(s)
 **/
    void startDAQ();

public:
/**
 * Gets the number of comedi devices actually used here
 **/
    int getNcomediDevices() {return nComediDevices;};

public:
/**
 * Gets the actual sampling rate the boards are running at.
 **/
    int getDAQSamplingRate() {return sampling_rate;};

private:
/**
 * checks for new data and sends it to python
 **/
    void checkForData();

public:
/**
 * sets the decimation factor (averaging over d samples)
 **/
    void setDecimation(int d);

public:
/**
 * gets the sampling rate perceived by the python callback
 **/
    float getCallbackSamplingRate() { 
	    return ((float)sampling_rate)/
		    ((float)tb_init );
    }

};


#endif
