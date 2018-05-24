import sys
import os
import json
import requests
import urllib.request, urllib.parse, urllib.error
import urllib.request, urllib.error, urllib.parse
import pydvid
import http.client

class ZDvidNode():
    def __init__(self, address = None, port = 8000, uuid = None):
        self.address = address
        self.port = port
        self.uuid = uuid
        
    def setAddress(self, address):
        self.address = address
        
    def setPort(self, port):
        self.port = port
        
    def setUuid(self, uuid):
        self.uuid = uuid
        
    def getRelativeUrl(self):
        return '/api/node/%s' % (self.uuid)
    
    def getUrl(self):
        return 'http://%s:%d%s' % (self.address, self.port, self.getRelativeUrl())
    
    def getKeyvalueUrl(self, dataName, key):
        return '%s/%s/%s' % (self.getUrl(), dataName, key)
    
    def getRelativeSparseVolUrl(self, dvidLabel, bodyId = -1):
        url = '%s%s' % (getRelativeUrl(), dvidLabel.getLabelVolName())
        if bodyId >= 0:
            url += '/' + str(bodyId)
            
        return url
    
    def getSparseVolUrl(self, dvidLabel, bodyId = -1):
        url = '%s/%s' % (getUrl(), dvidLabel.getLabelVolName())
        if bodyId >= 0:
            url += '/' + str(bodyId)
            
        return url
       
    def getSplitSeedUrl(self, dvidLabel, bodyId = -1, oldFormat = False):
        firstId = bodyId
        secondId = -1
        if isinstance(bodyId, tuple) or isinstance(bodyId, list):
            (firstId, secondId) = bodyId

        url = self.getUrl() + '/' + dvidLabel.prefixName('splits')
        
        if isinstance(firstId, str) or firstId >= 0:
            if oldFormat:
                seedName = dvidLabel.prefixSeedName(firstId)
            else:
                seedName = dvidLabel.seedName(firstId)
            url += '/' + seedName
            
        if isinstance(secondId, str) or secondId >= 0:
            if oldFormat:
                seedName = dvidLabel.prefixSeedName(secondId)
            else:
                seedName = dvidLabel.seedName(secondId)
            url += '/' + seedName
        
        return url
    
    def getSplitSeedStatus(self, dvidLabel):
        url = getUrl() + '/' + dvidLabel.prefixName('split_status')
        if bodyId >= 0:
            url += '/' + dvidLabel.seedName(bodyId)
        
        return url
       
       
class ZDvidLabel():
    def __init__(self, labelBlockName = None, labelVolName = 'bodies'):
        self.labelBlockName = labelBlockName
        self.labelVolName = labelVolName
        
    def getLabelBlockName(self):
        if not self.labelBlockName:
            return self.labelVolName

    def getLabelVolName(self):
        return self.labelVolName

    def setLabelBlockName(self, name):
        self.labelBlockName = name
        
    def setLabelVolName(self, name):
        self.labelVolName = name
        
    def setSegmenationName(self, name):
        self.labelVolName = name
        
    def getPrefix(self, name):
        prefix = ''
        if self.getLabelVolName() != 'bodies':
            prefix = name + '_'
            
    def prefixName(self, name):
        newName = name
        if self.getLabelVolName() != 'bodies':
            newName = self.getLabelVolName() + '_' + name
        
        return newName
    
    def prefixSeedName(self, bodyId):
        return self.getLabelVolName() + '_' + self.seedName(bodyId)
        
    def seedName(self, bodyId):
        return 'seed' + '_' + str(bodyId)
    
    def getBodyIdFromSeedKey(self, key):
        try:
            if key.startswith('seed_'):
                bodyId = int(key.replace('seed' + '_', ''))
            else:
                bodyId = int(key.replace(self.prefixName('seed' + '_'), ''))
        except:
            bodyId = None
            
        return bodyId
    
class ZDvidReader():
    def __init__(self, dvidNode, dvidLabel = None):
        self.node = dvidNode
        self.dvidLabel = dvidLabel
#         self.connection = httplib.HTTPConnection(self.node.getAddress, port= self.node.getPort())

    def readSplitSeedList(self, oldFormat = False):
        url = self.node.getSplitSeedUrl(self.dvidLabel, [0, 'a'], oldFormat)
        print(url)
        r = requests.get(url)
        
        return r.json()
        
    def readSplitSeed(self, bodyId, oldFormat = False):
        r = requests.get(self.node.getSplitSeedUrl(self.dvidLabel, bodyId, oldFormat))
        
        return r.json()

class ZDvidWriter():
    def __init__(self, dvidNode, dvidLabel = None):
        self.node = dvidNode
        self.dvidLabel = dvidLabel
        
    def writeSplitSeed(self, bodyId, seed):
#         print 'writing:', self.node.getSplitSeedUrl(self.dvidLabel, bodyId)
        r = requests.post(self.node.getSplitSeedUrl(self.dvidLabel, bodyId), json.dumps(seed))
        
    def deleteKey(self, dataName, key):
        requests.delete(self.node.getKeyvalueUrl(dataName, key))
        

if __name__ == '__main__':
#     sourceDvidNode = ZDvidNode('emdata2.int.janelia.org', 8000, '628')
#     sourceDvidLabel = ZDvidLabel()
#     sourceDvidLabel.setSegmenationName('segmentation030115')
#     sourceDvidReader = ZDvidReader(sourceDvidNode, sourceDvidLabel)
#     seedList = sourceDvidReader.readSplitSeedList(oldFormat = True)
#     print seedList
    
    targtDvidNode = ZDvidNode('emdata1.int.janelia.org', 7000, '97d')
    targetDvidLabel = ZDvidLabel()
#     targetDvidWriter = ZDvidWriter(targtDvidNode, targetDvidLabel)
#     
#     for seed in seedList:
#         bodyId = sourceDvidLabel.getBodyIdFromSeedKey(seed)
#         print bodyId
#         if bodyId:
#             seed = sourceDvidReader.readSplitSeed(bodyId, oldFormat = True)
#             print len(seed['seeds'])
#             targetDvidWriter.writeSplitSeed(bodyId, seed)
#       
    targetDvidReader = ZDvidReader(targtDvidNode, targetDvidLabel)
    seedList = targetDvidReader.readSplitSeedList()
    print(seedList)
    for seed in seedList:
        bodyId = targetDvidLabel.getBodyIdFromSeedKey(seed)
        print(bodyId)
        if bodyId:
            seed = targetDvidReader.readSplitSeed(bodyId)
            print(len(seed['seeds']))
    
    
#     
#     dvidWriter = ZDvidWriter(dvidNode, dvidLabel)
#     dvidWriter.deleteKey('splits', 'bodies_seed_4122761')
    
#     dvidServer = 'emdata2.int.janelia.org'
#     connection = httplib.HTTPConnection(dvidServer, port = 8000)
#     info = pydvid.general.get_repos_info(connection)
#     print info
#     
#     node = ZDvidNode(dvidServer, 8000, '628')
#     print node.getUrl()
#     
#     dvidLabel = ZDvidLabel()
#     dvidLabel.setSegmenationName('test')
#     dvidLabel.setSegmenationName('segmentation030115')
    
#     print dvidLabel.prefixName('seed')
#     
#     print node.getSplitSeedUrl(dvidLabel, [0, 'a'])
#     
#     r = requests.get(node.getSplitSeedUrl(dvidLabel, [0, 'a']))
#     print r.json()
    
#     dvidReader = ZDvidReader(node, dvidLabel)
#     seedList = dvidReader.readSplitSeedList()
#     
#     print dvidReader.readSplitSeed(10492041)
#     
#     print dvidLabel.getBodyIdFromSeedKey('segmentation030115_seed_10492041')
#     
#     for seed in seedList:
#         bodyId = dvidLabel.getBodyIdFromSeedKey(seed)
#         print bodyId
#         if bodyId:
#             print len(dvidReader.readSplitSeed(bodyId)['seeds'])
#     
#     outputDvidLabel = ZDvidLabel()
#     outputDvidLabel.setSegmenationName('test')
#     writer = ZDvidWriter(node, outputDvidLabel)
    
#     for seed in seedList:
#         bodyId = dvidLabel.getBodyIdFromSeedKey(seed)
#         print bodyId
#         if bodyId:
#             writer.writeSplitSeed(bodyId, dvidReader.readSplitSeed(bodyId))
            
#     dvidNodeUrl = 'http://emdata2.int.janelia.org/api/node/628'
#     splitKey = 'segmentation030115_splits'
#     url = dvidNodeUrl + '/segmentation030115_splits/segmentation030115_seed_0/segmentation030115_seed_a'
#     r = requests.get(url)
#     seedList = r.json()
#     for seed in seedList:
#         seedUrl = '%s/%s/%s' % (dvidNodeUrl, splitKey, seed)
#         r = requests.get(seedUrl)
#         print r.text


