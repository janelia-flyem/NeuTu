'''
Created on Jul 12, 2013

@author: zhaot
'''

import h5py
import numpy as np
import matplotlib.pyplot as plt
import pylab

def plotImage(arr):
    fig = plt.figure(figsize=(5, 5), dpi = 80, facecolor = 'w', edgecolor = 'w', frameon = True);
    imAx = plt.imshow(arr, origin = 'lower', interpolation = 'nearest');
    fig.colorbar(imAx, pad = 0.01, fraction = 0.1, shrink = 1.00, aspect = 20);
    pylab.show();
    
def print_hdf5_file_structure(file_name):
    file = h5py.File(file_name, 'r');
    item = file;
    print_hdf5_item_structure(item);
    file.close();
    
def print_hdf5_item_structure(g, offset = '  '):
    if (isinstance(g, h5py.File)):
        print g.file, '(File)', g.name
    elif isinstance(g, h5py.Dataset):
        print '(Dataset)', g.name, '  len=', g.shape;
    elif isinstance(g, h5py.Group):
        print '(Group)', g.name;
    else:
        print 'Working: unknow item', g.name;
        
    if isinstance(g, h5py.File) or isinstance(g, h5py.Group):
        for key, val in dict(g).iteritems():
            subg = val;
            print offset, key;
            print_hdf5_item_structure(subg, offset + '  ');
            
if __name__ == "__main__":
    print('test');