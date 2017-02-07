'''
Created on May 10, 2016

@author: zhaot
'''

import httplib
import requests
from dvidurl import DvidUrl
from dvidenv import DvidEnv


class DvidReader(object):
    '''
    DVID reader
    '''

    def __init__(self, dvidEnv = None):
        '''
        Constructor
        '''
        self.setDvidEnv(dvidEnv)

    def setDvidEnv(self, dvidEnv):
        self.dvidEnv = dvidEnv
        self.dvidUrl = DvidUrl(dvidEnv)
        
    def readMasterBranches(self):
        r = requests.get(self.dvidUrl.getMasterBranchUrl())
        print r.json
        
if __name__ == '__main__':
    reader = DvidReader(DvidEnv("emdata1.int.janelia.org", 8500, "99ef"))