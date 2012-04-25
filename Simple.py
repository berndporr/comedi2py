def comedistart(a):
    global b
    b = 0
    print "Sampling rate is:"
    print a

def comedidata(a):
    global b
    b = b + a[0]
    print b,a

def comedistop():
    print a
