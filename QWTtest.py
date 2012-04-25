import sys
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from PyQt4.Qwt5.anynumpy import *

# this is taken from the QWT demos and slightly modified
# to get this scrolling plot

class SimplePlot(Qwt.QwtPlot):

    def __init__(self, *args):
        Qwt.QwtPlot.__init__(self, *args)

        global x,y,cSin,samplingrate;

        # set axis titles
        self.setAxisTitle(Qwt.QwtPlot.xBottom, 't/sec -->')
        self.setAxisTitle(Qwt.QwtPlot.yLeft, 'u/Volt -->')

        # insert a few curves
        cSin = Qwt.QwtPlotCurve('y = sin(x)')
        cSin.setPen(Qt.QPen(Qt.Qt.red))
        cSin.attach(self)

        # make a Numeric array for the horizontal data
        x = arange(0.0, 500, 1)
        x = x / samplingrate;
        # some nice random data to begin with
        y = sin(x*1000)

        # initialize the data
        cSin.setData(x,y)

        # insert a horizontal marker at y = 0
        mY = Qwt.QwtPlotMarker()
        mY.setLabel(Qwt.QwtText('y = 0'))
        mY.setLabelAlignment(Qt.Qt.AlignRight | Qt.Qt.AlignTop)
        mY.setLineStyle(Qwt.QwtPlotMarker.HLine)
        mY.setYValue(0.0)
        mY.attach(self)

        # replot
        self.replot()

    # __init__()

    def new_data(self,d):
        global y,x,cSin;
# shift the dat
        ym = y[0:-1]
        y = concatenate( ([d], ym ) )
        cSin.setData(x,y)
        self.replot()

# class Plot


def make():
    demo = SimplePlot()
    demo.resize(500, 300)
    demo.show()
    return demo



#########################################################
# functions called by comedi2py

# called once with the samplingrate in Hz
def comedistart(a):
    global demo
    global samplingrate
    samplingrate = a
    demo = make()

# called every sample
def comedidata(a):
    global demo
    demo.new_data(a[0]);

# not used so far
def comedistop():
    print 0
