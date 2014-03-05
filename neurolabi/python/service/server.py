from bottle import route, run, get, post, request, static_file, abort, hook, response
import json
import subprocess
import sys

sys.path.append('..')
sys.path.append('module')
sys.path.append('flyem')

import skeletonize as skl


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
    bodyId = request.forms.get('bodyId');
    skl.Skeletonize(bodyId, 'dvid')
    
    return '<p>Skeletonization for ' + bodyId + ' is completed.</p>'

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

run(host='zhaot-ws1.janelia.priv', port=8880, debug=True)
