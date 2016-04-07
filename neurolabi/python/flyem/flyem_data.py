import json
from dvidenv import DvidEnv

class DataId:
    DATA_BODY = "body"
    DATA_SKELETON = "skeleton"
    def __init__(self, dataType, bodyId = 0):
        self._type = dataType
        self.bodyId = bodyId

    def __eq__(self, other):
        return self._type == other._type and self.bodyId == other.bodyId

    def getType(self):
        return self._type

    def getId(self):
        return self._type + ":" + str(self.bodyId)

    def getBodyId(self):
        return self.bodyId
    
class DataEvent:
    DATA_INVALIDATE = 1
    DATA_UPDATE = 2
    DATA_DELETE = 3

    def __init__(self, eventType, dataId, dvidEnv):
        self._type = eventType
        self.dataId = dataId
        self.dvidEnv = dvidEnv

    def __str__(self):
        s = str(self.dvidEnv) + "|"
        if self._type == DataEvent.DATA_INVALIDATE:
            s += "Invalidate" 
        elif self._type == DataEvent.DATA_UPDATE:
            s += "Update"
        elif self._type == DataEvent.DATA_DELETE:
            s += "Delete"

        s += "|" + self.getDataId().getId()

        return s

    def getType(self):
        return self._type

    def getDataId(self):
        return self.dataId

class DataValidator:
    def __init__(self, dvidEnv):
        self.dvidEnv = dvidEnv

    def setDvidEnv(self, dvidEnv):
        self.dvidEnv = dvidEnv

if __name__ == "__main__":
    event = DataEvent(DataEvent.DATA_INVALIDATE, DataId(DataId.DATA_BODY, 1), DvidEnv("emdata1.int.janelia.org", 8500, "372c"))
    print event
