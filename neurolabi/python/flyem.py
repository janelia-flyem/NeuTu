import os
import sys
import json

sys.path.append('module')

import neutube

class SwcTree:
    def __init__(self):
        self._data = neutube.CreateSwcTree()

    def __del__(self):
        if neutube:
            neutube.DeleteSwcTree(self._data)

    def load(self, filePath):
        print filePath
        self._data.load(filePath)

    def getCData(self):
        return self._data
        
class FlyEmNeuron:
    def __init__(self):
        self._resolution = [1, 1, 1]
        self._model = None
        self._id = -1

    def set(self, config, base = None):
        self._id = config['id']
        self._name = str(config['name'])
        self._modelPath = str(config['model'])
        if base:
            self._modelPath = \
                    os.path.abspath(os.path.join(base, self._modelPath))

    def getId(self):
        return self._id

    def getModelPath(self):
        return self._modelPath

    def getModel(self):
        if not self._model and self._modelPath:
            self._model = SwcTree()
            self._model.load(self._modelPath);

        return self._model

class FlyEmDataBundle:
    def __init__(self):
        self._neuronArray = []
        self._neuronMap = dict()

    def load(self, filePath):
        baseDir = os.path.dirname(os.path.abspath(filePath))
        print 'Loading bundle ...'
        with open(filePath) as f:
            data = json.load(f)
            if data.has_key('neuron'):
                for neuronConfig in data['neuron']:
                    neuron = FlyEmNeuron()
                    neuron.set(neuronConfig, base = baseDir)
                    self._neuronArray.append(neuron)
                    self._neuronMap[neuron.getId()] = neuron

            f.close()

    def getNeuron(self, bodyId):
        return self._neuronMap[bodyId]


        




