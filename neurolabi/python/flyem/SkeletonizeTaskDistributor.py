'''
Created on Sep 18, 2013

@author: zhaot
'''
import os;

class SkeletonizeTaskDistributor:
    '''
    classdocs
    '''

    def __init__(self):
        '''
        Constructor
        '''
        self.commandPath = '';
        self.bodyList = list();
        self.bodyDir = '';
        self.swcDir = '';
        self.jobNumber = 5;
        self.usingCluster = False;
        self.dsIntv = [1, 1, 1];
        self.interpolating = False;
        self.rebasing = True;
        self.keepingShort = True;
        self.minObjSize = 20;
        self.minLength = 40;
        self.fillingHole = True;
        self.subscripts = []
        self.masterScript = None
        #self.args = '--intv 1 1 1 --interpolate --rebase --keep_short --minobj 20'
        
    def setCommandPath(self, path):
        self.commandPath = path;
        
    def setBodyList(self, bodyList):
        self.bodyList = bodyList;
        
    def setBodyDir(self, path):
        self.bodyDir = path;
        
    def setSwcDir(self, path):
        self.swcDir = path;
    
    def setJobNumber(self, n):
        self.jobNumber = n;

    def setArgs(self, args):
        if args.has_key('ds_intv'):
            self.dsIntv = args['ds_intv']
        if args.has_key('minlen'):
            self.minLength = args['minlen']
        if args.has_key('minobj'):
            self.minObjSize = args['minobj']
        if args.has_key('fill_hole'):
            self.fillingHole = args['fill_hole']
        if args.has_key('interpolate'):
            self.interpolating = args['interpolate']
        if args.has_key('rebase'):
            self.rebasing = args['rebase']
        if args.has_key('keep_short'):
            self.keepingShort = args['keep_short']
        
    def useCluster(self, using):
        self.usingCluster = using;
        
    def getMinLength(self):
        return self.minLength

    def getSubscript(self):
        return self.subscripts

    def getMasterScript(self):
        return self.masterScript

    def getFullCommand(self, index):
        if self.bodyDir.endswith('.hf5'):
            bodyFile = self.bodyDir + ':/' + str(self.bodyList[index]) + '.sobj';
        else:
            bodyFile = self.bodyDir + '/' + str(self.bodyList[index]) + '.sobj';
        swcFile = self.swcDir + '/' + str(self.bodyList[index]) + '.swc';
        command = self.commandPath + ' ' + bodyFile  + ' -o ' + swcFile;
        if self.dsIntv[0] > 0 or self.dsIntv[1] > 0 or self.dsIntv[2] > 0:
            command += ' --intv ' + str(self.dsIntv[0]) + ' ' + \
                    str(self.dsIntv[1]) + ' ' + str(self.dsIntv[2])
        if self.rebasing:
            command += ' --rebase '
        if self.keepingShort:
            command += ' --keep_short '
        if self.fillingHole:
            command += ' --fill_hole '
        command += ' --minobj ' + str(self.minObjSize)
        command += ' --minlen ' + str(self.minLength)
        return command;
        
    def generateScript(self, outputDir):
        #split bodies
        self.subscripts = list();
        subscriptFile = list();
        jobNumber = min(len(self.bodyList), self.jobNumber);
        for i in range(0, jobNumber):
            self.subscripts.append(outputDir + '/skeletonize_' + str(i + 1) + '.sh');
            subscriptFile.append(open(self.subscripts[i], "w"));
        
        index = 0;
        print "Job Number: ", jobNumber
        while index < len(self.bodyList):
            for i in range(0, jobNumber):
                bodyId = self.bodyList[index];
                swcFile = self.swcDir + '/' + str(bodyId) + '.swc';
                subscriptFile[i].write('touch ' + swcFile + '.process\n');
                subscriptFile[i].write('if [ ! -f ' + swcFile + ' ]; then\n');
                subscriptFile[i].write('  ' + self.getFullCommand(index) + '\n');
                subscriptFile[i].write('fi\n');
                index = index + 1;
                if index >= len(self.bodyList):
                    break;
        
        for i in range(0, jobNumber):
            subscriptFile[i].write('touch ' + os.path.abspath(self.subscripts[i]) + '.done\n');
            subscriptFile[i].close();
            
        self.masterScript = outputDir + '/run.sh';
        f = open(self.masterScript, "w");
        for i in range(0, jobNumber):
            shcommand = 'sh ' + os.path.abspath(self.subscripts[i]) + ' > ' + os.path.abspath(self.subscripts[i]) + '.out';
            if self.usingCluster:
                shcommand = 'qsub -P flyemproj -N em_skeleton -j y -o /dev/null -b y -cwd -V ' + "'" + shcommand + "'";
            else:
                shcommand = shcommand + " &";
            f.write(shcommand + '\n');
        f.close();

if __name__ == '__main__':
    from os.path import expanduser
    home = expanduser("~")

    distr = SkeletonizeTaskDistributor();
    distr.setBodyList([1, 2]);
    distr.setCommandPath(home + '/Work/neutube/neurolabi/cpp/skeletonize-build-Qt_4_8_1_gcc-Debug/skeletonize');
    print distr.getFullCommand(0);
