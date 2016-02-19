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

import skeletonize as skl
import neutu_server as ns

@get('/home')
def home():
    return ns.home('skeletonize')

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
    dvidServer = ns.getDefaultDvidServer()
    uuid = ns.getDefaultUuid()
    if request.content_type == 'application/x-www-form-urlencoded':
        bodyIdStr = request.forms.get('bodyId')
        bodyArray = [int(bodyId) for bodyId in bodyIdStr.split()]
    elif request.content_type == 'application/json':
        print request.json
        jsonObj = request.json
        try:
            jsonschema.validate(jsonObj, json.loads(ns.getSchema('skeletonize', 'post')))
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
    
    output["swc-list"] = []
    output['error'] = []

    print dvidServer
    conn = httplib.HTTPConnection(dvidServer)
    conn.request("GET", '/api/node/' + uuid + '/skeletons/info')
    r1 = conn.getresponse()
    if not r1.status == 200:
        output['error'].append('Cannot connect to the DVID server.')
    else:
        for bodyId in bodyArray:
            bodyLink = '/api/node/' + uuid + '/skeletons/' + str(bodyId) + '.swc'
            print '************', bodyLink
            conn = httplib.HTTPConnection(dvidServer)
            conn.request("GET", bodyLink)
            r1 = conn.getresponse()
            swcAvailable = False
            if not r1.status == 200:
                try:
                    skl.Skeletonize(bodyId, 'dvid', config)
                    print 'skeletons retrieved.'
                except Exception as inst:
                    print str(inst)
                    output['error'].append(str(inst))
                else:
                    swcAvailable = True
            else:
                swcAvailable = True

            if swcAvailable:
                swc = {"id": bodyId, "url": dvidServer + bodyLink}
                output["swc-list"].append(swc)
    
    return json.dumps(output, sort_keys = False)

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

@route('/interface/interface.raml', method=['GET', 'OPTIONS'])
@enable_cors
def retrieveRaml():
    print 'retrieve raml'
    fileResponse = static_file('interface.raml', root='.', mimetype='application/raml+yaml')
    fileResponse.headers['Access-Control-Allow-Origin'] = '*'

    return fileResponse

def get_json_post():
    return ns.get_json_post(request)
        
@post('/json')
def parseJson():
    data = get_json_post()
    return '<p>' + data['head'] + '</p>'

if __name__ == '__main__': 
    print 'Starting the server ...'
    parser = argparse.ArgumentParser();
    parser.add_argument("--port", dest="port", type = int, help="port", default=8082);
    args = parser.parse_args()
    run(host=socket.gethostname(), port=args.port, debug=True)

# print getSchema('skeletonize', 'post')
# try:
#     jsonschema.validate({"bodies": [1, 2, 3]}, json.loads(getSchema('skeletonize', 'post')))
# except jsonschema.exceptions.ValidationError as inst:
#     print 'Invalid json input'
#     print inst
