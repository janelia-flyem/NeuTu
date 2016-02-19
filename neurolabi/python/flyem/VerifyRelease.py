from git import Repo
import urllib2
from DvidService import DvidService
from git import Repo
import json
import os.path

if __name__ == '__main__':
    print 'Verifying ...'
    
    vf = '/Users/zhaot/Work/neutube/neurolabi/json/verify.json'
    repoPath = '/Users/zhaot/Work/neutube'
    
    vp = None
    with open(vf) as fp:
        vp = json.load(fp)
    
    repoInfo = vp['dvid repo']
    codeInfo = vp['code branch']
    
    print 'Checking git branches ...'
    repo = Repo(repoPath)
#     print repo.head.commit
#     print codeInfo['head']
    if str(repo.head.commit) != codeInfo['head']:
        print 'Wrong branch:'
        print '  Actual:', repo.head.commit
        print '  Expected:', codeInfo['head']
        print 'Failed.'
        exit(1)
        
    print 'Checking database ...'
    dvidService = DvidService('emdata1.int.janelia.org', 8500, '86e1')
    print dvidService.getNodeUrl()
    
    configFile = os.path.join(repoPath, 'neurolabi', 'json', 'flyem_config.json')
    configJson = None
    with open(configFile) as fp:
        configJson = json.load(fp)
        
    configList =configJson['dvid repo']
    for config in configList:
        name = config['name']
        if repoInfo.has_key(name):
            print name
            uuid = repoInfo[name]
            if uuid != config['uuid']:
                print 'Wrong uuid:'
                print '  Actual:', config['uuid']
                print '  Expected:', uuid
                print 'Failed.'
                exit(1)
#     dvidService.getApiHelp()
    
    print 'Passed.'