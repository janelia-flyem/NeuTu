#include "zcommandline.h"

#include <QFileInfoList>
#include <QDir>
#include <QFileInfo>
#include <ostream>
#include <fstream>
#include <set>
#include <QApplication>

#include "tz_utilities.h"
#include "zstack.hxx"
//#include "zargumentprocessor.h"
#include "ztest.h"
#include "zobject3dscan.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "tz_error.h"
#include "zstring.h"
#include "zfiletype.h"

#include "zstackskeletonizer.h"
#include "neutubeconfig.h"
#include "zrandomgenerator.h"
#include "zneurontracer.h"
#include "zneurontracerconfig.h"
#include "zintcuboidarray.h"

#if defined (_FLYEM_)
#include "flyem/zflyemdatabundle.h"
#include "flyem/zflyemneuronfeatureanalyzer.h"
#include "flyem/zflyemneuronfeatureset.h"
#include "flyem/zflyemqualityanalyzer.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidurl.h"
#endif

//#include "mylib/utilities.h"

using namespace std;

ZCommandLine::ZCommandLine() : m_ravelerHeight(2599), m_zStart(1490)
{
  m_isVerbose = false;
  for (int i = 0; i < 3; ++i) {
    m_blockOffset[i] = 0;
  }
}

ZCommandLine::ECommand ZCommandLine::getCommand(const char *cmd)
{
  if (eqstr(cmd, "sobj_marker")) {
    return OBJECT_MARKER;
  }

  if (eqstr(cmd, "boundary_orphan")) {
    return BOUNDARY_ORPHAN;
  }

  if (eqstr(cmd, "flyem_neuron_feature")) {
    return FLYEM_NEURON_FEATURE;
  }

  return UNKNOWN_COMMAND;
}

int ZCommandLine::runObjectMarker()
{
  QDir dir(m_input[0].c_str());
  QStringList filters;
  filters << "*.sobj";

  std::cout << "Scanning object files: " << dir.absolutePath().toStdString()
            << "..." << std::endl;
  QFileInfoList fileList = dir.entryInfoList(filters);
  json_t *obj = json_object();
  json_t *dataObj = json_array();
  std::cout << "Computing marker locations ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    ZObject3dScan obj;
    std::cout << fileInfo.filePath().toStdString() << std::endl;
    obj.load(fileInfo.absoluteFilePath().toStdString());
    if (!obj.isEmpty()) {
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(m_ravelerHeight - 1 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);
    }
  }
  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, m_output.c_str(), JSON_INDENT(2));

  return 0;
}

int ZCommandLine::runBoundaryOrphan()
{
#if defined (_FLYEM_)
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(m_blockFile);

  int bodyOffset[3] = {0, 0, 0};
  Cuboid_I blockBoundBox = blockArray.getBoundBox();

  bodyOffset[2] += blockBoundBox.cb[2];

  if (!m_referenceBlockFile.empty()) {
    ZIntCuboidArray blockReference;
    blockReference.loadSubstackList(m_referenceBlockFile);
    Cuboid_I refBoundBox = blockReference.getBoundBox();
    std::cout << "Offset: " << refBoundBox.cb[0] << " " << refBoundBox.cb[1] << std::endl;

    int zStart = refBoundBox.cb[2] - 10;
    int ravelerHeight = refBoundBox.ce[1] - refBoundBox.cb[1] + 1 + 20;

    if (zStart != m_zStart || ravelerHeight != m_ravelerHeight) {
      std::cout << "Inconsistent values" << std::endl;
      std::cout << "z: " << zStart << "; H: " << ravelerHeight << std::endl;
      std::cout << "Abort" << std::endl;

      return 1;
    }

    blockArray.translate(-refBoundBox.cb[0], -refBoundBox.cb[1], -refBoundBox.cb[2]);
    blockArray.translate(10, 10, 10);
  }

  blockArray.translate(m_blockOffset[0], m_blockOffset[1], m_blockOffset[2]);
  blockArray.exportSwc(m_output + ".swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  QDir dir(m_input[0].c_str());
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  std::vector<int> synapseCount;
  if (!m_synapseFile.empty()) {
    FlyEm::ZSynapseAnnotationArray synapseAnnotation;
    synapseAnnotation.loadJson(m_synapseFile);
    synapseCount = synapseAnnotation.countSynapse();
  }

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() <= 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      ZString bodyPath = objFile.absoluteFilePath().toStdString();
      int bodyId = bodyPath.lastInteger();

      bool isCandidate = true;
      if (bodyId < (int) synapseCount.size()) {
        if (synapseCount[bodyId] == 0) {
          isCandidate = false;
        }
      }

      if (isCandidate && qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        voxel.translate(bodyOffset[0], bodyOffset[1], bodyOffset[2]);
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(m_ravelerHeight - 1 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z()));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, m_output.c_str(), JSON_INDENT(2));
#endif

  return 0;
}

int ZCommandLine::runObjectOverlap()
{
  if (m_input.size() != 2) {
    std::cerr << "Invalid number of input files" << std::endl;
    return 1;
  }

  QDir dir1(m_input[0].c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList1 = dir1.entryInfoList(filters);
  std::cout << fileList1.size() << " objects found in " << dir1.absolutePath().toStdString()
            << std::endl;

  QDir dir2(m_input[1].c_str());
  QFileInfoList fileList2 = dir2.entryInfoList(filters);
  std::cout << fileList2.size() << " objects found in " << dir2.absolutePath().toStdString()
            << std::endl;

  std::set<int> excludedBodySet;

  if (m_fullOverlapScreen) {
    foreach (QFileInfo fileInfo, fileList1) {
      QString fileName = fileInfo.fileName();
      QFileInfo targetPath = dir2.absoluteFilePath(fileName);
      if (targetPath.exists()) {
        ZObject3dScan obj1;
        obj1.load(fileInfo.absoluteFilePath().toStdString());
        ZObject3dScan obj2;
        obj2.load(targetPath.absoluteFilePath().toStdString());
        if (obj1.equalsLiterally(obj2)) {
          std::cout << "Untouched object: " << fileInfo.baseName().toStdString()
                    << std::endl;
        }
        excludedBodySet.insert(
              ZString::lastInteger(fileInfo.baseName().toStdString()));
      }
    }
  }

  QVector<ZObject3dScan> objArray1(fileList1.size());
  QVector<ZObject3dScan> objArray2(fileList2.size());
  //objArray1.resize(100);
  //objArray2.resize(100);

  QVector<int> idArray1(fileList1.size());
  QVector<int> idArray2(fileList2.size());

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    int id = ZString::lastInteger(fileList1[i].baseName().toStdString());

    if (excludedBodySet.count(id) == 0) {
      objArray1[i].load(fileList1[i].absoluteFilePath().toStdString());
      std::cout << i << ": Object size: " << objArray1[i].getVoxelNumber() << std::endl;
      objArray1[i].downsample(m_intv[0], m_intv[1], m_intv[2]);
      objArray1[i].canonize();
      std::cout << "Object size: " << objArray1[i].getVoxelNumber() << std::endl;
      idArray1[i] = id;
    }
  }

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    int id = ZString::lastInteger(fileList2[i].baseName().toStdString());
    if (excludedBodySet.count(id) == 0) {
      objArray2[i].load(fileList2[i].absoluteFilePath().toStdString());
      objArray2[i].downsample(m_intv[0], m_intv[1], m_intv[2]);
      objArray2[i].canonize();
      idArray2[i] = id;
    }
  }

  QVector<Cuboid_I> boundBoxArray1(objArray1.size());
  QVector<Cuboid_I> boundBoxArray2(objArray2.size());

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    objArray1[i].getBoundBox(&(boundBoxArray1[i]));
  }

  std::cout << "Computing bound box ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    objArray2[i].getBoundBox(&(boundBoxArray2[i]));
  }

  int index1 = 0;

  std::ofstream stream(m_output.c_str());

  int offset[3];
  foreach(Cuboid_I boundBox1, boundBoxArray1) {
    if (!objArray1[index1].isEmpty()) {
      Stack *stack = objArray1[index1].toStack(offset);
      for (int i = 0; i < 3; ++i) {
        offset[i] = -offset[i];
      }
      int id1 = idArray1[index1];
      std::cout << "ID: " << id1 << std::endl;
      int index2 = 0;
      foreach (Cuboid_I boundBox2, boundBoxArray2) {
        if (Cuboid_I_Overlap_Volume(&boundBox1, &boundBox2) > 0) {
          int id2 = idArray2[index2];
          size_t overlap =
              objArray2[index2].countForegroundOverlap(stack, offset);
          if (overlap > 0) {
            std::cout << id1 << " " << id2 << " " << overlap << std::endl;
            stream << id1 << " " << id2 << " " << overlap << std::endl;
          }
        }
        ++index2;
      }
      C_Stack::kill(stack);
    }
    ++index1;
  }

  stream.close();

  return 0;
}

int ZCommandLine::runSynapseObjectList()
{
#if defined (_FLYEM_)
  std::set<int> bodySet;
  FlyEm::ZSynapseAnnotationArray synaseArray;
  if (!synaseArray.loadJson(m_input[0])) {
    return 1;
  }

  for (FlyEm::SynapseLocation *location = synaseArray.beginSynapseLocation();
       location != NULL; location = synaseArray.nextSynapseLocation()) {
    bodySet.insert(location->bodyId());
  }

  std::ofstream stream(m_output.c_str());

  for (std::set<int>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    stream << *iter << std::endl;
  }

  stream.close();
#endif
  return 0;
}

int ZCommandLine::runOutputClassList()
{
#if defined (_FLYEM_)
  if (ZFileType::fileType(m_input[0]) == ZFileType::JSON_FILE) {
    ZFlyEmDataBundle bundle;
    bundle.loadJsonFile(m_input[0]);
    std::map<string, int> classMap = bundle.getClassIdMap();
    std::ofstream stream(m_output.c_str());

    for (map<string, int>::const_iterator iter = classMap.begin();
         iter != classMap.end(); ++iter) {
      stream << iter->first << std::endl;
    }

    stream.close();

    return 0;
  }
#endif
  return 1;
}

int ZCommandLine::runComputeFlyEmNeuronFeature()
{
#if defined (_FLYEM_)
  ZFlyEmDataBundle bundle;

  QFileInfo fileInfo(m_input[0].c_str());
  QString input = fileInfo.absoluteFilePath();
  bundle.loadJsonFile(input.toStdString());

  std::ofstream stream(m_output.c_str());

  ZFlyEmNeuronFeatureSet featureSet;
  featureSet << ZFlyEmNeuronFeature::OVERALL_LENGTH
             << ZFlyEmNeuronFeature::BRANCH_NUMBER
             << ZFlyEmNeuronFeature::TBAR_NUMBER
             << ZFlyEmNeuronFeature::PSD_NUMBER
             << ZFlyEmNeuronFeature::CENTROID_X
             << ZFlyEmNeuronFeature::CENTROID_Y;

  ZFlyEmNeuronFeatureAnalyzer featureAnalyzer;
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    featureAnalyzer.computeFeatureSet(neuron, featureSet);
    stream << "\"" << neuron.getId() << "\"," << "\""
           << neuron.getType() << "\"";
    for (size_t i = 0; i < featureSet.size(); ++i) {
      stream << "," << featureSet[i].getValue();
    }
    stream << endl;
  }

  stream.close();
#endif
  return 0;
}

int ZCommandLine::runTraceNeuron()
{
  if (m_input.empty()) {
    std::cout << "No input specified. Abort." << std::endl;
    return 1;
  }

  if (m_output.empty()) {
    std::cout << "No output specified. Abort." << std::endl;
    return 1;
  }

  if (m_configJson.hasKey("trace")) {
    ZNeuronTracerConfig::getInstance().loadJsonObject(
          ZJsonObject(m_configJson["trace"], ZJsonValue::SET_INCREASE_REF_COUNT));
  } else {
    ZNeuronTracerConfig::getInstance().load(m_configDir + "/trace_config.json");
  }

  ZStack signal;
  signal.load(m_input[0]);

  ZNeuronTracer tracer;
  tracer.setIntensityField(&signal);
//  tracer.initTraceWorkspace(&signal);
//  tracer.initConnectionTestWorkspace();

#if 0
  if (m_configJson.hasKey("trace")) {
    tracer.loadJsonObject(
          ZJsonObject(
            m_configJson["trace"], ZJsonValue::SET_INCREASE_REF_COUNT));
  }
#endif

  int level = 0;
  if (Is_Arg_Matched(const_cast<char*>("--level"))) {
    level = Get_Int_Arg(const_cast<char*>("--level"));
  }
  tracer.setTraceLevel(level);

  ZSwcTree *tree = tracer.trace(&signal);

  tree->save(m_output);

  return 0;
}

int ZCommandLine::runImageSeparation()
{
  if (m_input.size() < 2) {
    return 0;
  }

  ZStack signal;
  signal.load(m_input[0]);
  ZStack mask;
  mask.load(m_input[1]);

  std::map<uint64_t, ZObject3dScan*> *objMap =
      ZObject3dScan::extractAllObject(
        mask.array8(), mask.width(), mask.height(),
        mask.depth(), 0, 1, NULL);

  std::cout << objMap->size() << " objects extracted." << std::endl;

  for (std::map<uint64_t, ZObject3dScan*>::iterator iter = objMap->begin();
       iter != objMap->end(); ++iter) {
    ZObject3dScan *obj = iter->second;
    if (iter->first > 0) {
      ZString outputPath = m_output + "_";
      outputPath.appendNumber(iter->first);
      outputPath += ".tif";


      obj->translate(mask.getOffset().getX(), mask.getOffset().getY(),
                     mask.getOffset().getZ());
      ZStack *substack = obj->toStackObject();
      size_t offset = 0;
      for (int z = 0; z < substack->depth(); ++z) {
        for (int y = 0; y < substack->height(); ++y) {
          for (int x = 0; x < substack->width(); ++x) {
            if (substack->array8()[offset] == 1) {
              int v = signal.getIntValue(x + substack->getOffset().getX(),
                                         y + substack->getOffset().getY(),
                                         z + substack->getOffset().getZ());
              substack->array8()[offset] = v;
            }
            ++offset;
          }
        }
      }
      std::cout << "Saving " << outputPath << "..." << std::endl;
      substack->save(outputPath);
      std::cout << "Done." << std::endl;
      delete substack;
    }
    delete obj;
  }

  delete objMap;

  return 0;
}

std::set<uint64_t> ZCommandLine::loadBodySet(const std::string &input)
{
//  ZString

  std::set<uint64_t> bodySet;

  FILE *fp = fopen(input.c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<uint64_t> bodyArray = str.toUint64Array();
    bodySet.insert(bodyArray.begin(), bodyArray.end());
  }
  fclose(fp);

  return bodySet;
}

int ZCommandLine::runTest()
{
#if 1
  std::cout << GET_TEST_DATA_DIR << std::endl;
  loadConfig(GET_TEST_DATA_DIR + "/../json/command_config.json");
  std::cout << m_configJson.dumpString(2) << std::endl;
#endif

  return 0;
}

int ZCommandLine::runSkeletonize()
{
  if (m_input.empty()) {
    std::cout << "Please specify input." << std::endl;
    return 0;
  }

  if (m_isVerbose) {
    tic();
  }
#if defined (_FLYEM_)
  if (ZDvidTarget::isDvidTarget(m_input[0])) {
    ZDvidTarget target;
    //target.setBodyLabelName("sp2body");
    target.setFromSourceString(m_input[0]);
    //target.set("emdata2.int.janelia.org", "43f", 9000);

    ZDvidReader reader;
    reader.open(target);

    ZDvidWriter writer;
    writer.open(target);

    ZDvidUrl dvidUrl(target);

    ZSwcTree testTree;
    testTree.setDataFromNode(SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1));
    writer.writeSwc(0, &testTree);
    if (writer.getStatusCode() != 200) {
      std::cout << "Server return code: " << writer.getStatusCode() << std::endl;
      std::cout << writer.getStandardOutput().toStdString() << std::endl;
      std::cout << "Cannot access " << dvidUrl.getSkeletonUrl()
                << std::endl;
      std::cout << "Please create the keyvalue data for skeletons first:"
                << std::endl;
      std::cout << ">> curl -X POST -H \"Content-Type: application/json\" "
                   "-d '{\"dataname\": \""
                << ZDvidData::GetName(ZDvidData::ROLE_SKELETON,
                                      ZDvidData::ROLE_BODY_LABEL,
                                      target.getBodyLabelName())
                << "\", " << "\"typename\": \"keyvalue\"}' "
                << target.getAddressWithPort() + "/api/repo/" + target.getUuid() + "/instance"
                << std::endl;

      return 1;
    }

    std::set<uint64_t> bodyIdSet;

    if (m_input.size() == 1) {
      bodyIdSet = reader.readBodyId(100000);
      if (bodyIdSet.empty()) {
        bodyIdSet = reader.readAnnnotatedBodySet();
        if (bodyIdSet.empty()) {
          std::cout << "Done: No annotated body found in the database."
                    << std::endl;
          return 1;
        }
      }
    } else {
      bodyIdSet = loadBodySet(m_input[1]);
    }

    std::vector<uint64_t> bodyIdArray;
    bodyIdArray.insert(bodyIdArray.begin(), bodyIdSet.begin(), bodyIdSet.end());
    std::cout << bodyIdArray.size() << " bodies loaded." << std::endl;

    ZRandomGenerator rnd;
    std::vector<int> rank = rnd.randperm(bodyIdArray.size());
    std::set<uint64_t> excluded;
    //excluded.insert(16493);
    //excluded.insert(8772496);

    ZStackSkeletonizer skeletonizer;


    if (m_configJson.hasKey("skeletonize")) {
      skeletonizer.configure(
            ZJsonObject(m_configJson["skeletonize"],
            ZJsonValue::SET_INCREASE_REF_COUNT));
    } else {
      ZJsonObject config;
      if (m_input.size() <= 2) {
        config.load(NeutubeConfig::getInstance().getApplicatinDir() +
                    "/json/skeletonize.json");
      } else {
        config.load(m_input[2]);
      }
      skeletonizer.configure(config);
    }

    for (size_t i = 0; i < bodyIdArray.size(); ++i) {
      uint64_t bodyId = bodyIdArray[rank[i] - 1];
      if (excluded.count(bodyId) == 0) {
        ZSwcTree *tree = NULL;
        if (!m_forceUpdate) {
          tree = reader.readSwc(bodyId);
        }
        if (tree == NULL) {
          ZObject3dScan obj = reader.readBody(bodyId);
          tree = skeletonizer.makeSkeleton(obj);
          writer.writeSwc(bodyId, tree);
        }
        delete tree;
        std::cout << ">>>>>>>>>>>>>>>>>>" << i + 1 << " / "
                  << bodyIdArray.size() << std::endl;
      }
    }
  } else
  #endif
  {
    if (!fexist(m_input[0].c_str())) {
      m_reporter.report("Skeletonization Failed",
                        "The input file " + m_input[0] + " seems not exist.",
          NeuTube::MSG_ERROR);
      return 1;
    }

    ZStackSkeletonizer skeletonizer;

    ZSwcTree *tree = NULL;

    if (m_configJson.hasKey("skeletonize")) {
      skeletonizer.configure(
            ZJsonObject(m_configJson["skeletonize"],
            ZJsonValue::SET_INCREASE_REF_COUNT));
    }

    if (m_isVerbose) {
      skeletonizer.print();
    }

    if (ZFileType::fileType(m_input[0]) == ZFileType::TIFF_FILE) {
      if (m_output.empty()) {
        m_reporter.report("Skeletonization Failed",
                          "The input is not a binary image.",
                          NeuTube::MSG_ERROR);
        return 1;
      }
      ZStack stack;
      stack.load(m_input[0]);

      if (!stack.isBinary()) {
        std::cout << "The image is not binary. Binarizing..." << std::endl;
        stack.binarize();
      }
      tree = skeletonizer.makeSkeleton(stack);
    } else if (ZFileType::fileType(m_input[0]) == ZFileType::OBJECT_SCAN_FILE) {
      ZObject3dScan obj;
      obj.load(m_input[0]);
      if (m_isVerbose) {
        std::cout << obj.getVoxelNumber() << " foreground voxels." << std::endl;
      }
      tree = skeletonizer.makeSkeleton(obj);
    } else {
      m_reporter.report(
            "Skeletonization Failed",
            "Unrecognized output: " + m_input[0],
          NeuTube::MSG_ERROR);
//      std::cout << "Unrecognized output: " << m_input[0] << std::endl;
    }

    if (tree != NULL) {
      if (!tree->isEmpty()) {
        if (!m_output.empty()) {
          tree->save(m_output);
          std::cout << "SWC saved in " << m_output << std::endl;
        }
      } else {
        std::cout << "No SWC generated." << std::endl;
      }
      delete tree;
    } else {
      std::cout << "No SWC generated." << std::endl;
    }
  }

  if (m_isVerbose) {
    ptoc();
  }

  return 0;
}

int ZCommandLine::run(int argc, char *argv[])
{
  static const char *Spec[] = {
    "--command",
    "[--unit_test]",
    "[<input:string> ...]",
    "[-o <string>]",
    "[--config <string>]", "[--intv <int> <int> <int>]",
    "[--skeletonize] [--force]",
    "[--trace] [--level <int>]","[--separate <string>]",
    "[--test]", "[--verbose]",
    0
  };

  Process_Arguments(argc, argv, const_cast<char**>(Spec), 1);

  std::string applicationDir = ZString::dirPath(argv[0]);
  std::cout << applicationDir << std::endl;
  m_configDir = applicationDir + "/json";
  std::string configPath = m_configDir + "/command_config.json";

  if (Is_Arg_Matched(const_cast<char*>("--config"))) {
    configPath = Get_String_Arg(const_cast<char*>("--config"));
  }

  loadConfig(configPath);

  ECommand command = UNKNOWN_COMMAND;
  if (!m_configJson.isEmpty()) {
    command = getCommand(ZJsonParser::stringValue(m_configJson["command"]));
    m_input.push_back(ZJsonParser::stringValue(m_configJson["input"]));
    m_output = ZJsonParser::stringValue(m_configJson["output"]);
    if (m_configJson.hasKey("synapse")) {
      m_synapseFile = ZJsonParser::stringValue(m_configJson["synapse"]);
    }

    if (m_configJson.hasKey("raveler_height")) {
      m_ravelerHeight = ZJsonParser::integerValue(m_configJson["raveler_height"]);
    }

    if (m_configJson.hasKey("z_start")) {
      m_zStart = ZJsonParser::integerValue(m_configJson["z_start"]);
    }

    if (m_configJson.hasKey("block_offset")) {
      for (int i = 0; i < 3; ++i) {
        m_blockOffset[i] = ZJsonParser::numberValue(m_configJson["block_offset"], i);
      }
    }

    m_blockFile = ZJsonParser::stringValue(m_configJson["block_file"]);
    m_referenceBlockFile = ZJsonParser::stringValue(m_configJson["block_reference"]);
  }

  m_forceUpdate = false;
  if (Is_Arg_Matched(const_cast<char*>("--force"))) {
    m_forceUpdate = true;
  }

//  ZArgumentProcessor::processArguments(argc, argv, Spec);

  m_input.clear();
  int inputNumber = Get_Repeat_Count(const_cast<char*>("input"));
  //  if (ZArgumentProcessor::isArgMatched("input")) {
  //    int inputNumber = ZArgumentProcessor::getRepeatCount("input");
  m_input.resize(inputNumber);
  for (int i = 0; i < inputNumber; ++i) {
    m_input[i] = Get_String_Arg(const_cast<char*>("input"), i);;
    //      m_input[i] = ZArgumentProcessor::getStringArg("input", i);
  }
//  }

  if (Is_Arg_Matched(const_cast<char*>("-o"))) {
    m_output = Get_String_Arg(const_cast<char*>("-o"));
  }

  if (Is_Arg_Matched(const_cast<char*>("--unit_test"))) {
    return ZTest::runUnitTest(argc, argv);
  }

  for (int i = 0; i < 3; ++i) {
    m_intv[i] = 0;
  }

  if (Is_Arg_Matched(const_cast<char*>("--verbose"))) {
    m_isVerbose = true;
  }

  if (command == UNKNOWN_COMMAND) {/*
    if (ZArgumentProcessor::isArgMatched("--sobj_marker")) {
      command = OBJECT_MARKER;
      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = ZArgumentProcessor::getStringArg("-o");
    } else if (ZArgumentProcessor::isArgMatched("--boundary_orphan")) {
      command = BOUNDARY_ORPHAN;
      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = ZArgumentProcessor::getStringArg("-o");
      m_blockFile = ZArgumentProcessor::getStringArg("--boundary_orphan");
    } else if (ZArgumentProcessor::isArgMatched("--sobj_overlap")) {
      command = OBJECT_OVERLAP;
      m_fullOverlapScreen = false;
      int inputNumber = ZArgumentProcessor::getRepeatCount("input");
      m_input.resize(inputNumber);
      for (int i = 0; i < inputNumber; ++i) {
        m_input[i] = ZArgumentProcessor::getStringArg("input", i);
      }
      m_output = ZArgumentProcessor::getStringArg("-o");

      if (ZArgumentProcessor::isArgMatched("--intv")) {
        for (int i = 1; i < 3; ++i) {
          m_intv[i] = ZArgumentProcessor::getIntArg("--intv", i);
        }
      }

      m_fullOverlapScreen =
          ZArgumentProcessor::isArgMatched("--fulloverlap_screen");
    } else if (ZArgumentProcessor::isArgMatched("--synapse_object")) {
      command = SYNAPSE_OBJECT;
      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = ZArgumentProcessor::getStringArg("-o");
    } else if (ZArgumentProcessor::isArgMatched("--classlist")) {
      command = CLASS_LIST;
      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = ZArgumentProcessor::getStringArg("-o");
    } else if (ZArgumentProcessor::isArgMatched("--flyem_neuron_feature")) {
      command = FLYEM_NEURON_FEATURE;
      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = ZArgumentProcessor::getStringArg("-o");
    } else*/ if (Is_Arg_Matched(const_cast<char*>("--skeletonize"))) {
      command = SKELETONIZE;

      //    m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
    } else if (Is_Arg_Matched(const_cast<char*>("--separate"))) {
      command = SEPARATE_IMAGE;
//      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_input.push_back(Get_String_Arg(const_cast<char*>("--separate")));
      m_output = Get_String_Arg(const_cast<char*>("-o"));
    } else if (Is_Arg_Matched(const_cast<char*>("--trace"))) {
//      m_input.push_back(ZArgumentProcessor::getStringArg("input", 0));
      m_output = Get_String_Arg(const_cast<char*>("-o"));
      command = TRACE_NEURON;
    } else if (Is_Arg_Matched(const_cast<char*>("--test"))) {
      command = TEST_SELF;
    }
  }

  switch (command) {
  case OBJECT_MARKER:
    return runObjectMarker();
  case BOUNDARY_ORPHAN:
    return runBoundaryOrphan();
  case OBJECT_OVERLAP:
    return runObjectOverlap();
  case SYNAPSE_OBJECT:
    return runSynapseObjectList();
  case CLASS_LIST:
    return runOutputClassList();
  case FLYEM_NEURON_FEATURE:
    return runComputeFlyEmNeuronFeature();
  case SKELETONIZE:
    return runSkeletonize();
  case SEPARATE_IMAGE:
    return runImageSeparation();
  case TRACE_NEURON:
    return runTraceNeuron();
  case TEST_SELF:
    return runTest();
  default:
    std::cout << "Unknown command" << std::endl;
    return 1;
  }

  return 0;
}

void ZCommandLine::loadConfig(const std::string &filePath)
{
//  ZJsonObject m_configJson;
  m_configJson.load(filePath);

  expandConfig(filePath, "skeletonize");
  expandConfig(filePath, "trace");

#ifdef _DEBUG_
  std::cout << m_configJson.dumpString(2) << std::endl;
#endif
}

std::string ZCommandLine::extractIncludePath(
    const string &configFilePath, const string &key)
{
  QDir configDir = QDir(ZString::dirPath(configFilePath).c_str());

  QString filePath;

  ZJsonObject subJson(m_configJson.value(key.c_str()));

  if (subJson.hasKey("include")) {
    QFileInfo fileInfo(ZJsonParser::stringValue(subJson["include"]));
    if (fileInfo.isRelative()) {
      filePath = configDir.absoluteFilePath(fileInfo.filePath());
    } else {
      filePath = fileInfo.absoluteFilePath();
    }
  }

  return filePath.toStdString();
}

void ZCommandLine::expandConfig(
    const std::string &configFilePath, const std::string &objKey)
{
  if (m_configJson.hasKey(objKey.c_str())) {
    std::string includeFilePath = extractIncludePath(configFilePath, objKey);

    if (!includeFilePath.empty()) {
      if (fexist(includeFilePath.c_str())) {
        ZJsonObject subJson(m_configJson.value(objKey.c_str()));
        ZJsonObject includeJson;
        includeJson.load(includeFilePath.c_str());

        const char *key;
        json_t *value;
        ZJsonObject_foreach(includeJson, key, value) {
          if (!subJson.hasKey(key)) {
            ZJsonValue obj =
                ZJsonValue(value, ZJsonValue::SET_INCREASE_REF_COUNT);
            subJson.setEntry(key, obj);
          }
        }
        subJson.removeKey("include");
      } else {
        std::cout << "Missing include file: " << includeFilePath << std::endl;
      }
    }
  }
}
