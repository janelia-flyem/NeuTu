SOURCE = neutube.i $(shell ./parsedep)
#stack_io.i stack_attribute.i zobject3dscan.i zpointarray.i
UNAME := $(shell uname)
SHARE_FLAG = shared
ifeq ($(UNAME), Darwin)
  SHARE_FLAG = bundle
endif

all: _neutube.so

_neutube.so: neutube_wrap.o ../../cpp/lib/build/libneutube.a
	g++ -$(SHARE_FLAG) neutube_wrap.o -o $@ -L../../cpp/lib/build -lneutube -L../../c/lib -lneurolabi -L../../lib/fftw3/lib -lfftw3 -lfftw3f -L../../lib/png/lib -lpng -L../../lib/xml/lib -lxml2 -L../../lib/jansson/lib -ljansson -L../../lib/hdf5/lib -lhdf5 -lm -lz -L../../cpp/lib/build -lneutube `python-config --ldflags | sed 's/-u [^ ]* [^ ]*//'`

neutube_wrap.o: neutube_wrap.cxx
	g++ -c $< -I../../c -I../../lib/genelib/src -I../../c/include -I../../gui -I../../lib/jansson/include -fPIC `python-config --cflags`

neutube_wrap.cxx: $(SOURCE)
	swig -python -c++ -I../../gui -I../../c neutube.i

clean:
	rm -f *.o
	rm -f *.so
	rm -f neutube_wrap.cxx
	rm -f neutube.py
