from bottle import route, run, get, post, request, static_file, abort, hook, response
import json
import subprocess
import sys
import socket
import jsonschema
import httplib
import socket
import os
import argparse

sys.path.append('..')
sys.path.append('../..')
sys.path.append('../../module')
sys.path.append('../../flyem')

socket.setdefaulttimeout(1000)

import quality_analyzer as qa
import neutu_server as ns

qualityAnalyzer = qa.QualityAnalyzer()
#qualityAnalyzer.loadDataBundle(os.path.join(os.getcwd(), '../../../data/flyem/FIB/data_release/bundle5/data_bundle.json'))

@get('/home')
def home():
    return '<h1>Welcome to the hot spot service</h1>'

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
    dvidServer = ns.getDefaultDvidServer()
    uuid = ns.getDefaultUuid()
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
        bodyArray = [int(bodyId) for bodyId in bodyIdStr.split()]
    elif request.content_type == 'application/json':
        print request.json
        jsonObj = request.json
        try:
            jsonschema.validate(jsonObj, json.loads(ns.getSchema('hotspot', 'post')))
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
#        bodyLink = '/api/node/' + uuid + '/skeletons/' + str(bodyId) + '.swc'
#        print '************', bodyLink
        result = None
        try:
            result = qualityAnalyzer.computeHotSpot(bodyId)
        except Exception as e:
            result = {"error": str(e)}

        return result

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

if __name__ == '__main__': 
    print 'Starting the server ...'
    parser = argparse.ArgumentParser();
    parser.add_argument("--port", dest="port", type = int, help="port", default=8081);
    args = parser.parse_args()
    run(host=socket.gethostname(), port=args.port, debug=True)

#run(host=socket.gethostname(), port=8081, debug=True)

# print getSchema('skeletonize', 'post')
# try:
#     jsonschema.validate({"bodies": [1, 2, 3]}, json.loads(getSchema('skeletonize', 'post')))
# except jsonschema.exceptions.ValidationError as inst:
#     print 'Invalid json input'
#     print inst
