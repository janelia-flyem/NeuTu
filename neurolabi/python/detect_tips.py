"""

detect_tips.py


this script finds tips of skeletons that should be reviewed; it
places to do items on those locations, then reports back its status
via std out in json form:

output = {
    "status": true | false (success or failure),
    "message": "error or success message",
    }

(there is other optional data that is returned in the output json
on success)


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
    """
    POSTs the input data to DVID

    input: the URL to call; username; the data to be posted
    """
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
        find tips and place to do items; report results by printing json; quit
        """
        self.findtips()
        self.placetodos()
        self.reportquit()

    def findtips(self):
        """
        finds and stores tip locations for input body id
        """

        t1 = time.time()

        dt.set_param(self.serverport, self.uuid, self.username)

        # this routine spews output to stdout, which I want to control; so
        #   trap and ignore it
        output = StringIO()
        with redirect_stdout(output):
            tips = dt.detect_tips(self.bodyid)
        self.locations = tips.loc[:, ["x", "y", "z"]].values.tolist()

        t2 = time.time()
        self.tfind = t2 - t1

    def placetodos(self):
        """
        posts a to do item at each previously found tip location
        """

        if len(self.locations) == 0:
            self.ntodosplaced = 0
            self.tplace = 0.0
            return

        t1 = time.time()
        annlist = [self.maketodo(loc) for loc in self.locations]
        self.postannotations(annlist)
        t2 = time.time()
        self.tplace = t2 - t1

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
            # bail out; later I'd prefer to have the error percolate up and be
            #   handled by the calling routine, but for now, just quit:
            message = "to do placement failed!\n"
            message += f"url: {todocall}\n"
            message += f"status code: {r.status_code}\n"
            message += f"returned text: {r.text}\n"
            errorquit(message)
        else:
            # successful
            self.ntodosplaced = len(annlist)

    def reportquit(self):
        message = f"{len(self.locations)} tips found in {self.tfind}s; {self.ntodosplaced} to do items placed in {self.tplace}s"
        result = {
            "status": True,
            "message": message,
            "tfind": self.tfind,
            "tplace": self.tplace,
            "ttotal": self.tfind + self.tplace,
            "nlocations": len(self.locations),
            "locations": self.locations,
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