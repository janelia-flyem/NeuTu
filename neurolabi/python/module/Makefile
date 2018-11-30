SOURCE = neutube.i $(shell ./parsedep)
#stack_io.i stack_attribute.i zobject3dscan.i zpointarray.i
UNAME := $(shell uname)
SHARE_FLAG = shared

ifeq ($(CONDA_ENV),)
  $(info 'no conda env')
else
  $(info CONDA_ENV)
  PREFIX=$(CONDA_ENV)
endif

ifeq ($(PREFIX),)
  $(error 'No prefix specified. Exit')
endif

ifeq ($(UNAME), Darwin)
  SHARE_FLAG = bundle
  STDLIB_FLAG = -stdlib=libc++
  #SHARE_FLAG = dynamiclib
  HDF5_FLAG = $(PREFIX)/lib/libhdf5.10.dylib
  #RPATH_FLAG = -install_name @rpath/$(PREFIX)/lib/libhdf5.dylib
else
  HDF5_FLAG = -lhdf5
endif


all: _neutube.so
debug: _neutube_d.so

# On Mac, this command:
# python3-config --ldflags
# ...returns a string that includes '-Wl,-stack_size,1000000', which results in an error:
# ld: -stack_size option can only be used when linking a main executable
# So, let's get rid of it.
# (BTW, we're not the only ones with this problem: https://github.com/waf-project/waf/issues/1745#issuecomment-220554258)
python_ldflags := $(shell python3-config --ldflags \
				   | python -c "import sys; print(' '.join(filter(lambda s: not 'stack_size' in s, sys.stdin.read().split())))" \
				   | sed 's/-u [^ ]* [^ ]*//')

define compile_so
        @echo $(PREFIX)
	$(CXX) -std=c++11 $(STDLIB_FLAG) -$(SHARE_FLAG) neutube_wrap.o $(RPATH_FLAG) -o $@ -L$(PREFIX)/lib -L../../cpp/lib/build -l$(1) -L../../c/lib -l$(2) -lfftw3 -lfftw3f -lxml2 $(HDF5_FLAG) -ljansson -lm -lz $(python_ldflags)
endef

fix: _neutube.so
	install_name_tool -change libhdf5.10.dylib '@rpath/libhdf5.dylib' _neutube.so

_neutube.so: neutube_wrap.o
	$(call compile_so,neutube,neurolabi)

_neutube_d.so: neutube_wrap.o
	$(call compile_so,neutube,neurolabi_debug)
	mv $@ _neutube.so


neutube_wrap.o: neutube_wrap.cxx
	$(CXX) -DSWIG -std=c++11 $(STDLIB_FLAG) -c $< -I../../c -I../../lib/genelib/src -I../../c/include -I../../gui -I../../lib/jansson/include -I../../lib/fftw3/include -fPIC `python3-config --cflags`

neutube_wrap.cxx: $(SOURCE)
	@which gcc
	swig -python -c++ -I../../gui -I../../c -I$(PREFIX)/include neutube.i

clean:
	rm -f *.o
	rm -f *.so
	rm -f neutube_wrap.cxx
	rm -f neutube.py
