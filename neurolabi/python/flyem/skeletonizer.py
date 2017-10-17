import os;
from dvidenv import DvidEnv;
import subprocess;
import urllib.parse;
import http.client;
from dvidurl import DvidUrl;


class Skeletonizer:
    def __init__(self):
        self.dvidEnv = None
        self.processMap = {}
        self.executable = "/opt/bin/neutu"

    def setDvidEnv(self, env):
        self.dvidEnv = env

    def getDvidEnv(self):
        return self.dvidEnv

    def setExecutable(self, exe):
        self.executable = exe

    def loadDvidConfig(self, config):
        self.dvidEnv = DvidEnv()
        self.dvidEnv.loadServerConfig(config)

    def loadFlyEmConfig(self, config):
        self.dvidEnv = DvidEnv()
        self.dvidEnv.loadFlyEmConfig(config)

    def skeletonize(self, bodyId, bg = False, forceUpdate = False):
        if bodyId < 1:
            raise Exception("Invalid body ID")

        if self.dvidEnv and self.dvidEnv.isValid():
            print(self.dvidEnv.getNeuTuInput())
            args = [self.executable, "--command", "--skeletonize", "--bodyid", str(bodyId), self.dvidEnv.getNeuTuInput()]
            if forceUpdate:
                args.append("--force")
            print(args)
            p = subprocess.Popen(args)
            if not bg:
                p.wait()
            return p
        else:
            raise Exception("Invalid DVID env")

if __name__ == "__main__":
    skeletonizer = Skeletonizer()
    skeletonizer.setDvidEnv(DvidEnv("emdata1.int.janelia.org", 8500, "372c")) 
    dvidUrl = DvidUrl(skeletonizer.getDvidEnv())
    conn = http.client.HTTPConnection(dvidUrl.getServerUrl())
    #conn.request("DELETE", dvidUrl.getSkeletonEndPoint(15363212))
    skeletonizer.skeletonize(15363212, bg = True, forceUpdate = True)
    print("Done")


