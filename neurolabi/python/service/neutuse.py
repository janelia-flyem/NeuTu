from bottle import route, run, get, post, request, static_file, abort, hook, response
import json
import subprocess
import sys
import socket
import jsonschema
import httplib
import socket
import os
import timer
import time
import threading
import datetime
import copy
from Queue import *

sys.path.append('..')
sys.path.append('../flyem')

from dvidenv import DvidEnv
from skeletonizer import Skeletonizer
from dvidurl import DvidUrl
from dvidwriter import DvidWriter
import flyem_data as fd

skl = Skeletonizer()

socket.setdefaulttimeout(1000)

configFile = "../../json/flyem_config.json"
with open(configFile, "r") as fp:
    flyemConfig = json.load(fp)

dvidEnvMap = {}
eventQueue = Queue()
#eventLock = Lock()
dvidWriter = DvidWriter()

for config in flyemConfig["dvid repo"]:
    if config.get("user_name") != "[]":
        dvidEnv = DvidEnv()
        print config
        dvidEnv.loadFlyEmConfig(config)
        dvidEnvMap[config["name"]] = dvidEnv 

def processEvent(event):
    print "Processing event ..."
    print event.dvidEnv
    dvidWriter.setDvidEnv(event.dvidEnv)
    if event.getType() == fd.DataEvent.DATA_INVALIDATE:
        if event.getDataId().getType() == fd.DataId.DATA_BODY:
            print "Invalidating body", event.getDataId().getBodyId()
            dvidWriter.deleteSkeleton(event.getDataId().getBodyId())
            dvidWriter.deleteThumbnail(event.getDataId().getBodyId())
    elif event.getType() == fd.DataEvent.DATA_DELETE:
        if event.getDataId().getType() == fd.DataId.DATA_BODY:
            print "Deleting body data", event.getDataId().getBodyId()
            dvidWriter.deleteSkeleton(event.getDataId().getBodyId())
            dvidWriter.deleteThumbnail(event.getDataId().getBodyId())
            dvidWriter.deleteBodyAnnotation(event.getDataId().getBodyId())
    elif event.getType() == fd.DataEvent.DATA_UPDATE:
        time.sleep(5)
        if event.getDataId().getType() == fd.DataId.DATA_SKELETON:
            print "Skeletionzing body", event.getDataId().getBodyId(), event.dvidEnv
            skl.setDvidEnv(event.dvidEnv)
            print skl.getDvidEnv()
            skl.skeletonize(event.getDataId().getBodyId())
            
def process():
    eqcopy =  []
    while True:
        try:
            elem = eventQueue.get(block=False)
        except Empty:
            break
        else:
            eqcopy.append(elem)

    for i, e in reversed(eqcopy):
        print  e.getDataId().getBodyId(), i


    while True:
        try:
            event = eventQueue.get()
            print datetime.datetime.now(), "Processing event ...", event
            processEvent(event)
        except Exception as e:
            print e

        #threading.Timer(1, process).start()
    
threading.Timer(1, process).start()

def getSchema(service, method):
    with open(service  + '/interface.raml') as f:
        content = f.readlines()
    f.close()
    
    if not content:
        raise Exception("Invalid schema")

    #print content

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

@get('/home')
def home():
    return '<h1>Welcome to the skeletonization service</h1>'


@get('/skeletonize')
def skeletonize():
    response = '''
        <form action="/skeletonize" method="post">
            Body ID: <input name="bodyId" type="text"/>
            <input value="Submit" type="submit"/>
            '''
    response += "<select name=\"database\">"
    for name in sorted(dvidEnvMap):
        response += "<option value=\"" + name + "\">" + name + "</option>"
    response += "</select>"

    response += "</form>"

    return response 

@get('/update_body')
def requestBodyUpdate():
    response = '''
        <form action="/update_body" method="post">
            Body ID: <input name="bodyId" type="text"/>
            <input value="Submit" type="submit"/>
            '''
    response += "<select name=\"database\">"
    for name in sorted(dvidEnvMap):
        response += "<option value=\"" + name + "\">" + name + "</option>"
    response += "</select>"

    response += "</form>"

    return response 

@post('/update_body')
def updateBody():
    print request.content_type
    bodyArray = [];
    dvidEnv = None
    #dvidServer = getDefaultDvidServer()
    #uuid = getDefaultUuid()
    option = None
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
        dvidName = request.forms.get('database')
        dvidEnv = dvidEnvMap[dvidName]
        bodyArray = [int(bodyId) for bodyId in bodyIdStr.split()]
    elif request.content_type == 'application/json':
        print request.json
        jsonObj = request.json
        try:
            schema = getSchema('update_body', 'post')
            #print schema
            jsonschema.validate(jsonObj, json.loads(schema))
        except jsonschema.exceptions.ValidationError as inst:
            print 'Invalid json input'
            print inst
            return '<p>Update for ' + str(bodyArray) + ' failed.</p>'
        bodyArray = jsonObj.get('bodies')
        #dvidServer = jsonObj.get('dvid-server')
        option = jsonObj.get('option')
        #uuid = jsonObj['uuid']
        #config = {'dvid-server': dvidServer, 'uuid': uuid}
        dvidEnv = DvidEnv()
        dvidEnv.loadServerConfig(jsonObj)
        print dvidEnv

    if not option:
        option = "update"

    for bodyId in bodyArray:
        if option == "delete":
            event = fd.DataEvent(fd.DataEvent.DATA_DELETE, fd.DataId(fd.DataId.DATA_BODY, bodyId), dvidEnv)
            eventQueue.put(event)
            print "+++Event added:", event

        if option == "update" or option == "invalidate":
            event = fd.DataEvent(fd.DataEvent.DATA_INVALIDATE, fd.DataId(fd.DataId.DATA_BODY, bodyId), dvidEnv)
            eventQueue.put(event)
            print "+++Event added:", event

        if option == "update":
            event = fd.DataEvent(fd.DataEvent.DATA_UPDATE, fd.DataId(fd.DataId.DATA_SKELETON, bodyId), dvidEnv)
            eventQueue.put(event)
            print "+++Event added:", event

@post('/skeletonize')
def do_skeletonize():
    print request.content_type
    bodyArray = [];
    #dvidServer = getDefaultDvidServer()
    #uuid = getDefaultUuid()
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
        dvidName = request.forms.get('database')
        skl.setDvidEnv(dvidEnvMap[dvidName])
        print skl.getDvidEnv()
        bodyArray = [int(bodyId) for bodyId in bodyIdStr.split()]
    elif request.content_type == 'application/json':
        print request.json
        jsonObj = request.json
        try:
            jsonschema.validate(jsonObj, json.loads(getSchema('skeletonize', 'post')))
        except jsonschema.exceptions.ValidationError as inst:
            print 'Invalid json input'
            print inst
            return '<p>Skeletonization for ' + str(bodyArray) + ' failed.</p>'
        uuid = jsonObj['uuid']
        if jsonObj.has_key('dvid-server'):
            dvidServer = jsonObj['dvid-server']
        bodyArray = jsonObj['bodies']
        config = {'dvid-server': dvidServer, 'uuid': uuid}

        skl.loadDvidConfig(config)
    
    output = {}

    dvidUrl = DvidUrl(skl.getDvidEnv())

    for bodyId in bodyArray:
        print dvidUrl.getServerUrl()
        conn = httplib.HTTPConnection(dvidUrl.getServerUrl())
        bodyLink = dvidUrl.getSkeletonEndPoint(bodyId)
        print '************', bodyLink
        conn.request("GET", bodyLink)

        outputUrl = dvidUrl.getServerUrl() + bodyLink

        r1 = conn.getresponse()
        if not r1.status == 200:
            try:
                print "Skeletonizing " + str(bodyId)
                skl.skeletonize(bodyId)
                output[str(bodyId)] = outputUrl
            except Exception as inst:
                return '<p>' + str(inst) + '</p>'
        else:
            print "skeleton is ready for " + str(bodyId)
            output[str(bodyId)] = outputUrl
    
    print output
    return json.dumps(output, sort_keys = False)
#     return '<p>Skeletonization for ' + str(bodyArray) + ' is completed.</p>'

@get('/hotspot')
def computing_hotspot():
    return '''
        <form action="/hotspot" method="post">
            Body ID: <input name="bodyId" type="text"/>
            <input value="Submit" type="submit"/>
        </form>
    '''

@post('/hotspot')
def compute_hotspot():
    print request.content_type
    bodyArray = [];
    dvidServer = getDefaultDvidServer()
    uuid = getDefaultUuid()
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
        dvidName = requrest.forms.get('database')
        bodyArray = [int(bodyId) for bodyId in bodyIdStr.split()]
    elif request.content_type == 'application/json':
        print request.json
        jsonObj = request.json
        try:
            jsonschema.validate(jsonObj, json.loads(getSchema('skeletonize', 'post')))
        except jsonschema.exceptions.ValidationError as inst:
            print 'Invalid json input'
            print inst
            return '<p>Hotspot computation for ' + str(bodyArray) + ' failed.</p>'
        uuid = jsonObj['uuid']
        if jsonObj.has_key('dvid-server'):
            dvidServer = jsonObj['dvid-server']
        bodyArray = jsonObj['bodies']
    
    output = {}
    config = {'dvid-server': dvidServer, 'uuid': uuid}

    print '********'
    print config
    
    global qualityAnalyzer

    for bodyId in bodyArray:
        #conn = httplib.HTTPConnection(dvidServer)
        bodyLink = '/api/node/' + uuid + '/skeletons/' + str(bodyId) + '.swc'
        print '************', bodyLink
        #conn.request("GET", bodyLink)

        #r1 = conn.getresponse()
        #if not r1.status == 200:
        #    return qualityAnalyzer.computeHotSpot(bodyId).toJsonString()
        return qualityAnalyzer.computeHotSpot(bodyId)

@get('/skeleton/<bodyId>')
def retrieveSkeleton(bodyId):
    return static_file(str(bodyId) + '.swc',
                       root='/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/data_release/bundle1/swc')

@get('/thumbnail/<bodyId>')
def retrieveThumbnail(bodyId):
    return static_file(str(bodyId) + '.tif',
                       root='/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/skeletonization/session25/500k+/len40_100k+/thumbnails')

@hook('after_request')
def enable_cors(fn=None):
    def _enable_cors(*args, **kwargs):
        print 'enable cors'
        response.headers['Access-Control-Allow-Origin'] = '*'
        response.headers['Access-Control-Expose-Headers'] = 'Content-Type'
        response.headers['Access-Control-Allow-Methods'] = 'GET, POST, PUT, OPTIONS'
        response.headers['Access-Control-Allow-Headers'] = 'Origin, Accept, Content-Type, X-Requested-With, X-CSRF-Token'
        if request.method != 'OPTIONS':
            return fn(*args, **kwargs)
    return _enable_cors

#@hook('after_request')
#def enable_cors():
#    response.headers['Access-Control-Allow-Origin'] = '*'
#    response.headers['Access-Control-Allow-Methods'] = 'GET, POST, PUT, OPTIONS'
#    response.headers['Access-Control-Allow-Headers'] = 'Origin, Accept, Content-Type, X-Requested-With, X-CSRF-Token'

@route('/interface/interface.raml', method=['GET', 'OPTIONS'])
@enable_cors
def retrieveRaml():
    print 'retrieve raml'
    fileResponse = static_file('interface.raml', root='.', mimetype='application/raml+yaml')
    fileResponse.headers['Access-Control-Allow-Origin'] = '*'

    return fileResponse
    #with open('interface.raml', 'r') as ramlFile:
    #    ramlContent = ramlFile.read()
    #return ramlContent

def get_json_post():
    try:
        return json.load(request.body)
    except ValueError:
        abort(400, 'Bad request: Could not decode request body (expected JSON).')
        
@post('/json')
def parseJson():
    data = get_json_post()
    return '<p>' + data['head'] + '</p>'

port = 8080
if len(sys.argv) > 1:
    port = sys.argv[1]

run(host=socket.gethostname(), port=port, debug=True)

# print getSchema('skeletonize', 'post')
# try:
#     jsonschema.validate({"bodies": [1, 2, 3]}, json.loads(getSchema('skeletonize', 'post')))
# except jsonschema.exceptions.ValidationError as inst:
#     print 'Invalid json input'
#     print inst
