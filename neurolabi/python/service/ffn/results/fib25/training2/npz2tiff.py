import sys
from libtiff import TIFF
import numpy as np
from tensorflow import gfile

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
    tiff.close()
    

if __name__=='__main__':
    data = np.load(sys.argv[1])
    writeTiff(sys.argv[2],data['segmentation'].astype(np.uint8)*50)
    '''
    if 'segmentation' in data:
        seg = data['segmentation']
    else:
        raise ValueError('FFN NPZ file %s does not contain valid segmentation.' %
                           target_path)
    output = seg.astype(np.uint64)
    writeTiff(sys.argv[2],output)'''
