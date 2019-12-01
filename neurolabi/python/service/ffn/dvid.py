import sys
import os
import requests
import json
import gzip
import numpy as np
from baseio import save_tif


def dvid_get_image(addr,uuid,name,offset,size):
    if not addr.startswith('http'):
        addr='http://'+addr
    shape=(size[2],size[1],size[0])
    url=addr+'/api/node/'+str(uuid)+'/'+name+'/'+'raw/0_1_2/'+'_'.join(map(str,size))+'/'+'_'.join(map(str,offset))
    data=requests.get(url)
    if data.status_code==200:
        return True,np.fromstring(data.content,dtype=np.uint8).reshape(shape)
    return False,None
        
def dvid_put_label(addr,uuid,name,offset,data,mutate=False):
    if not addr.startswith('http'):
        addr='http://'+addr
    size=data.shape[2],data.shape[1],data.shape[0]
    headers = {'Content-Type': 'application/octet-stream'}
    url=addr+'/api/node/'+str(uuid)+'/'+name+'/'+'raw/0_1_2/'+'_'.join(map(str,size))+'/'+'_'.join(map(str,offset))
    url+='?compression=gzip'
    if mutate:
        url+='&mutate=true'
    return requests.post(url,data=gzip.compress(data.astype(np.uint64).tostring())).status_code==200
    
def dvid_get_label(addr,uuid,name,offset,size):
    if not addr.startswith('http'):
        addr='http://'+addr
    url=addr+'/api/node/'+str(uuid)+'/'+name+'/'+'raw/0_1_2/'+'_'.join(map(str,size))+'/'+'_'.join(map(str,offset))
    url+='?compression=gzip'
    rv=requests.get(url)
    if rv.status_code==200:
        return True,np.fromstring(gzip.decompress(rv.content),dtype=np.uint64).reshape((size[2],size[1],size[0])).astype(np.uint8)
    return False,None
     
if __name__=='__main__':
    #print(requests.post('http://10.14.111.154:8000/api/node/ffe/test/sync?replace=true',json= { "sync": "" }))
    #_,block=dvid_get_image('10.14.111.154:8000','ffe','grayscale',[2500,2000,3496],[50,50,50])
    #_,label=dvid_get_label('10.14.111.154:8000','ffe','labels',[2500,2000,3496],[50,50,50])
    #save_tif(block,'1.tif')
    #save_tif(label,'2.tif')
    #lb=np.ones(shape=(32,64,128),dtype=np.uint64)*20
    #print(dvid_put_block('10.14.111.154:8000','ffe','test',(3200,1824,1024),lb,True))
    #save_tif(block,'test.tif')
    pass
