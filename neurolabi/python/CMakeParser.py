import os;

class CMakeParser:
    def __init__(self):
        self.sourceVar = '@SOURCE@';
        self.cmakeList = list();
        
    def parse(self, cmakeFile):
        f = open(cmakeFile);
        self.cmakeList = f.readlines();
        for i, line in enumerate(self.cmakeList):
            self.cmakeList[i] = line.rstrip("\n");
        f.close();
        
    def dump(self, filePath, sourceList):
        outputDir = os.path.dirname(os.path.abspath(filePath));
        outFile = open(filePath, "w");
        for line in self.cmakeList:
            if "@SOURCE@" in line:
                token = line.split('@SOURCE@');
                outFile.write(token[0] + "\n");
                indent = len(line) - len(line.lstrip());
                for source in sourceList:
                    relativeSourcePath = os.path.relpath(source, outputDir)
#                     sourceName = os.path.basename(source);
                    outFile.write(' ' * (indent + 2) + relativeSourcePath + "\n");
                if len(token) > 1:
                    outFile.write(' ' * indent + token[1] + '\n');
            else:
                outFile.write(line + "\n");
                
        outFile.close();
        

if __name__ == '__main__':
    dataDir = '/Users/zhaot/Work/neutube/neurolabi/data';
    parser = CMakeParser();
    parser.parse(dataDir + "/test.txt.qc");
    sourceList = ['/Users/zhaot/Work/neutube/neurolabi/test.cpp', 
                  '/Users/zhaot/Work/neutube/neurolabi/test2.c'];
    parser.dump(dataDir + "/test.txt", sourceList);
    