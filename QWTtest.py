import sys
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
import PyQt4.Qwt5.anynumpy as np

# simple application which plots the voltage on channel 0 in a plot window
# and channel 1 in a thermometer (LM35 connected)

class DAQThermo(Qt.QWidget):

    def __init__(self, *args):
        Qt.QWidget.__init__(self, *args)

        self.thermo = Qwt.QwtThermo(self)
        self.thermo.setOrientation(Qt.Qt.Vertical,Qwt.QwtThermo.LeftScale)
        self.thermo.setFillColor(Qt.Qt.green)

        label = Qt.QLabel("Temperature", self)
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


# this is taken from the QWT demos and slightly modified
# to get this scrolling plot
class ScrollingPlot(Qwt.QwtPlot):

    def __init__(self, *args):
        Qwt.QwtPlot.__init__(self, *args)

    def initPlotwindow(self,y,samplingrate):
        self.samplingrate = samplingrate;

        # set axis titles
        self.setAxisTitle(Qwt.QwtPlot.xBottom, 't/sec -->')
        self.setAxisTitle(Qwt.QwtPlot.yLeft, 'U/Volt -->')

        # insert a few curves
        self.cData = Qwt.QwtPlotCurve('y = voltage')
        self.cData.setPen(Qt.QPen(Qt.Qt.red))
        self.cData.attach(self)

        # make a Numeric array for the horizontal data
        self.x = np.arange(0.0, 500, 1)
        self.x = self.x / samplingrate;
        # sneaky way of creating an array of just zeroes
        self.y = self.x * 0 + y

        # initialize the data
        self.cData.setData(self.x,self.y)

        # insert a horizontal marker at y = 0
        mY = Qwt.QwtPlotMarker()
        mY.setLineStyle(Qwt.QwtPlotMarker.HLine)
        mY.setYValue(0.0)
        mY.attach(self)

        # replot
        self.replot()

    # __init__()

    def new_data(self,d):
        # shift the data to create a scrolling dataplotx
        self.y = np.concatenate( ([d], self.y[0:-1] ) )
        self.cData.setData(self.x,self.y)
        self.replot()

# class Plot


def makePlot(samplingrate):
    scrollplot = ScrollingPlot()
    scrollplot.initPlotwindow(0,samplingrate)
    scrollplot.resize(500, 300)
    scrollplot.show()
    return scrollplot

def makeThermo():
    thermo = DAQThermo()
    thermo.resize(100,400)
    thermo.setRange(0,50)
    thermo.show()
    return thermo


#########################################################
# functions called by comedi2py

# called once with the samplingrate in Hz
def comedistart(samplingrate,minValue,maxValue):
    global scrollplot
    global thermo
    scrollplot = makePlot(samplingrate)
    thermo = makeThermo()

# called every sample
def comedidata(a):
    global scrollplot
    global thermo
    voltage = a[0]
    temperature = a[1] / 10E-3
    scrollplot.new_data(voltage);
    thermo.setValue(temperature);

# called at the end
def comedistop():
    print "\n"
