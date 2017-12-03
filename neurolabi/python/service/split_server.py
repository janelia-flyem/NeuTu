import timer
import threading
import time
import argparse
import yaml

from pyzem.dvid import dvidenv
from pyzem.dvid import dvidio
from pyzem.compute import bodysplit

parser = argparse.ArgumentParser(description='Process arguments for running splitting service')
parser.add_argument('--config', dest='config', type=str, help='Configuration file in YAML format')
parser.add_argument('--info', help='Show task information', action='store_true')
args=parser.parse_args()
print(args.config)

with open(args.config, 'r') as fp:
    config = yaml.load(fp)

taskServer = config['task_server']

taskEnv = dvidenv.DvidEnv(host = taskServer['host'], port=taskServer['port'], uuid=taskServer['uuid'])
split = bodysplit.BodySplit(config['command'], taskEnv)
split.set_committing(config.get('commit', False))
print(split._neutu)
print(split._commit)
#split.run('task__http-++emdata1.int.janelia.org-8500+api+node+b6bc+bodies+sparsevol+12007338')

dc = dvidio.DvidClient(env = taskEnv)
if args.info:
    dc.print_split_task()
    exit()

splitTaskList = dc.read_split_task_keys()

def process():
    while True:
        splitTaskList = dc.read_split_task_keys()
        print(splitTaskList)
        for task in splitTaskList:
            split.run(task)
        time.sleep(10)

process()
#threading.Timer(1, process).start()

