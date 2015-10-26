'''
Created on Oct 25, 2015

@author: zhaot
'''
import urllib2
import json

class DvidService(object):
    '''
    classdocs
    '''

    def __init__(self, address, port, node):
        '''
        Constructor
        '''
        self.address = address
        self.port = port
        self.node = node
        self.opener = urllib2.build_opener()
        
    def getRootUrl(self):
        return 'http://%s:%d' % (self.address, self.port)
    
    def getNodeUrl(self):
        return 'http://%s:%d/api/node/%s' % (self.address, self.port, self.node)
    
    def getApiHelpUrl(self):
        return self.getRootUrl() + '/api/help'
    
    def getApiHelp(self):
        req = urllib2.Request(self.getApiHelpUrl())
        r = self.opener.open(req)
        content  = r.read()
        return content
    
if __name__ == '__main__':
    service = DvidService('emdata1.int.janelia.org', 8500, '86e1')
    print service.getNodeUrl()
    print service.getApiHelp()
    
    