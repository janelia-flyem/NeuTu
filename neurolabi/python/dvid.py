import os
import sys
baseDir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(os.path.join(baseDir, 'module'))
sys.path.append(os.path.join(baseDir, 'flyem'))

import neutube
from optparse import OptionParser
import json
import httplib
import dvid

class DvidServer:
    def __init__(self, host = '', uuid = ''):
        self._host = host
        self._uuid = uuid
    
    def getBodyRequest(self, id):
        return '/api/node/' + self._uuid +'/sp2body/sparsevol/' + str(id)
    
    def getSkeletonRequest(self, id):
        return '/api/node/' + self._uuid + '/skeletons/' + str(id) + '.swc'

    def retrieveBody(self, id):
        print self._host
        conn = httplib.HTTPConnection(self._host)
        
        print self.getBodyRequest(id)
        print '**********Response:'
        conn.request("GET", self.getBodyRequest(id))
        r1 = conn.getresponse()
        
        print r1.status
        
        print '**********'
        print r1
        
        data = r1.read();
        
        print len(data)
        
        dataArray = neutube.Char_Array()
        dataArray = data
        
        sparseObj = neutube.CreateObject3dScan()
        sparseObj.importDvidObjectBuffer(dataArray)
        
        return sparseObj
    
    def retrieveSkeleton(self, id):
        conn = httplib.HTTPConnection(self._host) 
        print self.getBodyRequest(id)
        print '**********Response:'
        conn.request("GET", self.getSkeletonRequest(id))
        r1 = conn.getresponse()
        
        print r1.status
        
        print '**********'
        print r1
        
        data = r1.read();
        
        print len(data)
        
        dataArray = neutube.Char_Array()
        dataArray = data
        skeleton = neutube.CreateSwcTree()
        skeleton.loadFromBuffer(dataArray)
        
        skeleton._print()
        
        return skeleton
        
    
    def hasSkeleton(self, id):
        conn = httplib.HTTPConnection(self._host)
        conn.request("GET", self.getSkeletonRequest(id))

        r1 = conn.getresponse()
        
        return r1.status == 200
    
    def retrieveNeuron(self, id):
        body = self.retrieveBody(id)
        skeleton = self.retrieveSkeleton(id)
        return neutube.CreateZFlyEmNeuron(id, skeleton, body)
    
if __name__ == '__main__':
    server = DvidServer('emdata1.int.janelia.org', '240a')
    body = server.retrieveBody(1)
    body._print()
    
    print server.hasSkeleton(1)
    
    