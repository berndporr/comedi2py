import sys
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from PyQt4.Qwt5.anynumpy import *


class SimplePlot(Qwt.QwtPlot):

    

    def __init__(self, *args):
        Qwt.QwtPlot.__init__(self, *args)

        global x,y,cSin;

        # set axis titles
        self.setAxisTitle(Qwt.QwtPlot.xBottom, 'x -->')
        self.setAxisTitle(Qwt.QwtPlot.yLeft, 'y -->')

        # insert a few curves
        cSin = Qwt.QwtPlotCurve('y = sin(x)')
        cSin.setPen(Qt.QPen(Qt.Qt.red))
        cSin.attach(self)

        # make a Numeric array for the horizontal data
        x = arange(0.0, 500, 1)
        y = sin(x*0.1)

        # initialize the data
        cSin.setData(x,y)

        # insert a horizontal marker at y = 0
        mY = Qwt.QwtPlotMarker()
        mY.setLabel(Qwt.QwtText('y = 0'))
        mY.setLabelAlignment(Qt.Qt.AlignRight | Qt.Qt.AlignTop)
        mY.setLineStyle(Qwt.QwtPlotMarker.HLine)
        mY.setYValue(0.0)
        mY.attach(self)

        # insert a vertical marker at x = 2 pi
        mX = Qwt.QwtPlotMarker()
        mX.setLabel(Qwt.QwtText('x = 2 pi'))
        mX.setLabelAlignment(Qt.Qt.AlignRight | Qt.Qt.AlignTop)
        mX.setLineStyle(Qwt.QwtPlotMarker.VLine)
        mX.setXValue(2*pi)
        mX.attach(self)

        # replot
        self.replot()

    # __init__()

    def new_data(self,d):
        global y,x,cSin;
# shift the data
        y = [d] + y[0:-1]
        cSin.setData(x,y)
        self.replot()

# class Plot


def make():
    demo = SimplePlot()
    demo.resize(500, 300)
    demo.show()
    return demo

# make()


def comedistart(a):
    global demo
    demo = make()
    global b
    b = 0
    print a

def comedidata(a):
    global demo
    ch1 = a[0];
    demo.new_data(ch1);

def comedistop():
    print a