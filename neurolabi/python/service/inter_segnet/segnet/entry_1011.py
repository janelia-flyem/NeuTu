import os
import sys
from shutil import rmtree
from random import random,randint
from glob import glob
import tensorflow as tf
import numpy as np
from tiffio.baseio import open_tif,save_tif


MODEL = './segnet/data/model/model.ckpt-30000'
SHAPE = (24,24,24)
BATCH_SIZE = 50
SEG_THRESHOLD = 0.5
MOVE_THRESHOLD = 0.9
DELTA = 8
    

def define_unet(x,region_seeds,boundary_seeds):
    with tf.variable_scope('unet',reuse = False) as sc:
          
        layer = tf.concat([x,region_seeds,boundary_seeds],axis=4)
        layer1 = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer = tf.layers.conv3d(layer1,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer2 = tf.nn.relu(tf.add(layer,layer1))
        #layer = tf.nn.max_pool3d(layer_1,(1,2,2,2,1),(1,2,2,2,1),'SAME')

        #13*25*25
        layer = tf.layers.conv3d(layer2,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer3 = tf.nn.relu(tf.add(layer,layer2))
        #layer = tf.nn.max_pool3d(layer_2,(1,2,2,2,1),(1,2,2,2,1),'SAME')

        #7*13*13
        layer = tf.layers.conv3d(layer3,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer4 = tf.nn.relu(tf.add(layer,layer3))
        #layer = tf.nn.max_pool3d(layer_3,(1,2,2,2,1),(1,2,2,2,1),'SAME')

        layer = tf.layers.conv3d(layer4,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer5 = tf.nn.relu(tf.add(layer,layer4))
        #layer = tf.nn.max_pool3d(layer_1,(1,2,2,2,1),(1,2,2,2,1),'SAME')

        #13*25*25
        layer = tf.layers.conv3d(layer5,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer6 = tf.nn.relu(tf.add(layer,layer5))
        #layer = tf.nn.max_pool3d(layer_2,(1,2,2,2,1),(1,2,2,2,1),'SAME')

        #7*13*13
        layer = tf.layers.conv3d(layer6,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer7 = tf.nn.relu(tf.add(layer,layer6))
        
        layer = tf.layers.conv3d(layer7,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer8 = tf.nn.relu(tf.add(layer,layer7))
        
        layer = tf.layers.conv3d(layer8,20,3,1,'SAME')
        layer = tf.nn.relu(layer)
        layer = tf.layers.conv3d(layer,20,3,1,'SAME')
        layer = tf.nn.relu(tf.add(layer,layer8))
        
        #layer = tf.nn.max_pool3d(layer_3,(1,2,2,2,1),(1,2,2,2,1),'SAME')
        logits = tf.layers.conv3d(layer,1,1,1,'SAME')
        
        return logits
    

def define_graph():
    rv = {}
    
    rv['imgs'] = tf.placeholder(tf.float32,[BATCH_SIZE,*SHAPE,1])
    rv['region_seeds'] = tf.placeholder(tf.float32,[BATCH_SIZE,*SHAPE,1])
    rv['boundary_seeds'] = tf.placeholder(tf.float32,[BATCH_SIZE,*SHAPE,1])
    rv['labels'] = tf.placeholder(tf.float32,[BATCH_SIZE,*SHAPE,1])
    #weights = (rv['labels']*10+1)

    rv['logits'] = define_unet(rv['imgs'],rv['region_seeds'],rv['boundary_seeds'])
    
    #rv['loss'] = tf.reduce_sum(tf.square(rv['logits']-rv['labels']))
    #loss_a = tf.reduce_mean((-rv['labels']*tf.log(1e-4+rv['logits'])-(1-rv['labels'])*tf.log(1e-4+1-rv['logits'])))
    
    loss_a = tf.reduce_mean(
                            tf.nn.sigmoid_cross_entropy_with_logits(
                            labels = rv['labels'],
                            logits = rv['logits']))
    loss_b = tf.reduce_mean(
                            tf.multiply(
                            tf.square(rv['logits']-1),rv['region_seeds']))
    loss_c = tf.reduce_mean(
                            tf.multiply(
                            tf.square(rv['logits']-0.5),rv['boundary_seeds']))

    rv['loss'] = loss_a + 100*loss_b + 10*loss_c
    
    tf.summary.scalar('loss', rv['loss'])
    tf.summary.scalar('loss_a', loss_a)
    tf.summary.scalar('loss_b', loss_b)
    tf.summary.scalar('loss_c', loss_c)
    
    rv['train_op'] = tf.train.AdamOptimizer(1e-4).minimize(rv['loss'])
    
    img = tf.cast(rv['imgs'],tf.uint8)
    label = 255 * tf.cast(rv['labels'],tf.uint8)
    
    rv['segmentation'] = segmentation = 255 * tf.cast(tf.to_int32(rv['logits'] >= SEG_THRESHOLD),tf.uint8)
    
    rv['acc'] = tf.reduce_mean(tf.cast(tf.equal(label,rv['segmentation']),tf.float32))
    tf.summary.scalar('acc', rv['acc'])
    
    for j in range(SHAPE[0]):
        tf.summary.image('depth_'+str(j),tf.concat([img[:,j,:,:,:],label[:,j,:,:,:],segmentation[:,j,:,:,:]],axis=2))
        
    rv['summary'] = tf.summary.merge_all()
       
    return rv


def run(sess,g,img,bseed,rseed):
    imgs = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
    region_seeds = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
    boundary_seeds = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
    
    r_d,r_h,r_w = img.shape
    imgs[0,:r_d,:r_h,:r_w] = img.reshape(r_d,r_h,r_w,1).astype(np.float32)
    region_seeds[0,:r_d,:r_h,:r_w] = rseed.reshape(r_d,r_h,r_w,1).astype(np.float32)
    boundary_seeds[0,:r_d,:r_h,:r_w] = bseed.reshape(r_d,r_h,r_w,1).astype(np.float32)

    seg = sess.run(g['logits'], feed_dict = {  g['imgs']:imgs, 
                                               g['region_seeds']:region_seeds, 
                                               g['boundary_seeds']:boundary_seeds})
    rv = seg[0].reshape(SHAPE)
    #rv = (rv-np.min(rv))/(np.max(rv)-np.min(rv)+1e-6)
    return rv


def find_seeds(pred,coord,visited):
    z,y,x = coord
    
    if z <= DELTA or z >= pred.shape[0]-DELTA or y <= DELTA or y >= pred.shape[1]-DELTA or x <= DELTA or x >= pred.shape[2]-DELTA:
        return []
        
    seeds = []
    if np.all(pred[z-3:z+3,y-3:y+3,x+DELTA-3:x+DELTA+3] > MOVE_THRESHOLD) and (z,y,x+DELTA) not in visited:
        seeds.append((z,y,x+DELTA))
        visited.add((z,y,x+DELTA))
    if np.all(pred[z-3:z+3,y-3:y+3,x-DELTA-3:x+DELTA+3] > MOVE_THRESHOLD) and (z,y,x-DELTA) not in visited:
        seeds.append((z,y,x-DELTA))
        visited.add((z,y,x-DELTA))
    if np.all(pred[z-3:z+3,y+DELTA-3:y+DELTA+3,x-3:x+3] > MOVE_THRESHOLD) and (z,y+DELTA,x) not in visited:
        seeds.append((z,y+DELTA,x))
        visited.add((z,y+DELTA,x))
    if np.all(pred[z-3:z+3,y-DELTA-3:y-DELTA+3,x-3:x+3] > MOVE_THRESHOLD) and (z,y-DELTA,x) not in visited:
        seeds.append((z,y-DELTA,x))
        visited.add((z,y-DELTA,x))
    if np.all(pred[z+DELTA-3:z+DELTA+3,y-3:y+3,x-3:x+3] > MOVE_THRESHOLD) and (z+DELTA,y,x) not in visited:
        seeds.append((z+DELTA,y,x))
        visited.add((z+DELTA,y,x))
    if np.all(pred[z-DELTA-3:z-DELTA+3,y-3:y+3,x-3:x+3] > MOVE_THRESHOLD) and (z-DELTA,y,x) not in visited:
        seeds.append((z-DELTA,y,x))
        visited.add((z-DELTA,y,x))
    
    return seeds  
    

def check(seed,stack):
    z,y,x = seed
    if z < 0 or y < 0 or x < 0:
        return False
    Z,Y,X = stack.shape
    if z >= Z or y >= Y or x >= X:
        return False
    return True
    
    
def grow(sess,g,stack,rseeds,bseeds,coord):
    #coord:(z,y,x)
    segmentation = np.ones(dtype = np.float32,shape = stack.shape) * 100000
    rv = np.zeros(dtype = np.uint8,shape = stack.shape)
    
    seeds = [(int(coord[0]),int(coord[1]),int(coord[2]))]
    visited = set()
    
    while len(seeds) != 0:
        c = seeds.pop()
        z,y,x = c
        visited.add((z//DELTA,y//DELTA,x//DELTA))
        img = stack[z-SHAPE[0]//2:z+SHAPE[0]//2,y-SHAPE[1]//2:y+SHAPE[0]//2,x-SHAPE[2]//2:x+SHAPE[2]//2]
        bseed = bseeds[z-SHAPE[0]//2:z+SHAPE[0]//2,y-SHAPE[1]//2:y+SHAPE[0]//2,x-SHAPE[2]//2:x+SHAPE[2]//2]
        rseed = np.zeros_like(bseed)
        if rseed.shape[0] * rseed.shape[1] * rseed.shape[2] == 0:
            continue
        rseed[rseed.shape[0]//2,rseed.shape[1]//2,rseed.shape[2]//2] = 1
        seg = run(sess,g,img,bseed,rseed)
        prev = segmentation[z-SHAPE[0]//2:z+SHAPE[0]//2,y-SHAPE[1]//2:y+SHAPE[0]//2,x-SHAPE[2]//2:x+SHAPE[2]//2]
        seg = seg[:prev.shape[0],:prev.shape[1],:prev.shape[2]]
        segmentation[z-SHAPE[0]//2:z+SHAPE[0]//2,y-SHAPE[1]//2:y+SHAPE[0]//2,x-SHAPE[2]//2:x+SHAPE[2]//2] = np.minimum(seg,prev)
        new_seeds = find_seeds(segmentation,(z,y,x),visited)
        seeds.extend(new_seeds)
        
    segmentation[segmentation == 100000] = 0
    rv[segmentation >= SEG_THRESHOLD] = 1
    return rv
           
    
def inference(stack,rseeds,bseeds):
    g = define_graph()
    with tf.Session() as s:
        saver = tf.train.Saver()
        saver.restore(s,MODEL)
            
        zs,ys,xs = np.where(rseeds != 0)
        z,y,x = np.mean(zs),np.mean(ys),np.mean(xs)
        
        return grow(s,g,stack,rseeds,bseeds,(z,y,x))

