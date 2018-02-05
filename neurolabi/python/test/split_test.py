import json
import yaml
import argparse
import subprocess

from pyzem.dvid import dvidenv
from pyzem.dvid import dvidio
from pyzem.compute import bodysplit

def runTask(neutu, task, config):
    args = [neutu, '--command', '--general', config, task, '-o', 'http://zhaot-ws1:9000/api/node/194d', task]
    p = subprocess.Popen(args)
    p.wait()
    return p

parser = argparse.ArgumentParser(description='Process arguments for running splitting service')
parser.add_argument('--config', dest='config', type=str, help='Configuration file in YAML format')
parser.add_argument('--info', help='Show task information', action='store_true')
args=parser.parse_args()
print(args.config)

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

#Read tasks
dc = dvidio.DvidClient(env = taskEnv)
#dc.print_split_task()
splitTaskList = dc.read_split_task_keys()
count = 0
for task in splitTaskList:
    if task.startswith('task__http-++emdata3.int.janelia.org-8300+api+node+2c15') or task.startswith('task__http-++emdata3-8300+api+node+2c15'):
        print(task)
        taskUrl = dvidUrl.get_url(dvidUrl.get_split_task_path(task))
        runTask(split._neutu, taskUrl, 'split_command.json')
        count += 1

print(count, 'tasks found.')
