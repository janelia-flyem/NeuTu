#!/usr/bin/python

from optparse import OptionParser
import json
from pprint import pprint

#Take options
parser = OptionParser()
parser.add_option("-f", "--file", dest="filename", help="configuration file",
        metavar="FILE")
(options, args) = parser.parse_args()

print options.filename

for arg in args:
    print arg


json_data = open(config_file);
data = json.load(json_data);
json_data.close();

#Generate preparation file (flyem_prepare_tmp)
prepareFile = open(prepareFilePath, 'w');
f.write('!/usr/bin/bash\n');

prepareFile.close();

#Make len subfolder

#Generate skeletonize file (flyem_skeletonize_tmp)



