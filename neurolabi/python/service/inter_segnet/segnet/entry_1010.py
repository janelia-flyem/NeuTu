import os
import sys
from shutil import rmtree
from random import random,randint
from glob import glob
import tensorflow as tf
import numpy as np
from tiffio.baseio import open_tif,save_tif


MODEL = './segnet/data/model/model.ckpt-10000'
SHAPE = (24,24,24)
BATCH_SIZE = 50
SEG_THRESHOLD = 0.5
    

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



def inference(stack,rseeds,bseeds):
    g = define_graph()
    with tf.Session() as s:
        saver = tf.train.Saver()
        saver.restore(s,MODEL)
        
        d,h,w = stack.shape    
        segmentation = np.zeros_like(stack,dtype=np.float32)
        seg_list = []
            
        imgs = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
        region_seeds = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
        boundary_seeds = np.zeros(dtype = np.float32, shape = (BATCH_SIZE,*SHAPE,1))
        
        cnt = 0

        for k in range(0,d,SHAPE[0]):
            for j in range(0,h,SHAPE[1]):
                for i in range(0,w,SHAPE[2]):
                    img = stack[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]]
                    region_seed = rseeds[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]]
                    boundary_seed = bseeds[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]]
                    if np.any(region_seed != 0):
                        r_d,r_h,r_w = img.shape
                        imgs[cnt,:r_d,:r_h,:r_w] = img.reshape(r_d,r_h,r_w,1).astype(np.float32)
                        region_seeds[cnt,:r_d,:r_h,:r_w] = region_seed.reshape(r_d,r_h,r_w,1).astype(np.float32)
                        boundary_seeds[cnt,:r_d,:r_h,:r_w] = boundary_seed.reshape(r_d,r_h,r_w,1).astype(np.float32)
                        cnt += 1
                        if cnt == BATCH_SIZE:
                            seg = s.run(g['logits'], feed_dict = {  g['imgs']:imgs, 
                                                              g['region_seeds']:region_seeds, 
                                                              g['boundary_seeds']:boundary_seeds})
                            for ss in seg:
                                seg_list.append(ss.reshape(SHAPE))
                            cnt = 0
        if cnt > 0:
           seg = s.run(g['logits'], feed_dict = {  g['imgs']:imgs, 
                                                   g['region_seeds']:region_seeds, 
                                                   g['boundary_seeds']:boundary_seeds})
           for ss in seg:
               seg_list.append(ss.reshape(SHAPE))
          
        index = 0
        for k in range(0,d,SHAPE[0]):
            for j in range(0,h,SHAPE[1]):
                for i in range(0,w,SHAPE[2]):
                    region_seed = rseeds[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]]
                    if np.any(region_seed != 0):
                        shape = segmentation[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]].shape 
                        seg = seg_list[index]
                        segmentation[k:k+SHAPE[0],j:j+SHAPE[1],i:i+SHAPE[2]] = seg[:shape[0],:shape[1],:shape[2]]
                        index += 1
            

        result = np.zeros(shape = segmentation.shape,dtype=np.uint8)
        result[segmentation >= SEG_THRESHOLD] = 1
        return result

