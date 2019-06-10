import sys
import numpy as np
import requests
import TiffFile
import json

SERVER_URL=r'http://10.14.111.154:4321/'

def run(x,y,z,sx,sy,sz,seedx,seedy,seedz,result_file_name):
    config={'offset':(z,y,x),'size':(sz,sy,sx),'seed':(seedz,seedy,seedx)}
    r=requests.post(SERVER_URL+'skeleton',data=json.dumps(config),headers={'content-type':'application/json'})
    with open('./tmp.npz','wb') as f:
        f.write(r.content)
    data = np.load('./tmp.npz')
    TiffFile.imsave(result_file_name,data['segmentation'].astype(np.uint8))
    
if __name__=='__main__':
    x,y,z=sys.argv[1],sys.argv[2],sys.argv[3]
    sx,sy,sz=sys.argv[4],sys.argv[5],sys.argv[6]
    seedx,seedy,seedz=sys.argv[7],sys.argv[8],sys.argv[9]
    output_name=sys.argv[10]
    run(x,y,z,sx,sy,sz,seedx,seedy,seedz,output_name)
