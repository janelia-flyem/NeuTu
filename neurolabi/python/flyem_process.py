import os
import sys
import json
import subprocess
import uuid
import h5py

def get_body_list(source):
    bodyFileList = None
    if os.path.isdir(source):
        bodyFileList = glob.glob(os.path.join(source, '*.sobj'))
    else:
        f = h5py.File(source)
        bodyFileList = f['/'].keys()

    return bodyFileList


configFile = open('config.json')
if not configFile:
    print 'Cannot find configuration file. Abort.'
    sys.exit(1)

config = json.load(configFile)
exportDir = config['export']
superpixelDir = exportDir
if config.has_key('superpixel'):
    superpixelDir = config['superpixel']


bodyAnnotationFile = 'annotations-body.json'
sourceBodyAnnotationFile = os.path.join(exportDir, bodyAnnotationFile)
if os.path.exists(sourceBodyAnnotationFile) and \
        not os.path.exists(bodyAnnotationFile):
    print 'Creating body annotation link ...'
    os.symlink(sourceBodyAnnotationFile, bodyAnnotationFile) 
    print 'Done.'

synapseAnnotationFile = 'annotations-synapse.json'
sourceSynapseAnnotationFile = os.path.join(exportDir, synapseAnnotationFile)
if os.path.exists(sourceSynapseAnnotationFile) and \
        not os.path.exists(synapseAnnotationFile):
    print 'Creating synapse annotation link ...'
    os.symlink(sourceSynapseAnnotationFile, synapseAnnotationFile)
    print 'Done.'

inputDir = 'input'
if not os.path.exists(inputDir):
    print 'Creating input directory ...'
    os.mkdir(inputDir)
    print 'Done.'

bodyMapFile = 'segment_to_body_map.txt'
sourceBodyMapFile = os.path.join(exportDir, bodyMapFile)
if not os.path.exists(sourceBodyMapFile):
    print 'Cannot find', sourceBodyMapFile
    sys.exit(1)

targetBodyMapFile = os.path.join(inputDir, bodyMapFile)
if not os.path.exists(targetBodyMapFile):
    print 'Creating segment_to_body link ...'
    os.symlink(sourceBodyMapFile, targetBodyMapFile)
    print 'Done'

segmentMapFile = 'superpixel_to_segment_map.txt'
sourceSegmentMapFile = os.path.join(exportDir, segmentMapFile)
if not os.path.exists(sourceSegmentMapFile):
    print 'Cannot find', sourceSegmentMapFile
    sys.exit(1)

targetSegmentMapFile = os.path.join(inputDir, segmentMapFile)
if not os.path.exists(targetSegmentMapFile):
    print 'Creating superpixel_to_segment link ...'
    os.symlink(sourceSegmentMapFile, targetSegmentMapFile)
    print 'Done.'

sourceSuperpixelPath = os.path.join(superpixelDir, 'superpixel_maps')
if not os.path.exists(sourceSuperpixelPath):
    print 'Cannt find', sourceSuperpixelPath
    sys.exit(1)

targetSuperpixelPath = os.path.join(inputDir, 'superpixel_maps')
if not os.path.exists(targetSuperpixelPath):
    print 'Creating superpixel link ...'
    os.symlink(sourceSuperpixelPath, targetSuperpixelPath)
    print 'Done.'

home = os.path.expanduser('~')
neutubeDir = os.path.join(home, 'Work/neutube_ws')

if not os.path.exists('body_maps'):
    print 'Preparing body maps ...'
    subprocess.call([os.path.join(neutubeDir, 'neurolabi/shell/flyem_prepare_imat'), 'input', '.'])
    print 'Done.'

if not config.has_key("body_partition"):
    print 'Skip body generation because no body partition is specified. Done.'
    sys.exit(1)

bodyPartition = config['body_partition']
bodySizeList = [None] * len(bodyPartition)
bodySkeletonizeOn = [True] * len(bodyPartition)
zRangeList = [None] * len(bodyPartition)
index = 0
for body in bodyPartition:
    bodyRange = body["size"]
    if len(bodyRange) != 2:
        print 'Invalid body size range. Abort.'
        sys.exit(1)

    bodySizeList[index] = bodyRange
    if body.has_key("skeletonize"):
        bodySkeletonizeOn[index] = body["skeletonize"]

    if body.has_key("z_range"):
        zRangeList[index] = body["z_range"]

    index += 1

print 'Extracting bodies ...'
neutubePythonPath = os.path.join(neutubeDir, 'neurolabi/python')
sys.path.append(neutubePythonPath)
sys.path.append(os.path.join(neutubePythonPath, 'flyem'))
import neututils
import BodyTaskManager
    
targetContainer = 'stacked.hf5'
taskManager = BodyTaskManager.ExtractBodyTaskManager()
taskManager.setBodyMapDir(os.path.abspath('body_maps'))
taskManager.setCommandPath(os.path.join(neutubeDir, 'neurolabi/cpp/map_body-build-desktop-Qt_4_8_2_in_PATH__System__Debug/map_body'))
taskManager.setTargetContainer(targetContainer)


#if config.has_key('z_offset'):
#    taskManager.setZOffset(config['z_offset'])

bodyDirList = []
for body in bodyPartition:
    if body.has_key('z_range'):
        zRange = body['z_range']
        taskManager.setZRange(zRange[0], zRange[1])
    else:
        taskManager.zRange = None

    bodyRange = body["size"]
    taskManager.setRange(bodyRange)
    outputDir = str(bodyRange[0]) + '_'
    if bodyRange[1] >= bodyRange[0]:
        outputDir += str(bodyRange[1])
    
    body['output_dir'] = outputDir
    print outputDir
    bodyDirList.append(outputDir)
    bodySizeFile = os.path.abspath(os.path.join(outputDir, 'bodysize.txt'))
    if not os.path.exists(bodySizeFile):
        if not os.path.exists(outputDir):
            os.makedirs(outputDir)
        taskManager.setOutput(os.path.abspath(outputDir))
        taskManager.setBodySizeFile(bodySizeFile)
        taskManager.generateScript(os.path.join(outputDir, 'extract_body.sh'))

        subprocess.call(['sh', os.path.join(outputDir, 'extract_body.sh')])

print 'Generating skeletonization scripts...'

import TaskManager
scheduler = TaskManager.Scheduler()

import glob
import SkeletonizeTaskDistributor as st
distr = st.SkeletonizeTaskDistributor()
excludedBodyList = []
if config.has_key('skeletonize'):
    distr.setArgs(config['skeletonize'])
    if config['skeletonize'].has_key('excluded'):
        excludedBodyList = config['skeletonize']['excluded']

scriptList = []

print bodyDirList

#skeletonization and bundle creation
for body in bodyPartition:
    if body['skeletonize']:
        bodyDir = body['output_dir']
        print 'Building body list for ', bodyDir
        distr.setBodyDir(os.path.abspath(os.path.join(bodyDir, targetContainer)))
        swcDir = os.path.join(bodyDir, 'len' + str(distr.getMinLength()) + '/swc')
        distr.setSwcDir(os.path.abspath(swcDir))
        #bodyFileList = glob.glob(os.path.join(bodyDir, 'stacked/*.sobj'))
        bodyFileList = get_body_list(os.path.join(bodyDir, targetContainer))
        bodySizeFile = os.path.abspath(os.path.join(bodyDir, 'bodysize.txt'))
        print ' ', len(bodyFileList), ' bodies found.'
        bodyList = []
        print '  extracting body IDs...'
        for bodyFile in bodyFileList:
            bodyId = int(os.path.splitext(os.path.basename(bodyFile))[0]);
            if not bodyId in excludedBodyList:
                bodyList.append(bodyId);
     
        distr.setBodyList(bodyList);
        distr.setCommandPath('/groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/cpp/skeletonize/build/bin/skeletonize');
     
        scriptDir = os.path.join(swcDir, 'scripts')
        if not os.path.exists(scriptDir):
            os.makedirs(scriptDir)
        distr.generateScript(scriptDir);
     
        runScript = os.path.join(os.path.abspath(swcDir), 'generate.sh')
        neututils.create_text_file(runScript, ['sh ' + os.path.join(os.getcwd(), distr.getMasterScript())])
     
        print runScript, 'created.'
        scriptList.append(runScript)
        scheduler.submit('sh ' + runScript, dependency = [bodySizeFile])
     
        skeletonizeScriptList = distr.getSubscript()
        nextDepList = []
        print scriptDir
        for subscript in skeletonizeScriptList:
            print subscript
            nextDepList.append(os.path.abspath(subscript) + '.done')
     
        #Generate skeleton
        #commandList = []
        #scriptDir = os.path.dirname(runScript)
        #commandList.append('cd ' + scriptDir)
        #commandList.append('sh ' + runScript)
        #scheduler.submit(commandList, dependency = [os.path.abspath(os.path.join(bodyDir, 'bodysize.txt'))])
     
        #Create data bundle
        commandList = []
        commandList.append('python ' + neutubeDir + \
            '/neurolabi/python/app/create_data_bundle.py --config ' + \
            os.path.join(os.getcwd(), 'config.json') + \
            ' --session ' + os.getcwd() + ' --swc_dir ' + os.path.abspath(swcDir) + \
            ' --body_annotation ' + os.path.abspath(bodyAnnotationFile) + \
            ' --synapse_annotation ' + os.path.abspath(synapseAnnotationFile))
     
        scheduler.submit(commandList, dependency = nextDepList)
        #sys.exit(1)



#scripts for generate skeletons
#for script in scriptList:
#    commandList = []
#    scriptDir = os.path.dirname(script)
#    commandList.append('cd ' + scriptDir)
#    commandList.append('sh ' + script)
#    scheduler.submit(commandList, dependency = [os.path.abspath(scriptDir + '/../bodysize.txt')])

