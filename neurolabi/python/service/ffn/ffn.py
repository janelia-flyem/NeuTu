import sys
import numpy as np
import requests
import npz2tiff 

SERVER_URL=r'http://10.0.10.2:8080/segmentation/ffn'

def run(fname,output_name):
    data={'stack':open(fname,'rb')}
    r=requests.post(SERVER_URL,files=data)
    with open('./tmp.npz','wb') as f:
        f.write(r.content)
    data = np.load('./tmp.npz')
    npz2tiff.writeTiff(output_name,data['segmentation'].astype(np.uint8))
    
if __name__=='__main__':
    fname=sys.argv[1]
    output_name=sys.argv[2]
    run(fname,output_name)
