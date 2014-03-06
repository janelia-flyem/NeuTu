import os
import sys
baseDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(baseDir, 'module'))
sys.path.append(os.path.join(baseDir, 'flyem'))

import neutube
from optparse import OptionParser
import json
from LoadDvidObject import LoadDvidObject
import httplib

def Skeletonize(source, target, config = None):
    if isinstance(source, int):
        sparseObj = LoadDvidObject(source)
        stack = sparseObj.toStackObject()
    elif source.isdigit():
        sparseObj = LoadDvidObject(int(source))
        stack = sparseObj.toStackObject()
    else:
        stack = neutube.ZStack()
        stackFile = neutube.ZStackFile()
        stackFile._import(source)
        stack = stackFile.readStack();

    skeletonizer = neutube.ZStackSkeletonizer()
    
    if not config:
        print 'null config'
        config = {'description': 'skeletonize configuration'}
    
    print config
    
    if not config.has_key('args'):
        configPath = os.path.join(baseDir, '../json/skeletonize.json')
        configFile = None
        print '********************'
        print configPath
        if os.path.exists(configPath):
            configFile = open(configPath);
        if configFile:
            config['args'] = json.load(configFile)
            configFile.close()
    

    if config['args'].has_key('downsampleInterval'):
        skeletonizer.setDownsampleInterval(config['args']['downsampleInterval'][0],
                                           config['args']['downsampleInterval'][1],
                                           config['args']['downsampleInterval'][2])
            
        if config['args'].has_key('minimalLength'):
            skeletonizer.setLengthThreshold(config['args']['minimalLength'])
            
        if config['args'].has_key('keepingSingleObject'):
            skeletonizer.setKeepingSingleObject(config['args']['keepingSingleObject'])
                
        if config['args'].has_key('rebase'):
            skeletonizer.setRebase(config['args']['rebase'])
            
    tree = skeletonizer.makeSkeleton(stack)
    
        
    if target == 'dvid':
        dvidServer = 'emdata1.int.janelia.org'
        if config.has_key('dvid-server'):
            dvidServer = config['dvid-server']
        
        uuid = '339'
        if config.has_key('uuid'):
            uuid = config['uuid']
            
        conn = httplib.HTTPConnection(dvidServer)
        array = neutube.EncodeSwcTree(tree)
        conn.request('POST', '/api/node/' + uuid + ' /skeletons/' + str(source) + '.swc',
                     ''.join(array))
        response = conn.getresponse()
        print 'Uploaded into dvid'
        print response.status, response.reason
    else:
        tree.save(target)
    
    neutube.DeleteStackObject(stack)
    neutube.DeleteSwcTree(tree)

if __name__ == '__main__': 
    parser = OptionParser();
    parser.add_option("-i", "--input", dest="input", help="input image");
    parser.add_option("-o", "--output", dest="output", help="output folder");
    parser.add_option("--config", dest="config", help="configuration",
                      default="../json/skeletonize.json");
    (options, args) = parser.parse_args();
    
    configFile = open(options.config);
    config = { 'args': json.load(configFile) }    
    configFile.close()
        
    Skeletonize(options.input, options.output, config)
    
    print options.input
    print 'done'
