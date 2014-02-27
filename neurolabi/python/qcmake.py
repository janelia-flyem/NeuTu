import argparse;
import QMakeParser as qp;
import CMakeParser as cp;
from pprint import pprint

#Take options
parser = argparse.ArgumentParser();

parser.add_argument("--qt", type=str)
parser.add_argument("--cmake", type=str)
parser.add_argument("--output", type=str)

# parser.add_argument("echo");
# parser.add_argument("-q", "--qt", dest="filename", metavar = "FILE",
#                    help="Create cmake file from qt source")
# parser.add_option("-c", "--cmake", type = "string")
# parser.add_option("-o", "--output", type = "string");
# (options, args) = parser.parse_args()

args = parser.parse_args();

qmakeParser = qp.QMakeParser();
qmakeParser.display();
 
qmakeFile = args.qt;
qmakeParser.parse(qmakeFile);
qmakeParser.display();
 
cmakeFileTemplate = args.cmake;
cmakeParser = cp.CMakeParser();
cmakeParser.parse(cmakeFileTemplate);
cmakeFile = args.output;
cmakeParser.dump(cmakeFile, qmakeParser.source)