import sys
from libtiff import TIFF
import numpy as np
import struct

def readTiff(file_name):
    img=TIFF.open(file_name)
    try:
        _img=img.read_image()
        for i in img.iter_images():
            _img=np.dstack((_img,i))
        return _img[:,:,1:].astype(np.uint8)
    except:
        return None
    
def writeTiff(file_name,data):
    tiff=TIFF.open(file_name,mode='w')
    for i in range(data.shape[2]):
        tiff.write_image(data[:,:,i])
    tiff.close()

def readPgm(file_name):
    with open(file_name,'rb') as f:
       magic=f.readline()
       w,h,d=map(int,f.readline().split())
       max_v=f.readline()
       image=np.zeros((w,h,d),dtype=np.uint8)
       for k in range(d):
           for j in range(h):
               for i in range(w):
                   image[i,j,k]=ord(f.read(1))    
       return image

def writePgm(file_name,data):
    w,h,d=data.shape
    with open(file_name,'w') as f:
       magic='P5\n'
       f.write(magic)
       size=str(w)+' '+str(h)+' '+str(d)+'\n'
       f.write(size)
       max_v=str(255)+'\n'
       f.write(max_v)
    with open(file_name,'ab+') as f:
       for k in range(d):
           for j in range(h):
               for i in range(w):
                   f.write(data[i,j,k])    


def tiff2pgm(src,dst):
    tiff=readTiff(src)
    writePgm(dst,tiff)

def pgm2tiff(src,dst):
    pgm=readPgm(src)
    writeTiff(dst,pgm)

if __name__=='__main__':
    if len(sys.argv)<4:
        print('invalid arguments number')
    src=sys.argv[1]
    dst=sys.argv[2]
    method=sys.argv[3]
    if method=="t2p":
        tiff2pgm(src,dst)
    else:
        pgm2tiff(src,dst)



