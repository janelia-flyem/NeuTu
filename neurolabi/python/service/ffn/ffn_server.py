import os
import sys
from bottle import run, template, get, post, request, static_file
import json
import run_inference.py
    
@post('/segmentation/ffn')
def surfrecon_post():
    stack=request.files.get('stack')
    stack.save('./stack.tif',overwrite=True)
    param=r'--inference_request="image{hdf5:"/home/deli/ftp/ffn-c/stack.tif:local"} image_mean:130 image_stddev:30 checkpoint_interval:1800 seed_policy: "PolicyPeaks" model_checkpoint_path: "models/fib25/model.ckpt-27465036" model_name: "convstack_3d.ConvStack3DFFNModel" model_args: "{\"depth\": 12, \"fov_size\": [33, 33, 33], \"deltas\": [8, 8, 8]}" segmentation_output_dir: "results/fib25/training2" inference_options {" init_activation: 0.95 pad_value: 0.05 move_threshold: 0.9 min_boundary_dist { x: 1 y: 1 z: 1} segment_threshold: 0.6 min_segment_size: 500 } --bounding_box "start { x:0 y:0 z:0 } size { x:250 y:250 z:50 }"
    os.popen(r'python run_inference.py '+param)'
    
    return static_file(r"results/fib25/training2/seg-0_0_0.npz","./")
    '''f=request.forms.get('data')
    cmd=neutu+" --command --general "
    config={"command":"surfrecon"}
    config["npoints"]=points.count('(')
    cmd+=r"'"+json.dumps(config)+r"'"
    print(cmd)
    p=Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, stdin=PIPE)
    for a in points.split(')')[:-1]:
        a=a[1:]
        x, y, z=a.split(',')
        p.stdin.write('{0} {1} {2} \n'.format(x, y, z))
    p.wait()
    output={}
    for index, line in enumerate(p.stdout.readlines()):
        if line.startswith('result id:'):
            output['result url']='/surfrecon/result/'+line[line.find(':')+1:].strip()
        elif line.startswith('md5:'):
            output['md5']=line[line.find(':')+1:].rstrip()
    return json.dumps(output)'''
    
if __name__=='__main__':
    run(host='127.0.0.1', port=8080)
