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

def Skeletonize(source, target,
                configPath = os.path.join(baseDir, '../../json/skeletonize.json')):
    if source.isdigit():
      sparseObj = LoadDvidObject(int(source))
      stack = sparseObj.toStackObject()
    else:
      stack = neutube.ZStack()
      stackFile = neutube.ZStackFile()
      stackFile._import(source)
      stack = stackFile.readStack();

    skeletonizer = neutube.ZStackSkeletonizer()

    if os.path.exists(configPath):
        configFile = open(configPath);

        if configFile:
            config = json.load(configFile)
            if config.has_key('downsampleInterval'):
                skeletonizer.setDownsampleInterval(config['downsampleInterval'][0],
                                                   config['downsampleInterval'][1],
                                                   config['downsampleInterval'][2])
            
            if config.has_key('minimalLength'):
                skeletonizer.setLengthThreshold(config['minimalLength'])
            
            if config.has_key('keepingSingleObject'):
                skeletonizer.setKeepingSingleObject(config['keepingSingleObject'])
                
            if config.has_key('rebase'):
                skeletonizer.setRebase(config['rebase'])
            
            configFile.close()

    tree = skeletonizer.makeSkeleton(stack)
    
    if target == 'dvid':
        conn = httplib.HTTPConnection('emdata1.int.janelia.org')
        array = neutube.EncodeSwcTree(tree)
        conn.request('POST', '/api/node/339/skeletons/' + source + '.swc',
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
    
    Skeletonize(options.input, options.output, options.config)
    
    print options.input
    print 'done'
