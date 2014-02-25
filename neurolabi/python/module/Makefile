SOURCE = neutube.i stack_io.i stack_attribute.i

all: _neutube.so

_neutube.so: neutube_wrap.o
	g++ -bundle -m64 neutube_wrap.o -L../../c/lib -lneurolabi -o $@ -L../../lib/fftw3/lib -lfftw3 -lfftw3f -L../../lib/png/lib -lpng -L../../lib/xml/lib -lxml2 -L../../lib/jansson/lib -ljansson -L../../lib/hdf5/lib -lhdf5 -lm -lz -L/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config -ldl -framework CoreFoundation -lpython2.7 -L../../cpp/lib/build -lneutube

neutube_wrap.o: neutube_wrap.cxx
	g++ -c $< -I../../c -I../../lib/genelib/src -I/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 -I../../c/include -I../../gui -m64 -fPIC

neutube_wrap.cxx: $(SOURCE)
	swig -python -c++ -I../../gui -I../../c neutube.i

clean:
	rm -f *.o
	rm -f *.so
	rm -f neutube_wrap.cxx
	rm -f neutube.py
