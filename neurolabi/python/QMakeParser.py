import re;
import os;

# parsingState = enumerate(['Null', 'Pri', 'Header', 'Source'])
# class QMakeParserState:
#     def __init__(self):
#         self.state = 0;
#         self.tokenList = list();
#         
#     def process(self, line):
#         return transit(toToken(line));
# 
# class QMakeParserNullState(QMakeParserState):
#     def transit(self, tokenList):
#         if tokenList[0] == "include":
#             return QMakeParserPriState(tokenList);
#         elif tokenList[0] == "HEADERS":
#             return QMakeParserHeaderState(tokeList);
#         elif tokenList[0] == "SOURCES":
#             return QMakeParserHeaderState(tokeList);
#         else:
#             self.tokenList = tokeList;
#             return self;
#         
# class QMakeParserPriState(QMakeParserState):
#     def transit(self, tokenList):
          
    
class QMakeParser:
    def __init__(self):
        self.qtPath = "";
        self.header = list();
        self.source = list();
        
    def clear(self):
        self.header = list();
        self.source = list();
        
    def parse(self, qProFile):
        #Get the directory of the file
        qProFileDir = os.path.dirname(qProFile); 
        f = open(qProFile);
        allLine = f.readlines();
        f.close();

        for line in allLine:
            line = line.replace("$${PWD}", qProFileDir);
            print line
            #For each line
            tokenList = re.findall("[\w\/\.]+", line);
            #for each word in the file
            for token in tokenList:
                self.add(token);
        
    def add(self, filePath):
        if filePath.endswith(".h") or filePath.endswith(".hxx") or filePath.endswith("hpp"):
            self.addHeader(filePath);
        elif filePath.endswith(".c") or filePath.endswith(".cxx") or filePath.endswith("cpp"):
            self.addSource(filePath);
        elif filePath.endswith(".pri"):
            self.parse(filePath);
            
    def addHeader(self, filePath):
        self.header.append(filePath);
        
    def addSource(self, filePath):
        self.source.append(filePath);
        
    def display(self):
        for header in self.header:
            print(header);
        for source in self.source:
            print(source);
                                   
if __name__ == '__main__':
    parser = QMakeParser();
    parser.parse("/Users/zhaot/Work/neutube/neurolabi/gui/gui_free.pri");
    parser.display();
