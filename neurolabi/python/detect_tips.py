"""

detect_tips.py


this script finds tips of skeletons that should be reviewed

usage: detect_tips.py serverport uuid bodyid todoinstance


requires:
- dvid_tools library


"""

# ------------------------- imports -------------------------
# std lib
from dataclasses import dataclass
import getpass
import json
import sys


import requests


import dvidtools as dt


# ------------------------- constants -------------------------
appname = "NeuTu/detect_tips.py"


# ------------------------- code -------------------------
@dataclass
class Parameters:
    serverport: str
    uuid: str
    bodyid: str
    todoinstance: str
    username: str


def find_tips(params):
    """
    finds tips for input body id

    output: list of [x, y, z] locations
    """
    dt.set_param(params.serverport, params.uuid, params.username)
    tips = dt.detect_tips(params.bodyid)
    return tips.loc[:, ["x", "y", "z"]].values.tolist()


def place_todos(locations, params):
    """
    posts a to do item at each input location

    input: list of [x, y, z] locations
    """
    annlist = [maketodo(loc, params) for loc in locations]
    


    # test: only annotate a few
    postannotations(annlist[:3], params)
    # postannotations(annlist, params)


def maketodo(location, params):
    """
    input: [x, y, z] location
    output: json for a to do annotation at that location 
    """
    ann = {
        "Kind": "Note",
        "Prop": {},
    }
    ann["Pos"] = list(location)
    ann["Prop"]["comment"] = "placed by detect_tips.py"
    ann["Prop"]["user"] = params.username
    ann["Prop"]["checked"] = "0"
    return ann


def postannotations(annlist, params):
    """
    posts the list of annotations to dvid

    input: list of json annotations
    """

    todocall = params.serverport + "/api/node/" + params.uuid + "/segmentation_todo/elements"
    r = postdvid(todocall, params.username, data=annlist)
    if r.status_code != requests.codes.ok:
        print(call)
        print(r.status_code)
        print(r.text)

        # do something here




def postdvid(call, username, data):
    '''
    POSTs the input data to DVID

    input: the URL to call; username; the data to be posted
    '''
    # add user and app tags
    if "?" not in call:
        call += "?"
    else:
        call += "&"
    call += "u={}&app={}".format(username, appname)
    
    return requests.post(call, data=json.dumps(data))

def parseparameters():
    if len(sys.argv) < 5:
        errorquit("missing arguments; received:", ", ".join(sys.argv))

    params = Parameters(
        sys.argv[1],
        sys.argv[2],
        sys.argv[3],
        sys.argv[4],
        getpass.getuser()
        )

    if not params.serverport.startswith("http://"):
        params.serverport = "http://" + params.serverport

    return params


def textreportquit(message):
    print("success")
    print(message)
    sys.exit(0)


def errorquit(message):
    print("error")
    print(message)
    sys.exit(1)


def main():

    # probably need error checking after each step

    params = parseparameters()

    locations = find_tips(params)

    place_todos(locations, params)


    # temp return
    textreportquit(f"placed {len(locations)} tips")



# ------------------------- script starts here -------------------------
if __name__ == '__main__':
    main()