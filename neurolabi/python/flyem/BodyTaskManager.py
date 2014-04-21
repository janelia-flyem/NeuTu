'''
Created on Sep 18, 2013

@author: zhaot
'''
import os;

class ExtractBodyTaskManager:
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        self.commandPath = '';
        self.minSize = 0;
        self.maxSize = -1;
        self.overwriteLevel = 1
        #self.zOffset = 0
        self.zRange = None
        self.bodyMapDir = ''
        self.targetContainer = 'stacked'
        self.output = ''
        self.bodysizeFile = ''
        self.jobNumber = 5
        self.skipFileFlag = None
        
    def setCommandPath(self, path):
        self.commandPath = path;
        
    def setRange(self, bodySizeRange):
        self.minSize = bodySizeRange[0];
        self.maxSize = bodySizeRange[1];
        
    def setOverwriteLevel(self, level):
        self.overwriteLevel = level;
        
    #def setZOffset(self, offset):
    #    self.zOffset = offset;
    
    def setJobNumber(self, n):
        self.jobNumber = n;

    def setOutput(self, output):
        self.output = output;
        
    def setBodyMapDir(self, inputBodyMap):
        self.bodyMapDir = inputBodyMap

    def setTargetContainer(self, container):
        self.targetContainer = container;
        
    def setBodySizeFile(self, filePath):
        self.bodysizeFile = filePath
        
    def useCluster(self, using):
        self.usingCluster = using;

    def setSkipFileFlag(self, flag):
        self.skipFileFlag = flag

    def setZRange(self, z0, z1):
        self.zRange = [z0, z1]
        
    def getFullCommand(self):
        command = self.commandPath + ' ' + self.bodyMapDir + \
                ' -o ' + self.output + \
                ' --sobj' + ' --minsize ' + str(self.minSize);
        if self.maxSize >= self.minSize:
            command += ' --maxsize ' + str(self.maxSize)
        command += ' --overwrite_level ' + str(self.overwriteLevel);
        if self.bodysizeFile:
            command += ' --bodysize_file ' + self.bodysizeFile
        #command += ' --z_offset ' + str(self.zOffset)
        if self.zRange:
            command += ' --range ' + str(self.zRange[0]) + ' ' + \
                    str(self.zRange[1])
        command += ' --stacked_dir ' + self.targetContainer
        
        return command;
        
    def generateScript(self, script):
        scriptFile = open(script, 'w')
        if scriptFile:
            if self.skipFileFlag:
                scriptFile.write('if [ ! -f "' + self.skipFileFlag + '" ]; then\n')
            scriptFile.write(self.getFullCommand())
            scriptFile.write('\n')
            if self.skipFileFlag:
                scriptFile.write('fi\n')
        scriptFile.close()

if __name__ == '__main__':
    from os.path import expanduser
    home = expanduser("~")
    
    taskManager = ExtractBodyTaskManager()
    taskManager.setBodyMapDir('../body_maps')
    taskManager.setOutput('.')
    taskManager.setRange([100000, -1])
    taskManager.setOverwriteLevel(1)
    taskManager.setBodySizeFile('bodysize.txt')
    taskManager.setZOffset(1490)

    taskManager.setCommandPath(home + '/Work/neutube/neurolabi/cpp/'
                               'extract_body-build-Qt_4_8_1_gcc-Debug/extract_body');
    print taskManager.getFullCommand();
