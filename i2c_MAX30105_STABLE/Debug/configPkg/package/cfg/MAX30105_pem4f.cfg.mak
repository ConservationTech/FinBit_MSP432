# invoke SourceDir generated makefile for MAX30105.pem4f
MAX30105.pem4f: .libraries,MAX30105.pem4f
.libraries,MAX30105.pem4f: package/cfg/MAX30105_pem4f.xdl
	$(MAKE) -f /Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE/src/makefile.libs

clean::
	$(MAKE) -f /Users/dave/Documents/GitHub/FinBit_MSP432/i2c_MAX30105_STABLE/src/makefile.libs clean

