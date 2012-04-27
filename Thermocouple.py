import sys
from PyQt4 import Qt
import PyQt4.Qwt5 as Qwt
import PyQt4.Qwt5.anynumpy as np

# Thermocouple application: channel 0 has the thermocouple
# connected to and channel 1 receives the temperture of the cold junction
# check out http://www.linux-usb-daq.co.uk/howto2/thermocouple/

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
        self.setAxisTitle(Qwt.QwtPlot.yLeft, 'temperature/C -->')

        # insert a few curves
        self.cData = Qwt.QwtPlotCurve('y = temperature')
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


# calculate the temperature
def calcTemperature(voltageTheormocouple,temperatureLM35):
# gain of the instrumentation amplifier INA126
    GAIN_INSTR_AMP=((5+80/0.456))
# zero offset of the instrumentation amplifier
    ZERO_INSTR_AMP=(-0.05365)
    return (((voltageTheormocouple-ZERO_INSTR_AMP)/GAIN_INSTR_AMP)/39E-6) + temperatureLM35


def makePlot(samplingrate):
    scrollplot = ScrollingPlot()
    scrollplot.initPlotwindow(0,samplingrate)
    scrollplot.resize(500, 300)
    scrollplot.show()
    return scrollplot

def makeThermo():
    thermo = DAQThermo()
    thermo.resize(100,400)
    thermo.setRange(-20,300)
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
    voltage_thermo = a[0]
    temperature_lm35 = a[1] / 10E-3
    temperature = calcTemperature(voltage_thermo,temperature_lm35)
    scrollplot.new_data(temperature);
    thermo.setValue(temperature);

# called at the end
def comedistop():
    print "\n"
