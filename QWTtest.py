import sys
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
from PyQt4.Qwt5.anynumpy import *

# this is taken from the QWT demos and slightly modified
# to get this scrolling plot



class DAQThermo(Qt.QWidget):

    def __init__(self, *args):
        Qt.QWidget.__init__(self, *args)

        self.thermo = Qwt.QwtThermo(self)
        self.thermo.setOrientation(Qt.Qt.Vertical,Qwt.QwtThermo.LeftScale)
        self.thermo.setFillColor(Qt.Qt.green)

        label = Qt.QLabel("Volt", self)
        label.setAlignment(Qt.Qt.AlignCenter)

        layout = Qt.QVBoxLayout(self)
        layout.setMargin(0)
        layout.addWidget(self.thermo)
        layout.addWidget(label)

        self.setFixedWidth(3*label.sizeHint().width())

    # __init__()

    def setValue(self, value):
        self.thermo.setValue(value)

    # setValue()

    def setRange(self,mi,ma):
        self.thermo.setRange(mi,ma)


class SimplePlot(Qwt.QwtPlot):

    def __init__(self, *args):
        Qwt.QwtPlot.__init__(self, *args)

        global x,y,cSin,samplingrate;

        # set axis titles
        self.setAxisTitle(Qwt.QwtPlot.xBottom, 't/sec -->')
        self.setAxisTitle(Qwt.QwtPlot.yLeft, 'u/Volt -->')

        # insert a few curves
        cSin = Qwt.QwtPlotCurve('y = u/Volt')
        cSin.setPen(Qt.QPen(Qt.Qt.red))
        cSin.attach(self)

        # make a Numeric array for the horizontal data
        x = arange(0.0, 500, 1)
        x = x / samplingrate;
        # sneaky way of creating an array of just zeroes
        y = x * 0

        # initialize the data
        cSin.setData(x,y)

        # insert a horizontal marker at y = 0
        mY = Qwt.QwtPlotMarker()
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


def makePlot():
    demo = SimplePlot()
    demo.resize(500, 300)
    demo.show()
    return demo

def makeThermo():
    thermo = DAQThermo()
    thermo.resize(100,400)
    thermo.show()
    return thermo


#########################################################
# functions called by comedi2py

# called once with the samplingrate in Hz
def comedistart(a,minValue,maxValue):
    global demo
    global thermo
    global samplingrate
    samplingrate = a
    demo = makePlot()
    thermo = makeThermo()
    thermo.setRange(minValue,maxValue)

# called every sample
def comedidata(a):
    global demo
    demo.new_data(a[0]);
    thermo.setValue(a[1]);

# called at the end
def comedistop():
    print "That's it!"

