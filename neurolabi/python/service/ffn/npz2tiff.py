import sys
from libtiff import TIFF
import numpy as np
from tensorflow import gfile
from skimage import morphology

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
    for i in range(data.shape[0]):
        tiff.write_image(data[i,:,:])
        #tiff.write_image(data[i,:,:].transpose())
    tiff.close()
    
def removeHoles(img):
    rv=np.copy(img)
    d,r,c=img.shape
    for z in range(1,d-1):
        for y in range(1,r-1):
            for x in range(1,c-1):
                if rv[z,y,x]!=0:
                    if rv[z-1,y,x]==0:
                        rv[z-1,y,x]=rv[z,y,x]
                    if rv[z,y-1,x]==0:
                        rv[z,y-1,x]=rv[z,y,x]
                    if rv[z,y,x-1]==0:
                        rv[z,y,x-1]=rv[z,y,x]
    return rv
    
if __name__=='__main__':
    data = np.load(sys.argv[1])
    writeTiff(sys.argv[2],removeHoles(data['segmentation'].astype(np.uint8)))
    '''
    if 'segmentation' in data:
        seg = data['segmentation']
    else:
        raise ValueError('FFN NPZ file %s does not contain valid segmentation.' %
                           target_path)
    output = seg.astype(np.uint64)
    writeTiff(sys.argv[2],output)'''
