import json
from dvidenv import *

class DvidUrl(object):
    def __init__(self, env):
        self.dvidEnv = env

    def getServerUrl(self):
        return self.dvidEnv.getAddressWithPort()

    def getSkeletonUrl(self, bodyId):
        return self.getServerUrl() + self.getSkeletonEndPoint(bodyId)

    def getBodyAnnotationUrl(self, bodyId):
        return self.getServerUrl() + self.getBodyAnnotationEndPoint(bodyId)

    def getBodyAnnotationEndPoint(self, bodyId):
        dataName = "annotations"
        if self.dvidEnv.bodyLabel != "bodies":
            dataName = self.dvidEnv.bodyLabel + "_" + dataName

        return "/api/node/" + self.dvidEnv.node + "/" + dataName + "/key/" + str(bodyId)
    
    def getSkeletonEndPoint(self, bodyId):
        skeletonName = "skeletons"
        if self.dvidEnv.bodyLabel != "bodies":
            skeletonName = self.dvidEnv.bodyLabel + "_" + skeletonName

        return "/api/node/" + self.dvidEnv.node + "/" + skeletonName + "/key/" + str(bodyId) + "_swc"

    def getThumbnailEndPoint(self, bodyId):
        dataName = "thumbnails"
        if self.dvidEnv.bodyLabel != "bodies":
            dataName = self.dvidEnv.bodyLabel + "_" + dataName

        return "/api/node/" + self.dvidEnv.node + "/" + dataName + "/key/" + str(bodyId) + "_mraw"

    def getThumbnailUrl(self, bodyId):
        return self.getServerUrl() + self.getThumbnailEndPoint(bodyId)
    
    def getMasterBranchUrl(self):
        return self.getServerUrl() + "/api/repo/branches/key/master"

if __name__ == '__main__':
    dvidUrl = DvidUrl(DvidEnv('emdata1.int.janelia.org', 8500, '86e1'))
