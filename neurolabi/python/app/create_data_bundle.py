import os
import sys
import argparse
import json

parser = argparse.ArgumentParser()
parser.add_argument('--config', type = str, dest = 'config')
parser.add_argument('--session', type = str, dest = 'session')
parser.add_argument('--swc_dir', type = str, dest = 'swcDir')
parser.add_argument('--body_annotation', type = str, dest = 'bodyAnnotation')
parser.add_argument('--synapse_annotation', type = str, dest = 'synapseAnnotation')

args = parser.parse_args()

home = os.path.expanduser("~");
neulibDir= home + '/Work/neutube/neurolabi';
sys.path.append(neulibDir + '/python/flyem');

from CreateDataBundle import CreateDataBundle;

masterConfigFile = open(args.config)
masterConfig = json.load(masterConfigFile)
masterConfigFile.close()

config = masterConfig['bundle']
config['sessionPath'] = args.session
config['addingClass'] = False
config["swcDir"] = os.path.join(args.session, args.swcDir);
config["output"] = os.path.join(config['swcDir'], '../data_bundle.json');

config["bodyAnnotation"] = os.path.join(config["sessionPath"], args.bodyAnnotation)
config["synapse"] = os.path.join(config['sessionPath'], args.synapseAnnotation)
CreateDataBundle(config);

