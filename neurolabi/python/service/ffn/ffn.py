import sys
import numpy as np
import requests
import TiffFile
import json

SERVER_URL=r'http://10.14.111.154:4321/'

def run(fname,output_name,seed_file):
    tiff=TiffFile.imread(fname)
    np.savez_compressed('data.npz',tiff)
    data={'data.npz':open('data.npz','rb'),'seed.txt':open(seed_file,'r')}
    requests.post(SERVER_URL+'upload/neutu_client',files=data)
    #config={'in_file_name':'neutu_client/data.npz','seed_file_name':'neutu_client/seed.txt'}
    config={'in_file_name':'neutu_client/data.npz'}
    r=requests.post(SERVER_URL+'light_seg',data=json.dumps(config),headers={'content-type':'application/json'})
    with open('./tmp.npz','wb') as f:
        f.write(r.content)
    data = np.load('./tmp.npz')
    TiffFile.imsave(output_name,data['segmentation'].astype(np.uint8))

if __name__=='__main__':
    fname=sys.argv[1]
    output_name=sys.argv[2]
    seed_file=sys.argv[3]
    run(fname,output_name,seed_file)
