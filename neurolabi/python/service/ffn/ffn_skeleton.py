import os
import sys
sys.path.append('./ffn')
import numpy as np
from tiffio.baseio import open_tif,save_tif
import json
import run_inference
from absl import app
from ffn.inference import storage
import baseio
from io import BytesIO
import base64

def _exit(param):
    pass

def process(unused):
    run_inference.run()


def get_current_path():
    return os.path.split(os.path.realpath(__file__))[0]
    

def run():
    cur_dir=os.path.split(os.path.realpath(__file__))[0]
    policy='PolicyUserDefined'
    sys.argv=['run_inference.py']
    param = r'''--inference_request=image{hdf5:"'''+os.path.join(cur_dir,'data.tif')+r''':local"}'''
    
    img = open_tif('data.tif')
    size = img.shape
    
    param+= ' image_mean: '+str(np.mean(img))+ ' image_stddev:'+str(np.std(img))
    param+= r''' checkpoint_interval:1800 seed_policy: "'''+policy+r'''"'''
    param+= r''' seed_policy_args : "{\"seed_file\": '''
    
    param+= r'''\"seed.txt\"}" '''
    
    param+= r''' model_checkpoint_path: "'''+os.path.join(cur_dir,r'models/fib25/model.ckpt-27465036')+r'''"'''
    param+= r''' model_name: "convstack_3d.ConvStack3DFFNModel" '''
    param+= r''' model_args: "{\"depth\": 12, \"fov_size\": [33,33, 33], \"deltas\": [8, 8, 8]}" '''
    param+= r'''segmentation_output_dir: "'''+os.path.join(cur_dir,'results/fib25/training2')+r'''" '''
    param+= r'''inference_options { init_activation: 0.95 pad_value: 0.05 move_threshold: 0.9 segment_threshold: 0.6 min_segment_size: 100 }'''

    sys.argv.append(param)
    sys.argv.append('--bounding_box')
    sz,sy,sx=str(size[0]),str(size[1]),str(size[2])
    sys.argv.append(r'start {x:0'+ ' y:0'+' z:0'+'}'+'size{x:'+sx+' y:'+sy +'  z:'+sz+'}')
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/counters.txt'))
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/seg*'))
    
    sys.exit=_exit
    app.run(process)
    
    result=np.load(os.path.join(cur_dir,'results/fib25/training2/seg-0_0_0.npz'))['segmentation']
    
    save_tif(result,os.path.join(cur_dir,'result.tif'))
    
    
if __name__=='__main__':
    os.chdir(get_current_path())
    run()
