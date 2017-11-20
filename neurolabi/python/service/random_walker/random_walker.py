import sys
import os
import numpy as np
from skimage.segmentation import random_walker
from libtiff import TIFF
import time

def readTiff(file_name):
    img=TIFF.open(file_name)
    try:
        _img=img.read_image()
        for i in img.iter_images():
            _img=np.dstack((_img,i))
        return _img[:,:,1:]
    except:
        return None
    
def writeTiff(file_name,data):
    tiff=TIFF.open(file_name,mode='w')
    for i in range(data.shape[2]):
        tiff.write_image(data[:,:,i])
    tiff.close()

def rw(input_file,seed_file,output_file):
    data=readTiff(input_file).astype(np.float64)
    seed=readTiff(seed_file).astype(np.float64)
    seed[data==0]=-1
    labels=random_walker(data,seed,mode='bf',tol=1e-10)
    labels[data==0]=0
    writeTiff(output_file,labels.astype(np.int8))


if __name__=='__main__':
    if(len(sys.argv)<4):
        print('invalid arguments number')
    input_file=sys.argv[1]
    seed_file=sys.argv[2]
    output_file=sys.argv[3]
    mode=sys.argv[4] if len(sys.argv)>4 else 'cg_mg'
    rw(input_file,seed_file,output_file)
    
    
