from bottle import route, run, get, post, static_file, abort 
import json
import subprocess
import sys
import socket
import jsonschema
import httplib
import socket
import os

sys.path.append('..')
sys.path.append('module')
sys.path.append('flyem')

socket.setdefaulttimeout(1000)


def getDefaultDvidServer():
    return 'emdata1.int.janelia.org:7000'

def getDefaultUuid():
    return 'a75'

def getSkeletonServer():
    return 'emrecon100.janelia.priv:8082'

def getSchema(service, method):
    ramlPath = os.path.join(os.path.dirname(os.path.abspath(os.path.realpath(__file__))), service, 'interface.raml')
    with open(ramlPath) as f:
        content = f.readlines()
    f.close()
    
    serviceHead = False
    methodStart = False
    schemaStart = False
    objectLevel = 0
    schema =''
    
    for line in content:
        if schemaStart:
            schema += line
            if line.find('{') >= 0:
                objectLevel += 1
            if line.find('}') >= 0:
                objectLevel -= 1
            if objectLevel == 0:
                break
        line = line.strip(' \t\n\r')
        if methodStart:
            if line == 'schema: |':
                schemaStart = True 
        if serviceHead:
            methodStart = True
        if line == '/' + service + ":":
            serviceHead = True
                
    return schema

def home(service):
    return '<h1>Welcome to the', service, 'service</h1>'

def get_json_post(request):
    try:
        return json.load(request.body)
    except ValueError:
        abort(400, 'Bad request: Could not decode request body (expected JSON).')
        
def parseJson():
    data = get_json_post()
    return '<p>' + data['head'] + '</p>'

# print getSchema('skeletonize', 'post')
# try:
#     jsonschema.validate({"bodies": [1, 2, 3]}, json.loads(getSchema('skeletonize', 'post')))
# except jsonschema.exceptions.ValidationError as inst:
#     print 'Invalid json input'
#     print inst
