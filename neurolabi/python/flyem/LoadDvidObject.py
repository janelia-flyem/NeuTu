import sys
sys.path.append('../module')

import neutube
from optparse import OptionParser
import json
import os
import httplib

def LoadDvidObject(bodyId, dvidServer, uuid):
    print dvidServer
    conn = httplib.HTTPConnection(dvidServer)
    
    dvidRequest = '/api/node/' + uuid +'/sp2body/sparsevol/' + str(bodyId)
    print dvidRequest
    
    print '**********Response:'
    conn.request("GET", dvidRequest)
    r1 = conn.getresponse()
    
    print '**********'
    print r1
    
    data = r1.read();
    dataArray = neutube.Char_Array()
    dataArray = data
    
    print type(dataArray)
    
    #neutube.PrintCharArray(dataArray)
    
    sparseObj = neutube.CreateObject3dScan()
    sparseObj.importDvidObjectBuffer(dataArray)
    
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

