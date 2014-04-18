import os
import sys
import json

sys.path.append('module')
sys.path.append('service')

import neutube 
import flyem
import neutu_server as ns
import httplib
from urlparse import urlparse

class QualityAnalyzer:
    def __init__(self):
        self._engine = neutube.CreateZFlyEmQualityAnalyzer()
        self._dataBundle = flyem.FlyEmDataBundle()

    def __del__(self):
        if neutube:
            neutube.DeleteZFlyEmQualityAnalyzer(self._engine)

    def getSkeleton(self, bodyId):
        conn = httplib.HTTPConnection(ns.getSkeletonServer())
        conn.request('POST', '/skeletonize', \
                '{"uuid": "a75", "bodies": [' + str(bodyId) + ']}', \
                {"Content-Type": "application/json"})
        r = conn.getresponse()
        if r.status == 200:
            swcListString = r.read()
            swcList = json.loads(swcListString)
            #print swcList
            if swcList['swc-list']:
                for swc in swcList['swc-list']:
                    if swc['id'] == bodyId:
                        parsed = urlparse('//' + swc['url'])
                        conn = httplib.HTTPConnection(parsed.netloc)
                        conn.request('GET', parsed.path)
                        r = conn.getresponse()
                        if r.status == 200:
                            return neutube.DecodeSwcTree(r.read())
                        break


    def loadDataBundle(self, bundleFile):
        self._dataBundle.load(bundleFile)

    def computeHotSpot(self, bodyId):
        model = self.getSkeleton(bodyId)
        neuron = neutube.CreateZFlyEmNeuron(bodyId, model)
        hotspot = self._engine.computeHotSpotForSplit(neuron)
        print hotspot
        hotspot._print()
        return hotspot.toJsonString()

        #neuron = self._dataBundle.getNeuron(bodyId)
        #if neuron:
        #    return self._engine.computeHotSpot(neuron.getModel().getCData(), None, 10, 10, 10, 1000).toJsonString()
        #else:
        #    raise exception('Cannot retrieve the neuron: ' + str(bodyId))

if __name__ == '__main__':
    qa = QualityAnalyzer()
    #qa.loadDataBundle('/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/data_release/bundle5/data_bundle.json')
    print qa.computeHotSpot(1)
    #tree = qa.getSkeleton(1)
    #tree._print()
  

