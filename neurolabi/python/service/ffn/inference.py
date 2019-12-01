import os
import sys
sys.path.append('.')
import numpy as np
import json
import run_inference
from absl import app
from ffn.inference import storage
import dvid
import baseio
from io import BytesIO
import base64

DVID_ADDR='10.14.111.154:8000'

def exit(param):
    pass

def process(unused):
    run_inference.run()


def em_inference(offset,size,start_id,seed=None):
    status,img=dvid.dvid_get_image(DVID_ADDR,'27','grayscale',(offset[2],offset[1],offset[0]),(size[2],size[1],size[0]))
    if not status:
        return False,start_id
    cur_dir=os.path.split(os.path.realpath(__file__))[0]
    baseio.save_tif(img,os.path.join(cur_dir,'data.tif'))
    _exit=sys.exit
    sys.exit=exit
    
    cur_dir=os.path.split(os.path.realpath(__file__))[0]
    policy='PolicyPeaks'
    if seed:
        policy='PolicyUserDefined'
    sys.argv=['run_inference.py']
    param = r'''--inference_request=image{hdf5:"'''+os.path.join(cur_dir,'data.tif')+r''':local"}'''
    param+= ' image_mean: '+str(np.mean(img))+ ' image_stddev:'+str(np.std(img))
    param+= r''' checkpoint_interval:1800 seed_policy: "'''+policy+r'''"'''
    if seed:
        param+= r''' seed_policy_args : "{\"seed_pos\": '''
        param+= r'''\"'''+str(seed[0])+' '+str(seed[1])+' '+str(seed[2])+' ' +r'''\"}" '''
    param+= r''' model_checkpoint_path: "'''+os.path.join(cur_dir,r'models/fib25/model.ckpt-27465036')+r'''"'''
    param+= r''' model_name: "convstack_3d.ConvStack3DFFNModel" '''
    param+= r''' model_args: "{\"depth\": 12, \"fov_size\": [33,33, 33], \"deltas\": [8, 8, 8]}" '''
    param+= r'''segmentation_output_dir: "'''+os.path.join(cur_dir,'results/fib25/training2')+r'''" '''
    param+= r'''inference_options { init_activation: 0.95 pad_value: 0.05 move_threshold: 0.9 segment_threshold: 0.6 min_segment_size: 100 }'''
    #min_boundary_dist { x: 1 y: 1 z: 1}
    sys.argv.append(param)
    sys.argv.append('--bounding_box')
    sz,sy,sx=str(size[0]),str(size[1]),str(size[2])
    sys.argv.append(r'start {x:0'+ ' y:0'+' z:0'+'}'+'size{x:'+sx+' y:'+sy +'  z:'+sz+'}')
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/counters.txt'))
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/seg*'))
    app.run(process)
    sys.exit=_exit
    result=np.load(os.path.join(cur_dir,'results/fib25/training2/seg-0_0_0.npz'))['segmentation']
    result=result.astype(np.uint64)
    result+=start_id
    next_id=result.max()+1
    if dvid.dvid_put_label(DVID_ADDR,'27','ffn_segmentation',(offset[2],offset[1],offset[0]),result,False):
        return True,next_id
    if dvid.dvid_put_label(DVID_ADDR,'27','ffn_segmentation',(offset[2],offset[1],offset[0]),result,True):
        return True,next_id
    return False,start_id

def light_inference(config):
    _exit=sys.exit
    sys.exit=exit

    cur_dir=os.path.split(os.path.realpath(__file__))[0]
    
    img_buffer=BytesIO(base64.b64decode(config['img']))
    tiff=np.load(img_buffer)['arr_0']
    baseio.save_tif(tiff,'data.tif')
    sys.argv=['run_inference.py']
    param = r'''--inference_request=image{hdf5:"data.tif:local"}'''
    param+= ' image_mean: '+str(np.mean(tiff))+ ' image_stddev:'+str(np.std(tiff))
    param+= r''' checkpoint_interval:1800 seed_policy: "PolicyUserDefined"'''
    param+= r''' seed_policy_args : "{\"seeds\": '''
    param+= r''' \"'''+str(config['seeds']) +r'''\"}" '''
    param+= r''' model_checkpoint_path: "'''+config['model_path']+r'''"'''
    param+= r''' model_name: "convstack_3d.ConvStack3DFFNModel" '''
    param+= r''' model_args: "{\"depth\": 12, \"fov_size\": [33,33, 33], \"deltas\": [8, 8, 8]}" '''
    param+= r'''segmentation_output_dir: "'''+os.path.join(cur_dir,'results/fib25/training2')+r'''" '''
    param+= r'''inference_options { init_activation: 0.95 pad_value: 0.05 move_threshold: '''+str(config['move_threshold'])+''' min_boundary_dist { x: 1 y: 1 z: 1} segment_threshold: '''+str(config['segmentation_threshold'])+''' min_segment_size: 100 }'''
    
    sys.argv.append(param)
    sys.argv.append('--bounding_box')
    sz,sy,sx=str(tiff.shape[0]),str(tiff.shape[1]),str(tiff.shape[2])
    offset=(0,0,0)
    sys.argv.append(r'start {x:'+str(offset[2])+ ' y:'+str(offset[1])+' z:'+str(offset[0])+'}'+'size{x:'+sx+' y:'+sy +'  z:'+sz+'}')
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/counters.txt'))
    os.system(r'rm '+os.path.join(cur_dir,'results/fib25/training2/seg*'))
    app.run(process)
    sys.exit=_exit
    return os.path.join(cur_dir,'results/fib25/training2/seg-'+str(offset[2])+'_'+str(offset[1])+'_'+str(offset[0])+'.npz')

