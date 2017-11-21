import json;
import os;
import numpy as np;

def CreateDataBundle(config):
    dataBundleFile = open(config["input"]);
    inputDir = os.path.dirname(config["input"]);
    dataBundle = json.load(dataBundleFile);
    outputDir = os.path.dirname(config["output"]);
    if not os.path.exists(outputDir + "/swc"):
        os.makedirs(outputDir + "/swc");
    if not os.path.exists(outputDir + "/volume"):
        os.makedirs(outputDir + "/volume");
    if "synapse" in dataBundle:
        synapsePath = dataBundle["synapse"];
        if os.path.isabs(synapsePath) == False:
            synapsePath = inputDir + "/" + synapsePath;
        os.system("cp " + synapsePath + " " + outputDir);
        dataBundle["synapse"] = os.path.basename(synapsePath);
    for neuron in dataBundle["neuron"]:
        if "model" in neuron:
            modelPath = neuron["model"];
            if os.path.isabs(modelPath) == False:
                modelPath = inputDir + "/" + modelPath;
            ssPath = modelPath;
            ssPath = ssPath.replace(".swc", ".json");
            os.system("cp " + modelPath + " " + outputDir + "/swc");
            os.system("cp " + ssPath + " " + outputDir + "/swc");
            neuron["model"] = "swc/" + os.path.basename(neuron["model"]);
        if "volume" in neuron:
            volumePath = neuron["volume"];
            if os.path.isabs(volumePath) == False:
                volumePath = inputDir + "/" + volumePath;
            os.system("cp " + volumePath + " " + outputDir + "/volume");
            print(neuron["volume"]);
            neuron["volume"] = "volume/" + os.path.basename(neuron["volume"]);
            
    outFile = open(config["output"], "w");
    json.dump(dataBundle, outFile, indent = 2);
    outFile.close();

if __name__ == '__main__':
    config = {"input": "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/skeletonization/session18/len10/adjusted/data_bundle_with_class.json", "output": "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/data_release/data_bundle.json"}; 
    CreateDataBundle(config);
    print(config["output"], "saved");