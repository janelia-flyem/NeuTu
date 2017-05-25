# This variable is used to detect the presence of
# external libs (so their build can be skipped)
export CONDA_ENV=${PREFIX}

cd neurolabi

# Build libneutube.a
./buildlib.sh debug

# Build the swig bindings
cd python/module
make debug

# Install to the environment prefix
cp _neutube.so neutube.py ${PREFIX}/lib/python2.7/site-packages/

NEUTUBE_SO=${PREFIX}/lib/python2.7/site-packages/_neutube.so

# Adjust RPATH and lib references
if [ $(uname) == 'Darwin' ]; then
    install_name_tool -add_rpath ../../ ${NEUTUBE_SO}
    install_name_tool -change libpng16.16.dylib @rpath/libpng16.16.dylib _neutube.so
    install_name_tool -change libhdf5.10.dylib @rpath/libhdf5.10.dylib _neutube.so
else
    patchelf --set-rpath ../../ ${NEUTUBE_SO}
fi
