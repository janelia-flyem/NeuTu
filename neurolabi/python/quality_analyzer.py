import os
import sys
import json

sys.path.append('module')

import neutube 
import flyem

class QualityAnalyzer:
    def __init__(self):
        self._engine = neutube.CreateZFlyEmQualityAnalyzer()
        self._dataBundle = flyem.FlyEmDataBundle()

    def __del__(self):
        if neutube:
            neutube.DeleteZFlyEmQualityAnalyzer(self._engine)

    def loadDataBundle(self, bundleFile):
        self._dataBundle.load(bundleFile)

    def computeHotSpot(self, bodyId):
        neuron = self._dataBundle.getNeuron(bodyId)
        if neuron:
           return self._engine.computeHotSpot(neuron.getModel().getCData(), None, 10, 10, 10, 1000).toJsonString()

if __name__ == '__main__':
    qa = QualityAnalyzer()
    qa.loadDataBundle('/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/data_release/bundle5/data_bundle.json')
    print qa.computeHotSpot(538772)

