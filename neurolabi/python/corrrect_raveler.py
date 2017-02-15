import json
from optparse import OptionParser

parser = OptionParser();
parser.add_option("-i", "--input", dest="input", help="input file");
parser.add_option("-o", "--output", dest="output", help="output file");

(options, args) = parser.parse_args();
    
bodyListFile = open(options.input);
bodyList = json.load(bodyListFile);

for neuron in bodyList['data']:
    neuron['position'][1] -= 90
    
bodyListFile.close()
    
outputFile = open(options.output, 'w')
json.dump(bodyList, outputFile, indent = 2)
outputFile.close()
