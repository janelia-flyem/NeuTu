import json
import yaml
import argparse
import subprocess
import sys
import re

from pyzem.dvid import dvidenv
from pyzem.dvid import dvidio
from pyzem.compute import bodysplit

def runTask(neutu, task, config):
    args = [neutu, '--command', '--general', config, task, '-o', 'http://zhaot-ws1:9000/api/node/194d', task]
    p = subprocess.Popen(args)
    p.wait()
    return p

def isTask(task, filterReg):
    if not filterReg:
        return True

    if filterReg.match(task):
        return True

    return False

parser = argparse.ArgumentParser(description='Process arguments for running splitting service')
parser.add_argument('--config', dest='config', type=str, help='Configuration file in YAML format')
parser.add_argument('--command', dest='command', type=str, help='Command file in Json format')
parser.add_argument('--filter', dest='filter', type=str, help='Regular expression of filtering tasks')
parser.add_argument('--info', help='Show task information', action='store_true')
args=parser.parse_args()

print('Arguments:', args)

commandFile = args.command

with open(args.config, 'r') as fp:
    config = yaml.load(fp)

taskServer = config['task_server']

taskEnv = dvidenv.DvidEnv(host = taskServer['host'], port=taskServer['port'], uuid=taskServer['uuid'])
dvidUrl = dvidenv.DvidUrl(taskEnv)
nodeAddress = dvidUrl.get_node_url()
split = bodysplit.BodySplit(config['command'], taskEnv)
split.set_committing(config.get('commit', False))
print(split._neutu)
print(split._commit)
print(taskEnv)

taskFilter = args.filter

filterReg = None
if taskFilter:
    filterReg = re.compile(taskFilter)

#Read tasks
dc = dvidio.DvidClient(env = taskEnv)
#dc.print_split_task()
splitTaskList = dc.read_split_task_keys()
count = 0
for task in splitTaskList:
    if isTask(task, filterReg):
        count += 1
        print('>>>>', count, task)
        if not args.info and commandFile:
            taskUrl = dvidUrl.get_url(dvidUrl.get_split_task_path(task))
            runTask(split._neutu, taskUrl, commandFile)

print(count, 'tasks found.')
