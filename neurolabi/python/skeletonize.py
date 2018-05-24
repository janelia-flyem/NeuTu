import os
import sys
baseDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(baseDir, 'module'))
sys.path.append(os.path.join(baseDir, 'flyem'))

import neutube
from optparse import OptionParser
import json
from LoadDvidObject import LoadDvidObject
import http.client

def SubmitSkeletonizeService(dvidServer, uuid, bodyArray):
   #curl -X POST -H "Content-Type: application/json" #
   #-d '{"dvid-server": http://emdata1.int.janelia.org, "uuid": "339c", "bodies":[1, 2, 3]}'
   # http://emrecon100.janelia.priv:8082/skeletonize
    conn = http.client.HTTPConnection("emrecon100.janelia.priv:8082")
    jsonData = {"dvid-server": dvidServer, "uuid": uuid, "bodies": bodyArray};
    headers = {"Content-type": "application/json"}
    conn.request("POST", "/skeletonize", json.dumps(jsonData), headers)
    r = conn.getresponse()
    r.read()
    
def Skeletonize(source, target, config = None):
    dvidServer = None
    uuid = None
    if target == 'dvid':        
        if not config:
            raise Exception('Server configuration must be specified for DVID target.')
        else:
            if 'dvid-server' not in config:
                raise Exception('Server address must be specified for DVID target.')
            elif 'uuid' not in config:
                raise Exception('UUID must be specified for DVID target.')
        
    if config:
        dvidServer = config['dvid-server']
        uuid = config['uuid']

    skeletonizer = neutube.ZStackSkeletonizer()
    
    if not config:
        print('null config')
        config = {'description': 'skeletonize configuration'}
    
    print(config)
    
    if 'args' not in config:
        configPath = os.path.join(baseDir, '../json/skeletonize.json')
        configFile = None
        print('********************')
        print(configPath)
        if os.path.exists(configPath):
            configFile = open(configPath);
        if configFile:
            config['args'] = json.load(configFile)
            configFile.close()
    
    sparseObj = None
    stack = None
    if isinstance(source, int):
        sparseObj = LoadDvidObject(source, dvidServer, uuid)
        if sparseObj.isEmpty():
            raise Exception('Unable to load the body: ' + str(source))

        if 'fillingHole' in config['args']:
            if config['args']['fillingHole']:
                sparseObj.fillHole()
        #stack = sparseObj.toStackObject()
    elif source.isdigit():
        sparseObj = LoadDvidObject(int(source), dvidServer, uuid)
        if sparseObj.isEmpty():
            raise Exception('Unable to load the body: ' + str(source))
        if 'fillingHole' in config['args']:
            if config['args']['fillingHole']:
                sparseObj.fillHole()
        #stack = sparseObj.toStackObject()
    elif isinstance(source, str):
        print(source)
        if source.endswith('.sobj'):
            sparseObj = neutube.CreateObject3dScan()
            sparseObj.load(source)
    else:
        stack = neutube.ZStack()
        stackFile = neutube.ZStackFile()
        stackFile._import(source)
        stack = stackFile.readStack();

    if 'downsampleInterval' in config['args']:
        skeletonizer.setDownsampleInterval(config['args']['downsampleInterval'][0],
                                           config['args']['downsampleInterval'][1],
                                           config['args']['downsampleInterval'][2])
            
    if 'minimalLength' in config['args']:
        skeletonizer.setLengthThreshold(config['args']['minimalLength'])

    if 'maximalDistance' in config['args']:
        skeletonizer.setDistanceThreshold(config['args']['maximalDistance'])
            
    if 'minimalObjectSize' in config['args']:
        skeletonizer.setMinObjSize(config['args']['minimalObjectSize'])
            
    if 'keepingSingleObject' in config['args']:
        skeletonizer.setKeepingSingleObject(config['args']['keepingSingleObject'])
                
    if 'rebase' in config['args']:
        skeletonizer.setRebase(config['args']['rebase'])
            
    skeletonizer._print()
    
    tree = None
    if sparseObj:
        tree = skeletonizer.makeSkeleton(sparseObj)
    else:
        tree = skeletonizer.makeSkeleton(stack)
        
    if target == 'dvid':
        conn = http.client.HTTPConnection(dvidServer)
        array = neutube.EncodeSwcTree(tree)
        dvidRequest = '/api/node/' + uuid + '/skeletons/' + str(source) + '.swc'
        print(dvidRequest)
        conn.request('POST', dvidRequest, ''.join(array))
        response = conn.getresponse()
        print('Uploaded into dvid')
        print(response.status, response.reason)
    else:
        tree.save(target)
    
    if stack:
        print('Deleting stack')
        neutube.DeleteStackObject(stack)
    if sparseObj:
        print('Deleting object')
        neutube.DeleteObject3dScan(sparseObj)

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
        
    config['dvid-server'] = 'emdata1.int.janelia.org:7000'
    config['uuid'] = 'a75'
    Skeletonize(options.input, options.output, config)
    
    print(options.input)
    print('done')
