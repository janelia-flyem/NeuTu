import os
import sys
baseDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(baseDir, 'module'))
sys.path.append(os.path.join(baseDir, 'flyem'))
configDir = baseDir + '/../json'

import neutube
from optparse import OptionParser
import json
from LoadDvidObject import LoadDvidObject
import httplib
import dvid

def computeHotSpot(source, config):
    dvidServer = None
    uuid = None
    
    if not config:
        raise Exception('Server configuration must be specified for DVID target.')
    else:
        if not config.has_key('dvid-server'):
            raise Exception('Server address must be specified for DVID target.')
        elif not config.has_key('uuid'):
            raise Exception('UUID must be specified for DVID target.')
        
    dvidServer = config['dvid-server']
    uuid = config['uuid']
    
    analyzer = neutube.CreateZFlyEmQualityAnalyzer()
    
    jsonStr = ''
    
    #for each id
    for id in source:
        #Download sparse object from dvid
        sparseObj = dvid.retrieveBody(id)
        
        #if the skeleton is not in dvid
        if not dvid.hasSkeleton(id):
            #skeletonize the object
            if config.has_key['skeletonize']:
                skeletonConfig = config['skeletonize']
            else:
                f = open(configDir + '/skeletonize_fib25_len40.json')
                skeletonConfig = json.load(f)
                skeletonize(id, 'dvid', skeletonConfig)
                f.close()
        
        #Download skeleton 
        skeleton = dvid.retrieveSkeleton(id)
        
        neuron = neutube.CreateZFlyEmNeuron()
        
        neuron.setBody(sparseObj)
        neuron.setSwc(skeleton)
        
        pointArray = analyzer.computeHotSpot(neuron)
        jsonStr += pointArray.toJsonString()
        
        neutube.DeleteZFlyEmNeuron(neuron)
        
    neutube.DeleteZFlyEmQualityAnalyzer(analyzer)
        
    return jsonStr
        
if __name__ == '__main__':
    config = {'dvid-server': 'emdata1.int.janelia.org', 'uuid': '240a'}
    
    print computeHotSpot(1, config)
    