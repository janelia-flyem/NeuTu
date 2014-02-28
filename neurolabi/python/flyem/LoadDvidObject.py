import sys
sys.path.append('../module')

import neutube
from optparse import OptionParser
import json
import os
import httplib

def LoadDvidObject(bodyId):
    conn = httplib.HTTPConnection('emdata1.int.janelia.org')
    conn.request("GET", '/api/node/339/sp2body/sparsevol/' + str(bodyId))
    r1 = conn.getresponse()
    
    data = r1.read();
    
    print len(data)
    print type(data)
    
    dataArray = neutube.Char_Array()
    dataArray = data
    
    print type(dataArray)
    
    #neutube.PrintCharArray(dataArray)
    
    sparseObj = neutube.ZObject3dScan()
    sparseObj.importDvidObject(dataArray)
    
    return sparseObj

#
#sparseObj._print()
#sparseObj.save('/Users/zhaot/Work/neutube/neurolabi/data/test.sobj')

#array = ''.join(neutube.CharArrayTest())
#print array
#
#conn = httplib.HTTPConnection('emdata1.int.janelia.org')
#conn.request('POST', '/api/node/339/skeletons/1.swc', array)
#response = conn.getresponse()
#print response.status, response.reason
#
#tree = neutube.ZSwcTree()
#tree.load('/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/dense.swc')
#
#array = neutube.EncodeSwcTree(tree)
#
#print ''.join(array)

