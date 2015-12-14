from git import Repo
import urllib2
from DvidService import DvidService
from git import Repo
from git import Git
import json
import os.path
import platform

def checkNeuTuDir(path):
    print '\nChecking git branches ...'
    repo = Repo(neutuDir)
    print 'Active branch:', repo.active_branch
    print 'Current head:', repo.head.commit.hexsha[0:4], \
        '"' + repo.head.commit.message.rstrip() + '"'
    flyemReleaseBranch = None
    for branch in repo.branches:
        if branch.name == 'flyem_release':
            flyemReleaseBranch = branch
            print 'flyem_release branch found'
            print 'head:', branch.commit.hexsha[0:4], '"' + branch.commit.message.rstrip() + '"'

    if not flyemReleaseBranch:
        print 'WARNING: no flyem_release branch.'
            
if __name__ == '__main__':
    print '************Checking ...'
    print 'Operating system:', platform.system()
    
    if platform.system() == 'Darwin':
        neutuDir = '/Users/zhaot/Work/neutube'
    else:
        neutuDir = '/groups/flyem/home/zhaot/Work/neutube_ws'
     
    if os.path.isdir(neutuDir):
        print 'Master NeuTu directory', '\'' + neutuDir + '\'', 'found.'
        checkNeuTuDir(neutuDir)
        
    neutuDir = '/opt/Download/neutube'
    if os.path.isdir(neutuDir):
        print 'Install NeuTu directory', '\'' + neutuDir + '\'', 'found.'
        checkNeuTuDir(neutuDir)
        
    print '************Done.'
    
#             print dir(repo)
#             if str(repo.head.commit) != codeInfo['head']:
#                 print 'Wrong branch:'
#                 print '  Actual:', repo.head.commit
#                 print '  Expected:', codeInfo['head']
#                 print 'Failed.'
                
    exit(1)
            
    print os.environ['HOME']
    print os.name
    print platform.system()
    print platform.node()
    
    exit (1)
    
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