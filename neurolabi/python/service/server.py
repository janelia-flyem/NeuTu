from bottle import route, run, get, post, request, static_file, abort, hook, response
import json
import subprocess
import sys
import socket
import jsonschema
import httplib
import socket
import os

sys.path.append('..')
sys.path.append('../module')
sys.path.append('../flyem')

socket.setdefaulttimeout(1000)

import skeletonize as skl
import quality_analyzer as qa

qualityAnalyzer = qa.QualityAnalyzer()
qualityAnalyzer.loadDataBundle(os.path.join(os.getcwd(), '../../data/flyem/FIB/data_release/bundle5/data_bundle.json'))

def getDefaultDvidServer():
    return 'emdata1.int.janelia.org:7000'

def getDefaultUuid():
    return 'dcf'

def getSchema(service, method):
    with open(service  + '/interface.raml') as f:
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

@get('/home')
def home():
    return '<h1>Welcome to the skeletonization service</h1>'

@get('/skeletonize')
def skeletonize():
    return '''
        <form action="/skeletonize" method="post">
            Body ID: <input name="bodyId" type="text"/>
            <input value="Submit" type="submit"/>
        </form>
    '''

@post('/skeletonize')
def do_skeletonize():
    print request.content_type
    bodyArray = [];
    dvidServer = getDefaultDvidServer()
    uuid = getDefaultUuid()
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
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
    
    output = {}
    config = {'dvid-server': dvidServer, 'uuid': uuid}

    print '********'
    print config
    
    for bodyId in bodyArray:
        conn = httplib.HTTPConnection(dvidServer)
        bodyLink = '/api/node/' + uuid + '/skeletons/' + str(bodyId) + '.swc'
        print '************', bodyLink
        conn.request("GET", bodyLink)

        r1 = conn.getresponse()
        if not r1.status == 200:
            try:
                skl.Skeletonize(bodyId, 'dvid', config)
                output[str(bodyId)] = dvidServer + bodyLink
            except Exception as inst:
                return '<p>' + str(inst) + '</p>'
        else:
            output[str(bodyId)] = dvidServer + bodyLink
    
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

run(host=socket.gethostname(), port=8080, debug=True)

# print getSchema('skeletonize', 'post')
# try:
#     jsonschema.validate({"bodies": [1, 2, 3]}, json.loads(getSchema('skeletonize', 'post')))
# except jsonschema.exceptions.ValidationError as inst:
#     print 'Invalid json input'
#     print inst
