def comedistart(a,b,c):
    global accumulator
    accumulator = 0
    print "Sampling rate is:",a
    print "Minimum value:",b
    print "Maximum value:",c

def comedidata(a):
    global accumulator
    accumulator = accumulator + a[0]
    print accumulator,a

def comedistop():
    print "That's it"

