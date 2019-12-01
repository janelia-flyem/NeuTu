import os
import sys
import numpy as np
from tiffio.baseio import open_tif,save_tif
from segnet.entry_d import inference

def get_current_path():
    return os.path.split(os.path.realpath(__file__))[0]
    

def main():
    stack = open_tif('data.tif')
    mask = open_tif('mask.tif')
    rseeds = open_tif('rseeds.tif')
    bseeds = open_tif('bseeds.tif')
    save_tif(inference(stack,mask,rseeds,bseeds),'result.tif')
    
    
if __name__ == '__main__':
    os.chdir(get_current_path())
    main()
