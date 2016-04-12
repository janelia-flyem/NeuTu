import json
import urlparse

class DvidEnv(object):
    def __init__(self, address = None, port = None, node = None):
        self.address = address
        self.port = port
        self.node = node
        self.bodyLabel = "bodies"
        self.setDefaultBodyLabel()

    def __str__(self):
        return self.getNeuTuInput()

    def setDefaultBodyLabel(self):
        self.labelBlock = "labels"

    def getAddressWithPort(self):
        url = self.address
        if self.port:
            url += ":" + str(self.port)

        if not url.startswith("http://"):
            url = "http://" + url

        return url


    def setBodyLabel(self, name):
        if name:
            self.bodyLabel = name
        else:
            self.setDefaultBodyLabel()

    def setLabelBlock(self, name):
        self.labelBlock = name

    def loadServerConfig(self, config):
        if config.has_key("dvid-server"):
            dvidServer = config["dvid-server"]
            p = urlparse.urlsplit(dvidServer)
            print p
            if p.netloc:
                self.address = p.netloc
            else:
                self.address = p.path
            self.port = None

            self.node = config["uuid"]
            print config.get("labelvol")
            self.setBodyLabel(config.get("labelvol"))

    def loadFlyEmConfig(self, config):
        self.address = config["address"]
        self.port = config.get("port")
        self.node = config["uuid"]
        self.setBodyLabel(config.get("body_label"))

    def getNeuTuInput(self):
        if self.port:
            return "http:" + self.address + ":" + str(self.port) + ":" + self.node + ":" + self.bodyLabel
        else:
            return "http:" + self.address + ":" + self.node + ":" + self.bodyLabel


    def isValid(self):
        return self.address and self.node

if __name__ == "__main__":
    dvidEnv = DvidEnv("emdata1", 8500, "372c")
    dvidEnv.setBodyLabel("bodies3")
    print dvidEnv.getNeuTuInput()
    print dvidEnv.getAddressWithPort()

    dvidEnv.loadServerConfig({"dvid-server": "emdata1.int.janelia.org:8500", "uuid": "372c", "bodies": [ 1 ]})
    print dvidEnv.getNeuTuInput()
    print dvidEnv.getAddressWithPort()
