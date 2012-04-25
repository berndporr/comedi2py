/**
 * comediasync.h
 * (c) 2004-2011 Bernd Porr, no warranty, GNU-public license
 **/
#include <QTimer>
#include <QApplication>
#include <QTimerEvent>
#include <QPaintEvent>

#include<sys/ioctl.h>
#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdlib.h>

#include <comedilib.h>
#include <fcntl.h>


#include "comediasync.h"

ComediAsync::ComediAsync( Comedi2py *comedi2pyTmp, 
			  int channels, 
			  int maxComediDevs,
			  int req_sampling_rate
	)
    : QTextEdit( comedi2pyTmp ) {

	setReadOnly ( true );

	channels_in_use = channels;

	tb_init=1;
	tb_counter=tb_init;
	comedi2py=comedi2pyTmp;


	//////////////////////////////////////////////////////////////


	int range = 0;
	int aref = AREF_GROUND;
	comedi_set_global_oor_behavior(COMEDI_OOR_NUMBER);

	dev = new comedi_t*[maxComediDevs];
	for(int devNo=0;devNo<maxComediDevs;devNo++) {
		dev[devNo] = NULL;
	}
	nComediDevices = 0;
	for(int devNo=0;devNo<maxComediDevs;devNo++) {
		char filename[128];
		sprintf(filename,"/dev/comedi%d",devNo);
		dev[devNo] = comedi_open(filename);
		if(dev[devNo]){
			nComediDevices = devNo + 1;
		} else {
			break;
		}
	}

	if (nComediDevices<1) {
		fprintf(stderr,"No comedi devices detected!\n");
		exit(1);
	}

	insertPlainText ( QString().sprintf("%d comedi devices\n",
					  nComediDevices) );

	chanlist = new unsigned int*[nComediDevices];
	cmd = new comedi_cmd*[nComediDevices];
	subdevice = comedi_find_subdevice_by_type(dev[0],COMEDI_SUBD_AI,0);

	if (channels_in_use == 0) {
		channels_in_use = comedi_get_n_channels(dev[0],subdevice);
	}

	insertPlainText ( QString().sprintf("%d channels are used.\n",
					    channels_in_use) );

	for(int devNo=0;devNo<nComediDevices;devNo++) {
		chanlist[devNo] = new unsigned int[channels_in_use];
		for(int i=0;i<channels_in_use;i++){
			chanlist[devNo][i] = CR_PACK(i,range,aref);
		}
		cmd[devNo] = new comedi_cmd;
		if (!dev[devNo]) {
			fprintf(stderr,"BUG! dev[%d]=NULL\n",devNo);
			exit(1);
		}
		int r = comedi_get_cmd_generic_timed(dev[devNo],
						     subdevice,
						     cmd[devNo],
						     channels_in_use,
						     (int)(1e9/req_sampling_rate));
		if(r<0){
			printf("comedi_get_cmd_generic_timed failed\n");
			exit(-1);
		}
		/* Modify parts of the command */
		cmd[devNo]->chanlist           = chanlist[devNo];
		cmd[devNo]->chanlist_len       = channels_in_use;
		cmd[devNo]->scan_end_arg = channels_in_use;
		cmd[devNo]->stop_src=TRIG_NONE;
		cmd[devNo]->stop_arg=0;
		int ret = comedi_command_test(dev[devNo],cmd[devNo]);
		if(ret<0){
			comedi_perror("comedi_command_test");
			exit(-1);
		}
		insertPlainText ( QString().sprintf("1st command test successful!\n"));

		ret = comedi_command_test(dev[devNo],cmd[devNo]);
		if(ret<0){
			comedi_perror("comedi_command_test");
			exit(-1);
		}
		if(ret!=0){
			fprintf(stderr,"Error preparing command\n");
			exit(-1);
		}
		insertPlainText ( QString().sprintf("2nd command test successful!\n"));
	}

	// the timing is done channel by channel
	if ((cmd[0]->convert_src ==  TRIG_TIMER)&&(cmd[0]->convert_arg)) {
		sampling_rate=((1E9 / cmd[0]->convert_arg)/channels_in_use);
	}
	
	// the timing is done scan by scan (all channels at once)
	if ((cmd[0]->scan_begin_src ==  TRIG_TIMER)&&(cmd[0]->scan_begin_arg)) {
		sampling_rate=1E9 / cmd[0]->scan_begin_arg;
	}

	nsamples=0;

	maxdata = new lsampl_t[nComediDevices];
	crange = new comedi_range*[nComediDevices];

	for(int devNo=0;devNo<nComediDevices;devNo++) {
		maxdata[devNo]=comedi_get_maxdata(dev[devNo],subdevice,0);
		crange[devNo]=comedi_get_range(dev[devNo],subdevice,0,0);
		insertPlainText ( QString().sprintf("comedi%d has a raw "
						    "data range [0:%x] "
						    "which maps to [%fV:%fV]\n",
						    devNo,
						    maxdata[devNo],
						    crange[devNo]->min,
						    crange[devNo]->max) );

	}

        adAvgBuffer = new float*[nComediDevices];
        for(int devNo=0;devNo<nComediDevices;devNo++) {
                adAvgBuffer[devNo]=new float[channels_in_use];
                for(int i=0;i<channels_in_use;i++) {
                        adAvgBuffer[devNo][i]=0;
                }
        }

}


void ComediAsync::startDAQ() {
	for(int devNo=0;devNo<nComediDevices;devNo++) {
		/* start the command */
		int ret=comedi_command(dev[devNo],cmd[devNo]);
		if(ret<0){
			comedi_perror("comedi_command");
			exit(1);
		}
	}
	insertPlainText ( QString().sprintf("Async acquisition is running.\n"));
	startTimer( 50 );		// run continuous timer
}


ComediAsync::~ComediAsync() {
	for(int devNo=0;devNo<nComediDevices;devNo++) {
		comedi_close(dev[devNo]);
	}
}


void ComediAsync::checkForData() {
        int ret;
	while (1) {
		// we need data in all the comedi devices
		for(int n=0;n<nComediDevices;n++) {
			if (!comedi_get_buffer_contents(dev[n],subdevice))
				return;
		}
		
		for(int n=0;n<nComediDevices;n++) {
			int subdev_flags = comedi_get_subdevice_flags(dev[n],
								      subdevice);
			int bytes_per_sample;
			if(subdev_flags & SDF_LSAMPL) {
				bytes_per_sample = sizeof(lsampl_t);
			} else {
				bytes_per_sample = sizeof(sampl_t);
			}
			
			unsigned char buffer[bytes_per_sample*channels_in_use];
			ret = read(comedi_fileno(dev[n]),
				   buffer,
				   bytes_per_sample*channels_in_use);

			if (ret==0) {
				printf("BUG! No data in buffer.\n");
				exit(1);
			}

			if (ret<0) {
				printf("\n\nError %d during read! Exiting.\n\n",ret);
				exit(1);
			}
			
			for(int i=0;i<channels_in_use;i++) {
				float value;
				if(subdev_flags & SDF_LSAMPL) {
					value= ((float)((lsampl_t *)buffer)[i]);
				} else {
					value= ((float)((sampl_t *)buffer)[i]);
				}
				float phys=comedi_to_phys(value,
							  crange[n],
							  maxdata[n]);
				adAvgBuffer[n][i] = adAvgBuffer[n][i] + phys;
			}
		}

		nsamples++;
		tb_counter--;

		if (tb_counter<=0) {
			for(int n=0;n<nComediDevices;n++) {
				for(int i=0;i<channels_in_use;i++) {
					adAvgBuffer[n][i]=adAvgBuffer[n][i]/tb_init;
				}
			}
		
			comedi2py->runPython(adAvgBuffer);

			tb_counter=tb_init;
			for(int n=0;n<nComediDevices;n++) {
				for(int i=0;i<channels_in_use;i++) {
					adAvgBuffer[n][i]=0;
				}
			}
		}
	}
}


void ComediAsync::setTB(int us) {
	tb_init=us/(1000000/sampling_rate);
	tb_counter=tb_init;
	for(int n=0;n<nComediDevices;n++) {
		for(int i=0;i<channels_in_use;i++) {
			adAvgBuffer[n][i]=0;
		}
	}}

//
// Handles timer events for the Comediasync widget.
//

void ComediAsync::timerEvent( QTimerEvent * )
{
	checkForData();
}
