import os
import sys
from bottle import run, template, get, post, request, static_file
from subprocess import Popen, PIPE
import json

neutu=r"/home/deli/NeuTu/neurolabi/build-gui-Desktop_Qt_5_6_2_GCC_64bit-flyem/neutu"


@get('/surfrecon/result/<fid:path>')
def get_result(fid):
    return static_file(fid, root='./result')
    
@get('/surfrecon')
def surfrecon_get():
    html='''
    <p>surface reconstruction service:</p>
    <p>input-points format (a,b,c)(c,d,e)...</p>
    <form action="/surfrecon" method="POST">
        <p>points:</p>
        <input type="textarea" name="points" style="width:400px;height:200px;"/>
        <input type="submit" value="submit"/>
    </form>
    '''
    return html
    
@post('/surfrecon')
def surfrecon_post():
    points=request.forms.get('points')
    cmd=neutu+" --command --general "
    config={"command":"surfrecon"}
    config["npoints"]=points.count('(')
    cmd+=r"'"+json.dumps(config)+r"'"
    print(cmd)
    p=Popen(cmd, shell=True, stdout=PIPE, stderr=PIPE, stdin=PIPE)
    for a in points.split(')')[:-1]:
        a=a[1:]
        x, y, z=a.split(',')
        p.stdin.write('{0} {1} {2} \n'.format(x, y, z))
    p.wait()
    output={}
    for index, line in enumerate(p.stdout.readlines()):
        if line.startswith('result id:'):
            output['result url']='/surfrecon/result/'+line[line.find(':')+1:].strip()
        elif line.startswith('md5:'):
            output['md5']=line[line.find(':')+1:].rstrip()
    return json.dumps(output)
    
if __name__=='__main__':
    run(host='localhost', port=8080)
