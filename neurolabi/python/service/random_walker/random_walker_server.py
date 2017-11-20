import os
import sys
from bottle import run,template,get,post,request,static_file
from subprocess import Popen,PIPE
import json
from main import rw
from time import time
import random

@post('/random_walker/data/input')
def upload_input():
    f=request.files.get('data')
    f.save(r'./data/input.tif',overwrite=True)
    print(1)
    return json.dumps({'status':'done'})

@post('/random_walker/data/seed')
def upload_seed():
    f=request.files.get('data')
    f.save(r'./data/seed.tif',overwrite=True)
    print(2)
    return json.dumps({'status':'done'})

@get('/random_walker/result')
def get_result():
    return static_file('result.tif',root='./result')
    return json.dumps({'status':'done'})

@get('/random_walker')
def random_walker_get():
    html='''
    <p>random walker segmentation service</p>
    '''
    return html
    
@post('/random_walker/start')
def random_walker_post():
    output_file=str(time())+str(random.random())+r'.tif'
    rw(r'./data/input.tif',r'./data/seed.tif',r'./result/result.tif')
    return json.dumps({'status':'done'})
    
if __name__=='__main__':
    run(host='192.168.123.88',port=9090)
