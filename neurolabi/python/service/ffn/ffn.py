import sys
import numpy as np
import requests
import npz2tiff 

SERVER_URL=r'http://10.14.111.154:4321/segmentation/ffn'

def run(fname,output_name,seed_file):
    data={'stack':open(fname,'rb'),'seed':open(seed_file,'r')}
    r=requests.post(SERVER_URL,files=data)
    with open('./tmp.npz','wb') as f:
        f.write(r.content)
    data = np.load('./tmp.npz')
    npz2tiff.writeTiff(output_name,data['segmentation'].astype(np.uint8))
    
if __name__=='__main__':
    fname=sys.argv[1]
    output_name=sys.argv[2]
    seed_file=sys.argv[3]
    run(fname,output_name,seed_file)
