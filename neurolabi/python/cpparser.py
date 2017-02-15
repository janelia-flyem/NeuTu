import os
from optparse import OptionParser
from sets import Set
import re

class DotNode:
    def __init__(self, name = '', label = '', shape = ''):
        self._name = name
        self._label = label
        self._shape = shape
    
    def isEmpty(self):
        return not self._name.stripe()
    
    def toString(self):
        dotString = self._name + ' ['
        
        if self._label:
#             print "Label: ", self._label
            dotString += 'label = "' + self._label + '",'
            
        if self._shape:
            dotString += 'shape = "' + self._shape + '"'
        
        dotString += '];'
        
        return dotString
    
    def toLine(self):
        return self.toString() + '\n'

class DotEdge:
    def __init__(self, v1 = None, v2 = None, arrowhead = ''):
        self._v1 = v1
        self._v2 = v2
        self._arrowhead = arrowhead
        
    def toString(self):
        v1 = self._v1
        if isinstance(self._v1, DotNode):
            v1 = self._v1._name
        
        v2 = self._v2
        if isinstance(self._v2, DotNode):
            v2 = self._v2._name
            
        value = v1 + ' -> ' + v2
        if self._arrowhead:
            value += ' [arrowhead = ' + self._arrowhead + '];'
        return value
    
    def toLine(self):
        return self.toString() + '\n'
        
class DotGraph:
    def __init__(self):
        self._nodeList = []
        self._edgeList = []
        
    def addNode(self, node):
        self._nodeList.append(node)
        
    def addEdge(self, edge):
        self._edgeList.append(edge)
        
    def dumps(self, ps = []):
        dotString = 'digraph {\n'
        lineSet = Set()
        for node in self._nodeList:
            line = node.toLine()
            if not line in lineSet:
                dotString += ' ' + line
                lineSet.add(line)
        for edge in self._edgeList:
            line  = edge.toLine()
            if not line in lineSet:
                dotString += ' ' + line
                lineSet.add(line)
                
        for line in ps:
            if not line in lineSet:
                dotString += ' ' + line + '\n'
                lineSet.add(line)
            
        dotString += '}'
        
        return dotString
    
    def export(self, filePath, ps = []):
        f = open(filePath, 'w')
        if f:
            f.write(self.dumps(ps))
            f.close()
        
class CppClass:
    def __init__(self, name = ''):
        self._name = name
        self._parentList = []
    
    def toDotNode(self):
        node = DotNode(name = self._name, label = self._name, shape = 'box')
        return node
    
    def toEdgeList(self):
        edgeList = []
        for parent in self._parentList:
            edge = DotEdge(v2 = self._name, v1 = parent)
            edgeList.append(edge)
            
        return edgeList
        
class CppFunction:
    def __init__(self, host = None, name = '', isSignal = False, isInterface = False):
        self._host = host
        if name.startswith('@'): #signal-slot
            name = name[1:]
            isSignal = True
            
       #if name.startswith('$'):
            
        self._name = name
        self._isSignal = isSignal
    
    def toDotNodeName(self):
        return self._host + '__' + self._name
    
    def toDotNode(self):
        node = DotNode(name = self.toDotNodeName(), label = self._name)
        if self._isSignal:
            node._shape = 'none'
            node._label = self._name + '()';
            
        return node
    
    def toDotEdge(self):
        if self._host:
            return DotEdge(v1 = self._host, v2 = self.toDotNodeName(), arrowhead = 'odot')
        else:
            return None
    
    def toDotString(self):
        return self.toDotNode().toString()
        
class CppStatement:
    def __init__(self, value = '', name = ''):
        self._name = name
        self._value = value
        
    def toDotNode(self):
        return DotNode(name = self._name, label = self._value, shape = 'none')
        
class QtConnection:
    def __init__(self, signal = None, slot = None):
        self._signal = signal
        self._slot = slot
    
    def toDotEdge(self):
        return DotEdge(v1 = self._signal.toDotNode(), v2 = self._slot.toDotNode())
    
class FunctionCall:
    def __init__(self, caller = None, callee = None):
        self._caller = caller
        self._callee = callee
        
    def toDotEdge(self):
#         print self._caller.toDotNode()
#         print self._callee.toDotNode()
        return DotEdge(v1 = self._caller.toDotNode(), 
                       v2 = self._callee.toDotNode(), arrowhead = 'onormal')    

class FunctionChain:
    def __init__(self, prev = None, next = None):
        self._prev = prev
        self._next = next
        
    def toDotEdge(self):
        return DotEdge(v1 = self._prev.toDotNode(), 
                       v2 = self._next.toDotNode(), arrowhead = 'none')    
    
class ClassComposition:
    def __init__(self, host = '', guest = ''):
        self._host = host
        self._guest = guest
    
    def toDotEdge(self):
        return DotEdge(v1 = self._host, v2 = self._guest, arrowhead = 'diamond')

class ClassAggregation:
    def __init__(self, host = '', guest = ''):
        self._host = host
        self._guest = guest
    
    def toDotEdge(self):
        return DotEdge(v1 = self._host, v2 = self._guest, arrowhead = 'odiamond')

def generateToken(line):
    tokens = []
    
    token1 = line.split('"')
    
    for i in range(len(token1)):
        if i % 2 == 0:
            token2 = re.split('[ ={}:,()\r\n\t]', token1[i])
            for token in token2:
                if token:
                    tokens.append(token)
        else:
            tokens.append(token1[i])
    
    return tokens            
    
class TemplateParser():
    def __init__(self):
        self._fileSet = Set()
    
    def importFile(self, filePath):
        baseDir = os.path.dirname(filePath)
        if filePath not in self._fileSet:
            with open(filePath, 'r') as f:
                self._fileSet.add(filePath)
                lineList = f.readlines()
                for line in lineList:
                    if line.startswith('%include'):
                        tokens = line.split()
                        if len(tokens) > 1:
                            if not tokens[1] in self._fileSet:
                                self.importFile(baseDir + '/' + tokens[1])
    
    def readlines(self):
        lines = []
#         print self._fileSet
        for file in self._fileSet:
            with open(file, 'r') as f:
                if file.endswith('.dot'):
                    newlines = f.readlines();
                    for line in newlines:
                        line.strip()
                        if line:
                            if not '}' in line and not '{' in line:
                                lines.append('@' + line)
                else:
                    lines += f.readlines()
        
#         print lines
        return lines
    
if __name__ == '__main__':
    parser = OptionParser();
    parser.add_option("-i", "--input", dest="input", help="input file");
    parser.add_option("-o", "--output", dest="output", help="output file",
                      default = 'stdout');
    (options, args) = parser.parse_args();

    parser = TemplateParser()
    parser.importFile(options.input)
    lineList = parser.readlines()
    
#     print generateToken('class = ZStack')
    classSet = Set()
    functionSet = Set()
    otherEdgeList = []
    stmList = []
    dotLineList = []
     
#     with open(options.input, 'r') as f:
        
#         lineList = f.readlines()
    if lineList:
        content = ''
        isComment = False
         
        for line in lineList:
            line.strip()
            if line.startswith('#'):
                line = ''
            elif line.startswith('%include'):
                line = ''
            elif line.startswith('@'):
                dotLineList.append(line[1:].strip())
            else:
                if not isComment:
                    commentStart = line.find('/*')
                    if commentStart >= 0:
                        line  = line[:commentStart]
                        isComment = True
                    else:
                        commentStart = line.find('//')
                        if commentStart >= 0:
                            line  = line[:commentStart]
                else:
                    commentEnd = line.find('*/')
                    if commentEnd >= 0:
                        line = line[commentEnd+2:]
                        isComment = False
                    else:
                        line = ''
                     
            if line:
                content += line
             
#         print 'Contecnt:'
#         print content
#         print 'End of content'
        
        statementList = content.split(';')
        

        connList = []
        for statement in statementList:
            statement = statement.strip()
#             print 'Statement: ', statement
            if statement:
                print 'Statement: ', statement
#                 tokens = statement.split()
                tokens = generateToken(statement)
                if tokens[0] == 'class':
                    classItem = CppClass(name = tokens[1])
                    subtokens = tokens[2:]
                    for token in subtokens:
                        classItem._parentList.append(token)
                    classSet.add(classItem)
                elif tokens[0] == 'stm':
                    stmList.append(CppStatement(name = tokens[1], value = tokens[2]))
                elif tokens[0] == 'connect':
                    for i in range(len(tokens) - 1):
                        if tokens[i] == 'SIGNAL':
                            tokens[i + 1] = '@' + tokens[i + 1]
                    try:
                        tokens.remove('SIGNAL')
                    except:
                        pass
                    
                    try:
                        tokens.remove('SLOT')
                    except:
                        pass
                    
#                     print tokens
                    classSet.add(CppClass(name = tokens[1]))
                    signal = CppFunction(host = tokens[1], name = tokens[2], isSignal = True)
                    classSet.add(CppClass(name = tokens[3]))
                    slot = CppFunction(host = tokens[3], name = tokens[4])
#                     print slot._name
                    if slot._name.startswith('@'):
                        slot._isSignal = True
                        slot._name = slot._name.replace('@', '')
                    functionSet.add(signal)
                    functionSet.add(slot)
                    connList.append(QtConnection(signal, slot))
                elif tokens[0] == 'function':
                    functionSet.add(CppFunction(host = tokens[1], name = tokens[2]))
                elif tokens[0] == 'call':
                    f1 = CppFunction(host = tokens[1], name = tokens[2])
                    f2 = CppFunction(host = tokens[3], name = tokens[4])
                    functionSet.add(f1)
                    functionSet.add(f2)
                    otherEdgeList.append(FunctionCall(f1, f2))
                elif tokens[0] == 'cmp':
                    for token in tokens[2:]:
                        comp = ClassComposition(host = tokens[1], guest = token)
                        classSet.add(CppClass(name = comp._host))
                        classSet.add(CppClass(name = comp._guest))
                        otherEdgeList.append(comp)
                elif tokens[0] == 'agg':
                    for token in tokens[2:]:
                        comp = ClassAggregation(host = tokens[1], guest = token)
                        classSet.add(CppClass(name = comp._host))
                        classSet.add(CppClass(name = comp._guest))
                        otherEdgeList.append(comp)
                     
    # print classSet
    graph = DotGraph()
    for classItem in classSet:
#         print className
#         classItem = CppClass(name = className)
#         print classItem.toDotNode()
        graph.addNode(classItem.toDotNode())
        edgeList = classItem.toEdgeList()
        if edgeList:
            for edge in edgeList:
                graph.addEdge(edge)
    
    for state in stmList:
#         print state._value
#         node = state.toDotNode()
#         print "Label: ", node._label
        graph.addNode(state.toDotNode())
        
    for func in functionSet:
        graph.addNode(func.toDotNode())
        edge = func.toDotEdge()
        if edge:
            graph.addEdge(edge)
         
    for conn in connList:
        graph.addEdge(conn.toDotEdge())
    
    for conn in otherEdgeList:
        graph.addEdge(conn.toDotEdge())

    if options.output == 'stdout':
        print graph.dumps(ps = dotLineList)
    else:
        graph.export(options.output, ps = dotLineList)
#     print 'digraph{'
#     for classItem in classSet:
#         print ' ', classItem, '[_shape = box];'
#     print '}'
