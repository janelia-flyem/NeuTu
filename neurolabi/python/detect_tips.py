"""

detect_tips.py


this script finds tips of skeletons that should be reviewed; it
places to do items on those locations, then reports back its status
via std out in json form:

output = {
    "status": true | false (success or failure),
    "message": "error or success message",
    }


usage: detect_tips.py serverport uuid bodyid todoinstance


requires:
- dvid_tools library (and its dependencies)


"""

# ------------------------- imports -------------------------
# std lib
from contextlib import redirect_stdout
import getpass
from io import StringIO
import json
import sys
import time


import requests


import dvidtools as dt


# ------------------------- constants -------------------------
appname = "NeuTu/detect_tips.py"


# ------------------------- code -------------------------

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


def errorquit(message):
    result = {
        "status": False,
        "message": message,
    }
    print(json.dumps(result))
    sys.exit(1)


class TipDetector:
    def __init__(self, serverport, uuid, bodyid, todoinstance):
        self.serverport = serverport
        self.uuid = uuid
        self.bodyid = bodyid
        self.todoinstance = todoinstance
        self.username = getpass.getuser()
        self.locations = []
        self.ntodosplaced = 0

    def findandplace(self):
        """
        find tips and place to do items; report results
        """
        self.find_tips()
        self.place_todos()
        self.reportquit()

    def find_tips(self):
        """
        finds tips for input body id
        """

        dt.set_param(self.serverport, self.uuid, self.username)

        # this routine spews output to stdout, which I want to control; so
        #   trap and ignore it
        output = StringIO()
        with redirect_stdout(output):
            tips = dt.detect_tips(self.bodyid)

        self.locations = tips.loc[:, ["x", "y", "z"]].values.tolist()

    def place_todos(self):
        """
        posts a to do item at each input location

        input: list of [x, y, z] locations (if None, use last set found)
        """

        if len(self.locations) == 0:
            # needs error handling?
            return

        annlist = [self.maketodo(loc) for loc in self.locations]
        self.postannotations(annlist)

    def maketodo(self, location):
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
        ann["Prop"]["user"] = self.username
        ann["Prop"]["checked"] = "0"
        return ann

    def postannotations(self, annlist):
        """
        posts the list of annotations to dvid

        input: list of json annotations
        """

        todocall = self.serverport + "/api/node/" + self.uuid + "/segmentation_todo/elements"
        r = postdvid(todocall, self.username, data=annlist)
        if r.status_code != requests.codes.ok:

            # need better error handling
            print(todocall)
            print(r.status_code)
            print(r.text)

            self.ntodosplaced = 0
        else:
            # successful
            self.ntodosplaced = len(annlist)

    def reportquit(self):
        message = f"{len(self.locations)} tips located; {self.ntodosplaced} to do items placed"
        result = {
            "status": True,
            "message": message,
        }
        print(json.dumps(result))
        sys.exit(0)


def main():
    if len(sys.argv) < 5:
        errorquit("missing arguments; received: " + ", ".join(sys.argv))

    serverport = sys.argv[1]
    if not serverport.startswith("http://"):
        serverport = "http://" + serverport
    uuid = sys.argv[2]
    bodyid = sys.argv[3]
    todoinstance = sys.argv[4]

    detector = TipDetector(serverport, uuid, bodyid, todoinstance)
    detector.findandplace()


# ------------------------- script starts here -------------------------
if __name__ == '__main__':
    main()