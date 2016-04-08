import httplib
import requests
from dvidurl import DvidUrl
from dvidenv import DvidEnv


class DvidWriter(object):
    def __init__(self, dvidEnv = None):
        self.setDvidEnv(dvidEnv)

    def setDvidEnv(self, dvidEnv):
        self.dvidEnv = dvidEnv
        self.dvidUrl = DvidUrl(dvidEnv)

    def deleteSkeleton(self, bodyId):
        url = self.dvidUrl.getSkeletonUrl(bodyId)
        r = requests.delete(url)
        print r.url

    def deleteThumbnail(self, bodyId):
        url = self.dvidUrl.getThumbnailUrl(bodyId)
        r = requests.delete(url)
        print r.url
        print r.status_code

    def deleteBodyAnnotation(self, bodyId):
        r = requests.delete(self.dvidUrl.getBodyAnnotationUrl(bodyId))
        print r.url
        print r.status_code

    def deleteKeyValue(self, dataName, key):
        r = requests.delete(self.dvidUrl.getKeyUrl(dataName, key))

if __name__ == "__main__":
    dvidWriter = DvidWriter(DvidEnv("http://emdata1.int.janelia.org", 8500, "372c"))
    #dvidWriter.dvidEnv.setBodyLabel("bodies3")
    dvidWriter.deleteThumbnail(1)
    dvidWriter.deleteSkeleton(1)


    
