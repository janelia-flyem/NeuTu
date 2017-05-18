SOURCE = neutube.i $(shell ./parsedep)
#stack_io.i stack_attribute.i zobject3dscan.i zpointarray.i
UNAME := $(shell uname)
SHARE_FLAG = shared
ifeq ($(UNAME), Darwin)
  SHARE_FLAG = bundle
  #SHARE_FLAG = dynamiclib
  HDF5_FLAG = $(PREFIX)/lib/libhdf5.10.dylib
  #RPATH_FLAG = -install_name @rpath/$(PREFIX)/lib/libhdf5.dylib
else
  HDF5_FLAG = -lhdf5
endif


all: _neutube.so
debug: _neutube_d.so

define compile_so
        @echo $(PREFIX)
	g++ -$(SHARE_FLAG) neutube_wrap.o $(RPATH_FLAG) -o $@ -L$(PREFIX)/lib -L../../cpp/lib/build -l$(1) -L../../c/lib -l$(2) -lfftw3 -lfftw3f -lxml2 $(HDF5_FLAG) -ljansson -lm -lz `python-config --ldflags | sed 's/-u [^ ]* [^ ]*//'`
endef

fix: _neutube.so
	install_name_tool -change libhdf5.10.dylib '@rpath/libhdf5.dylib' _neutube.so

_neutube.so: neutube_wrap.o
	$(call compile_so,neutube,neurolabi)

_neutube_d.so: neutube_wrap.o
	$(call compile_so,neutube,neurolabi_debug)
	mv $@ _neutube.so

neutube_wrap.o: neutube_wrap.cxx
	g++ -c $< -I../../c -I../../lib/genelib/src -I../../c/include -I../../gui -I../../lib/jansson/include -I../../lib/fftw3/include -fPIC `python-config --cflags`

neutube_wrap.cxx: $(SOURCE)
	@which gcc
	swig -python -c++ -I../../gui -I../../c -I$(PREFIX)/include neutube.i

clean:
	rm -f *.o
	rm -f *.so
	rm -f neutube_wrap.cxx
	rm -f neutube.py
