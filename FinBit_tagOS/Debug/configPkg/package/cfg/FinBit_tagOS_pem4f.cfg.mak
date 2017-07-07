# invoke SourceDir generated makefile for FinBit_tagOS.pem4f
FinBit_tagOS.pem4f: .libraries,FinBit_tagOS.pem4f
.libraries,FinBit_tagOS.pem4f: package/cfg/FinBit_tagOS_pem4f.xdl
	$(MAKE) -f /Users/dave/workspace_v7/FinBit_tagOS/src/makefile.libs

clean::
	$(MAKE) -f /Users/dave/workspace_v7/FinBit_tagOS/src/makefile.libs clean

