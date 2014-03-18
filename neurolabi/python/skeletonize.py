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
    dvidServer = None
    uuid = None
    if target == 'dvid':        
        if not config:
            raise Exception('Server configuration must be specified for DVID target.')
        else:
            if not config.has_key('dvid-server'):
                raise Exception('Server address must be specified for DVID target.')
            elif not config.has_key('uuid'):
                raise Exception('UUID must be specified for DVID target.')
        
        dvidServer = config['dvid-server']
        uuid = config['uuid']

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
    
    if isinstance(source, int):
        sparseObj = LoadDvidObject(source, dvidServer, uuid)
        if config['args'].has_key('fillingHole'):
            if config['args']['fillingHole']:
                sparseObj.fillHole()
        stack = sparseObj.toStackObject()
    elif source.isdigit():
        sparseObj = LoadDvidObject(int(source), dvidServer, uuid)
        if config['args'].has_key('fillingHole'):
            if config['args']['fillingHole']:
                sparseObj.fillHole()
        stack = sparseObj.toStackObject()
    else:
        stack = neutube.ZStack()
        stackFile = neutube.ZStackFile()
        stackFile._import(source)
        stack = stackFile.readStack();

    if config['args'].has_key('downsampleInterval'):
        skeletonizer.setDownsampleInterval(config['args']['downsampleInterval'][0],
                                           config['args']['downsampleInterval'][1],
                                           config['args']['downsampleInterval'][2])
            
        if config['args'].has_key('minimalLength'):
            skeletonizer.setLengthThreshold(config['args']['minimalLength'])
            
        if config['args'].has_key('minimalObjectSize'):
            skeletonizer.setMinObjSize(config['args']['minimalObjectSize'])
            
        if config['args'].has_key('keepingSingleObject'):
            skeletonizer.setKeepingSingleObject(config['args']['keepingSingleObject'])
                
        if config['args'].has_key('rebase'):
            skeletonizer.setRebase(config['args']['rebase'])
            
    skeletonizer._print()
    
    tree = skeletonizer.makeSkeleton(stack)
    
        
    if target == 'dvid':
        conn = httplib.HTTPConnection(dvidServer)
        array = neutube.EncodeSwcTree(tree)
        dvidRequest = '/api/node/' + uuid + '/skeletons/' + str(source) + '.swc'
        print dvidRequest
        conn.request('POST', dvidRequest, ''.join(array))
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
        
    config['dvid-server'] = 'emdata1.int.janelia.org'
    config['uuid'] = '240a'
    Skeletonize(options.input, options.output, config)
    
    print options.input
    print 'done'
