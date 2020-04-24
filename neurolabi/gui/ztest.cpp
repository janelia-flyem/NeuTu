//#define _NEUTU_USE_REF_KEY_
#include "ztest.h"

#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QUndoCommand>
#include <QUndoStack>
#include <QImage>
#include <QDateTime>
#include <QPainter>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QTime>
#include <QTimer>
#include <QProcess>
#include <QtCore>
#include <iostream>
#include <ostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
//#include <boost/filesystem.hpp>
#if defined(_USE_WEBENGINE_)
#include <QWebEngineView>
#endif

#if defined(_QT5_)
#include <QtConcurrent>
#else
#include <QtConcurrentRun>
#endif

//#include <sys/time.h>
//#include <sys/resource.h>
#include <string>
#include <set>
#include <unordered_set>
#include <vtkOBBTree.h>
#include <vtkPolyData.h>

#if defined(_ENABLE_LOWTIS_)
#include <lowtis/LowtisConfig.h>
#endif

#include <draco/mesh/mesh.h>
#include <draco/point_cloud/point_cloud.h>
#include <draco/compression/decode.h>
#include <draco/compression/encode.h>

#include "tz_sp_grow.h"
#include "tz_stack_bwmorph.h"
#include "tz_stack_stat.h"
#include "tz_stack_attribute.h"
#include "tz_graph_defs.h"
#include "tz_graph_utils.h"
#include "tz_workspace.h"
#include "tz_graph.h"
#include "tz_stack_objlabel.h"
#include "tz_stack_threshold.h"
#include "tz_color.h"
#include "tz_swc_tree.h"

#include "geometry/zaffinerect.h"
#include "filesystem/utilities.h"
#include "tr1_header.h"
#include "zopencv_header.h"
#include "zglobal.h"
#include "neutube.h"
#include "imgproc/zstackprocessor.h"
#include "zfilelist.h"
#include "zspgrowparser.h"
//#include "zvoxelarray.h"
#include "zsuperpixelmaparray.h"
#include "zsegmentmaparray.h"
//#include "tz_xml_utils.h"
#include "zswctree.h"
#include "zswcforest.h"
#include "znormcolormap.h"
#include "dialogs/flyemskeletonizationdialog.h"
//#include "zstackaccessor.h"
#include "zmatrix.h"
#include "zswcbranch.h"
#include "zswctreematcher.h"
#include "widgets/z3dtabwidget.h"
#include "dialogs/ztestdialog.h"
#include "dialogs/parameterdialog.h"
#include "zstring.h"
#include "zdialogfactory.h"
#include "zrandomgenerator.h"
#include "zjsonobject.h"
#include "geometry/zpoint.h"
#include "zpixmap.h"
#include "flyem/zfileparser.h"
#include "zstackpatch.h"
#include "zswcgenerator.h"
#include "zpunctumio.h"
#include "tz_stack_math.h"
#include "flyem/zsynapseannotationarray.h"
#include "flyem/zfileparser.h"
#include "flyem/zsynapseannotationanalyzer.h"
#include "flyem/zneuronnetwork.h"
#include "tz_geo3d_utils.h"
#include "zsvggenerator.h"
#include "flyem/zfileparser.h"
#include "zdendrogram.h"
#include "zobject3dscanarray.h"
#include "geometry/zcuboid.h"
#include "widgets/zstringparameter.h"
#include "zswcsizefeatureanalyzer.h"
#include "zobject3darray.h"
#include "zswcshollfeatureanalyzer.h"
#include "zswcspatialfeatureanalyzer.h"
#include "swctreenode.h"
#include "widgets/zparameterarray.h"
#include "zviewproj.h"
#include "zswcnetwork.h"
#include "neurolabi/zdoublevector.h"
#include "zswcdisttrunkanalyzer.h"
#include "zswcbranchingtrunkanalyzer.h"
#include "flyem/zflyemroiproject.h"
#include "flyem/zsynapselocationmatcher.h"
#include "flyem/zsynapselocationmetric.h"
#include "zstackfile.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "zwindowfactory.h"
#include "flyem/zsegmentationanalyzer.h"
#include "flyem/zsegmentationbundle.h"
#include "flyem/zflyemneuronmatchtaskmanager.h"
#include "zstackblender.h"
#include "zgraph.h"
#include "zarray.h"
#include "zintpairmap.h"
#include "tz_u8array.h"
#include "flyem/zflyembodyannotation.h"
#include "zfiletype.h"
#include "tz_geometry.h"
#include "z3dgraph.h"
#include "zpunctum.h"
#include "zswctreenodeselector.h"
#include "zswcsizetrunkanalyzer.h"
#include "zswcweighttrunkanalyzer.h"
#include "imgproc/zstackbinarizer.h"
#include "widgets/zoptionparameter.h"
#include "zdebug.h"
#include "tz_color.h"
#include "zhdf5reader.h"
#include "tz_farray.h"
#include "misc/zmarchingcube.h"
#include "zxmldoc.h"
#include "neutubeconfig.h"
#include "zhdf5writer.h"
#include "zmesh.h"
#include "zmeshio.h"
#include "flyem/zbcfset.h"
#include "flyem/zflyemstackframe.h"
#include "zmoviemaker.h"
#include "mvc/zstackdoc.h"
#include "bigdata/zstackblockgrid.h"
#include "z3dwindow.h"
#include "flyem/zhotspot.h"
#include "flyem/zhotspotarray.h"
#include "flyem/zhotspotfactory.h"
#include "z3dswcfilter.h"
#include "z3dinteractionhandler.h"
#include "z3dcompositor.h"
#include "zjsonfactory.h"
#include "z3dvolumeraycasterrenderer.h"
#include "flyem/zproofreadwindow.h"
#include "z3dpunctafilter.h"
#include "tz_stack.h"
#include "zswclayerfeatureanalyzer.h"
#include "flyem/zflyemdatabundle.h"
#include "mainwindow.h"
#include "zmoviescriptgenerator.h"
#include "zobject3dscan.h"
#include "zswclayertrunkanalyzer.h"
#include "zswclayershollfeatureanalyzer.h"
#include "zstackgraph.h"
#include "zgraphcompressor.h"
#include "zswcpositionadjuster.h"
#include "zgraph.h"
#include "tz_cuboid_i.h"
#include "zswcglobalfeatureanalyzer.h"
#include "zlogmessagereporter.h"
#include "zerror.h"
#include "zmatlabprocess.h"
#include "flyem/zflyemneuronexporter.h"
#include "flyem/zflyemneuronarray.h"
#include "flyem/zflyembodyanalyzer.h"
#include "swc/zswcresampler.h"
#include "flyem/zflyemneuronfeatureanalyzer.h"
#include "swc/zswcnodedistselector.h"
#include "zmultitaskmanager.h"
#include "flyem/zflyembodywindowfactory.h"
#include "dvid/zdvidbufferreader.h"
#include "dvid/zdvidenv.h"
#include "misc/miscutility.h"
#include "imgproc/zstackprinter.h"
#include "zneurontracer.h"

#include "swc/zswcterminalsurfacemetric.h"

#include "zswcgenerator.h"
#include "zrect2d.h"
#include "z3dmainwindow.h"
#include "misc/zvtkutil.h"
#include "flyem/zfileparser.h"

#include "ztextlinecompositer.h"
#include "zstackskeletonizer.h"
#include "flyem/zflyemcoordinateconverter.h"
#include "zstringarray.h"
#include "zflyemdvidreader.h"
#include "zstroke2d.h"
#include "flyem/zflyemservice.h"
#include "zintset.h"
#include "zstackfactory.h"
#include "zsparseobject.h"

#include "bigdata/zdvidblockgrid.h"

#include "flyem/zflyembookmark.h"
#include "flyem/zflyembookmarkarray.h"
#include "flyem/flyemdatareader.h"

//#include "zcircle.h"

#include "dvid/libdvidheader.h"

#include "imgproc/zstackwatershed.h"
#include "flyem/zflyembodymerger.h"

#include "tz_int_histogram.h"
#include "zsegmentationproject.h"
#include "zstackviewmanager.h"
#include "flyem/zflyemneuronbodyinfo.h"
#include "flyem/zflyemneurondensitymatcher.h"
#include "flyem/zflyemneurondensity.h"
#include "dvid/zdvidversiondag.h"
#include "jneurontracer.h"
#include "biocytin/swcprocessor.h"
#include "zcommandline.h"
#include "z3dgraphfactory.h"
#include "flyem/zflyemsupervisor.h"
#include "flyem/zflyembody3ddoc.h"
#include "mvc/zstackview.h"
#include "flyem/zflyemproofdoc.h"
#include "zswcfactory.h"
#include "biocytin/zbiocytinprojmaskfactory.h"
#include "zsleeper.h"
#include "flyem/zflyemneuroninfo.h"
#include "zlinesegmentobject.h"
#include "mvc/zstackmvc.h"
//#include "misc/zstackyzmvc.h"
#include "dvid/zdvidlabelslice.h"

#include "dialogs/zflyemsplituploadoptiondialog.h"
#include "dialogs/zstresstestoptiondialog.h"

#include "zflyemutilities.h"
#include "dialogs/zcomboeditdialog.h"
#include "zlocalneuroseg.h"
#include "dialogs/zflyembodycomparisondialog.h"
#include "zstackwriter.h"
#include "zstackreader.h"

#include "qt/network/znetworkutils.h"
#include "zdvidutil.h"
#include "dvid/libdvidheader.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidpath.h"
#include "dvid/zdvidtileensemble.h"
#include "dvid/zdvidsynapse.h"
#include "dvid/zdvidsynapseensenmble.h"
#include "dvid/zdvidtile.h"
#include "dvid/zdvidtileinfo.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidwriter.h"
#include "dvid/zdvidinfo.h"
#include "dvid/zdvidroi.h"
#include "dvid/zdvidtargetfactory.h"
#include "dvid/zdvidneurontracer.h"
#include "dvid/zdvidgrayslice.h"
#include "dvid/zdvidbodyhelper.h"
#include "dvid/zdvidurl.h"
#include "dvid/zdvidstackblockfactory.h"
#include "dvid/zdvidneurontracer.h"

#include "widgets/ztextedit.h"
#include "dialogs/stringlistdialog.h"
#include "widgets/zbodylistwidget.h"
#include "zcontrastprotocol.h"
#include "zmeshfactory.h"
#include "widgets/taskprotocolwindow.h"
#include "zstackdoccommand.h"
#include "zstackobjectinfo.h"
#include "sandbox/zbrowseropener.h"
#include "widgets/zpythonprocess.h"
#include "zmenuconfig.h"
#include "zmeshutils.h"
#include "zarrayfactory.h"
#include "zstackblocksource.h"
#include "neutuse/taskwriter.h"
#include "neutuse/task.h"
#include "neutuse/taskfactory.h"
#include "qt/network/znetbufferreader.h"
#include "common/memorystream.h"
#include "service/neuprintreader.h"
#include "zjsonparser.h"
#include "zjsonobjectparser.h"
#include "widgets/zoptionlistwidget.h"
#include "dialogs/neuprintquerydialog.h"
#include "service/cypherquery.h"
#include "test/zunittest.h"
#include "zstackobjectpainter.h"
#include "logging/neuopentracing.h"
#include "logging/zlog.h"
#include "logging/utilities.h"
#include "zsysteminfo.h"
#include "neulib/core/utilities.h"

#include "flyem/zglobaldvidrepo.h"
#include "flyem/zflyemarbmvc.h"
#include "flyem/zflyembodystatus.h"
#include "flyem/zflyembodyannotationprotocol.h"
#include "flyem/zflyemroimanager.h"
#include "flyem/flyemdatawriter.h"
#include "flyem/zflyemmeshfactory.h"
#include "flyem/neuroglancer/zneuroglancerlayerspecfactory.h"
#include "flyem/neuroglancer/zneuroglancerpath.h"
#include "flyem/neuroglancer/zneuroglancerannotationlayerspec.h"
#include "flyem/neuroglancer/zneuroglancerpathfactory.h"
#include "flyem/neuroglancer/zneuroglancerpathparser.h"
#include "flyem/zflyemproofmvc.h"
#include "flyem/zflyemorthomvc.h"
#include "flyem/zflyemorthodoc.h"
#include "flyem/zflyemorthowindow.h"
#include "flyem/zneutuservice.h"
#include "flyem/zstackwatershedcontainer.h"
#include "flyem/zserviceconsumer.h"
#include "flyem/zflyemroiproject.h"
#include "flyem/zflyemmisc.h"
#include "flyem/dialogs/flyembodyannotationdialog.h"
#include "flyem/zdvidtileupdatetaskmanager.h"
#include "flyem/zflyemarbdoc.h"

#include "ext/http/HTTPRequest.hpp"

//#include "test/ztestall.h"

using namespace std;

ostream& ZTest::m_failureStream = cerr;

ZTest::ZTest()
{
}

#ifdef _JANELIA_WORKSTATION_
const static string dataPath("/groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/data");
#else
const static string dataPath("/Users/zhaot/Work/neutube/neurolabi/data");
#endif

void ZTest::setCommandLineArg(int argc, char *argv[])
{
  m_argc = argc;
  m_argv = argv;
}


void ZTest::runUnitTest()
{
  RunUnitTest(m_argc, m_argv);
}

int ZTest::RunUnitTest(int argc, char *argv[])
{
  return ZUnitTest(argc, argv).run();
}

void ZTest::CommandLineTest()
{
  ZCommandLine command;
  command.runTest();
}

void ZTest::CrashTest()
{
  /*
  int a[1];
  a[1] = 1;
  */

#if 0
  int *a = new int;

  delete a;
  delete a;
#endif

#if 0
  ZSwcTree *obj1 = new ZSwcTree;
  obj1->setSource(ZStackObjectSourceFactory::MakeWatershedBoundarySource(1));
  if (1) {
    ZSwcTree obj2;
    obj2.setSource(ZStackObjectSourceFactory::MakeWatershedBoundarySource(1));
    obj1 = &obj2;
  }

  std::cout << obj1->getType() << std::endl;
  std::cout << obj1->getSource() << std::endl;
#endif

}

void ZTest::stressTest(MainWindow *host)
{
  if (host != NULL) {
#if defined(_FLYEM_)
    host->testFlyEmProofread();
#else
    ZStackFrame *frame = host->currentStackFrame();
    if (frame != NULL) {
      frame->stressTest();
    }
#endif
  }
}

void ZTest::test(MainWindow *host)
{
  std::cout << "Start testing ..." << std::endl;

  std::cout << host << std::endl;

#if 0
  ZStackFrame *frame = (ZStackFrame *) mdiArea->currentSubWindow();
  if (frame != NULL) {
    //QMessageBox::question(frame, tr("test"), tr("change size"),
    //			  QMessageBox::Yes | QMessageBox::No);
    frame->resize(frame->view()->imageWidget()->screenSize().width(),
      frame->height());
  }
#endif

#if 0
  ::testtrace();
  Stack *stack = Read_Stack("../data/diadem_e1.tif");
  LocationSimple *ptarray = new LocationSimple[3];
  ptarray[0].x = 165;
  ptarray[0].y = 248;
  ptarray[0].z = 40;
  ptarray[0].radius = 3;

  ptarray[1].x = 195;
  ptarray[1].y = 262;
  ptarray[1].z = 43;
  ptarray[1].radius = 3;

  ptarray[0].x = 231;
  ptarray[0].y = 249;
  ptarray[0].z = 44;
  ptarray[0].radius = 3;

  main_zhaot_neuron_tracing(stack->array, stack->width, stack->height, stack->depth,
      ptarray, 3);

  delete []ptarray;
  Kill_Stack(stack);
#endif

#if 0
  //The minimum and maximum is the number of steps in the operation for which this progress dialog shows progress.
  //for example here 0 and 100.
  QProgressDialog* progress = new QProgressDialog("Fetching data...", "Cancel", 0, 100);

  //Set dialog as modal dialog, if you want.
  progress->setWindowModality(Qt::WindowModal);
//  progress->show();
  for (int i = 0; i <= 100; i++) {
    progress->setValue(i);
    ZSleeper::sleep(1);
  }
//  delete progress;
#endif

#if 0
//  m_progress->setRange(0, 0);
//  m_progress->show();
//  QApplication::processEvents();
  currentStackFrame()->document()->test();
//  currentStackFrame()->updateView();
//  m_progress->reset();
#endif

#if 0
  QGraphicsScene *scene = new QGraphicsScene(0);
  scene->clear();

  QGraphicsSvgItem *m_svgItem = new QGraphicsSvgItem("../data/test.svg");
      //new QGraphicsSvgItem("/Developer/Examples/Qt/painting/svgviewer/files/spheres.svg");
  m_svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
  m_svgItem->setCacheMode(QGraphicsItem::NoCache);
  m_svgItem->setZValue(0);

  scene->addItem(m_svgItem);

  QGraphicsView *gv = new QGraphicsView(scene, 0);
  gv->show();

#endif

#if 0
  QProgressDialog *pd = new QProgressDialog("Testing", "Cancel", 0, 100, this);

  pd->setRange(0, 100);
  pd->show();
  QApplication::processEvents();

  loadFile("../data/benchmark/mouse_neuron_single/stack.tif");

  pd->setValue(m_progress->value() + 10);
  pd->show();
  QApplication::processEvents();

  if (currentStackFrame() != NULL) {
    currentStackFrame()->presenter()->autoTrace();
    currentStackFrame()->updateView();
  }

  pd->setValue(m_progress->value() + 10);
  pd->show();
  QApplication::processEvents();

  currentStackFrame()->document()->
      exportSwcTree("../data/benchmark/mouse_neuron_single/auto.swc");

  pd->setValue(m_progress->maximum());
  pd->reset();

  delete pd;

  QMessageBox::information(currentStackFrame(), "Testing Completed",
                           "No problem found.", QMessageBox::Ok);
#endif

#if 0
  BcAdjustDialog dlg;
  dlg.setRange(0, 255);
  dlg.setValue(10, 100);
  dlg.exec();
#endif


#if 0
  ZStackFrame *frame = new ZStackFrame(this);
  const char *filePath = "E:\\data\\diadem\\diadem1\\nc_01.tif";
  Mc_Stack *stack = Read_Mc_Stack(filePath, -1);
  frame->document()->loadStack(Mc_Stack_To_Stack(stack, -1, NULL));
  frame->document()->setStackSource(filePath);
  frame->setWindowTitle(filePath);
  frame->presenter()->optimizeStackBc();
  frame->view()->reset();
  setCurrentFile(filePath);
  addStackFrame(frame);
#endif

#if 0
  ZSwcTree tree1;
  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc2/C2_214.swc");

  std::vector<double> angleArray = tree1.computeAllContinuationAngle();
  double mu = darray_mean(angleArray.data(),
                          angleArray.size());
  double sigma = sqrt(darray_var(angleArray.data(),
                                 angleArray.size()));

  std::vector<double> branchAngleArray = tree1.computeAllBranchingAngle();
  double branchMu = darray_mean(branchAngleArray.data(),
                                branchAngleArray.size());
  double branchSigma = sqrt(darray_var(branchAngleArray.data(),
                                       branchAngleArray.size()));

  double probBranch = static_cast<double>(branchAngleArray.size()) /
      (angleArray.size() + branchAngleArray.size());

  int n = tree1.size();

  ZSwcTree *tree = ZSwcTree::generateRandomSwcTree(n, probBranch, mu, sigma,
                                                   branchMu, branchSigma);

  angleArray = tree->computeAllContinuationAngle();
  mu = darray_mean(angleArray.data(),
                            angleArray.size());
  sigma = sqrt(darray_var(angleArray.data(),
                                   angleArray.size()));

  tree->resortId();
  tree->save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZSwcTree tree1;
  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/C2_214.swc");
  ZSwcTree tree2;
  tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/L1_209.swc");
  ZSwcTree tree3;
  tree3.load("/Users/zhaot/Work/neutube/neurolabi/data/demo/circuit/Mi1_215.swc");

  ZSwcTree connect_tree;
  connect_tree.load(
        "/Users/zhaot/Work/neutube/neurolabi/data/209_214.swc");

  Swc_Tree *raw_tree1 = tree1.data();
  Swc_Tree *raw_tree2 = tree2.data();
  Swc_Tree *raw_tree3 = tree3.data();

  double bound1[6], bound2[6], bound3[6];
  Swc_Tree_Bound_Box(raw_tree1, bound1);
  Swc_Tree_Bound_Box(raw_tree2, bound2);
  Swc_Tree_Bound_Box(raw_tree3, bound3);

  Cuboid_I cuboid1;
  Cuboid_I cuboid2;
  Cuboid_I cuboid3;
  Cuboid_I intersect_cuboid;
  Cuboid_I union_cuboid;

  Cuboid_I_Set_S(&cuboid1, iround(bound1[0]), iround(bound1[1]),
                 iround(bound1[2]), iround(bound1[3] - bound1[0]),
                 iround(bound1[4] - bound1[1]),
                 iround(bound1[5] - bound1[2]));
  Cuboid_I_Set_S(&cuboid2, iround(bound2[0]), iround(bound2[1]),
                 iround(bound2[2]),
                 iround(bound2[3] - bound2[0]),
                 iround(bound2[4] - bound2[1]),
                 iround(bound2[5] - bound2[2]));
  Cuboid_I_Set_S(&cuboid3, iround(bound3[0]), iround(bound3[1]),
                 iround(bound3[2]),
                 iround(bound3[3] - bound3[0]),
                 iround(bound3[4] - bound3[1]),
                 iround(bound3[5] - bound3[2]));

  Cuboid_I_Union(&cuboid1, &cuboid2, &union_cuboid);
  Cuboid_I_Intersect(&cuboid1, &cuboid2, &intersect_cuboid);
  double scale;

  if (Cuboid_I_Is_Valid(&intersect_cuboid)) {
    for (int i = 0; i < 3; i++) {
      int margin = intersect_cuboid.cb[i] - union_cuboid.cb[i];
      int distance = intersect_cuboid.ce[i] - intersect_cuboid.cb[i] + 1;
      scale = static_cast<double>(distance) / static_cast<double>(margin);
      break;
    }
  }

  Cuboid_I_Intersect(&cuboid1, &cuboid3, &intersect_cuboid);

  if (Cuboid_I_Is_Valid(&intersect_cuboid)) {

    for (int i = 0; i < 3; i++) {
      int margin = intersect_cuboid.cb[i] - union_cuboid.cb[i];
      int distance = intersect_cuboid.ce[i] - intersect_cuboid.cb[i] + 1;
      scale = std::max(scale,
                       static_cast<double>(distance) /
                       static_cast<double>(margin));
      break;
    }
  }
  double offset1[3] = {0.0, 0.0, 0.0};
  double offset2[3] = {0.0, 0.0, 0.0};
  double offset3[3] = {0.0, 0.0, 0.0};

  for (int i = 0; i < 2; i++) {
    offset1[i] = scale * (cuboid1.cb[i] - union_cuboid.cb[i]);
    offset2[i] = scale * (cuboid2.cb[i] - union_cuboid.cb[i]);
    offset3[i] = scale * (cuboid3.cb[i] - union_cuboid.cb[i]);
  }

  Swc_Tree_Translate(raw_tree1, offset1[0], offset1[1], offset1[2]);
  Swc_Tree_Translate(raw_tree2, offset2[0], offset2[1], offset2[2]);
  Swc_Tree_Translate(raw_tree3, offset3[0], offset3[1], offset3[2]);

  connect_tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, _FALSE_);

  for (Swc_Tree_Node *tn = connect_tree.begin(); tn != NULL;
       tn = connect_tree.next()) {
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Is_Root(tn)) {
        tn->node.type = 6;
      } else {
        tn->node.type = 1;
      }
      if (Swc_Tree_Node_Label(tn) == 209) {
        Swc_Tree_Node_Translate(tn, offset1[0], offset1[1], offset1[2]);
        tree1.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else if (Swc_Tree_Node_Label(tn) == 214) {
        Swc_Tree_Node_Translate(tn, offset2[0], offset2[1], offset2[2]);
        tree2.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else if (Swc_Tree_Node_Label(tn) == 215) {
        Swc_Tree_Node_Translate(tn, offset3[0], offset3[1], offset3[2]);
        tree3.moveToSurface(&(tn->node.x), &(tn->node.y), &(tn->node.z));
      } else {
        Swc_Tree_Node_Merge_To_Parent(tn);
      }
    }
  }

  tree1.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  tree2.save("/Users/zhaot/Work/neutube/neurolabi/data/test2.swc");
  tree3.save("/Users/zhaot/Work/neutube/neurolabi/data/test3.swc");
  connect_tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test4.swc");
#endif


#if 0
  ZCuboid cuboid1(0, 0, 0, 2, 1, 3);
  ZCuboid cuboid2(0, 0, 0, 2, 1, 3);

  cuboid1.moveOutFrom(cuboid2, 1);

  std::ofstream stream("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  stream << cuboid1[0] << ' ' << cuboid1[1] << ' ' << cuboid1[2] << ' '
         << cuboid1[3] << ' ' << cuboid1[4] << ' ' << cuboid1[5] << std::endl;

  stream << cuboid2[0] << ' ' << cuboid2[1] << ' ' << cuboid2[2] << ' '
         << cuboid2[3] << ' ' << cuboid2[4] << ' ' << cuboid2[5] << std::endl;

  stream.close();
#endif

#if 0
  ZCuboid b0(0, 0, 0, 2, 3, 10);

  std::vector<ZCuboid> cuboidArray;
  cuboidArray.push_back(ZCuboid(1, 1, 0, 4, 4, 10));
  cuboidArray.push_back(ZCuboid(5, 1, 0, 8, 4, 10));
  cuboidArray.push_back(ZCuboid(7, 0, 0, 9, 5, 10));
  cuboidArray.push_back(ZCuboid(3, 2, 0, 6, 6, 10));
  cuboidArray.push_back(ZCuboid(4, 5, 0, 5, 8, 10));
  cuboidArray.push_back(ZCuboid(2, 7, 0, 8, 9, 10));

  std::ofstream stream("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  stream << b0[0] << ' ' << b0[1] << ' ' << b0[2] << ' '
         << b0[3] << ' ' << b0[4] << ' ' << b0[5] << std::endl;

  b0.layout(&cuboidArray, 1.0);

  for (size_t i = 0; i < cuboidArray.size(); i++) {
    stream << cuboidArray[i][0] << ' ' << cuboidArray[i][1] << ' '
           << cuboidArray[i][2] << ' '
           << cuboidArray[i][3] << ' ' << cuboidArray[i][4] << ' '
           << cuboidArray[i][5] << std::endl;
  }

  stream.close();
#endif

#if 0
  FlyEm::ZSwcNetwork network;
  network.import("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");
  //network.import(
  //      "/Users/zhaot/Work/neutube/neurolabi/data/benchmark/flyem/network.txt");

  network.exportSwcFile("/Users/zhaot/Work/neutube/neurolabi/data/test.swc",
                        209);
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc3/adjusted/Y6_3_6097.swc");
  ZSwcBranch *branch = tree.extractFurthestBranch();
  branch->label(3);

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  delete branch;
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/result/swc3/adjusted/C2_214.swc");

  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  tree.label(0);

  ZSwcBranch *branch = tree.extractFurthestBranch();
  branch->label(1);

  tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST, _FALSE_);

  //For every node in the tree
  Swc_Tree_Node *tn;
  for (tn = tree.begin(); tn != tree.end(); tn = tree.next()) {
    //If the node is not labeled and its parent is labeled
    if (Swc_Tree_Node_Is_Regular(tn)) {
      if (Swc_Tree_Node_Label(tn) == 0 && Swc_Tree_Node_Label(tn->parent) > 0) {
        //Get the downstream leaves
        vector<Swc_Tree_Node*> leafArray = tree.extractLeaf(tn);
        //Get the furthest leaf
        Swc_Tree_Node *leaf = leafArray[0];
        double maxDist = Swc_Tree_Node_Dist(tn, leaf);

        for (size_t i = 0; i < leafArray.size(); i++) {
          double dist = Swc_Tree_Node_Dist(tn, leafArray[i]);
          if (dist > maxDist) {
            dist = maxDist;
            leaf = leafArray[i];
          }
        }

        //Label the branch from the leaf to the node parent with parent.label+1
        Swc_Tree_Node_Set_Label(tn, Swc_Tree_Node_Label(tn->parent) + 1);
        while (leaf != tn) {
          Swc_Tree_Node_Set_Label(leaf, Swc_Tree_Node_Label(tn));
          leaf = leaf->parent;
        }
      }
    }
  }

  Swc_Tree_Set_Type_As_Label(tree.data());

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
  delete branch;
#endif

#if 0
  FlyEm::ZSwcNetwork network;
  //network.import(
  //      "/Users/zhaot/Work/neutube/neurolabi/data/benchmark/flyem/network.txt");

  network.import("/Users/zhaot/Work/neutube/neurolabi/data/test.txt");

  Graph *graph = network.toGraph();
  Graph_To_Dot_File(graph, "/Users/zhaot/Work/neutube/neurolabi/data/test.dot");
  Kill_Graph(graph);

  network.layoutSwc();

  network.exportSwcFile("/Users/zhaot/Work/neutube/neurolabi/data/test.swc",
                        209, FlyEm::ZSwcNetwork::EXPORT_ALL);

#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/single_branch.swc");
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/fork2.swc");
  tree.resample(1000.0);
  tree.resortId();
  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;

  tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/Mi1_215.swc");
  tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/Mi1a_446263.swc");
  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare2.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare3.swc");

  ZSwcTree *originalTree1 = tree1.clone();
  ZSwcTree *originalTree2 = tree2.clone();

  tree1.resample(20.0);
  tree2.resample(20.0);

  //tree2.translate(10, 0, 50.0);

  ZSwcTreeMatcher matcher;
  ZSwcLayerFeatureAnalyzer analyzer;
  //ZSwcSizeFeatureAnalyzer analyzer;
  //ZSwcShollFeatureAnalyzer analyzer;

  //analyzer.setExcludedLabel(0);
  matcher.setFeatureAnalyzer(&analyzer);

  matcher.matchAll(tree1, tree2);

  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
      matcher.matchingResult();

  ZSwcTree tree;
  tree.merge(originalTree1->data(), false);
  tree.merge(originalTree2->data(), false);

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {
      ZPoint startPos(matchingResult[i].first->node.x,
                      matchingResult[i].first->node.y,
                      matchingResult[i].first->node.z);
      ZPoint endPos(matchingResult[i].second->node.x,
                    matchingResult[i].second->node.y,
                    matchingResult[i].second->node.z);

      Swc_Tree_Node *tn =
          ZSwcTree::makeArrow(startPos, 1, 6, endPos, 0.2, 0);
      Swc_Tree_Node_Set_Parent(tn, tree.data()->root);
    }
  }

  tree.resortId();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  ZObject3dArray objArray;
  ZObject3d obj;
  obj.append(0, 0, 0);
  obj.append(0, 0, 1);
  objArray.append(obj);

  obj.clear();
  obj.append(0, 1, 0);
  obj.append(1, 1, 0);
  objArray.append(obj);

  objArray.writeIndex("/Users/zhaot/Work/neutube/neurolabi/data/test.txt",
                      100, 100, 100);
#endif

#if 0
  ZObject3dArray objArray;
  objArray.readIndex("/Users/zhaot/Work/neutube/neurolabi/data/test.txt",
                     100, 100, 100);
  cout << objArray.size() << endl;

  objArray.writeIndex("/Users/zhaot/Work/neutube/neurolabi/data/test2.txt",
                      100, 100, 100);
#endif

#if 0
  ZString str("L1_209.swc");
  std::vector<std::string> parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/L1_209.swc/";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/L1_209.swc";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/L1_209";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  str = "/test.test/";
  parts = str.fileParts();
  for (std::vector<std::string>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter) {
    std::cout << *iter << std::endl;
  }
#endif

#if 0
  ZString str("L1_209.swc");
  cout << str.changeExt("tif") << endl;

  str = "/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/L1_209.swc/";
  cout << str.changeExt("tif") << endl;

  str = "test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/L1_209.swc";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/L1_209";
  cout << str.changeExt("tif") << endl;

  str = "/test.test/";
  cout << str.changeExt("tif") << endl;
#endif

#if 0
  ZString str("L1_209.swc");
  cout << str.changeDirectory("test") << endl;

  str = "/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/L1_209.swc/";
  cout << str.changeDirectory("test") << endl;

  str = "test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/L1_209.swc";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/L1_209";
  cout << str.changeDirectory("test") << endl;

  str = "/test.test/";
  cout << str.changeDirectory("/test") << endl;
#endif

#if 0
  //Read a tree
  ZSwcTree tree;
  //tree.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");

  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  //Generate 100 random trees and save the trees above a center size threshold
  ZString prefix = "/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p";
  tree.save((prefix + ".swc").c_str());

  for (int i = 0; i < 5; i++) {
    ZSwcTree *partialTree = tree.clone();
    partialTree->removeRandomBranch();
    ZString filePath = prefix;

    filePath.appendNumber(i + 1, 3);

    filePath += ".swc";

    cout << filePath << endl;

    partialTree->save(filePath.c_str());

    cout << partialTree->size() << endl;

    delete partialTree;
  }

  //Compare each random tree with the original tree
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/breadth_first.swc");
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  tree.labelTrunkLevel(&trunkAnalyzer);

  cout << SwcTreeNode::downstreamSize(tree.data()->root,
                                      SwcTreeNode::labelDifference) << endl;

  Swc_Tree_Node *tn = tree.queryNode(2);
  cout << SwcTreeNode::downstreamSize(tn,
                                      SwcTreeNode::labelDifference) << endl;

#endif

#if 0
  ZSwcTree *tree1;
  ZSwcTree *tree2;


  vector<string> fileList;
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2_214.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2c_3668.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2d_207375.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2e_228998.swc");
  fileList.push_back("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/C2f_445362.swc");
  //const char *swcFile = "/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p.swc";

  vector<ZSwcTree*> treeArray(fileList.size());

  ZSwcTree *maxTree = NULL;
  int maxSize = 0;

  for (size_t i = 0; i < fileList.size(); i++) {
    treeArray[i] = new ZSwcTree;
    treeArray[i]->load(fileList[i].c_str());
    if (maxSize < treeArray[i]->size()) {
      maxSize = treeArray[i]->size();
      maxTree = treeArray[i];
    }
  }

  cout << maxTree->source() << endl;

  tree1 = maxTree;
  tree2 = treeArray[1];
  /*
  for (size_t i = 0; i < fileList.size(); i++) {
    if (tree1 != treeArray[i]) {
      tree2 = treeArray[i];
      break;
    }
  }
  */

  /*
  tree1.load(swcFile);
  tree2.load(swcFile);

  tree2.removeRandomBranch();

  while (tree2.regularDepth() < 2) {
    tree2.load(swcFile);
    tree2.removeRandomBranch();
  }
*/


  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt000.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt001.swc");

  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/L1_209/p005.swc");

  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1_209.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/L1d_181639.swc");
  //tree1.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");
  //tree2.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare3.swc");

  tree1->resample(50.0);
  tree2->resample(50.0);

  //tree2.translate(0, 0, 50.0);

  ZSwcTreeMatcher matcher;

  //ZSwcSpatialFeatureAnalyzer analyzer;



  ZSwcShollFeatureAnalyzer analyzer;
  vector<double> parameterArray(3);
  parameterArray[0] = 5.0;
  parameterArray[1] = 100.0;
  parameterArray[2] = 30.0;
  analyzer.setParamter(parameterArray);


  /*
  ZSwcSizeFeatureAnalyzer analyzer;
  analyzer.setExcludedLabel(0);
*/

  ZSwcDistTrunkAnalyzer trunkAnalyzer;

  matcher.setFeatureAnalyzer(&analyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(*tree1, *tree2);

  vector<pair<Swc_Tree_Node*, Swc_Tree_Node*> > matchingResult =
      matcher.matchingResult();

  Swc_Tree_Node_Set_Root(matchingResult[0].first);
  tree1->setDataFromNode(matchingResult[0].first, ZSwcTree::LEAVE_ALONE);
  Swc_Tree_Node_Set_Root(matchingResult[0].second);
  tree2->setDataFromNode(matchingResult[0].second, ZSwcTree::LEAVE_ALONE);

  tree1->setType(0);
  tree2->setType(1);


  tree1->resortId();
  tree2->translate(50, 0, 0);
  tree2->resortId();

  ZSwcNetwork network;
  network.addSwcTree(tree1);
  network.addSwcTree(tree2);

  tree1->labelTrunkLevel(&trunkAnalyzer);
  tree2->labelTrunkLevel(&trunkAnalyzer);

  tree1->setTypeByLabel();
  tree2->setTypeByLabel();

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {

      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first,
                                              SwcTreeNode::labelDifference);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second,
                                              SwcTreeNode::labelDifference);
      double weight = 1.0 -
          static_cast<double>(min(size1, size2)) / max(size1, size2);
      /*
      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second);

      int size3 = SwcTreeNode::singleTreeSize(matchingResult[i].first)
                  - size1 + 1;
      int size4 = SwcTreeNode::singleTreeSize(matchingResult[i].second)
                  - size2 + 1;

      cout << "Size: " << size1 << " " << size2 << " " << endl;
      cout << "Size 2: " << size3 << " " << size4 << " " << endl;

      double weight = 1.0 -
          min(static_cast<double>(min(size1, size2)) / max(size1, size2),
              static_cast<double>(min(size3, size4)) / max(size3, size4));
*/
      if (weight > 0.5) {
        network.addConnection(matchingResult[i].first, matchingResult[i].second,
                              weight);
      }
    }
  }

  network.exportTxtFile("/Users/zhaot/Work/neutube/neurolabi/data/test.nnt");

  for (size_t i = 0; i < treeArray.size(); i++) {
    delete treeArray[i];
  }

#if 0
  ZSwcTree tree;
  tree.merge(tree1.data(), false);
  tree2.translate(0, 0, 50.0);
  tree.merge(tree2.data(), false);

  for (size_t i = 0; i < matchingResult.size(); i++) {
    if (!Swc_Tree_Node_Is_Continuation(matchingResult[i].first) ||
        !Swc_Tree_Node_Is_Continuation(matchingResult[i].second)) {
      ZPoint startPos(matchingResult[i].first->node.x,
                      matchingResult[i].first->node.y,
                      matchingResult[i].first->node.z);
      ZPoint endPos(matchingResult[i].second->node.x,
                    matchingResult[i].second->node.y,
                    matchingResult[i].second->node.z);

      int size1 = SwcTreeNode::downstreamSize(matchingResult[i].first);
      int size2 = SwcTreeNode::downstreamSize(matchingResult[i].second);

      size1 = min(size1,
                  SwcTreeNode::singleTreeSize(matchingResult[i].first)
                  - size1 + 1);
      size2 = min(size2,
                  SwcTreeNode::singleTreeSize(matchingResult[i].second)
                  - size2 + 1);

      cout << "Size: " << size1 << " " << size2 << " " << endl;

      int colorCode = 6;

      if (min(size1, size2) * 3 <= max(size1, size2) * 2) {
        colorCode = 2;
      }

      cout << SwcTreeNode::singleTreeSize(matchingResult[i].first) << endl;

      Swc_Tree_Node *tn =
          ZSwcTree::makeArrow(startPos, 1, colorCode, endPos, 0.2, 0);
      Swc_Tree_Node_Set_Parent(tn, tree.data()->root);
    }
  }

  tree.resortId();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/compare/compare1.swc");

  ZSwcDistTrunkAnalyzer analyzer;
  analyzer.labelTraffic(&tree);
  //tree.labelBusyLevel();
  tree.setTypeByLabel();

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/test.swc");
#endif

#if 0
  //Load all swc files
  ZFileList fileList;
  fileList.load("/Users/zhaot/Work/neutube/neurolabi/data/tmp/swc3/adjusted/labeled",
                "swc");

  ZDoubleVector ratio(fileList.size(), 0.0);

  //For each swc file
  for (int i = 0; i < fileList.size(); i++) {
    //Extract the main trunk
    ZSwcTree  tree;
    tree.load(fileList.getFilePath(i));

    ZSwcBranchingTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.setDistanceWeight(1.0, 0.0);
    ZSwcPath branch = tree.mainTrunk(&trunkAnalyzer);

    //Count the nodes that have type 0
    int count = 0;

    for (ZSwcPath::iterator iter = branch.begin(); iter != branch.end();
         ++iter) {
      if (SwcTreeNode::type(*iter) == 0) {
        count++;
      }
    }

    //Calculate the ratio
    ratio[i] = static_cast<double>(count) / branch.size();
  }

  ratio.exportDataFile("/Users/zhaot/Work/neutube/neurolabi/data/test.bn");

#endif

#if 0
  //json project test

#endif

#if 0
  if (testTreeIterator()) {
    cout << "Testing passed." << endl;
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/benchmark/swc/breadth_first.swc");
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  tree.labelTrunkLevel(&trunkAnalyzer);
#endif

#if 0
  ZCuboid cuboid1;
  ZCuboid cuboid2;

  /*
  cuboid1.set(1, 3, 1, 6, 8, 2);
  cuboid2.set(3, 1, 1, 7, 5, 2);

  ZPoint movingVec(0.5, 0.5, 0);

  cout << cuboid2.estimateSeparateScale(cuboid1, movingVec) << endl;
  */

  cuboid1.set(365.136, 205.338, -4.12311, 804.793, 739.338, 1462.38);
  cuboid2.set(373.934, 208.455, -7, 857.17, 740.455, 1433.12);
  //cout << cuboid2.estimateSeparateScale(cuboid1, ZPoint(1, 1, 1)) << endl;

  double scale = cuboid2.estimateSeparateScale(cuboid1, ZPoint(1, 1, 1));

  cuboid1.scale(scale);
  cuboid2.scale(scale);

  cuboid1.print();
  cuboid2.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/test/flyem/adjusted/test_65535.swc");
  ZPoint pt;
  Swc_Tree_Centroid(tree.data(), pt.xRef(), pt.yRef(), pt.zRef());

  cout << pt.x() << " " << pt.y() << " " << pt.z() << endl;

  double corner[6];
  tree.boundBox(corner);

#endif

#if 0
  tr1::shared_ptr<int> a(new int[2]);
  cout << a.use_count() << endl;

  tr1::shared_ptr<int> b(a);
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;

  tr1::shared_ptr<int> c;
  c = a;
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;
  cout << c.use_count() << endl;

  a.reset();
  cout << a.use_count() << endl;
  cout << b.use_count() << endl;
  cout << c.use_count() << endl;

#endif

#if 0
  ZString str("/home/zhaot/test/test.tif");
  cout << str.toDirPath() << endl;

  ZString str2("Test2.tif");
  cout << str2.toAbsolutePath(str.toDirPath()) << endl;

  cout << str2.isAbsolutePath() << endl;
  cout << str2.toAbsolutePath(str.toDirPath()).isAbsolutePath() << endl;

  str2 = "Test3.tif";
  cout << str2 << endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa1;
  FlyEm::ZSynapseAnnotationArray sa2;

  sa1.loadJson(dataPath + "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
               "psd-530-exports/annotations-synapse_part.json");
  sa2.loadJson(dataPath + "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
               "psd-529-exports/annotations-synapse_part.json");
  /*
  sa1.loadJson(dataPath + "/benchmark/flyem/psd/synapse_annotation1.json");
  sa2.loadJson(dataPath + "/benchmark/flyem/psd/synapse_annotation2.json");
*/
  sa1.printSummary();
  sa2.printSummary();

  FlyEm::ZSynapseLocationMatcher matcherForEvaluate;
  /*
  matcherForEvaluate.load(sa1, sa2,
                          dataPath + "/benchmark/flyem/psd/golden_match.txt");
                          */
  matcherForEvaluate.load(sa1, sa2,
                          dataPath +
                          "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/"
                          "changl_sigmundc_matches.csv");
  FlyEm::ZSynapseLocationMatcher overallPsdMatcher;

  FlyEm::ZSynapseLocationAngleMetric metric;
  //FlyEm::ZSynapseLocationEuclideanMetric metric;
  overallPsdMatcher.setMetric(&metric);


  overallPsdMatcher.matchPsd(sa1, sa2, 0.4);

  cout << "Matching result: " << endl;
  for (size_t i = 0; i < overallPsdMatcher.size(); i++) {
    cout << overallPsdMatcher.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE) << " "
         << overallPsdMatcher.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE) << endl;
  }

  cout << "Unmatched: "
       << overallPsdMatcher.unmatchedSize(
            FlyEm::ZSynapseLocationMatcher::LEFT_SIDE)
       << " "
       << overallPsdMatcher.unmatchedSize(
            FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE)
       << endl;

  cout << "Golden result: " << endl;
  for (size_t i = 0; i < matcherForEvaluate.size(); i++) {
    cout << matcherForEvaluate.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE) << " "
         << matcherForEvaluate.getIndex(
              i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE) << endl;
  }

  overallPsdMatcher.evaluate(matcherForEvaluate);
  vector<pair<int, int> > wrongMatch = overallPsdMatcher.falseMatch();
  vector<pair<int, int> > missingMatch = overallPsdMatcher.missingMatch();

  vector<FlyEm::SynapseLocation*> locArray1 = sa1.toSynapseLocationArray();
  vector<FlyEm::SynapseLocation*> locArray2 = sa2.toSynapseLocationArray();

  cout << "Wrong matches:" << endl;
  for (size_t i = 0; i < wrongMatch.size(); i++) {
    FlyEm::SynapseLocation *loc1 = locArray1[wrongMatch[i].first];
    FlyEm::SynapseLocation *loc2 = locArray2[wrongMatch[i].second];
    cout << wrongMatch[i].first << " " << wrongMatch[i].second << " "
         << loc1->pos().toString() << " " << loc2->pos().toString() << endl;
  }

  cout << "Missing matches:" << endl;
  for (size_t i = 0; i < missingMatch.size(); i++) {
    FlyEm::SynapseLocation *loc1 = locArray1[missingMatch[i].first];
    FlyEm::SynapseLocation *loc2 = locArray2[missingMatch[i].second];
    cout << missingMatch[i].first << " " << missingMatch[i].second << " "
         << loc1->pos().toString() << " " << loc2->pos().toString() << endl;
  }

  overallPsdMatcher.exportPerformance(dataPath + "/test.html");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson(dataPath +
              "/benchmark/flyem/psd/synapse_annotation_multiple.json");
  ZIntTree mtbar = sa.buildTbarSequence(10.0);
  mtbar.print();
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson(dataPath +
              "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-528-exports/annotations-synapse.json");
  vector<vector<FlyEm::SynapseLocation*> > mtbar = sa.buildTbarSequence(50.0);

  for (size_t i = 0; i < mtbar.size(); i++) {
    cout << mtbar[i][0]->pos().toString() ;
    for (size_t j = 1; j < mtbar[i].size(); j++) {
      cout << "-->" << mtbar[i][j]->pos().toString() << " " << endl;
    }
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa1;
  FlyEm::ZSynapseAnnotationArray sa2;

  sa1.loadJson(dataPath +
               "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-529-exports/annotations-synapse.json");
  sa2.loadJson(dataPath +
               "/flyem/psd/00044_3008-3507_2259-2758_1500-1999/psd-531-exports/annotations-synapse.json");

  //For the first and second file
  vector<FlyEm::SynapseLocation*> tbarSet1 = sa1.toTBarRefArray();
  cout << "Tbar number 1: " << tbarSet1.size() << endl;

  vector<FlyEm::SynapseLocation*> tbarSet2 = sa2.toTBarRefArray();
  cout << "Tbar number 2: " << tbarSet2.size() << endl;

  //Match the tbars
  FlyEm::ZSynapseLocationMatcher matcher;
  const double tbarDistThre = 100.0 / 10.0;

  cout << "Matching TBars ..." << endl;
  matcher.match(tbarSet1, tbarSet2, tbarDistThre);
  cout << "Done." << endl;

  for (size_t i = 0; i < matcher.size(); i++) {
    int index1 = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE);
    int index2 = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE);

    cout << i << ": "
         << index1
         << " "
         << index2 << " "
         << tbarSet1[index1]->pos().distanceTo(tbarSet2[index2]->pos())
         << " | " << tbarSet1[index1]->pos().toString() << " "
         << tbarSet2[index2]->pos().toString()
         << endl;
  }

  cout << "Unmatched in 1: " << endl;
  for (size_t i = 0;
       i < matcher.unmatchedSize(FlyEm::ZSynapseLocationMatcher::LEFT_SIDE);
       i++) {
    int index = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::LEFT_SIDE,
                                 FlyEm::ZSynapseLocationMatcher::UNMATCHED);
    cout << index << " " << tbarSet1[index]->pos().toString() << endl;

  }

  cout << "Unmatched in 2: " << endl;
  for (size_t i = 0;
       i < matcher.unmatchedSize(FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE);
       i++) {
    int index = matcher.getIndex(i, FlyEm::ZSynapseLocationMatcher::RIGHT_SIDE,
                                 FlyEm::ZSynapseLocationMatcher::UNMATCHED);
    cout << index << " " << tbarSet2[index]->pos().toString() << endl;

  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson("/Users/zhaot/Work/neutube/neurolabi/cpp/psd/test/"
              "synapse_annotation1.json");
  sa.exportJsonFile(dataPath + "/test.json");

  vector<vector<int> > selected(2);
  selected[0].resize(2);
  selected[0][0] = 0;
  selected[0][1] = 1;

  selected[1].resize(4);
  selected[1][0] = 2;
  selected[1][1] = 0;
  selected[1][2] = 2;
  selected[1][3] = 3;

  sa.exportJsonFile(dataPath + "/test2.json", &selected);
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson("/Users/zhaot/Work/neutube/neurolabi/cpp/psd/test/"
              "synapse_annotation1.json");
  sa.printSummary();

  int count = 0;
  size_t n = sa.size() + sa.getPsdNumber();

  for (size_t index = 0; index <= n; index++) {
    pair<int, int> sub = sa.relativePsdIndex(index);
    if (sub.first >= 0 && sub.second >= 0) {
      if (sa.getPsdIndex(sub.first, sub.second) != (int) index) {
        cout << "Wrong index: " << index << " " << sub.first << " "
             << sub.second << " " << sa.getPsdIndex(sub.first, sub.second)
             << endl;
      }
    } else {
      cout << "Invalid index: " << index << " " << sub.first << " "
           << sub.second << endl;
      count++;
    }
  }
  cout << count << endl;

#endif

#if 0
  ZStackFile sf;
  sf.import(dataPath + "/*.tif");
  sf.print();
  /*
  int kind, width, height, depth;
  sf.retrieveAttribute(&kind, &width, &height, &depth);
  cout << width << " " << height << " " << depth << " " << kind << endl;
  ZStack *stack = sf.readStack();
  stack->save((dataPath + "/test.tif").c_str());
  */
#endif

#if 0
  ZSuperpixelMapArray mapArray;
  mapArray.load(dataPath +
                "/flyem/segmentation/ground_truth/superpixel_to_segment_map.txt",
                0);
  mapArray.print();

  ZIntMap segMapArray;
  segMapArray.load(dataPath +
                   "/flyem/segmentation/ground_truth/segment_to_body_map.txt");
  //segMapArray.print();

  mapArray.setBodyId(segMapArray);
  mapArray.print();
#endif

#if 0
  //string segPath = dataPath + "/flyem/segmentation/assignments/assignment_2";
  string segPath = dataPath + "/flyem/segmentation/ground_truth";
  ZSuperpixelMapArray mapArray;
  /*
  mapArray.load(dataPath +
                "/benchmark/flyem/superpixel_to_segment_map.txt");
                */
  mapArray.load(segPath + "/superpixel_to_segment_map.txt");
  //mapArray.print();

  ZIntMap segMapArray;
  //segMapArray.load(dataPath + "/benchmark/flyem/segment_to_body_map.txt");
  segMapArray.load(segPath +"/segment_to_body_map.txt");
  mapArray.setBodyId(segMapArray);
  //mapArray.print();


  ZStackFile file;

  /*
  file.importImageSeries(dataPath +
                         "/benchmark/flyem/superpixel_maps/sp_map.00030.png");
                         */
  file.importImageSeries(segPath + "/superpixel_maps/*.png");

  ZStack *stack = file.readStack();

  ZStack *newStack = mapArray.mapStack(*stack);
  newStack->save((segPath + "/body.tif").c_str());

  delete stack;
  delete newStack;
#endif

#if 0
  FlyEm::ZSegmentationAnalyzer analyzer;
  analyzer.compare(trueSeg, testSeg);

  ZStackFile file;
  file.importSeries(superpixelPath);
  ZStack *superpixel = file.readStack();

  ZStack *stack = analyzer.createErrorStack(superpixel);
  stack->save("../data/test.tif");
#endif

#if 0
  ZStackFile file;
  string groundTruthPath = dataPath + "/flyem/segmentation/ground_truth";
  file.import(groundTruthPath + "/superpixel_maps/*.png");
  ZStack *stack = file.readStack();

  ZStackProcessor processor;
  processor.mexihatFilter(stack, 3.0);
  stack->save(dataPath + "/test.tif");

#endif

#if 0
  string groundTruthPath = dataPath + "/flyem/segmentation/ground_truth";
  string testPath = dataPath + "/flyem/segmentation/assignments/assignment_2";

  ZStackFile greyFile;
  greyFile.import(groundTruthPath + "/superpixel_maps/*.png");

  ZStackFile superpixelFile;
  superpixelFile.import(groundTruthPath + "/superpixel_maps/*.png");

  string superpixelMapFile = groundTruthPath + "/superpixel_to_segment_map.txt";
  string segmentationMapFile = groundTruthPath + "/segment_to_body_map.txt";

  FlyEm::ZSegmentationBundle bundle;

  bundle.setGreyScaleSource(greyFile);
  bundle.setSuperpixelSource(superpixelFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);
  bundle.update();

  ZStack *stack = bundle.createBodyStack();
  stack->save(dataPath + "/test.tif");

  /*
  delete stack;

  bundle.setGreyScaleSource(greyFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);

  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL_MAP);

  stack = bundle.createBodyStack();
  stack->save(dataPath + "/test2.tif");
  */

  superpixelFile.import(testPath + "/superpixel_maps/*.png");
  superpixelMapFile = testPath + "/superpixel_to_segment_map.txt";
  segmentationMapFile = testPath + "/segment_to_body_map.txt";
  bundle.setSuperpixelSource(superpixelFile);
  bundle.setSuperpixelMapSource(superpixelMapFile);
  bundle.setSegmentMapSource(segmentationMapFile);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL);
  bundle.update(FlyEm::ZSegmentationBundle::SUPERPIXEL_MAP);

  ZStack *testStack = bundle.createBodyBoundaryStack();
  testStack->save(dataPath + "/test2.tif");

  ZStackBlender blender;
  ZStack *blending = blender.blend(*stack, *testStack, 0.9);
  blending->save(dataPath + "/test3.tif");

  delete stack; stack = NULL;
  delete testStack; testStack = NULL;
  delete blending; blending = NULL;
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/benchmark/flyem/segmentation.json");
  bundle.print();

  bundle.compressBodyId();

  bundle.getSuperpixelMap()->print();

  ZStack *stack = bundle.getBodyStack();

  uint8_t color_map[] = {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF};
  int ncolor = sizeof(color_map) / 3;

  Stack *out = Stack_Blend_Label_Field(bundle.getGreyScaleStack()->c_stack(),
                                       stack->c_stack(), 0.5, color_map, ncolor, NULL);


  C_Stack::write(dataPath + "/test.tif", out);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/TEM/slice_figure/segmentation/segmentation.json");
  bundle.trimSuperpixelMap();
  bundle.compressBodyId(0);
  //bundle.getSuperpixelMap()->print();

  ZStack *stack = bundle.getBodyStack();

  stack->save(dataPath + "/flyem/TEM/slice_figure/segmentation/blend_color.tif");
  Stack *out = Stack_Blend_Label_Field(
        bundle.getGreyScaleStack()->c_stack(),
        stack->c_stack(), 0.5, Discrete_Colormap, Discrete_Color_Number, NULL);

  C_Stack::write(dataPath + "/flyem/TEM/slice_figure/segmentation/blend_body.tif", out);
#endif

#if 0
  ZGraph *graph  = new ZGraph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);
  graph->addEdge(0, 2);
  graph->addEdge(1, 2);
  graph->addEdge(3, 2);
  graph->addEdge(3, 4);
  graph->addEdge(4, 5);
  graph->addEdge(2, 4);
  graph->addEdge(0, 4);
  graph->addEdge(0, 6);
  graph->addEdge(1, 7);
  graph->addEdge(0, 8);
  graph->addEdge(4, 9);
  graph->print();

  graph->exportDotFile(dataPath + "/test.dot");

  set<int> neighborSet = graph->getNeighborSet(2);

  cout << "Neighbors of " << 2 << ":" << endl;
  for (set<int>::const_iterator iter = neighborSet.begin();
       iter != neighborSet.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  vector<int> vertexArray(3);
  vertexArray[0] = 0;
  vertexArray[1] = 1;
  vertexArray[2] = 2;

  neighborSet = graph->getNeighborSet(vertexArray);

  cout << "Neighbors" << ":" << endl;
  for (set<int>::const_iterator iter = neighborSet.begin();
       iter != neighborSet.end(); ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  delete graph; graph = NULL;
#endif

#if 0
  ZArray::Dimn_Type dims[2] = { 2, 3 };
  ZArray array(mylib::UINT8_TYPE, 2, dims);
  array.printInfo();

  ZArray array2 = array;
  array2.printInfo();

#endif

#if 0
  ZStackFile file;
  file.import(dataPath + "/flyem/segmentation/assignments/assignment_2/mask.tif");

  tic();
  ZStack *stack = file.readStack();
  ptoc();

  vector<vector<double> > borderColor;

  int m, n;
  int *array = iarray_load_matrix(
        (dataPath + "/flyem/segmentation/assignments/assignment_2/seed.txt").c_str(),
        NULL, &m, &n);

  borderColor.resize(n);
  for (int i = 0; i < n; ++i) {
    borderColor[i].resize(m);
    for (int j = 0; j < m; ++j) {
      borderColor[i][j] = array[m * i + j];
    }
  }

  for (size_t i = 0; i < borderColor.size(); ++i) {
    for (size_t j = 0; j < borderColor[i].size(); ++j) {
      cout << borderColor[i][j] << " ";
    }
    cout<< endl;
  }

  free(array);

/*
  c1[0] = 151;
  c1[1] = 67;
  c1[2] = 0;

  c2[0] = 244;
  c2[1] = 64;
  c2[2] = 0;

  ZStack *stack = new ZStack(GREY, 3, 3, 1, 3);
  stack->setValue(0, 0, c1[0]);
  stack->setValue(0, 1, c1[1]);
  stack->setValue(0, 2, c1[2]);

  stack->setValue(1, 0, c2[0]);
  stack->setValue(1, 1, c2[1]);
  stack->setValue(1, 2, c2[2]);
*/

  for (size_t i = 0; i < borderColor.size() / 2; ++i){
    vector<double> feature = FlyEm::ZSegmentationAnalyzer::touchFeature(
          *stack, borderColor[i * 2], borderColor[i * 2 + 1]);
    cout << feature[0] << " " << feature[1] << endl;
  }

  delete stack;
#endif

#if 0
  ZIntPairMap pairMap;
  pairMap.incPairCount(1, 2);
  pairMap.incPairCount(2, 3);
  pairMap.incPairCount(1, 2);
  pairMap.incPairCount(3, 4);
  pairMap.printSummary();
  pairMap.print();

  ZIntMap intMap;
  intMap.incValue(1);
  intMap.incValue(2);
  intMap.incValue(2);
  intMap.incValue(2);
  intMap.incValue(3);
  intMap.incValue(4);
  intMap.print();

  ZIntMap bodyCorrespondence =
      FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
        pairMap, intMap);
  bodyCorrespondence.print();
  bodyCorrespondence.print(ZIntMap::KEY_GROUP);
#endif

#if 0
  string testSegPath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  string trueSegPath = dataPath + "/flyem/segmentation/ground_truth/segmentation.json";

  //string testSegPath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  //string trueSegPath = dataPath + "/benchmark/flyem2/truth/segmentation.json";

  FlyEm::ZSegmentationBundle testSegBundle;
  FlyEm::ZSegmentationBundle trueSegBundle;

  cout << "Load " << dataPath << endl;
  testSegBundle.importJsonFile(testSegPath);
  //testSegBundle.update();

  cout << "Load " << trueSegPath << endl;
  trueSegBundle.importJsonFile(trueSegPath);
  //trueSegBundle.update();

  ZStack *testBodyStack = testSegBundle.getBodyStack();
  ZStack *trueBodyStack = trueSegBundle.getBodyStack();

  ZStack *b1 = testSegBundle.getBodyBoundaryStack();
  ZStack *b2 = trueSegBundle.getBodyBoundaryStack();

  uint8_t *array1 = b1->array8(0);
  uint8_t *array2 = b2->array8(0);

  size_t volume = b1->getVoxelNumber();

  u8array_max2(array1, array2, volume);

#ifdef _DEBUG_2
  printf("%p\n", testBodyStack->singleChannelStack()->array8());
  cout << *(testBodyStack->singleChannelStack()->array8() + 100) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  printf("%p\n", testBodyStack->singleChannelStack()->array8());
  cout << (int) *(testBodyStack->singleChannelStack(0)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(1)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(2)->array8() + 100) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  cout << testBodyStack->singleChannelStack(0)->value(0) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(0)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(1)->array8() + 100) << endl;
  cout << (int) *(testBodyStack->singleChannelStack(2)->array8() + 100) << endl;
  cout << FlyEm::ZSegmentationAnalyzer::channelCodeToId(testBodyStack->color(100)) << endl;
  ZDoubleVector::print(testBodyStack->color(100));
  return;
#endif

  ZIntPairMap overlap = FlyEm::ZSegmentationAnalyzer::computeOverlap(
        testBodyStack, trueBodyStack, b1);

  cout << "Overlap: " << endl;
  overlap.print();

  ZIntMap testBodySize =
      FlyEm::ZSegmentationAnalyzer::computeBodySize(testBodyStack,
                                                    testSegBundle.getBodyBoundaryStack());

  cout << "Body size: " << endl;
  ofstream testBodyStream((dataPath + "/test_body_size.txt").c_str());

  testBodySize.print(testBodyStream);

#ifdef _DEBUG_2
  return;
#endif

  ZIntMap bodyCorrespondence =
      FlyEm::ZSegmentationAnalyzer::inferBodyCorrespondence(
        overlap, testBodySize);

  cout << "Body correspondence: " << endl;
  bodyCorrespondence.print();

  bodyCorrespondence.print(cout, ZIntMap::KEY_GROUP);

  ZGraph *bodyGraph = testSegBundle.getBodyGraph();
  cout << "Body graph: " << endl;
  bodyGraph->exportDotFile(dataPath + "/test.dot");

#ifdef _DEBUG_2
  bodyGraph->clean();
  //bodyGraph->addEdge(68206, 61888);
  bodyGraph->addEdge(17303, 9128);
  bodyGraph->print();
#endif
/*
  vector<vector<double> > feature =
      FlyEm::ZSegmentationAnalyzer::touchFeature(*testSegBundle.getBodyStack(),
                                                 *bodyGraph,
                                                 *testSegBundle.getBodyBoundaryStack());
*/
  vector<vector<double> > feature =
      FlyEm::ZSegmentationAnalyzer::touchFeature(testSegBundle);
  //ZGraph oversplit(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  ofstream stream((dataPath + "/feature.txt").c_str());

  for (size_t i = 0; i < feature.size(); i++) {
    int id1 = bodyGraph->edgeStart(i);
    int id2 = bodyGraph->edgeEnd(i);

    stream << id1 << " " << id2 << " ";
    for (vector<double>::const_iterator featureIter = feature[i].begin();
         featureIter != feature[i].end(); ++featureIter) {
      stream << *featureIter << " ";
    }

    if (bodyCorrespondence[id1] == bodyCorrespondence[id2]) {
      stream << 1 << endl;
    } else {
      stream << 0 << endl;
    }
  }

  stream.close();

/*
  cout << "oversplit: " << endl;
  oversplit.print();
  oversplit.exportDotFile(dataPath + "/test.dot");

  ZDoubleVector::exportTxtFile(feature, dataPath + "/feature.txt");
  */
#endif

#if 0
  vector<double> vec(10);
  for (size_t i = 0; i < vec.size(); ++i) {
    vec[i] = i;
  }

  vector<vector<double> > array2d = ZDoubleVector::reshape(vec, 4);

  ZDoubleVector::print(vec);
  ZDoubleVector::print(array2d);

#endif

#if 0
  Stack *stack = Read_Stack_U(
        (dataPath + "/benchmark/binary/2d/disk_n1.tif").c_str());
  int seedArray[] = { 2935, 1111, 2927 };

  vector<double> direction =
      FlyEm::ZSegmentationAnalyzer::computeRayburstDirection(
        stack, seedArray, sizeof(seedArray) / sizeof(int));

  ZDoubleVector::print(direction);

  double x, y, z;
  Geo3d_Orientation_Normal(direction[0], direction[1], &x, &y, &z);
  cout << x << " " << y << " " << z << endl;
#endif

#if 0
  ZJsonObject jsonObject;
  jsonObject.load(dataPath + "/test.json");
  map<const char*, json_t *> entryMap = jsonObject.toEntryMap();
  for (map<const char*, json_t *>::const_iterator iter = entryMap.begin();
       iter != entryMap.end(); ++iter) {
    cout << "\"" << iter->first << "\": " << ZJsonParser::type(iter->second)
         << endl;
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationAnalyzer analyzer;
  analyzer.loadConfig(dataPath + "/config.txt");
  analyzer.print();

  cout << endl;
  analyzer.loadConfig(dataPath + "/config.json");
  analyzer.print();
#endif

#if 0
  //Test if a body is mitochondria
  FlyEm::ZSegmentationBundle bundle;

  string testPath = dataPath + "/flyem/segmentation/assignments/assignment_2";
  string segFile = testPath + "/segmentation.json";

  cout << "Load segmetnation ..." << endl;
  bundle.importJsonFile(segFile);
  //bundle.update();

  cout << "Retrieve body stack ..." << endl;
  ZStack *bodyStack = bundle.getBodyStack();

  vector<double> bodyCode(3);
  bodyCode[0] = 147;
  bodyCode[1] = 231;
  bodyCode[2] = 0;

  cout << "Create body mask ..." << endl;
  vector<vector<double> > selected;
  selected.push_back(bodyCode);
  ZStack *objStack = bodyStack->createSubstack(selected);

  //objStack->save(dataPath + "/test.tif");

  string mitoConfidenceFile = testPath + "/mito_pred.tif";

  cout << "Load prediction ..." << endl;
  ZStackFile stackFile;
  stackFile.import(mitoConfidenceFile);
  ZStack *mito = stackFile.readStack();

  cout << "Average intensity:" << endl;
  double conf = mito->averageIntensity(objStack);

  cout << conf << endl;

  map<int, double> allConf =
      FlyEm::ZSegmentationAnalyzer::computeAverageIntensity(bodyStack, mito);

  ofstream stream((testPath + "/mito_conf.txt").c_str());

  vector<int> mitoId;

  for (map<int, double>::const_iterator iter = allConf.begin();
       iter != allConf.end(); ++iter) {
    stream << iter->first << " " << iter->second << endl;
    if (iter->second > 80.0) {
      mitoId.push_back(iter->first);
    }
  }

  cout << "Extract orphan bodies ..." << endl;
  set<int> orphanSet = bundle.getAllOrphanBody();
  cout << "Retrieve body graph ..." << endl;
  ZGraph* bodyGraph = bundle.getBodyGraph();

  bodyGraph->exportTxtFile(dataPath + "/graph.txt");

  //Output ophan bodies connected to mitochondria
  //For each mito id
  for (vector<int>::const_iterator iter = mitoId.begin(); iter != mitoId.end();
       ++iter) {
    cout << "Mito " << *iter << ": ";
    //Find the neighbors
    set<int> neighbors = bodyGraph->getNeighborSet(*iter);
    //For each neighbor
    for (set<int>::const_iterator neighborIter = neighbors.begin();
         neighborIter != neighbors.end(); ++neighborIter) {
      //If it is an ophan
      if (orphanSet.count(*neighborIter) > 0) {
        //Output the id
        cout << *neighborIter << " ";
      } else {
        cout << "[" << *neighborIter << "]" << " ";
      }
    }
    cout << endl;
  }
#endif

#if 0
  ZString str("test.tif");
  cout << str.endsWith("tif") << endl;
  cout << str.endsWith("TIF") << endl;
  cout << str.endsWith(".tif") << endl;
  cout << str.endsWith(".TIF") << endl;
  cout << str.endsWith(".tiff") << endl;
  cout << endl;

  cout << str.endsWith("tif", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith("TIF", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".tif", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".TIF", ZString::CASE_INSENSITIVE) << endl;
  cout << str.endsWith(".tiff", ZString::CASE_INSENSITIVE) << endl;
  cout << endl;

  cout << str.startsWith("test") << endl;
  cout << str.startsWith("Test") << endl;
  cout << str.startsWith("test", ZString::CASE_INSENSITIVE) << endl;
  cout << str.startsWith("Test", ZString::CASE_INSENSITIVE) << endl;

  cout << ZFileType::typeName(ZFileType::fileType(str.c_str())) << endl;

  cout << ZFileType::typeName(ZFileType::fileType("threata.LSM")) << endl;
#endif

#if 0
  string testSegPath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  //string trueSegPath = dataPath + "/benchmark/flyem2/truth/segmentation.json";

  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(testSegPath);
  bundle.update();

  vector<int> *bodyList = bundle.getBodyList();
  iarray_print(&(*bodyList)[0], bodyList->size());

  map<int, size_t> *bodyIndexMap = bundle.getBodyIndexMap();
  for (map<int, size_t>::const_iterator iter = bodyIndexMap->begin();
       iter != bodyIndexMap->end(); ++iter) {
    cout << iter->first << " : " << iter->second << endl;
  }

#endif

#if 0
  string testSegPath = dataPath + "/benchmark/flyem3/test/segmentation.json";
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(testSegPath);
  //bundle.update();

  bundle.getSuperpixelMap()->print();

  /*
  set<int> orphans = bundle.getAllOrphanBody();
  for (set<int>::const_iterator iter = orphans.begin(); iter != orphans.end();
       ++iter) {
    cout << *iter << " ";
  }
  cout << endl;

  bundle.getBodyStack()->save(dataPath + "/test.tif");
  */
  ZDoubleVector::print(bundle.getBodyStack()->color(0));
#endif

#if 0
  ZStack stack;
  ZStackFile stackFile;
  stackFile.import(dataPath + "/benchmark/series/image*.tif");
  stackFile.readStack(&stack);

  stack.save(dataPath + "/test.tif");
#endif

#if 0
  ZStackFile stackFile;
  stackFile.import(dataPath + "/benchmark/fork_2d.tif");
  ZStack *stack = stackFile.readStack();

  //stack->binarize(0);

  //stack->clone()->save(dataPath + "/test.tif");

  ZDoubleVector::print(stack->color(10));
  ZDoubleVector::print(stack->color(10));
#endif

#if 0
  ZStack *stack = new ZStack(COLOR, 10, 10, 5, 1);
  stack->setValue(0, 0, 255);
  //cout << stack->hasSameValue(0, 1) << endl;
  //cout << stack->hasSameValue(1, 2) << endl;

  stack->save(dataPath + "/test.tif");
#endif

#if 0
  //Calculate boundary features
  string boundaryFilePath =
      dataPath + "/flyem/segmentation/assignments/assignment_2/boundary_pred.tif";
  string segFilePath =
      dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";

/*
  string boundaryFilePath =
      dataPath + "/benchmark/flyem2/test/boundary_pred.tif";
  string segFilePath =
      dataPath + "/benchmark/flyem2/test/segmentation.json";
*/
  ZStackFile stackFile;
  stackFile.import(boundaryFilePath);

  ZStack *boundaryProb = stackFile.readStack();

  FlyEm::ZSegmentationBundle bundle;

  bundle.importJsonFile(segFilePath);

  ZGraph *bodyGraph = bundle.getBodyGraph();

  ZGraph newBodyGraph(ZGraph::UNDIRECTED_WITH_WEIGHT);

  //For each edge in the body graph, calculate the weight
  for (size_t edgeIndex = 0; edgeIndex < bodyGraph->size(); ++edgeIndex) {
    cout << edgeIndex + 1 << "/" << bodyGraph->size() << endl;

    int id1 = bodyGraph->edgeStart(edgeIndex);
    int id2 = bodyGraph->edgeEnd(edgeIndex);

    ZObject3d *border = bundle.getBodyBoundaryObject(id1, id2);

    //border->print();

    if (border->size() == 0) {
      border = bundle.getBodyBoundaryObject(id1, id2);
      cout << id1 << " " << id2 << " " <<bodyGraph->getEdgeWeight(id1, id2) << endl;
      cout << "debug here" << endl;
    }

    double v = border->averageIntensity(boundaryProb->c_stack());
    newBodyGraph.addEdge(id1, id2, v);

    delete border;
  }

  string graphFilePath = dataPath + "/graph.txt";

  newBodyGraph.exportTxtFile(graphFilePath);
#endif

#if 0
  ofstream script((dataPath + "/test.sh").c_str());

  ZFileList fileList;
  fileList.load("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/T4viz",
                "swc");
  for (int i = 0; i < fileList.size(); ++i) {
    ZString str = fileList.getFilePath(i);
    ZString output = str;
    output.replace(".swc", ".marker");
    int id = str.lastInteger();
    script << "/Users/zhaot/Work/neutube/neurolabi/cpp/debug/bin/project_synapse --json /Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/annotations-synapse.json --swc "
         << str.c_str() << " --body_id " << id << " -o " << output.c_str()
         << endl;
  }

  script.close();
#endif

#if 0
  ZJsonValue jValue;
  jValue.decodeString("{ \"test\": 1, \"value\": [1, 2, 3, 4] }");
  jValue.print();
#endif

#if 0
  FlyEm::ZFileParser::readVaa3dMarkerFile(dataPath + "/flyem/TEM/T4viz/T4_1_277709_2.marker");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/benchmark/flyem/annotations-synapse.json");
  synapseArray.print();

  FlyEm::SynapseDisplayConfig displayConfig;
  displayConfig.mode = FlyEm::SynapseDisplayConfig::PSD_ONLY;
  displayConfig.bodyId = 16711935;

  vector<ZVaa3dMarker> markerArray =
      synapseArray.toMarkerArray(FlyEm::SynapseAnnotationConfig(),
                                 FlyEm::SynapseLocation::ORIGINAL_SWC_SPACE,
                                 displayConfig);

  for (size_t i = 0; i < markerArray.size(); ++i) {
    cout << markerArray[i].toString() << endl;
  }
#endif

#if 0
  ZPoint pt1(10, 0, 0);
  ZPoint pt2(0, 0, 0);
  ZPoint pt = pt2 - pt1;

  cout << pt.toString() << endl;
#endif

#if 0
  //string neuron = "T4_1_277709";
  //string neuron = "T4_2_386464";
  //string neuron = "T4_10_476680";
  //string neuron = "T4_16_475117";
  string neuronArray[4] = { "T4_2_386464", "T4_17_547009", "T4_10_476680",
                            "T4_16_475117" };

  for (size_t neuronIndex = 0; neuronIndex < 4; ++neuronIndex) {
    string neuron = neuronArray[neuronIndex];

    string markerFile = dataPath +
        "/skeletonization/session2/len15/adjusted/" + neuron + ".marker";
    vector<ZVaa3dMarker> markerArray =
        FlyEm::ZFileParser::readVaa3dMarkerFile(markerFile);

    string mi1ListFile = dataPath + "/flyem/TEM/mi1_list.txt";
    string tm3ListFile = dataPath + "/flyem/TEM/tm3_list.txt";

    set<int> mi1IdArray;
    set<int> tm3IdArray;

    ZString str;
    FILE *fp = fopen(mi1ListFile.c_str(), "r");
    while (str.readLine(fp)) {
      int bodyId = str.firstInteger();
      if (bodyId > 0) {
        mi1IdArray.insert(bodyId);
      }
    }
    fclose(fp);

    fp = fopen(tm3ListFile.c_str(), "r");
    while (str.readLine(fp)) {
      int bodyId = str.firstInteger();
      if (bodyId > 0) {
        tm3IdArray.insert(bodyId);
      }
    }
    fclose(fp);

    vector<ZVaa3dMarker> newMarkerArray;

    for (size_t i = 0; i < markerArray.size(); ++i) {
      int id = String_Last_Integer(markerArray[i].name().c_str());
      markerArray[i].setRadius(markerArray[i].radius() * 2.5);
      if (mi1IdArray.count(id) > 0) {
        markerArray[i].setName("Mi1");
        markerArray[i].setColor(0, 0, 255);
        newMarkerArray.push_back(markerArray[i]);
      }
      if (tm3IdArray.count(id) > 0) {
        markerArray[i].setName("Tm3");
        markerArray[i].setColor(255, 0, 0);
        newMarkerArray.push_back(markerArray[i]);
      }
    }

    string outFile = dataPath +
        "/skeletonization/session2/len15/adjusted/" + neuron + "_sorted.marker";
    FlyEm::ZFileParser::writeVaa3dMakerFile(outFile, newMarkerArray);
  }
#endif

#if 0
  ZStack stack;
  stack.load("/Users/feng/Downloads/For_Programming.lsm");
  stack.logLSMInfo();
#endif

#if 0
  string mi1ListFile = dataPath + "/flyem/TEM/mi1_list.txt";
  ZString str;
  FILE *fp = fopen(mi1ListFile.c_str(), "r");

  while (str.readLine(fp)) {
    vector<string> wordArray = str.toWordArray();
    if (wordArray.size() == 2) {
      cout << "sh -x ./flyem_skeletonize " << wordArray[0]
           << " /groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/data/skeletonization/session2 "
           << wordArray[1] << endl << endl;
    } else if (wordArray.size() == 1 || wordArray.size() > 3) {
      cout << "bug?" << endl;
    }
  }
  fclose(fp);
#endif

#if 0
  string neuronArray[4] = { "T4_2_386464", "T4_17_547009", "T4_10_476680",
                            "T4_16_475117" };

  for (size_t neuronIndex = 0; neuronIndex < 4; ++neuronIndex) {
    ZSwcTree tree;
    string filePath = dataPath + "/flyem/skeletonization/session2/len15/adjusted/" +
        neuronArray[neuronIndex] + "_rooted.swc";
    tree.load(filePath.c_str());
    ofstream output((filePath + ".angle.txt").c_str());

    tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    //double vec[3];
    double angle;
    for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
      if (Swc_Tree_Node_Is_Root(tn) == _FALSE_) {
        ZPoint vec = SwcTreeNode::localDirection(tn, 5);
        //cout << vec[0] << " " << vec[1] << " " << vec[2] << endl;
        angle = Vector_Angle(vec.x(), vec.y());
        output << SwcTreeNode::localRadius(tn, 5) << " " << angle << endl;
      }
    }
    output.close();
  }
#endif

#if 0
  Z3DGraph graph;
  graph.importJsonFile(dataPath + "/test.json");
  graph.print();
#endif

#if 0
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_2_386464_trunk.swc";
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_10_476680_trunk.swc";
  //string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/T4_16_475117_trunk.swc";
  string neuronArray[] = {
    "T4_1_277709", "T4_2_386464", "T4_3_280303", "T4_4_2341",
    "T4_5_278848", "T4_6_545716", "T4_7_455131", "T4_8_545944",
    "T4_9_5189", "T4_10_476680", "T4_11_588435", "T4_12_460193",
    "T4_13_515936", "T4_14_586983", "T4_15_546035", "T4_16_475117",
    "T4_17_547009", "T4_18_547221", "T4_19_455135"
  };

  for (size_t neuronIndex = 0; neuronIndex < sizeof(neuronArray) / sizeof(string); ++neuronIndex) {
    string swcFile = dataPath + "/flyem/skeletonization/session2/len15/adjusted/" + neuronArray[neuronIndex] + "_labeled.swc";
    //string swcFile = dataPath + "/benchmark/swc/fork.swc";
    string outFile = swcFile + ".angle.txt";

    ZSwcTree tree;
    tree.load(swcFile.c_str());

    tree.labelBranchLevelFromLeaf();
/*
    ZSwcWeightTrunkAnalyzer trunkAnalyzer;
    trunkAnalyzer.labelTraffic(&tree);
    */
    //tree.labelBranchLevel();
    //ZSwcDistTrunkAnalyzer trunkAnalyzer;
    /*
    ZSwcWeightTrunkAnalyzer trunkAnalyzer;
    tree.setBranchSizeWeight();
    //trunkAnalyzer.labelTraffic(&tree);
    tree.labelTrunkLevel(&trunkAnalyzer);
    //tree.print();
*/
    ofstream out(outFile.c_str());

    tree.updateIterator(SWC_TREE_ITERATOR_DEPTH_FIRST);
    for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
      if (SwcTreeNode::type(tn) != 1) {
        ZWeightedPointArray segment = SwcTreeNode::localSegment(tn, 3);
        if (segment.size() == 7) {
          //cout << "Segment: " << segment[0].toString() << "-->"
          //     << segment.back().toString() << endl;
          //ZPoint vec = segment.principalDirection();
          ZPoint vec = segment.front() -segment.back();
          vec.setZ(0);
          vec.normalize();

          //cout << "Direction: ";

          out << SwcTreeNode::label(tn) << " " << Vector_Angle(vec.x(), vec.y())
              << endl;
          //out << vec.x() << " " << vec.y() << " "
          //    << SwcTreeNode::label(tn) << endl;

          /*
        if (neuronIndex == 3) {
          cout << outFile.c_str();
          cout << vec.x() << " " << vec.y() << " "
              << SwcTreeNode::label(tn) << endl;
        }
        */
        }
      }
    }

    out.close();
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/forest1.swc").c_str());
  tree.labelBranchLevel();

  tree.print();
#endif

#if 0
  vector<int> array(10);
  for (int i = 0; i < 10; i++) {
    array[i] = 10;
  }

  ZDebugPrintArray(array, 1, 5);

  array.resize(5);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/TEM/slice_figure/segmentation/segmentation.json");
  bundle.print();

//  ZStack *stack = bundle.getBodyStack();

//  stack->save(dataPath + "test.tif");

  ofstream stream((dataPath + "/flyem/TEM/slice_figure/segmentation/segment_to_body_map_partial.txt").c_str());

  ZSuperpixelMapArray *mapArray = bundle.getSuperpixelMap();
  //mapArray->print();
  int count = 0;
  for (ZSuperpixelMapArray::iterator iter = mapArray->begin();
       iter != mapArray->end(); ++iter) {

    stream << iter->segmentId() << " " << iter->bodyId() << endl;

    if (iter->planeId() >= 638 && iter->planeId() <= 640) {
      count++;
    }
  }

  //stream.close();
  cout << count << " superpixels" << endl;

#endif

#if 0
  vector<Swc_Tree_Node*> nodeArray(5);
  for (size_t i = 0; i < 5; ++i) {
    nodeArray[i] = SwcTreeNode::makeVirtualNode();
    SwcTreeNode::setId(nodeArray[i], i);
    SwcTreeNode::setPos(nodeArray[i], i * i, 0, 0);
  }

  SwcTreeNode::connect(nodeArray);

  ZSwcTree tree;
  tree.setDataFromNodeRoot(nodeArray[0]);

  tree.print();
#endif

#if 0
  ZSuperpixelMapArray mapArray;
  mapArray.append(0, 1, 2, 3);
  mapArray.append(0, 2, 3, 13);
  mapArray.append(1, 3, 5, 35);

  mapArray.print();

  mapArray.compressBodyId();
  mapArray.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree.labelBranchLevelFromLeaf();
  tree.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/flyem/TEM/T4_Axon/T4_2_w_axon_final.swc").c_str());

  ZSwcTree *boxTree = tree.createBoundBoxSwc();
  boxTree->save((dataPath + "/test.swc").c_str());

  delete boxTree;
#endif

#if 0
  ZCuboid box(200, -400, 1000, 800, 600, 3000);
  ZSwcTree *boxTree = ZSwcTree::createCuboidSwc(box);
  boxTree->save((dataPath + "/test.swc").c_str());

  delete boxTree;

  ZSwcTree tickSwc;
  tickSwc.forceVirtualRoot();


  for (int z = 1000; z <= 3000; z += 100) {
    double x = box.corner(0).x();
    double y = box.corner(0).y();
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(x, y, z), 10);
    SwcTreeNode::setParent(tn, tickSwc.root());
    Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(x + 100, y, z), 10);
    SwcTreeNode::setParent(tn2, tn);
  }

  for (int z = 1000; z <= 3000; z += 100) {
    double x = box.corner(3).x();
    double y = box.corner(3).y();
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(x, y, z), 10);
    SwcTreeNode::setParent(tn, tickSwc.root());
    Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(x - 100, y, z), 10);
    SwcTreeNode::setParent(tn2, tn);
  }

  tickSwc.resortId();

  tickSwc.save((dataPath + "/test2.swc").c_str());

#endif

#if 0
  hid_t       file_id, dataset_id, dataspace_id;  /* identifiers */
  hsize_t     dims[2];
  herr_t      status;

  /* Create a new file using default properties. */
  file_id = H5Fcreate((dataPath + "/test.h5").c_str(),
                      H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  /* Create the data space for the dataset. */
  dims[0] = 4;
  dims[1] = 6;
  dataspace_id = H5Screate_simple(2, dims, NULL);

  /* Create the dataset. */
  dataset_id = H5Dcreate(file_id, "/dset", H5T_STD_I32BE, dataspace_id,
                         H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* End access to the dataset and release resources used by it. */
  status = H5Dclose(dataset_id);

  /* Terminate access to the data space. */
  status = H5Sclose(dataspace_id);

  /* Close the file. */
  status = H5Fclose(file_id);

#endif

#if 0
  ZHdf5Reader reader;
  reader.open(dataPath + "/test.h5");

  mylib::Array *array = reader.readArray("/dset");
  mylib::printArrayInfo(array);
  mylib::Print_Array(array, stdout, 0, "%d");
#endif

#if 0
  ZHdf5Reader reader(dataPath + "/flyem/segmentation/assignments/assignment_2/stack.h5");

  reader.printInfo();

  mylib::Array *array = reader.readArray("/segment_superpixels");
  mylib::printArrayInfo(array);
#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1] - 1;
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  cv::Mat var_type = cv::Mat(columnNumber, 1, CV_8U);
  var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) );

  float priors[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  CvRTParams params = CvRTParams(5, // max depth
                                 2, // min sample count
                                 0, // regression accuracy: N/A here
                                 false, // compute surrogate split, no missing data
                                 2, // max number of categories (use sub-optimal algorithm for larger numbers)
                                 priors, // the array of priors
                                 true,  // calculate variable importance
                                 0,      // number of variables randomly selected at node and used to find the best split(s).
                                 5,	 // max number of trees in the forest
                                 0.01f,				// forrest accuracy
                                 CV_TERMCRIT_EPS // termination cirteria
                                 );
  CvRTrees* rtree = new CvRTrees;

  rtree->train(trainingData, CV_ROW_SAMPLE, trainingLabel,
               cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);
  rtree->save(dataPath + "/segmentation_classifier.cvt");


  float *testingArray = farray_load_matrix((dataPath + "/testing.txt").c_str(), NULL,
                                           &columnNumber, &rowNumber);
  float *testingLabelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    testingLabelArray[i] = testingArray[i * columnNumber + columnNumber - 1] - 1;
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(testingArray + i * (columnNumber - 1), testingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat testingData(rowNumber, columnNumber - 1, CV_32FC1, testingArray);

  for (int i = 0; i < testingData.rows; ++i) {
    float result = rtree->predict(testingData.row(i), cv::Mat());
    cout << " " << testingLabelArray[i] << " " << (result > 0.5) << endl;
  }
#endif

#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/flyem/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1];
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  cv::Mat var_type = cv::Mat(columnNumber, 1, CV_8U);
  var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) );

  float priors[] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1};
  CvRTParams params = CvRTParams(5, // max depth
                                 3, // min sample count
                                 0, // regression accuracy: N/A here
                                 false, // compute surrogate split, no missing data
                                 2, // max number of categories (use sub-optimal algorithm for larger numbers)
                                 priors, // the array of priors
                                 true,  // calculate variable importance
                                 3,      // number of variables randomly selected at node and used to find the best split(s).
                                 100,	 // max number of trees in the forest
                                 0.01f,				// forrest accuracy
                                 CV_TERMCRIT_EPS // termination cirteria
                                 );
  CvRTrees* rtree = new CvRTrees;

  rtree->train(trainingData, CV_ROW_SAMPLE, trainingLabel,
               cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);
  NeutubeConfig &config = NeutubeConfig::getInstance();
  rtree->save(config.getPath(NeutubeConfig::FLYEM_BODY_CONN_CLASSIFIER).c_str());
#endif

#endif

#if 0

#if defined(_USE_OPENCV_)
  int rowNumber, columnNumber;
  float *trainingArray = farray_load_matrix((dataPath + "/flyem/train.txt").c_str(), NULL,
                                          &columnNumber, &rowNumber);
  float *labelArray = farray_malloc(rowNumber);
  for (int i = 0; i < rowNumber; ++i) {
    labelArray[i] = trainingArray[i * columnNumber + columnNumber - 1];
  }

  for (int i = 0; i < rowNumber; ++i) {
    //cout << "Move from " << i * columnNumber << " to " << i * (columnNumber - 1) << endl;
    memmove(trainingArray + i * (columnNumber - 1), trainingArray + i * columnNumber,
            sizeof(float) * (columnNumber - 1));
  }

  cv::Mat trainingData(rowNumber, columnNumber - 1, CV_32FC1, trainingArray);
  cv::Mat trainingLabel(rowNumber, 1, CV_32FC1, labelArray);

  CvRTrees rtree;
  rtree.load((dataPath + "/segmentation_classifier.cvt").c_str());

  for (int i = 0; i < trainingData.rows; ++i) {
    float result = rtree.predict(trainingData.row(i), cv::Mat());
    cout << " " << trainingLabel.at<float>(i) << " " << (result > 0.5) << endl;
  }
#endif

#endif

#if 0
  ZXmlDoc doc;
  doc.parseFile(dataPath + "/test.xml");
  doc.printInfo();
#endif

#if 0
  ZObject3d obj;
  obj.append(1, 1, 1);
  obj.append(2, 2, 2);
  obj.append(3, 3, 3);
  int offset[3];
  Stack *stack = obj.toStack(offset);
  cout << offset[0] << " " << offset[1] << " " << offset[2] << endl;
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  ZObject3d obj;
  obj.append(1, 1, 1);
  obj.append(2, 2, 2);
  obj.append(3, 3, 3);

  ZObject3d seed;
  seed.append(1, 1, 1);
  ZObject3dArray *objArray = obj.growLabel(seed);

  Stack *stack = objArray->toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
  /*
  Stack *stack = C_Stack::make(GREY, 5, 5, 5);
  C_Stack::setZero(stack);
  obj.labelStack(stack, 255);
  C_Stack::write(dataPath + "/test.tif", stack);
  */
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  bundle.importJsonFile(filePath);
  ZObject3d *obj = bundle.getBodyBoundaryObject(66746);
  ZObject3d *seed = bundle.getBodyBorderObject(66746, 58351);

  ZObject3dArray *objArray = obj->growLabel(*seed);

  Stack *stack = objArray->toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0 //test BCF

  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/benchmark/flyem2/test/segmentation.json";
  bundle.importJsonFile(filePath);
  vector<double> feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 600, 800, FlyEm::ZSegmentationAnalyzer::BCF_RAYBURST);
  darray_print(&(feature[0]), feature.size());
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_2/segmentation.json";
  bundle.importJsonFile(filePath);

  vector<double> feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_RAYBURST);
  darray_print(&(feature[0]), feature.size());

  feature =
        FlyEm::ZSegmentationAnalyzer::computeBcf(
          bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BODY_GROW);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BODY_SIZE);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BORDER_INTENSITY);
  darray_print(&(feature[0]), feature.size());

  feature =
      FlyEm::ZSegmentationAnalyzer::computeBcf(
        bundle, 66746, 58351, FlyEm::ZSegmentationAnalyzer::BCF_BOUNDARY_GROW);
  darray_print(&(feature[0]), feature.size());
#endif

#if 0
  ZObject3d *obj1 = new ZObject3d;
  obj1->append(1, 1, 1);
  obj1->append(2, 2, 2);
  obj1->append(3, 3, 3);

  ZObject3d *obj2 = new ZObject3d;
  obj2->append(4, 1, 1);
  obj2->append(5, 2, 2);
  obj2->append(6, 3, 3);

  ZObject3dArray objArray;
  objArray.push_back(obj1);
  objArray.push_back(obj2);

  /*
  Stack *stack = C_Stack::make(GREY, 10, 5, 5);
  C_Stack::setZero(stack);
  objArray.labelStack(stack);
  C_Stack::write(dataPath + "/test.tif", stack);
  */

  Stack *stack = objArray.toStack();
  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  ZHdf5Writer writer;
  writer.open(dataPath + "/test.h5");
  writer.createGroup("/bcf");

  int ndims = 3;
  mylib::Dimn_Type dims[3] = {10, 10, 5};
  mylib::Array *array = mylib::Make_Array(
        mylib::PLAIN_KIND, mylib::UINT8_TYPE, ndims, dims);
  writer.writeArray("/bcf/feature1", array);
  mylib::Kill_Array(array);

  writer.close();

  ZHdf5Reader reader;
  reader.open(dataPath + "/test.h5");
  reader.printInfo();
  reader.close();
#endif

#if 0
  ZMatrix matrix(3, 4);
  for (int i = 0; i < matrix.getRowNumber(); ++i) {
    for (int j = 0; j < matrix.getColumnNumber(); ++j) {
      matrix.at(i, j) = i * j;
    }
  }

  matrix.debugOutput();

  matrix.resize(20, 20);
  matrix.resize(3, 4);
  matrix.debugOutput();
#endif

#if 0
  ZHdf5Reader reader;
  reader.open(dataPath + "/benchmark/flyem2/test/Bcf/Boundary_Grow.h5");
  reader.printInfo();

  mylib::Array *array = reader.readArray("Boundary_Grow");
  mylib::printArrayInfo(array);
#endif

#if 0
  FlyEm::ZBcfSet bcfSet(dataPath + "/benchmark/flyem2/test/Bcf");
  ZMatrix *matrix = bcfSet.load("Boundary_Grow");

  matrix->debugOutput();

  delete matrix;
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  bundle.importJsonFile(dataPath + "/flyem/segmentation/assignments/assignment_1/segmentation.json");
  bundle.print();

  int id1, id2;
  id1 = 32661;
  id2 = 40999;

  cout << bundle.getGalaProbability(id1, id2) << endl;

#endif

#if 0
  ZString str("1geag");
  cout << str.containsDigit() << endl;

  str = "efe";
  cout << str.containsDigit() << endl;

  str = "feat2fe";
  cout << str.containsDigit() << endl;
#endif

#if 0
  NeutubeConfig &config = NeutubeConfig::getInstance();
  config.load(config.getConfigPath());

  ZFlyEmStackFrame::trainBodyConnection();
#endif

#if 0
  ZStackFile stackFile;
  string filePath = dataPath + "/test.json";
  stackFile.importJsonFile(filePath);
  ZStack *stack = stackFile.readStack();

  Mc_Stack *stackdata = C_Stack::translate(stack->data(), GREY);

  C_Stack::write(dataPath + "/test.tif", stackdata);
#endif

#if 0
  FlyEm::ZSegmentationBundle bundle;
  std::string filePath = dataPath + "/flyem/segmentation/assignments/assignment_1/segmentation.json";
  bundle.importJsonFile(filePath);

  ZStack *stack = bundle.getPixelClassfication();
  Mc_Stack *stackdata = C_Stack::translate(stack->data(), GREY);

  C_Stack::write(dataPath + "/test.tif", stackdata);
#endif

#if 0
  ZMovieScript script;

  script.addActor(1, dataPath + "/tmp/swc3/adjusted/C2_214.swc");
  script.addActor(2, dataPath + "/tmp/swc3/adjusted/C2c_3668.swc");

  ZMovieScene scene;
  scene.setDuration(2.0);
  MovieAction action;
  action.actorId = 1;
  action.isVisible = true;
  //action.movingOffset.set(10, 0, 0);
  scene.addAction(action);

  action.actorId = 2;
  action.isVisible = false;
  scene.addAction(action);

  scene.setCameraRotation(ZPoint(0.0, 1.0, 0.0), 0.1);
  script.addScene(scene);

  scene.setDuration(2.0);
  action.actorId = 2;
  action.isVisible = true;
  scene.addAction(action);

  scene.setCameraRotation(ZPoint(0.0, 1.0, 0.0), 0.1);
  script.addScene(scene);

  ZMovieMaker director;
  director.setScript(script);
  director.make(dataPath + "/test/movie");
#endif

#if 0
  ZFileList fileList;
  fileList.load(dataPath + "/tmp/swc3/adjusted", "swc");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
    ZSwcTree *tree = new ZSwcTree;
    tree->load((*inputIter).c_str());
    tree->setColor(255, 0, 0);
    academy->addSwcTree(tree);

    Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

    //stage->getSwcFilter()->getRendererBase()->setZScale(0.5);

    Z3DCameraParameter* camera = stage->getCamera();
    camera->setProjectionType(Z3DCamera::Orthographic);
    //camera->setUpVector(glm::vec3(0.0, 0.0, -1.0));
    stage->getInteractionHandler()->getTrackball()->rotate(
          glm::vec3(1.0, 0.0, 0.0), TZ_PI_2);
    stage->resetCameraClippingRange();
    stage->getBackground()->setFirstColor(1, 1, 1, 1);
    stage->getBackground()->setSecondColor(1, 1, 1, 1);

    stage->show();
    stage->getSwcFilter()->setColorMode("intrinsic");

    stage->takeScreenShot((*inputIter + ".tif").c_str(), 1024, 1024, MonoView);
    stage->close();
  }

#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 250, 10, 1300);
  Zero_Stack(stack);
  stack->depth = 25;
  One_Stack(stack);
  stack->depth = 1300;

  Write_Stack_U((dataPath + "/flyem/skeletonization/session3/scale_bar.tif").c_str(),
                stack, NULL);
#endif

#if 0
  //Generate neuron figures
  std::string sessionDir = "flyem/skeletonization/session3";
  std::string dataDir = sessionDir + "/smoothed/snapshots/contrast/selected";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif", ZFileList::SORT_ALPHABETICALLY);

  FILE *fp = fopen((dataPath + "/" + sessionDir + "/" + "neuron_type.txt").c_str(), "r");
  if (fp == NULL) {
    std::cout << "Cannot open " << sessionDir + "/" + "neuron_type.txt" << std::endl;
  }
  ZString neuronTypeLine;
  std::vector<std::string> neuronTypeArray;
  neuronTypeArray.push_back("scale_bar");
  while (neuronTypeLine.readLine(fp)) {
    neuronTypeLine.trim();
    if ((neuronTypeLine[0] >= 'A' && neuronTypeLine[0] <= 'Z') ||
        (neuronTypeLine[0] >= 'a' && neuronTypeLine[0] <= 'z')) {
      neuronTypeArray.push_back(neuronTypeLine);
    }
  }

  fclose(fp);
  std::cout << neuronTypeArray.size() << " neuron types" << std::endl;

  std::vector<Stack*> textPatchArray;
  Stack *textImage = Read_Stack_U((dataPath + "/benchmark/mlayer_label.tif").c_str());
  for (int i =0; i < 10; ++i) {
    Stack textPatch2 = *textImage;
    textPatch2.depth = 1;
    textPatch2.array = textImage->array +
        i * C_Stack::width(textImage) * C_Stack::height(textImage);
    Stack *textPatch = C_Stack::boundCrop(&textPatch2);
    textPatchArray.push_back(textPatch);
  }
  C_Stack::kill(textImage);

  cout << neuronTypeArray.size() << " cell types" << endl;
  //ParameterDialog dlg;

  int totalCellNumber = 0;

  //if (dlg.exec()) {
  for (size_t neuronTypeIndex = 0; neuronTypeIndex < neuronTypeArray.size();
       ++neuronTypeIndex) {
    //std::string neuronType = dlg.parameter().toStdString();
    std::string neuronType = neuronTypeArray[neuronTypeIndex];

    std::vector<std::string> input;
    for (int i = 0; i < fileList.size(); ++i) {
      ZString path(fileList.getFilePath(i));
      std::vector<std::string> parts = path.fileParts();
      ZString fileName = parts[1];
      if (fileName.startsWith(neuronType)) {
        bool isTarget = true;
        if (isdigit(neuronType[neuronType.length() - 1])) {
          if (isdigit(fileName[neuronType.length()])) {
            isTarget = false;
          }
        }
        if (isTarget) {
          input.push_back(path.c_str());
        }
      } else {
        if (neuronType == "T4") {
          if (fileName.startsWith("nT4")) {
            input.push_back(path.c_str());
          }
        }
      }
    }

    /*
    if (neuronType == "Tm3") {
      input.resize(11);
    }
*/
    std::cout << neuronType << ": " << input.size() << " cells" << std::endl;

    if (neuronType != "scale_bar") {
      totalCellNumber += input.size();
    }

    /*
    int textSpace = 40;
    int textWidth = 96;
    Stack *croppedTextImage = C_Stack::boundCrop(textImage);
    C_Stack::kill(textImage);
    int left = 0;
    for (int i =0; i < 10; ++i) {
      int width;
      if (i == 0) {
        width = textWidth + textSpace;
      } else if (i == 9) {
        width = textWidth * 2 + textSpace * 2;
      } else {
        width = textWidth + textSpace * 2;
      }
      Stack *textPatch2 = C_Stack::crop(croppedTextImage, left, 0, 0,
                                       width, C_Stack::height(croppedTextImage), 1, NULL);
      Stack *textPatch = C_Stack::boundCrop(textPatch2);
      C_Stack::kill(textPatch2);
      textPatchArray.push_back(textPatch);
      left += width;
    }

    C_Stack::kill(croppedTextImage);
    */

    int mLayerStart = 317;
    //int mLayerStart = 692;
    //int mLayerEnd = 3538;
    int mLayerEnd = 3534;

    double layerPercent[] = {0, 9.4886, 18.5061, 29.6097, 33.3782, 40.3096,
                             46.9717, 57.8735, 72.1400, 89.0982, 100.0000};
    int layerArray[11];
    for (int i = 0; i < 11; ++i) {
      layerArray[i] = mLayerStart +
          iround(layerPercent[i] * (mLayerEnd - mLayerStart) / 100.0);
    }
    //input.resize(5);

    int rowSize = 5;
    int nrow = input.size() / rowSize + (input.size() % rowSize > 0);

    std::vector<std::string>::const_iterator inputIter = input.begin();
    for (int row = 0; row < nrow; ++row) {
      std::vector<Stack*> stackArray;

      int finalWidth = 0;
      int count = 1;
      for (; inputIter != input.end(); ++inputIter) {
        if (count > rowSize) {
          break;
        }
        Stack *stack = Read_Stack_U(inputIter->c_str());
        Stack *stack2 = Crop_Stack(stack, 200, 0, 0, C_Stack::width(stack) - 201,
                                   C_Stack::height(stack), 1, NULL);
        C_Stack::kill(stack);
        Cuboid_I box;
        Stack_Bound_Box(stack2, &box);
        int left = box.cb[0] -50;
        int width = box.ce[0] - box.cb[0] + 100;
        Stack *crop = Crop_Stack(stack2, left, 0, 0, width, stack2->height, 1, NULL);

        if (neuronType != "scale_bar") {
          //Draw body id
          int bodyId = String_Last_Integer(inputIter->c_str());
          int interval = 15;
          int intWidth = C_Stack::integerWidth(bodyId, interval);
          C_Stack::drawInteger(crop, bodyId,
                               (C_Stack::width(crop) - intWidth) / 2,
                               C_Stack::height(crop) - 200, 0, interval);
        }

        finalWidth += width;
        stackArray.push_back(crop);
        C_Stack::kill(stack2);
        ++count;
      }

      int leftMargin = 100;
      int rightMargin = 800;

      Stack *out = C_Stack::make(GREY, finalWidth + leftMargin + rightMargin, C_Stack::height(stackArray[0]), 1);

      Zero_Stack(out);
      uint8_t *outArray = out->array;

      for (int h = 0; h < C_Stack::height(stackArray[0]); ++h) {
        outArray += leftMargin;
        for (size_t i = 0; i < stackArray.size(); ++i) {
          memcpy(outArray,
                 stackArray[i]->array + h * C_Stack::width(stackArray[i]),
                 C_Stack::width(stackArray[i]));
          outArray += C_Stack::width(stackArray[i]);
        }
        outArray += rightMargin;
      }

      for (size_t i = 0; i < stackArray.size(); ++i) {
        C_Stack::kill(stackArray[i]);
      }

      if (neuronType != "scale_bar") {
        //Draw lines
        for (int i = 0; i < C_Stack::width(out); ++i) {
          int v = 250;
          for (int layer = 0; layer < 11; ++layer) {
            if (out->array[i + C_Stack::width(out) * layerArray[layer]] == 0) {
              out->array[i + C_Stack::width(out) * layerArray[layer]] = v;
            }
            for (int w = 1; w <= 2; ++w) {
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] - w)] = v / (w + 1);
              }
              if (out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] == 0) {
                out->array[i + C_Stack::width(out) * (layerArray[layer] + w)] = v / (w + 1);
              }
            }
          }
        }

        //Draw texts
        for (int layer = 0; layer < 10; ++layer) {
          int dx = C_Stack::width(out) - 250;
          int dy = (layerArray[layer] + layerArray[layer + 1]) / 2 -
              C_Stack::height(textPatchArray[layer]) / 2;
          C_Stack::drawPatch(out, textPatchArray[layer], dx, dy, 0, 0);
        }

        Stack *scaleBar = Read_Stack_U((dataPath + "/" + dataDir + "/row/scale_bar_row1.tif").c_str());
        Stack *croppedScaleBar = C_Stack::boundCrop(scaleBar);
        C_Stack::kill(scaleBar);
        C_Stack::drawPatch(out, croppedScaleBar, C_Stack::width(out) - 700, C_Stack::height(out) - 200, 0, 0);
        C_Stack::kill(croppedScaleBar);
      }

      std::ostringstream stream;
      stream << dataPath + "/" + dataDir + "/row/" + neuronType
             << "_row" << row + 1 << ".tif";

      Write_Stack_U(stream.str().c_str(), out, NULL);
      C_Stack::kill(out);
    }
  }

  for (int layer = 0; layer < 10; ++layer) {
    C_Stack::kill(textPatchArray[layer]);
  }

  cout << "Total: " << totalCellNumber << " neurons" << endl;
#endif

#if 0
  //Volume rendering snapshots

  std::string dataDir = "flyem/skeletonization/session3/smoothed";
  //std::string dataDir = "benchmark/binary/3d/block";
  ZFileList fileList;
  fileList.load(dataPath + "/" + dataDir, "tif");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);
  //Filter_3d *filter = Gaussian_Filter_3d(0.5, 0.5, 0.5);
  //input.resize(1);
  //input[0] = dataPath + "/" + dataDir + "/" + "Pm2-8_171795.tif";

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    std::string output;
    ZString inputPath(*inputIter);
    std::vector<std::string> parts = inputPath.fileParts();
    output = dataPath + "/" + dataDir + "/snapshots/" + parts[1] + ".tif";

    if (!fexist(output.c_str())) {

      std::string offsetFile = *inputIter + ".offset.txt";
      FILE *fp = fopen(offsetFile.c_str(), "r");
      ZString offsetStr;
      offsetStr.readLine(fp);
      std::vector<int> offset =offsetStr.toIntegerArray();
      fclose(fp);

      tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);

      academy->loadFile((*inputIter).c_str());

      Z3DWindow *stage = new Z3DWindow(academy, false, NULL);
      stage->getVolumeSource()->setZScale(1.125);

      //const std::vector<double> &boundBox = stage->getBoundBox();

      Z3DCameraParameter* camera = stage->getCamera();
      camera->setProjectionType(Z3DCamera::Orthographic);
      //stage->resetCamera();

      //camera->setUpVector(glm::vec3(0.0, 0.0, -1.0));
      /*
  stage->getInteractionHandler()->getTrackball()->rotate(
        glm::vec3(1.0, 0.0, 0.0), TZ_PI_2);
        */

      glm::vec3 referenceCenter = camera->getCenter();

      double eyeDistance = 3000;//boundBox[3] - referenceCenter[1] + 2500;
      //double eyeDistance = 2000 - referenceCenter[1];
      glm::vec3 viewVector(0, -1, 0);

      viewVector *= eyeDistance;
      glm::vec3 eyePosition = referenceCenter - viewVector;

      referenceCenter[2] = (650 - offset[2]) * 1.125;
      camera->setCenter(referenceCenter);
      eyePosition[2] = (650 - offset[2]) * 1.125;
      camera->setEye(eyePosition);
      camera->setUpVector(glm::vec3(0, 0, -1));
      stage->resetCameraClippingRange();

      stage->getBackground()->setFirstColor(0, 0, 0, 1);
      stage->getBackground()->setSecondColor(0, 0, 0, 1);

      //std::cout << "scales: " << stage->getVolumeRaycaster()->getRenderer()->getCoordScales() << std::endl;

      camera->setNearDist(2000.0);

      stage->show();

      std::cout << output << std::endl;
      stage->takeScreenShot(output.c_str(), 4000, 4000, MonoView);
      stage->close();
      delete stage;
    }
  }
#endif

#if 0
  //Draw synapses
  ZFileList fileList;
  fileList.load(dataPath + "/flyem/skeletonization/session3/len15/adjusted", "swc");

  std::vector<std::string> input;
  for (int i = 0; i < fileList.size(); ++i) {
    input.push_back(fileList.getFilePath(i));
  }

  //input.resize(1);

  std::map<std::string, QColor> punctaColorMap;
  punctaColorMap[""] = QColor(0, 255, 0);
  punctaColorMap["Unknown"] = QColor(0, 255, 0);
  punctaColorMap["_1"] = QColor(255, 0, 0);
  punctaColorMap["_2"] = QColor(0, 0, 255);
  punctaColorMap["Tm3"] = QColor(255, 0, 0);
  punctaColorMap["Mi1"] = QColor(0, 0, 255);

  for (std::vector<std::string>::const_iterator inputIter = input.begin();
       inputIter != input.end(); ++inputIter) {
    ZString markerFile = ZString(*inputIter).changeExt("marker");
    markerFile = markerFile.changeDirectory(dataPath + "/flyem/TEM/T4viz/synapse2");
    if (fexist(markerFile.c_str())) {
      tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
      ZSwcTree *tree = new ZSwcTree;
      tree->load((*inputIter).c_str());
      tree->setColor(255, 255, 0);
      academy->addSwcTree(tree);


      academy->loadFile(markerFile.c_str());

      QList<ZPunctum*> *puncta = academy->punctaList();
      for (QList<ZPunctum*>::const_iterator punctaIter = puncta->begin();
           punctaIter != puncta->end(); ++punctaIter) {
        (*punctaIter)->setColor(punctaColorMap[(*punctaIter)->name().toStdString()]);
      }

      Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

      //stage->getSwcFilter()->getRendererBase()->setZScale(0.5);

      const std::vector<double> &boundBox = stage->getBoundBox();
      Z3DCameraParameter* camera = stage->getCamera();
      camera->setProjectionType(Z3DCamera::Orthographic);
      glm::vec3 referenceCenter = camera->getCenter();

      double eyeDistance = boundBox[5] - referenceCenter[2] + 40000;
      glm::vec3 viewVector(0, 0, 1);

      viewVector *= eyeDistance;
      glm::vec3 eyePosition = referenceCenter - viewVector;

      camera->setEye(eyePosition);
      camera->setUpVector(glm::vec3(0, -1, 0));

      stage->resetCameraClippingRange();
      stage->getBackground()->setFirstColor(1, 1, 1, 1);
      stage->getBackground()->setSecondColor(1, 1, 1, 1);

      stage->getPunctaFilter()->setSizeScale(2.0);
      stage->getPunctaFilter()->setColorMode("Original Point Color");

      stage->getSwcFilter()->setColorMode("Intrinsic");
      stage->getSwcFilter()->getRendererBase()->setOpacity(0.3);

      stage->show();

      stage->takeScreenShot((markerFile + ".tif").c_str(), 2048, 2048, MonoView);
      stage->close();
    }
  }
#endif

#if 0
  ZMovieCamera camera;

  ZMovieScene scene;

  scene.print();
#endif

#if 0
  ZMovieScript script;
  int frameNumber = 0;
  string outDir = dataPath + "/test/movie";

  if (script.loadScript(dataPath + "/flyem/TEM/movie/script4.json")) {
    ZMovieMaker director;
    director.setFrameSize(512, 512);
    director.setScript(script);
    director.setFrameInterval(40);
    frameNumber = director.make(outDir);
  }

  /*
  if (host != NULL) {
    ZStackFile stackFile;
    for (int index = 0; index < frameNumber; ++index) {
      stackFile.appendUrl(ZMovieMaker::getFramePath(outDir, index));
    }
    stackFile.setType("file list");
    ZStack *stack = stackFile.readStack();
    if (stack != NULL) {
      ZStackFrame *frame = new ZStackFrame;
      frame->loadStack(stack);
      host->addStackFrame(frame);
    }
  }
  */
#endif

#if 0
  ZMovieScriptGenerator writer;

  writer.addActor("slice",
                  "/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif");

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");

  ZPoint colorL1(1.0, 1.0, 0.0);
  ZPoint colorMi1(0.0, 0.0, 1.0);
  ZPoint colorTm3(1.0, 0.0, 0.0);
  ZPoint colorT4(0.0, 1.0, 0.0);
  ZPoint colorT4Mi1(0.0, 0.0, 1.0);
  ZPoint colorT4Tm3(1.0, 0.0, 0.0);

  vector<string> L1Neurons;
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      L1Neurons.push_back(iter->getName());
    }
  }

  set<string> excludedL1Neurons;
  excludedL1Neurons.insert("L1-i8");
  excludedL1Neurons.insert("L1-k10");

  vector<string> selectedL1Neurons;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      selectedL1Neurons.push_back(*iter);
    }
  }

  vector<string> T4Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "T4") {
      T4Neurons.push_back(iter->getName());
    }
  }

  vector<string> Mi1Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "Mi1") {
      Mi1Neurons.push_back(iter->getName());
    }
  }

  string Mi1Neuron_T4_12 = "Mi1-a";
  vector<string> Tm3Neuron_T4_12;
  Tm3Neuron_T4_12.push_back("Tm3-A");
  Tm3Neuron_T4_12.push_back("Tm3-b-A");
  Tm3Neuron_T4_12.push_back("Tm3-r5-P");
  Tm3Neuron_T4_12.push_back("Tm3-h7-A");
  Tm3Neuron_T4_12.push_back("Tm3-f-P");

  vector<string> T4_12_connectedNeurons;
  T4_12_connectedNeurons.push_back(Mi1Neuron_T4_12);
  T4_12_connectedNeurons.insert(T4_12_connectedNeurons.end(),
                                Tm3Neuron_T4_12.begin(),
                                Tm3Neuron_T4_12.end());

  set<string> Tm3Neuron_T4_12_set;
  for (vector<string>::const_iterator iter = Tm3Neuron_T4_12.begin();
       iter != Tm3Neuron_T4_12.end(); ++iter) {
    Tm3Neuron_T4_12_set.insert(*iter);
  }


  vector<string> Tm3Neurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "Tm3") {
      Tm3Neurons.push_back(iter->getName());
    }
  }

  //Write cast
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "T4" || iter->getClass() == "Tm3" ||
        iter->getClass() == "Mi1" || iter->getClass() == "L1")
    writer.addActor(iter->getName(), iter->getModelPath());
  }

  writer.addActor("T4-Mi1", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_Mi1_a.marker");
  writer.addActor("T4-Tm3", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_Tm3-f-P.marker");
  writer.addActor("tbar_box", dataPath + "/flyem/TEM/movie/actor/tbar-bound.swc");
  writer.addActor("arrow1", dataPath + "/flyem/TEM/movie/actor/arrow1.swc");
  writer.addActor("arrow2", dataPath + "/flyem/TEM/movie/actor/arrow2.swc");
  writer.addActor("T4-tbar", dataPath + "/flyem/TEM/movie/actor/puncta/T4_12_tbar.marker");

  std::ofstream stream((dataPath + "/flyem/TEM/movie/script11.json").c_str());
  stream << "{" << endl;

  writer.writeCast(stream, 1);
  stream << "," << endl;

  writer.writePlotStart(stream, 1);

  vector<string> actions;

  //Show Slice
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 80, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.push_back("move");
  actions.push_back("[0, 0, 13.825]");
  actions.push_back("visible");
  actions.push_back("true");
  writer.writeAction(stream, "slice", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << ",";
  stream << endl;
  writer.writeCameraStart(stream, 3);

  vector<string> reset;
  reset.push_back("eye");
  reset.push_back("[599, 599, 0]");
  reset.push_back("center");
  reset.push_back("[599, 599, 1064]");
  reset.push_back("up_vector");
  reset.push_back("[0, -1, 0]");
  writer.writeCameraReset(stream, reset, 4);
  stream << endl;

  writer.writeCameraEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  vector<string> camera;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.056, 0.056, 1.0]");
  camera.push_back("center");
  camera.push_back("[0.056, 0.056, 0]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hightlight t-bar
  int interval = 200;
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "tbar_box", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Show single syanpse
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 200, 3);
  stream << "," << endl;
  stream << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, "T4-tbar", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4
  writer.writeSceneStart(stream,2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");

  writer.writeAction(stream, "T4-12", actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //hide slice
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeAction(stream, "slice", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4-Mi1 and T4-Tm3 synapses
  writer.writeSceneStart(stream, 2);

  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("color");
  actions.push_back(colorT4Mi1.toJsonString());
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");

  writer.writeAction(stream, "T4-Mi1", actions, 4);
  stream << "," << endl;

  actions[1] = colorT4Tm3.toJsonString();
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;

  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[0.168035, 0.147384, -0.285]");
  actions.push_back("up_vector");
  actions.push_back("[-0.000189209, 0.000132543, -0.000460128]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Show Mi1 neuron
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("color");
  actions.push_back("[0.0, 0.0, 1.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  //Zoom in
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.0188, -0.1868, 0.1833]");
  camera.push_back("center");
  camera.push_back("[0.1487, -0.0397, -0.1641]");
  camera.push_back("up_vector");
  camera.push_back("[-0.000619616, 0.000370943, 0.000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 900, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.00188, -0.01868, 0.01833]");
  camera.push_back("center");
  camera.push_back("[0.01487, -0.00397, -0.01641]");
  camera.push_back("up_vector");
  camera.push_back("[-0.0000619616, 0.0000370943, 0.0000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 100, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show T4-Tm3 synapses
  /*
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 0.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
  */

  //Show Tm3
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  //zoom back
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.0188, 0.1868, -0.1833]");
  camera.push_back("center");
  camera.push_back("[-0.1487, 0.0397, 0.1641]");
  camera.push_back("up_vector");
  camera.push_back("[0.000619616, -0.000370943, -0.000135057]");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out more and show all Mi1, Tm3 neurons
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("visible");
  actions.push_back("true");

  for (vector<string>::const_iterator iter = Mi1Neurons.begin();
       iter != Mi1Neurons.end(); ++iter) {
    if (*iter != "Mi1-a") {
      writer.writeAction(stream, *iter, actions, 4);
      stream << "," << endl;
    }
  }

  actions[1] = colorTm3.toJsonString();
  for (vector<string>::const_iterator iter = Tm3Neurons.begin();
       iter != Tm3Neurons.end(); ++iter) {
    if (*iter != "Tm3-f-P") {
      if (iter != Tm3Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.186843, 0.49986, -1.17363]");
  camera.push_back("up_vector");
  camera.push_back("[-0.000372487, 0.000161579, 0.0000285799]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //zoom out more
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[0.758089, 0.584229, 0.120433]");
  actions.push_back("center");
  actions.push_back("[-0.392549, -0.54059, -0.288692]");
  actions.push_back("up_vector");
  actions.push_back("[0.000231308, 0.00041455, -0.00047294]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream <<",";
  stream << endl;

  //change view angle
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[-1.42134, -1.02811, -1.02052]");
  actions.push_back("center");
  actions.push_back("[0.578985, 0.570409, 0.390763]");
  actions.push_back("up_vector");
  actions.push_back("[-0.0000664507, -0.000593431, 0.000936398]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3 neuron
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Tm3-f-P", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;
#endif

  //Add L1 neurons one by one
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeSceneStart(stream, 2);
      writer.writeDuration(stream, 200, 3);
      stream << "," << endl;
      actions.clear();
      actions.push_back("color");
      actions.push_back("[1.0, 1.0, 0.0]");
      actions.push_back("visible");
      actions.push_back("true");
      actions.push_back("alpha");
      actions.push_back("0.1");
      actions.push_back("fade");
      actions.push_back("0.005");
      writer.writeActionStart(stream, 3);
      writer.writeAction(stream, *iter, actions, 4);
      stream << endl;
      writer.writeActionEnd(stream, 3);
      stream << endl;
      writer.writeSceneEnd(stream, 2);
    }
  }
  stream << ",";
  stream << endl;

  //Adjust view
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  actions.clear();
  actions.push_back("eye");
  actions.push_back("[-0.500935, -0.538451, -0.878699]");
  actions.push_back("center");
  actions.push_back("[0.0716949, 0.0258559, 0.0328573]");
  writer.writeCameraMove(stream, actions, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

#if 0
  //Iterate through L1 neurons
  string sortedL1Neurons[] = {
    "L1-m12", "L1-n1", "L1-o2", "L1-l11", "L1-d", "L1-e", "L1-p3", "L1-c",
    "L1", "L1-f", "L1-q4", "L1-j9", "L1-b", "L1-a", "L1-r5", "L1-h7", "L1-g6"
  };
  for (size_t i = 0; i < L1Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 80, 3);
    stream << "," << endl;
    actions.clear();
    actions.push_back("color");
    actions.push_back("[1.0, 1.0, 0.6]");
    actions.push_back("visible");
    actions.push_back("true");
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, sortedL1Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);

    stream << "," << endl;
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 200, 3);
    stream << "," << endl;
    actions[1] = "[1.0, 1.0, 0.0]";
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, sortedL1Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);
  }
  stream << ",";
  stream << endl;
#endif

  //Rotate to the other side
#  if 1
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Hide Tm3 and Mi1 neurons other than connected ones
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 2);
  stream << "," << endl;
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");
  actions.push_back("visible");
  actions.push_back("true");
  writer.writeActionStart(stream, 3);

  vector<string> selectedNeurons;

  selectedNeurons.clear();
  for (vector<string>::const_iterator iter = Tm3Neurons.begin();
       iter != Tm3Neurons.end(); ++iter) {
    if (Tm3Neuron_T4_12_set.count(*iter) == 0) {
      selectedNeurons.push_back(*iter);
    }
  }

  for (vector<string>::const_iterator iter = Mi1Neurons.begin();
       iter != Mi1Neurons.end(); ++iter) {
    if (*iter != "Mi1-a") {
      selectedNeurons.push_back(*iter);
    }
  }

  writer.writeAction(stream, selectedNeurons, actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeAction(stream, selectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  //Add more Tm3 neurons
  vector<string> connectedTm3Neurons;
  connectedTm3Neurons.push_back("Tm3-A");
  connectedTm3Neurons.push_back("Tm3-b-A");
  connectedTm3Neurons.push_back("Tm3-r5-P");
  connectedTm3Neurons.push_back("Tm3-h7-A");
  connectedTm3Neurons.push_back("Tm3-f-P");

#  if 0
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeSceneStart(stream, 2);
    writer.writeDuration(stream, 1000, 3);
    stream << "," << endl;
    actions.clear();
    actions.push_back("color");
    actions.push_back("[1.0, 0.0, 0.0]");
    actions.push_back("visible");
    actions.push_back("true");
    actions.push_back("alpha");
    actions.push_back("0.1");
    actions.push_back("fade");
    actions.push_back("0.001");
    writer.writeActionStart(stream, 3);
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
    stream << endl;
    writer.writeActionEnd(stream, 3);
    stream << endl;
    writer.writeSceneEnd(stream, 2);
  }
  stream << ",";
  stream << endl;
#endif

#if 0
  //Rotate back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Hide L1 neurons
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Mi1 neuron
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;
  stream << endl;
#endif

  //Rotate 180 degrees
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change transparency of Tm3 neurons
  double connectionStrengthTm3[] = {
    4, 3, 6, 4, 3
  };

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;

  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << "," << endl;

  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("0.0");

  int i = 0;
  for (vector<string>::const_iterator iter = connectedTm3Neurons.begin();
       iter != connectedTm3Neurons.end(); ++iter, ++i) {
    if (iter != connectedTm3Neurons.begin()) {
      stream << "," << endl;
    }
    ostringstream strstream;
    strstream << (connectionStrengthTm3[i] - 6.0) / 5000.0;
    actions[3] = strstream.str();
    writer.writeAction(stream, *iter, actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3
#if 0
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");

  writer.writeActionStart(stream, 3);
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");

  writer.writeActionStart(stream, 3);
  for (size_t i = 0; i < connectedTm3Neurons.size(); ++i) {
    if (i != 0) {
      stream << "," << endl;
    }
    writer.writeAction(stream, connectedTm3Neurons[i], actions, 4);
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;
#endif

  //Add back L1
  vector<string> L1ColorArray(19);
  L1ColorArray[0] = "[1, 0.79224, 0.79224]";
  L1ColorArray[1] = "[0.5462, 0, 1]";
  L1ColorArray[2] = "[1, 0.9836, 0.9836]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.99453, 0.99453]";
  L1ColorArray[5] = "[1, 0.95079, 0.95079]";
  L1ColorArray[6] = "[1, 0.61591, 0.61591]";
  L1ColorArray[7] = "[1, 0.83051, 0.83051]";
  L1ColorArray[8] = "[1, 0.90159, 0.90159]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 1, 1]";
  L1ColorArray[14] = "[1, 1, 1]";
  L1ColorArray[15] = "[1, 1, 1]";
  L1ColorArray[16] = "[1, 0.9836, 0.9836]";
  L1ColorArray[17] = "[1, 0.92619, 0.92619]";
  L1ColorArray[18] = "[1, 0.73346, 0.73346]";

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  i = 0;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (iter != L1Neurons.begin()) {
        stream << "," << endl;
      }
      actions[3] = L1ColorArray[i];
      writer.writeAction(stream, *iter, actions, 4);
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Adjust view
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.3476, -1.0482, 0.0401]");
  camera.push_back("center");
  camera.push_back("[0.0778, 0.2182, 0.0304]");
  camera.push_back("up_vector");
  camera.push_back("[-0.0000768, -0.0000820, -0.0003514]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Color L1 neuron for Mi1
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[0.8051, 0.0, 1.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeAction(stream, L1Neurons[1], actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide Tm3 and Mi1 neurons
  actions.clear();
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, T4_12_connectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, T4_12_connectedNeurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.3020, 0.4281, -1.6749]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add arrow
  interval = 200;
  ZPoint arrowColor(1.0, 1.0, 0.0);
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(arrowColor.toJsonString());
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow1", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.3020, -0.4281, 1.6749]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Hide T4-12 and turn L1 to white
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-12", actions, 4);
  stream << ",";
  stream << endl;
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("transit");
  actions.push_back("0.001");
  writer.writeAction(stream, selectedL1Neurons, actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("false");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-12", actions, 4);
  stream << ",";
  stream << endl;
  writer.writeAction(stream, "T4-Mi1", actions, 4);
  stream << ",";
  stream << endl;
  writer.writeAction(stream, "T4-Tm3", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add T4-10
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, "T4-10", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show Tm3 and Mi1-neurons
  vector<string> Tm3Neurons_T4_10;
  string Mi1Neuron_T4_10 = "Mi1-e";
  Tm3Neurons_T4_10.push_back("Tm3-A");
  Tm3Neurons_T4_10.push_back("Tm3-e-P");
  Tm3Neurons_T4_10.push_back("Tm3-p3-P");
  Tm3Neurons_T4_10.push_back("Tm3-f-A");
  Tm3Neurons_T4_10.push_back("Tm3-o2-P");
  Tm3Neurons_T4_10.push_back("Tm3-e-A");

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.01");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("color");
  actions.push_back(colorMi1.toJsonString());
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  writer.writeAction(stream, Mi1Neuron_T4_10, actions, 4);
  stream << "," << endl;

  actions[7] = colorTm3.toJsonString();

  actions[5] = "0.0001";
  writer.writeAction(stream, Tm3Neurons_T4_10[0], actions, 4);
  stream << "," << endl;
  actions[5] = "0.001";
  writer.writeAction(stream, Tm3Neurons_T4_10[1], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0003";
  writer.writeAction(stream, Tm3Neurons_T4_10[2], actions, 4);
  stream << "," << endl;
  actions[5] = "0.00001";
  writer.writeAction(stream, Tm3Neurons_T4_10[3], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0009";
  writer.writeAction(stream, Tm3Neurons_T4_10[4], actions, 4);
  stream << "," << endl;
  actions[5] = "0.0002";
  writer.writeAction(stream, Tm3Neurons_T4_10[5], actions, 4);
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Add Mi1 neuron and Tm3 neuorns connected to T4-10
  L1ColorArray[0] = "[1, 0.49153, 0.49153]";
  L1ColorArray[1] = "[1, 0.99263, 0.99263]";
  L1ColorArray[2] = "[1, 1, 1]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.68497, 0.68497]";
  L1ColorArray[5] = "[0.18755, 0, 1]";
  L1ColorArray[6] = "[1, 0.85262, 0.85262]";
  L1ColorArray[7] = "[1, 1, 1]";
  L1ColorArray[8] = "[1, 1, 1]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 0.98342, 0.98342]";
  L1ColorArray[14] = "[1, 0.54864, 0.54864]";
  L1ColorArray[15] = "[1, 0.37546, 0.37546]";
  L1ColorArray[16] = "[1, 0.70339, 0.70339]";
  L1ColorArray[17] = "[1, 0.9392, 0.9392]";
  L1ColorArray[18] = "[1, 1, 1]";

  //Rotate 360 degrees
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 2000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.18");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change color of L1 and fade out Mi1, Tm3
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("fade");
  actions.push_back("-0.001");
  writer.writeAction(stream, Mi1Neuron_T4_10, actions, 4);
  stream << "," << endl;

  writer.writeAction(stream, Tm3Neurons_T4_10[0], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[1], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[2], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[3], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[4], actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, Tm3Neurons_T4_10[5], actions, 4);
  stream << "," << endl;

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");
  actions.push_back("alpha");
  actions.push_back("0.1");
  actions.push_back("fade");
  actions.push_back("0.001");
  i = 0;
  bool isFirst = true;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (L1ColorArray[i] != "[1, 1, 1]") {
        if (!isFirst) {
          stream << "," << endl;
        }
        actions[3] = L1ColorArray[i];
        writer.writeAction(stream, *iter, actions, 4);
        isFirst = false;
      }
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom in
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[-0.159282, 0.108322, -1.615]");
  camera.push_back("center");
  camera.push_back("[0.152259, -0.378408, -0.130522]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Show arrow
  interval = 200;
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back(arrowColor.toJsonString());
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "true";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << "," << endl;

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, interval, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions[1] = "false";
  writer.writeAction(stream, "arrow2", actions, 4);
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Zoom back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  camera.clear();
  camera.push_back("eye");
  camera.push_back("[0.159282, -0.108322, 1.615]");
  camera.push_back("center");
  camera.push_back("[-0.152259, 0.378408, 0.130522]");
  writer.writeCameraMove(stream, camera, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  connectedTm3Neurons.clear();

  connectedTm3Neurons.push_back("Tm3-A");
  connectedTm3Neurons.push_back("Tm3-b-A");
  connectedTm3Neurons.push_back("Tm3-f-P");
  connectedTm3Neurons.push_back("Tm3-p3-P");
  connectedTm3Neurons.push_back("Tm3-f-A");
  connectedTm3Neurons.push_back("Tm3-r5-P");
  connectedTm3Neurons.push_back("Tm3-h7-A");
  connectedTm3Neurons.push_back("Tm3-o2-P");
  connectedTm3Neurons.push_back("Tm3-e-A");

  vector<string> fading;
  fading.push_back("0.0005");
  fading.push_back("0.001");
  fading.push_back("0.0003");
  fading.push_back("0.0006");
  fading.push_back("0.0003");
  fading.push_back("0.0001");
  fading.push_back("0.0004");
  fading.push_back("0.0003");
  fading.push_back("0.0009");

  //Put T4-12 and its connected neurons back
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 1000, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);
  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  actions.push_back("color");
  actions.push_back(colorT4.toJsonString());

  writer.writeAction(stream, "T4-12", actions, 4);
  stream << "," << endl;

  actions[7] = colorMi1.toJsonString();
  writer.writeAction(stream, "Mi1-a", actions, 4);
  stream << "," << endl;
  writer.writeAction(stream, "Mi1-e", actions, 4);

  actions[7] = colorTm3.toJsonString();
  for (size_t index = 0; index < connectedTm3Neurons.size(); ++index) {
    stream << "," << endl;
    actions[5] = fading[index];
    writer.writeAction(stream, connectedTm3Neurons[index], actions, 4);
  }
  stream << endl;

  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Change L1 color
  L1ColorArray[0] = "[1, 0.2115, 0.2115]";
  L1ColorArray[1] = "[0.38099, 0, 1]";
  L1ColorArray[2] = "[1, 0.97789, 0.97789]";
  L1ColorArray[3] = "[1, 1, 1]";
  L1ColorArray[4] = "[1, 0.6776, 0.6776]";
  L1ColorArray[5] = "[0.12122, 0, 1]";
  L1ColorArray[6] = "[1, 0.33493, 0.33493]";
  L1ColorArray[7] = "[1, 0.77155, 0.77155]";
  L1ColorArray[8] = "[1, 0.86735, 0.86735]";
  L1ColorArray[9] = "[1, 1, 1]";
  L1ColorArray[10] = "[1, 1, 1]";
  L1ColorArray[11] = "[1, 1, 1]";
  L1ColorArray[12] = "[1, 1, 1]";
  L1ColorArray[13] = "[1, 0.98342, 0.98342]";
  L1ColorArray[14] = "[1, 0.54864, 0.54864]";
  L1ColorArray[15] = "[1, 0.37546, 0.37546]";
  L1ColorArray[16] = "[1, 0.68128, 0.68128]";
  L1ColorArray[17] = "[1, 0.83972, 0.83972]";
  L1ColorArray[18] = "[1, 0.64075, 0.64075]";

  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 40, 3);
  stream << "," << endl;
  writer.writeActionStart(stream, 3);

  actions.clear();
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 0.0, 0.0]");

  i = 0;
  isFirst = true;
  for (vector<string>::const_iterator iter = L1Neurons.begin();
       iter != L1Neurons.end(); ++iter, ++i) {
    if (excludedL1Neurons.count(*iter) == 0) {
      if (!isFirst) {
        stream << "," << endl;
      }
      actions[3] = L1ColorArray[i];
      writer.writeAction(stream, *iter, actions, 4);
      isFirst = false;
    }
  }
  stream << endl;
  writer.writeActionEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << ",";
  stream << endl;

  //Rotate
  writer.writeSceneStart(stream, 2);
  writer.writeDuration(stream, 4000, 3);
  stream << "," << endl;
  writer.writeCameraStart(stream, 3);
  reset.clear();
  reset.push_back("axis");
  reset.push_back("[0.0, 1.0, 0.0]");
  reset.push_back("angle");
  reset.push_back("0.09");
  writer.writeCameraRotate(stream, reset, 4);
  stream << endl;
  writer.writeCameraEnd(stream, 3);
  stream << endl;
  writer.writeSceneEnd(stream, 2);
  stream << endl;

  /*
  writer.writeSceneStart(stream, 2);

  writer.writeDuration(stream, )

  writer.writeSceneEnd(stream, 2);
  */

  writer.writePlotEnd(stream, 1);
  stream << endl;

  stream << "}";

#endif

#if 0 // for reconstruction movie
  ZMovieScriptGenerator writer;
  /*
  string id = "gray_stack";
  string source = dataPath + "/flyem/TEM/gray_ds_avg.tif";
  writer.addActor(id, source);
  id = "R7_205";
  source = dataPath + "/flyem/skeletonization/session3/len15/adjusted2/R7_205.swc";
  writer.addActor(id, source);
  writer.writeCast(cout, 1);
  */

  vector<ZPoint> colorArray;
  colorArray.push_back(ZPoint(1.0, 0.0, 0.0));
  colorArray.push_back(ZPoint(0.5, 0.0, 0.0));
  colorArray.push_back(ZPoint(0.0, 1.0, 0.0));
  colorArray.push_back(ZPoint(0.133, 0.545, 0.133));
  colorArray.push_back(ZPoint(0.0, 0.0, 1.0));
  colorArray.push_back(ZPoint(0.117, 0.546, 1.0));
  colorArray.push_back(ZPoint(0.25, 0.875, 0.8125));
  colorArray.push_back(ZPoint(0.439, 0.858, 0.576));
  colorArray.push_back(ZPoint(1.0, 1.0, 0.0));
  colorArray.push_back(ZPoint(1.0, 1.0, 0.5));
  colorArray.push_back(ZPoint(1.0, 0.705, 0.549));
  colorArray.push_back(ZPoint(1.0, 0.84, 0.0));
  colorArray.push_back(ZPoint(1.0, 0.64, 0.0));
  colorArray.push_back(ZPoint(0.625, 0.125, 0.9375));
  colorArray.push_back(ZPoint(1.0, 0.0, 1.0));
  colorArray.push_back(ZPoint(0.93, 0.508, 0.93));

  vector<string> laminaInputs;
  laminaInputs.push_back("R7");
  laminaInputs.push_back("R8");
  laminaInputs.push_back("L1");
  laminaInputs.push_back("L2");
  laminaInputs.push_back("L3");
  laminaInputs.push_back("L4");
  laminaInputs.push_back("L5");
  laminaInputs.push_back("C2");
  laminaInputs.push_back("C3");
  laminaInputs.push_back("T1");
  laminaInputs.push_back("lawf1 224"); //LaWF1 neuron
  //laminaInputs.push_back("lawf1 446840"); //LaWF1 neuron

  vector<string> firstOrderNeurons;
  firstOrderNeurons.push_back("Tm1");
  firstOrderNeurons.push_back("Tm2");
  firstOrderNeurons.push_back("Tm3-P");
  firstOrderNeurons.push_back("Tm4");
  firstOrderNeurons.push_back("Tm5a");
  firstOrderNeurons.push_back("Tm9");
  firstOrderNeurons.push_back("Tm20");
  firstOrderNeurons.push_back("Mi1");
  firstOrderNeurons.push_back("Mi4");
  firstOrderNeurons.push_back("Mi9");
  firstOrderNeurons.push_back("Dm2 109");
  firstOrderNeurons.push_back("Dm4 147809");
  firstOrderNeurons.push_back("Dm8");
  firstOrderNeurons.push_back("Dm9 178127");

  vector<string> secondOrderNeurons;
  secondOrderNeurons.push_back("Tm5b");
  secondOrderNeurons.push_back("Tm5Y 1170");
  secondOrderNeurons.push_back("Tm6/14 4781");
  secondOrderNeurons.push_back("unknown Tm-1 3474");
  secondOrderNeurons.push_back("Tm12/25 3450");
  secondOrderNeurons.push_back("unknown Tm-1 305305");
  secondOrderNeurons.push_back("Tm23/Tm24 132");
  secondOrderNeurons.push_back("Tm23/Tm24 89");
  secondOrderNeurons.push_back("Tm28/TmY9 195");
  secondOrderNeurons.push_back("TmY14 221791");
  secondOrderNeurons.push_back("TmY13 4248");
  secondOrderNeurons.push_back("TmY3 305999");
  secondOrderNeurons.push_back("TmY4 5816");
  secondOrderNeurons.push_back("TmY5-like 61791");
  secondOrderNeurons.push_back("TmY5a 352887");
  secondOrderNeurons.push_back("TmY10 5743");
  secondOrderNeurons.push_back("TmY11/Y13-1");
  secondOrderNeurons.push_back("Mi3-like 230");
  secondOrderNeurons.push_back("Mi10");
  secondOrderNeurons.push_back("Mi11-like 1648");
  secondOrderNeurons.push_back("Mi13 2511");
  secondOrderNeurons.push_back("Mi14 6042");
  secondOrderNeurons.push_back("Mi15");
  secondOrderNeurons.push_back("Dm1-like 306420");
  secondOrderNeurons.push_back("Dm3-like 92550");
  secondOrderNeurons.push_back("Dm5-like 22293");
  secondOrderNeurons.push_back("Dm7-like 3647");
  secondOrderNeurons.push_back("Dm10 306715");
  secondOrderNeurons.push_back("Pm1 1980");
  secondOrderNeurons.push_back("Pm2-like 171795");
  secondOrderNeurons.push_back("unknown Pm-1 1773");
  secondOrderNeurons.push_back("T2a");
  secondOrderNeurons.push_back("T3 1554");
  secondOrderNeurons.push_back("T4-11");
  secondOrderNeurons.push_back("Y4 1593971");
  secondOrderNeurons.push_back("Y3/Y6 2443");

  vector<string> interColumnarNeurons;
  interColumnarNeurons.push_back("Tm3-A");
  interColumnarNeurons.push_back("Tm3-e-P");
  interColumnarNeurons.push_back("Tm3-b-A");
  interColumnarNeurons.push_back("Tm3-j9-A");
  interColumnarNeurons.push_back("Tm3-d-P");
  interColumnarNeurons.push_back("Tm3-f-P");
  interColumnarNeurons.push_back("Tm3-d-A");
  interColumnarNeurons.push_back("Tm4-c");
  interColumnarNeurons.push_back("Tm4-e");
  interColumnarNeurons.push_back("Tm4-d");
  interColumnarNeurons.push_back("Tm4-a/f");
  interColumnarNeurons.push_back("Tm4-b/c");
  interColumnarNeurons.push_back("Dm3-like 82307");
  interColumnarNeurons.push_back("Dm3-like 133922");
  interColumnarNeurons.push_back("Dm8-b");
  interColumnarNeurons.push_back("Dm8-c");
  interColumnarNeurons.push_back("Pm1 106054");
  interColumnarNeurons.push_back("Pm1 1172");
  interColumnarNeurons.push_back("Pm1 2236");
  interColumnarNeurons.push_back("Pm1 4390");
  interColumnarNeurons.push_back("Pm2-like 1196");
  interColumnarNeurons.push_back("Pm2-like 1513");
  interColumnarNeurons.push_back("Pm2-like 1974");
  interColumnarNeurons.push_back("Pm2-like 4366");
  interColumnarNeurons.push_back("Pm2-like 19223");
  interColumnarNeurons.push_back("Pm2-like 35779");
  interColumnarNeurons.push_back("Pm2-like 105822");
  interColumnarNeurons.push_back("Pm2-like 172362");
  interColumnarNeurons.push_back("Pm2-like 172393");
  interColumnarNeurons.push_back("Pm2-like 254342");
  interColumnarNeurons.push_back("Pm2-like 310051");

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (ZString(iter->getName()).startsWith("T4-") && iter->getName() != "T4-11") {
      interColumnarNeurons.push_back(iter->getName());
    }
  }

  std::ofstream stream((dataPath + "/flyem/TEM/movie/script5.json").c_str());

  std::ofstream neuronOrderStream(
        (dataPath + "/flyem/TEM/movie/script5_neuron_order.txt").c_str());

  vector<string> tanNeurons;

  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString className = iter->getClass();
    if (className.startsWith("Mt")) {
      tanNeurons.push_back(iter->getName());
    }
  }

  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString className = iter->getClass();
    if (className.startsWith("Tangential")) {
      tanNeurons.push_back(iter->getName());
    }
  }

  set<string> priorNeuronSet;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }
  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    priorNeuronSet.insert(*iter);
  }

  //Check name consistency
  for (set<string>::const_iterator iter = priorNeuronSet.begin();
       iter != priorNeuronSet.end(); ++iter) {
    if (!bundle.hasNeuronName(*iter)) {
      cout << *iter << ": unknown name" << endl;
      cout << bundle.getIdFromName(*iter) << endl;
    }
  }


  vector<string> otherNeurons;
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (priorNeuronSet.count(iter->getName()) == 0) {
      otherNeurons.push_back(iter->getName());
    }
    if (!fexist(iter->getModelPath().c_str())) {
      cout << "No model: " << iter->getName() << endl;
    }
  }

  vector<vector<string> > allNeuronList;
  allNeuronList.push_back(laminaInputs);
  allNeuronList.push_back(firstOrderNeurons);
  allNeuronList.push_back(secondOrderNeurons);
  allNeuronList.push_back(interColumnarNeurons);
  allNeuronList.push_back(tanNeurons);
  allNeuronList.push_back(otherNeurons);

  writer.clear();

  double rotateSpeed = 0.02;
  double clipSpeed = 0.17;

  //Write cast
  stream << "{" << endl;

  writer.addActor("gray_stack",
                  dataPath + "/flyem/TEM/gray_ds_avg.tif");
  //const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    writer.addActor(iter->getName(), iter->getModelPath());
  }

  writer.writeCast(stream, 1);
  stream << "," << endl;

  //Write stack show up
  writer.clear();
  writer.addActor("gray_stack", "");
  stream << "  \"plot\": [" << endl;
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeShowAction(stream, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"reset\": {" << endl;
  stream << "          \"center\": [621, 590, 617]," << endl;
  stream << "          \"eye\": [2603, -1871, -963]," << endl;
  stream << "          \"up_vector\": [0.267, -0.42, -0.87]" << endl;
  stream << "        }," << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;


  //Lamina inputs

  stream << "    {" << endl;
  stream << "      \"duration\": 6500," << endl;
  stream << "      \"action\": [" << endl;

  writer.clear();


  size_t colorIndex = 0;
  neuronOrderStream << "Lamina inputs" << " (" << laminaInputs.size() << ") " << endl;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != laminaInputs.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }
  stream << endl;
  stream << "      ]," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }," << endl;

  stream << "      \"clipper\": [" << endl;
  stream << "        {" << endl;
  stream << "          \"target\": \"volume\"," << endl;
  stream << "          \"axis\": \"z\"," << endl;
  stream << "          \"upper\": 0," << endl;
  stream << "          \"lower\": " << clipSpeed << endl;
  stream << "        }" << endl;
  stream << "      ]" << endl;
  stream << "    }," << endl;

  //First order neurons
  neuronOrderStream << "First order neurons" << " (" << firstOrderNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }," << endl;
    stream << "      \"clipper\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"target\": \"volume\"," << endl;
    stream << "          \"axis\": \"z\"," << endl;
    stream << "          \"upper\": 0," << endl;
    stream << "          \"lower\": " << clipSpeed << endl;
    stream << "        }" << endl;
    stream << "      ]" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Hide gray stack
  stream << "    {" << endl;
  stream << "      \"duration\": 40," << endl;
  stream << "      \"action\": [" << endl;
  stream << "        {" << endl;
  stream << "          \"id\":" << "\"gray_stack\"," <<endl;
  stream << "          \"visible\": false" << endl;
  stream << "        }" << endl;
  stream << "      ]" <<endl;
  stream << "    }," << endl;


  //Turn L inputs and first order neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;

  vector<string> actions;
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 1.0]");
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add second order neurons
  writer.clear();
  neuronOrderStream << "Second order neurons" << " (" << secondOrderNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn second order neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;


  //Add inter-columnar neurons
  writer.clear();
  neuronOrderStream << "Inter-columnar neurons" << " (" << interColumnarNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " <<*iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn inter-columnar neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add tangential neurons
  writer.clear();
  neuronOrderStream << "Tangential neurons" << " (" << tanNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }

  //Turn tangential neurons into white
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }," << endl;

  //Add other neurons
  writer.clear();
  neuronOrderStream << "Other neurons" << " (" << otherNeurons.size() << ") " << endl;
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    if (iter != otherNeurons.begin()) {
      stream << "," << endl;
    }

    stream << "    {" << endl;
    stream << "      \"duration\": 40," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": " << rotateSpeed << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }";

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));

    neuronOrderStream << "  " << *iter << ": " << bundle.getIdFromName(*iter) << endl;
  }
  stream << "," << endl;

  //Recolor all neurons
  stream << "    {" << endl;
  stream << "      \"duration\": 18000," << endl;
  stream << "      \"action\": [" << endl;
  colorIndex = 0;
  for (vector<string>::const_iterator iter = laminaInputs.begin();
       iter != laminaInputs.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != laminaInputs.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = firstOrderNeurons.begin();
       iter != firstOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != firstOrderNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = secondOrderNeurons.begin();
       iter != secondOrderNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != secondOrderNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = interColumnarNeurons.begin();
       iter != interColumnarNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != interColumnarNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  stream << "," << endl;

  for (vector<string>::const_iterator iter = tanNeurons.begin();
       iter != tanNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != tanNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  /*
  stream << "," << endl;

  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];
    if (iter != otherNeurons.begin()) {
      stream << "," << endl;
    }
    stream << "        {" << endl;
    stream << "          \"id\": " << "\"" << *iter << "\"," << endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }";
  }
  */

  stream << endl;

  stream << "      ]," << endl;
  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": " << rotateSpeed << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }" << endl;

  stream << "  ]" << endl;
  stream << "}" << endl;


  /*
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  */
  //writer.writeShowAction(stream, 3);


#if 0
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": 0.1" << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));
  }

  size_t colorIndex = 0;
  for (vector<string>::const_iterator iter = otherNeurons.begin();
       iter != otherNeurons.end(); ++iter) {
    if (colorIndex >= colorArray.size()) {
      colorIndex = 0;
    }
    ZPoint color = colorArray[colorIndex++];

    stream << "    {" << endl;
    stream << "      \"duration\": 80," << endl;
    stream << "      \"action\": [" << endl;
    stream << "        {" << endl;
    stream << "          \"id\":" << "\"" << *iter << "\"," <<endl;
    stream << "          \"visible\": true," << endl;
    stream << "          \"color\": " << color.toJsonString() << endl;
    stream << "        }" << endl;
    stream << "      ]," <<endl;
    stream << "      \"camera\": {" << endl;
    stream << "        \"rotate\": {" << endl;
    stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
    stream << "          \"angle\": 0.1" << endl;
    stream << "        }" << endl;
    stream << "      }" << endl;
    stream << "    }," << endl;

    writer.addActor(*iter, bundle.getModelPath(bundle.getIdFromName(*iter)));
  }

  /*
  const vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    //if (iter->getClass() == "Mi9") {
      writer.addActor(iter->getName(), iter->getModelPath());
    //}
  }

  writer.writeCast(stream, 1);
  stream << endl;
*/
  stream << "    {" << endl;
  stream << "      \"duration\": 80," << endl;
  vector<string> actions;
  actions.push_back("visible");
  actions.push_back("true");
  actions.push_back("color");
  actions.push_back("[1.0, 1.0, 1.0]");
  /*
  actions.push_back("alpha");
  actions.push_back("0.001");
  actions.push_back("fade");
  actions.push_back("0.001");
  */
  //writer.writeShowAction(stream, 3);
  writer.writeAction(stream, actions, 3);
  stream << "," << endl;

  stream << "      \"camera\": {" << endl;
  stream << "        \"rotate\": {" << endl;
  stream << "          \"axis\": [0.0, 1.0, 0.5]," << endl;
  stream << "          \"angle\": 0.1" << endl;
  stream << "        }" << endl;
  stream << "      }" << endl;
  stream << "    }" << endl;
#endif


  stream.close();
  neuronOrderStream.close();

  /*
  Z3DCamera camera;
  camera.setCenter(glm::vec3(577, 574, 1200));
  camera.setEye(glm::vec3(1000, 1000, 2400));

  writer.writeCast(cout, 1);

  writer.writePlotBegin(cout, 1);

  writer.writeSceneBegin(cout, 1);
  writer.writeDuration(1000, cout, 1);
  writer.writeActionBegin(cout, 1);
  writer.writeAction("grey_stack", "show");
  writer.writeCamera("reset", )
  writer.writeActionEnd(cout, 1);
  writer.writeSceneEnd(cout, 1);

  writer.writePlotEnd(cout, 1);
  */
#endif

#if 0
  ZMovieScript script;

#if 0
  script.addActor(1, dataPath + "/tmp/swc3/adjusted/C2_214.swc");
  ZMovieScene scene;
  scene.setDuration(1.0);
  MovieAction action;
  action.actorId = 1;
  action.isVisible = true;
  scene.addAction(action);
  script.addScene(scene);
#endif

  if (script.loadScript(dataPath + "/test/L1_pathway.json")) {
    script.printSummary();
    script.print();

    ZMovieMaker director;
    director.setScript(script);
    director.setFrameInterval(40);
    director.make(dataPath + "/test/movie");
  } else {
    cout << "Failed to load " << dataPath + "/test/L1_pathway.json" << endl;
  }
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle.json");
  bundle.print();
#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.printJson(&cout, 2);
#endif

#if 0
  cout << setfill('x');
  cout << setw(10) << "" << endl;
#endif

#if 0
  ZSwcTree tree;
  tree.load((dataPath + "/test.swc").c_str());
  ZSwcTree *tree2 = tree.createBoundBoxSwc(100.0);
  tree2->save((dataPath + "/test2.swc").c_str());

#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted3/T4-1_277709.swc").c_str());
  tree2.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted3/T4-10_476680.swc").c_str());

  tree1.resample(200);
  tree2.resample(200);

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcLayerFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 1);


  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree2.load((dataPath + "/benchmark/swc/compare/compare2.swc").c_str());
  tree2.translate(0, 0, 50.0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcSizeFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 1);

  ZSwcTree *tree = matcher.exportResultAsSwc();
  tree->merge(tree1.data());
  tree->merge(tree2.data());
  tree->resortId();
  tree->save((dataPath + "/test.swc").c_str());
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/benchmark/swc/compare/compare1.swc").c_str());
  tree2.load((dataPath + "/benchmark/swc/compare/compare2.swc").c_str());
  tree2.translate(0, 0, 50.0);

  tree1.resample(20.0);
  tree2.resample(20.0);

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcDistTrunkAnalyzer trunkAnalyzer;
  ZSwcSizeFeatureAnalyzer featureAnalyzer;

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  matcher.matchAllG(tree1, tree2, 3);

  tree1.setTypeByLabel();
  tree1.resortId();
  tree1.save((dataPath + "/test.swc").c_str());

  tree2.setTypeByLabel();
  tree2.resortId();
  tree2.save((dataPath + "/test2.swc").c_str());

  ZSwcTree *result = matcher.exportResultAsSwc();
  result->save((dataPath + "/test3.swc").c_str());

  delete result;

  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  string modelPath = dataPath +
      "/flyem/skeletonization/session3/len15/adjusted3";

  ZFileList modelList;
  modelList.load(modelPath, "swc");

  FILE *fp = fopen((dataPath + "/flyem/TEM/neuron.csv").c_str(), "r");
  ZString str;

  ofstream stream((dataPath + "/flyem/TEM/neuron.json").c_str());

  while (str.readLine(fp)) {
    ZFlyEmNeuron neuron;
    std::vector<string> tokenArray = str.tokenize(',');
    if (tokenArray.size() == 6) {
      if (tokenArray[0] != "Old Name") {
        neuron.setId(tokenArray[1]);
        if (neuron.getId() > 0) {
          neuron.setName(tokenArray[3]);
          neuron.setClass(tokenArray[2]);
          for (int i = 0; i < modelList.size(); ++i) {
            ZString filePath(modelList.getFilePath(i));
            ostringstream stream;
            stream << "_" << neuron.getId() << ".swc";

            if (filePath.endsWith(stream.str())) {
              neuron.setModelPath(filePath.c_str());
              break;
            }
          }
          neuron.printJson(&stream, 4);
        }
      }
    }
  }
  fclose(fp);

  stream.close();
#endif

#if 0
  ZSwcTree tree;
  cout << tree.className() << endl;

  ZPoint pt;
  cout << pt.className() << endl;
#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 1024, 1024, 1);
  Zero_Stack(stack);
  C_Stack::drawInteger(stack, 123456780, 100, 100, 0);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  tr1::shared_ptr<ZStackDoc> academy = tr1::shared_ptr<ZStackDoc>(new ZStackDoc);
  academy->loadFile((dataPath + "/flyem/TEM/gray_ds10_avg/xy-grayscale-01267.tif").c_str());
  Z3DWindow *stage = new Z3DWindow(academy, false, NULL);

  stage->show();

  ZStack *stack = new ZStack;
  stack->load("/Users/zhaot/Work/neutube/neurolabi/data/flyem/TEM/movie/actor/colored_slice.tif");
  academy->loadStack(stack);
#endif

#if 0
  map<int, ZObject3dScan*> bodySet;
  Stack *stack = Read_Stack_U((dataPath + "/benchmark/rice_label.tif").c_str());
  uint8_t *array = stack->array;
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int y = 0;
  int z = 0;

  for (z = 0; z < depth; ++z) {
    for (y = 0; y < height; ++y) {
      int x = 0;
      while (x < width) {
        int v = array[x];
        if (bodySet.count(v) == 0) {
          bodySet[v] = new ZObject3dScan;
        }
        ZObject3dScan *obj = bodySet[v];

        int length = obj->scanArray(array, x, y, z, width);

        x += length;
        //cout << length << " " << x << endl;
      }
      array += width;
    }
  }

  C_Stack::setZero(stack);

  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    //iter->second->print();
    iter->second->drawStack(stack, iter->first);
  }

  C_Stack::write(dataPath + "/test.tif", stack);

  C_Stack::kill(stack);
#endif

#if 0
  ZObject3dScan obj;
  /*
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  One_Stack(stack);
  stack->array[0] = 0;
  stack->array[4] = 0;
  stack->array[6] = 0;
  stack->array[7] = 0;
  stack->array[8] = 0;
  */

  //Stack *stack = Read_Stack_U((dataPath + "/benchmark/binary/2d/disk_n10.tif").c_str());
  Stack *stack = Read_Stack_U(
        (dataPath + "/flyem/skeletonization/session3/C2c_3668.tif").c_str());
  obj.loadStack(stack);

  C_Stack::kill(stack);

  obj.save(dataPath + "/test.obj");

  obj.load(dataPath + "/test.obj");

  obj.print();

  ZObject3d *obj2 = obj.toObject3d();
  Stack *recovered = obj2->toStack(NULL);
  C_Stack::write(dataPath + "/test.tif", recovered);
  delete obj2;
  //Stack *stack = Read_Stack(dataPath + "");
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(1);
  stripe.addSegment(3, 4);
  stripe.addSegment(3, 9);
  stripe.addSegment(5, 3);
//  stripe.sort();
//  stripe.print();
//  stripe.canonize();
//  stripe.print();

  ZObject3dScan obj;
  obj.addStripe(stripe);
  obj.addStripe(stripe);
  obj.print();

  obj.downsample(1, 0, 0);
  cout << "After downsample: " << endl;
  obj.print();

  obj.canonize();
  obj.print();
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(3, 9);

  ZObject3dScan obj;
  obj.addStripe(stripe);

  stripe.clearSegment();
  stripe.setY(2);
  stripe.setZ(1);
  stripe.addSegment(4, 5);
  stripe.addSegment(8, 9);

  obj.addStripe(stripe);

  obj.downsampleMax(1, 1, 1);
  obj.print();

  Stack *stack = obj.toStack();

  C_Stack::print(stack);
#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath + "/flyem/FIB/21784.sobj");
  obj.downsampleMax(4, 4, 4);
  int offset[3];
  Stack *stack = obj.toStack(offset);
  offset[2] -= 298;
  cout << offset[0] << " " << offset[1] << " " << offset[2] << endl;
  C_Stack::write(dataPath + "/test.tif", stack);

  ofstream stream((dataPath + "/flyem/FIB/21784.json").c_str());

  stream << "{" << endl;
  stream << "  \"transform\": {" << endl;
  stream << "    \"scale\": [1.6129, 1.6129, 1.2500]," << endl;
  stream << "    \"translate\": [" << offset[0] << ", " << offset[1] << ", "
         << offset[2] << "]," << endl;
  stream << "    \"scale_first\": false" << endl;
  stream << "  }" << endl;
  stream << "}";
  stream.close();
#endif

#if 0
  IMatrix *mat = IMatrix_Read((dataPath + "/test/session2/body_map/body_map00161.imat").c_str());
  int width = mat->dim[0];
  int height = mat->dim[1];
  int depth = mat->dim[2];
  map<int, ZObject3dScan*> bodySet;
  ZObject3dScan::extractAllObject(mat->array, width, height, depth, 0, &bodySet);
  Kill_IMatrix(mat);

  mat = IMatrix_Read((dataPath + "/test/session2/body_map/body_map00162.imat").c_str());
  ZObject3dScan::extractAllObject(mat->array, width, height, depth, 1, &bodySet);
  Kill_IMatrix(mat);

  Stack *stack = C_Stack::make(GREY16, width, height, 2);
  C_Stack::setZero(stack);

  int index = 1;
  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter, ++index) {
    iter->second->drawStack(stack, index);
    ZString filePath = dataPath + "/test/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  C_Stack::write(dataPath + "/test.tif", stack);
  C_Stack::kill(stack);
#endif

#if 0
  map<int, ZObject3dScan*> bodySet;
  for (int z = 161; z <= 1461; ++z) {
    //ZString filePath = dataPath + "/test/session2/body_map/body_map";
    ZString filePath = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/body_map/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      cout << "Loading " << filePath << endl;
      IMatrix *mat = IMatrix_Read(filePath.c_str());
      int width = mat->dim[0];
      int height = mat->dim[1];
      int depth = mat->dim[2];

      tic();
      ZObject3dScan::extractAllObject(mat->array, width, height, depth, z, &bodySet);
      ptoc();
      Kill_IMatrix(mat);
    }
  }

  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    ZString filePath = dataPath + "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    delete iter->second;
    iter->second = NULL;
  }
  bodySet.clear();

#endif

#if 0
  ZString bodyDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies";
  for (int z = 161; z <= 1461; ++z) {
    map<int, ZObject3dScan*> bodySet;
    //ZString filePath = dataPath + "/test/session2/body_map/body_map";
    ZString filePath = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/body_map/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      cout << "Loading " << filePath << endl;
      IMatrix *mat = IMatrix_Read(filePath.c_str());
      int width = mat->dim[0];
      int height = mat->dim[1];
      int depth = mat->dim[2];

      tic();
      ZObject3dScan::extractAllObject(mat->array, width, height, depth, z, &bodySet);
      ptoc();
      Kill_IMatrix(mat);

      ZString objDir;
      objDir.appendNumber(z, 5);
      QDir dir(bodyDir.c_str());
      dir.mkdir(objDir.c_str());

      for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
           iter != bodySet.end(); ++iter) {
        ZString filePath = bodyDir + "/" + objDir + "/";
        filePath.appendNumber(iter->first, 0);
        iter->second->save(filePath + ".sobj");
      }

      for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
           iter != bodySet.end(); ++iter) {
        delete iter->second;
        iter->second = NULL;
      }
      bodySet.clear();
    }
  }

  /*
  for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    ZString filePath = dataPath + "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies/";
    filePath.appendNumber(iter->first, 0);
    iter->second->save(filePath + ".sobj");
  }

  for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    delete iter->second;
    iter->second = NULL;
  }
  bodySet.clear();
*/
#endif

#if 0
  int bodyId = 209;
  ZString bodyDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/session2/bodies";
  ZObject3dScan obj;
  for (int z = 161; z < 200; ++z) {
    ZString objDir;
    objDir.appendNumber(z, 5);
    ZString objPath = bodyDir + "/" + objDir + "/";
    objPath.appendNumber(bodyId, 0);
    objPath += ".sobj";
    ZObject3dScan objSlice;
    objSlice.load(objPath);
    obj.concat(objSlice);
  }

  obj.save(dataPath + "/test.sobj");

#endif

#if 0
  const int zStart = 1490;
  const int zEnd = 7509;
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session3";
  ZString bodyDir = sessionDir + "/bodies";

  if (!dexist(bodyDir.c_str())) {
    mkdir(bodyDir.c_str(), 0755);
  }

  set<int> bodyIdSet;

  for (int z = zStart; z <= zEnd; ++z) {
    map<int, ZObject3dScan*> bodySet;
    ZString filePath = sessionDir + "/body_maps/body_map";
    filePath.appendNumber(z, 5);
    filePath = filePath + ".imat";

    if (fexist(filePath.c_str())) {
      ZString objDir = bodyDir + "/";
      objDir.appendNumber(z, 5);

      if (!dexist(objDir.c_str())) {
        mkdir(objDir.c_str(), 0755);

        cout << "Loading " << filePath << endl;
        IMatrix *mat = IMatrix_Read(filePath.c_str());
        int width = mat->dim[0];
        int height = mat->dim[1];
        int depth = mat->dim[2];

        ZObject3dScan::extractAllObject(mat->array, width, height, depth,
                                        z - zStart, &bodySet);
        Kill_IMatrix(mat);

        for (map<int, ZObject3dScan*>::const_iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          ZString filePath = objDir + "/";
          filePath.appendNumber(iter->first, 0);
          iter->second->save(filePath + ".sobj");
        }

        for (map<int, ZObject3dScan*>::iterator iter = bodySet.begin();
             iter != bodySet.end(); ++iter) {
          delete iter->second;
          iter->second = NULL;
        }
        bodySet.clear();
      }

      ZFileList fileList;
      fileList.load(objDir, "sobj");
      for (int i = 0; i < fileList.size(); ++i) {
        //cout << fileList.getFilePath(i) << endl;
        int id = String_Last_Integer(fileList.getFilePath(i));
        bodyIdSet.insert(bodyIdSet.end(), id);
      }
    }
  }

  ZString fullBodyDir = bodyDir + "/stacked";
  if (!dexist(fullBodyDir.c_str())) {
    mkdir(fullBodyDir.c_str(), 0755);
  }

  vector<std::pair<size_t, int> > objSizeArray;
  for (set<int>::const_iterator iter = bodyIdSet.begin(); iter != bodyIdSet.end();
       ++iter) {
    //Load the object
    int bodyId = *iter;
    ZObject3dScan obj;
    for (int z = zStart; z < zEnd; ++z) {
      ZString objPath = bodyDir + "/";
      objPath.appendNumber(z, 5);
      objPath += "/";
      objPath.appendNumber(bodyId);
      objPath += ".sobj";
      ZObject3dScan objSlice;
      if (objSlice.load(objPath)) {
        obj.concat(objSlice);
      }
    }

    std::pair<size_t, int> objSize;
    objSize.first = obj.getVoxelNumber();
    objSize.second = bodyId;

    if (objSize.first > 10000000) {
      objSizeArray.push_back(objSize);

      ZString stackedObjPath = fullBodyDir + "/";
      stackedObjPath.appendNumber(bodyId);
      stackedObjPath += ".sobj";
      obj.save(stackedObjPath);
    }
  }

  sort(objSizeArray.begin(), objSizeArray.end());
  ofstream stream((bodyDir +"/bodylist.txt").c_str());

  for (vector<std::pair<size_t, int> >::const_reverse_iterator
       iter = objSizeArray.rbegin(); iter != objSizeArray.rend(); ++iter) {
    stream << iter->second << ", " << iter->first << endl;
  }

  stream.close();
#endif

#if 0
  //Load all FIB objects
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session1";
  ZString bodyDir = sessionDir + "/bodies";

  set<int> bodyIdSet;

  for (int z = 1490; z <= 4509; ++z) {
    ZFileList fileList;
    ZString objDir = bodyDir + "/";
    objDir.appendNumber(z, 5);

    fileList.load(objDir, "sobj");

    for (int i = 0; i < fileList.size(); ++i) {
      cout << fileList.getFilePath(i) << endl;
      int id = String_Last_Integer(fileList.getFilePath(i));
      bodyIdSet.insert(bodyIdSet.end(), id);
    }
  }

  for (set<int>::const_iterator iter = bodyIdSet.begin(); iter != bodyIdSet.end();
       ++iter) {
    //Load the object
    int bodyId = *iter;
    ZObject3dScan obj;
    for (int z = 1490; z < 4509; ++z) {
      ZString objDir;
      objDir.appendNumber(z, 5);
      ZString objPath = bodyDir + "/" + objDir + "/";
      objPath.appendNumber(bodyId, 0);
      objPath += ".sobj";
      ZObject3dScan objSlice;
      if (objSlice.load(objPath)) {
        obj.concat(objSlice);
      }
    }
    cout << *iter << ": " << obj.getVoxelNumber() << endl;

    ZString stackedObjPath = bodyDir + "/stacked/";
    stackedObjPath.appendNumber(bodyId, 0);
    stackedObjPath += ".sobj";
    obj.save(stackedObjPath);
  }
#endif

#if 0
  //Index all object
  ZString sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session1";
  ZString bodyDir = sessionDir + "/bodies";
  ZFileList fileList;
  ZString objDir = bodyDir + "/stacked";
  fileList.load(objDir, "sobj");

  int fileNumber = fileList.size();
  vector<std::pair<size_t, int> > objSize(fileNumber);
  for (int i = 0; i < fileNumber; ++i) {
    ZObject3dScan obj;
    obj.load(fileList.getFilePath(i));
    int id = String_Last_Integer(fileList.getFilePath(i));
    objSize[i].first = obj.getVoxelNumber();
    objSize[i].second = id;
    if (objSize[i].first > 100000) {
      cout << i << "/" << fileList.size() << " " << id << ": "
           << objSize[i].first << endl;
    }
  }

  sort(objSize.begin(), objSize.end());
  ofstream stream((bodyDir +"/objlist.txt").c_str());

  for (vector<std::pair<size_t, int> >::const_reverse_iterator iter = objSize.rbegin();
       iter != objSize.rend(); ++iter) {
    stream << iter->second << ", " << iter->first << endl;
  }

  stream.close();
#endif

#if 0
  ZSwcLayerTrunkAnalyzer trunkAnalyzer;
  trunkAnalyzer.setStep(1);

  ZSwcTree tree;
  //tree.load((dataPath + "/benchmark/swc/layer_test.swc").c_str());
  tree.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/L1_209.swc").c_str());
  ZSwcPath path = trunkAnalyzer.extractMainTrunk(&tree);

  path.setType(0);

  tree.save((dataPath + "/test.swc").c_str());
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/T4-10_476680.swc").c_str());
  tree2.load((dataPath + "/flyem/skeletonization/session3/len15/adjusted2/T4-11_588435.swc").c_str());

  tree1.setType(0);
  tree2.setType(0);

  ZSwcTreeMatcher matcher;
  ZSwcLayerTrunkAnalyzer trunkAnalyzer;
  ZSwcLayerShollFeatureAnalyzer featureAnalyzer;
  featureAnalyzer.setLayerScale(4000);

  matcher.setFeatureAnalyzer(&featureAnalyzer);
  matcher.setTrunkAnalyzer(&trunkAnalyzer);

  tic();
  matcher.matchAllG(tree1, tree2, 3);
  ptoc();

  tree1.setTypeByLabel();
  tree1.resortId();
  tree1.save((dataPath + "/test.swc").c_str());

  tree2.setTypeByLabel();
  tree2.resortId();
  tree2.save((dataPath + "/test2.swc").c_str());

  ZSwcTree *result = matcher.exportResultAsSwc(ZSwcTreeMatcher::EXPORT_ALL_MATCHING);
  result->save((dataPath + "/test3.swc").c_str());

  delete result;

  cout << "Score:  " << matcher.matchingScore() << endl;
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);

  graph.print();
  graph.exportDotFile(dataPath + "/test.dot");

  int v = 1;
  int neighborNumber = graph.getNeighborNumber(v);
  cout << "Neighbors of " << v << ": ";
  for (int i = 0; i < neighborNumber; ++i) {
    cout << graph.getNeighbor(v, i) << " ";
  }
  cout << endl;

  v = 3;
  neighborNumber = graph.getNeighborNumber(v);
  cout << "Neighbors of " << v << ": ";
  for (int i = 0; i < neighborNumber; ++i) {
    cout << graph.getNeighbor(v, i) << " ";
  }
  cout << endl;

  std::vector<int> path = graph.getPath(1, 7);

  cout << "path: " << endl;
  if (!path.empty()) {
    ZDebugPrintArrayG(path, 0, path.size() - 1);
  }
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");

  std::vector<int> neuronArray;
  ZString str;
  FILE *fp = fopen((dataPath + "/flyem/TEM/neuron_id.txt").c_str(), "r");
  while (str.readLine(fp)) {
    std::vector<int> intArray = str.toIntegerArray();
    if (!intArray.empty()) {
      neuronArray.insert(neuronArray.end(), intArray.begin(), intArray.end());
    }
  }
  fclose(fp);

  std::set<int> neuronSet;

  cout << neuronArray.size() << endl;

  for (size_t i = 0; i < neuronArray.size(); ++i) {
    int bodyId = neuronArray[i];
    if (neuronSet.count(bodyId) > 0) {
      cout << "duplicated id: " << bodyId << endl;
    }
    neuronSet.insert(bodyId);
    if (bundle.getNeuron(bodyId) == NULL) {
      cout << "No neuron: " << bodyId << endl;
    }
  }

  cout << neuronSet.size() << endl;

  const vector<ZFlyEmNeuron>& allNeuronInDatabase = bundle.getNeuronArray();

  for (size_t i = 0; i < allNeuronInDatabase.size(); ++i) {
    if (neuronSet.count(allNeuronInDatabase[i].getId()) == 0) {
      cout << "Untraced neuron: " << allNeuronInDatabase[i].getId() << endl;
    }
  }
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);

  vector<int> vertexArray;
  vertexArray.push_back(1);
  vertexArray.push_back(4);
  vertexArray.push_back(6);
  graph.mergeVertex(vertexArray);

  graph.print();
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);
  graph.addEdge(0, 3);
  graph.addEdge(3, 4);
  graph.addEdge(4, 5);
  graph.addEdge(2, 6);
  graph.addEdge(4, 7);
  graph.addEdge(4, 8);
  graph.addEdge(5, 7);

  /*
  std::vector<int> path = graph.getPath(0, 0);
  graph.mergeVertex(path);
  graph.print();
  */

  vector<bool> labeled(graph.getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph.getPath(i, i);
      if (path.size() > 2) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  graph.exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(100, 200);
  graph.addEdge(100, 300);
  graph.addEdge(300, 400);
  graph.addEdge(300, 500);
  graph.addEdge(400, 500);

  graph.print();

  ZGraphCompressor compressor;
  compressor.setGraph(&graph);
  compressor.compress();
  graph.print();
  compressor.uncompress();

  graph.print();

#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 7);
  graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  graph.addEdge(5, 6);
  graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  graph.addEdge(3, 5);
  graph.addEdge(1, 3);
  graph.addEdge(1, 2);
  graph.addEdge(2, 3);

  vector<bool> labeled(graph.getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph.getPath(i, i);
      if (path.size() > 4) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  graph.exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZStackGraph stackGraph;
  Stack *stack = Read_Stack_U(
        (dataPath + "/MAX_loop_test.tif").c_str());
  //stack->depth = 299;
  ZGraph *graph = stackGraph.buildGraph(stack);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();
  graph->exportDotFile(dataPath + "/test.dot");
#endif

#if 0
  ZStackGraph stackGraph;
  //stackGraph.setStack(stack);

/*
  Stack *stack = Read_Stack_U(
        (dataPath + "/benchmark/binary/2d/btrig2_skel.tif").c_str());
        */
  /*
  Stack *stack = Read_Stack_U(
        (dataPath + "/digit8_skel.tif").c_str());
*/

  Stack *stack = Read_Stack_U(
        (dataPath + "/loop_test.tif").c_str());
  Stack_Threshold(stack, 100);

  ZGraph *graph = stackGraph.buildGraph(stack);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();
  //graph->exportDotFile(dataPath + "/test.dot");

  vector<bool> labeled(graph->getVertexNumber(), false);
  //For each loop in the graph, label it
  for (int i = 0; i < graph->getVertexNumber(); ++i) {
    if (!labeled[i]) {
      vector<int> path = graph->getPath(i, i);
      if (path.size() > 5) {
        for (vector<int>::const_iterator iter = path.begin(); iter != path.end();
             ++iter) {
          labeled[*iter] = true;
        }
      }
    }
  }

  for (size_t i = 0; i < labeled.size(); ++i) {
    if (labeled[i]) {
      stack->array[compressor.uncompress(i)] = 128;
    }
  }

  C_Stack::write(dataPath + "/test.tif", stack);
  //graph->exportDotFile(dataPath + "/test.dot", labeled);
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 7);
  graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  //graph.addEdge(5, 6);
  graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  graph.addEdge(3, 5);
  //graph.addEdge(1, 3);
  graph.addEdge(1, 2);
  //graph.addEdge(2, 3);

  graph.exportDotFile(dataPath + "/test.dot");

  std::vector<std::vector<int> > cycleArray = graph.getCycle();
  cout << cycleArray.size() << " cycles" << endl;

  for (size_t i = 0; i < cycleArray.size(); ++i) {
    for (size_t j = 0;  j < cycleArray[i].size(); ++j) {
      cout << cycleArray[i][j] << " ";
    }
    cout << endl;
  }
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITHOUT_WEIGHT);

  graph.addEdge(0, 1);
  graph.addEdge(0, 3);
  //graph.addEdge(0, 7);
  //graph.addEdge(1, 7);
  graph.addEdge(6, 7);
  graph.addEdge(5, 6);
  //graph.addEdge(4, 5);
  graph.addEdge(3, 4);
  //graph.addEdge(3, 5);
  graph.addEdge(1, 3);

  graph.exportDotFile(dataPath + "/test.dot");

  const vector<ZGraph*>& subgraphArray = graph.getConnectedSubgraph();
  for (size_t i = 0; i < subgraphArray.size(); ++i) {
    cout << "Subgraph " << i << ":" << endl;
    subgraphArray[i]->print();
    cout << endl;
  }

  std::vector<std::vector<int> > cycleArray = graph.getCycle();
  cout << cycleArray.size() << " cycles" << endl;

  for (size_t i = 0; i < cycleArray.size(); ++i) {
    for (size_t j = 0;  j < cycleArray[i].size(); ++j) {
      cout << cycleArray[i][j] << " ";
    }
    cout << endl;
  }
#endif

#if 0
  cout << ZString::removeFileExt("test/test.tif") << endl;
  cout << ZString::removeFileExt("testtest.tif") << endl;
  cout << ZString::removeFileExt("test/test./tif") << endl;
  cout << ZString::removeFileExt("test/.test/tif") << endl;
  cout << ZString::removeFileExt("testtest.tif/") << endl;
  cout << ZString::removeFileExt("test/test") << endl;
  cout << ZString::removeFileExt("testtest") << endl;
#endif

#if 0
  QDomDocument doc("MyML");
  QDomElement root = doc.createElement("MyML");
  doc.appendChild(root);

  QDomElement tag = doc.createElement("Greeting");
  root.appendChild(tag);

  QDomText t = doc.createTextNode("Hello World");
  tag.appendChild(t);

  QString xml = doc.toString();

  qDebug() << xml;
#endif


#if 0
  ZXmlDoc doc;
  doc.parseFile(NeutubeConfig::getInstance().getApplicatinDir() + "/config.xml");
  ZXmlNode root = doc.getRootElement();
  std::cout << root.name() << std::endl;
  std::cout << root.stringValue() << std::endl;
  std::cout << "Element names:" << std::endl;
  std::cout << root.empty() << std::endl;
  std::cout << root.firstChild().stringValue() << std::endl;
  root.printInfo(2);

  ZXmlNode tracingNode = root.queryNode("tracing");
  std::cout << "Status of <tracing>: "
            << tracingNode.getAttribute("status") << std::endl;
  ZXmlNode colorNode = root.queryNode("Color_Mode");
  std::cout << "<Color_Mode>: "
            << colorNode.stringValue() << std::endl;

  ZXmlNode node = root.queryNode("Visible");
  std::cout << "<Visible>: " << node.intValue() << std::endl;

#endif

#if 0
  QDomDocument doc("mydocument");
  QFile file((NeutubeConfig::getInstance().getApplicatinDir() + "/config.xml").c_str());
  if (!file.open(QIODevice::ReadOnly))
    return;
  if (!doc.setContent(&file)) {
    file.close();
    return;
  }
  file.close();

  // print out the element names of all elements that are direct children
  // of the outermost element.
  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();
  while(!n.isNull()) {
    QDomElement e = n.toElement(); // try to convert the node to an element.
    if(!e.isNull()) {
      std::cout << qPrintable(e.tagName()) << std::endl; // the node really is an element.
    } else {
      std::cout << "Non-element node" << std::endl;
    }
    n = n.nextSibling();
  }
#endif

#if 0
  if (host != NULL) {
    ZStackFrame *frame = host->currentStackFrame();
    if (frame != NULL) {
      QtConcurrent::run(frame->document().get(), &ZStackDoc::test);
      QtConcurrent::run(frame->document().get(), &ZStackDoc::test);
      QtConcurrent::run(frame->document().get(), &ZStackDoc::test);
    }

//    frame->document()->test();
    /*
    if (frame != NULL) {
      frame->view()->setSizeHintOption(NeuTube::SIZE_HINT_TAKING_SPACE);
      frame->resize(frame->sizeHint());
    }
    */
  }
#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 100, 100, 5);
  Zero_Stack(stack);

  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/swc/fork.swc").c_str());

  ZSwcPath path(tree.firstRegularRoot(),
                SwcTreeNode::firstChild(SwcTreeNode::firstChild(tree.firstRegularRoot())));

  path.labelStackAcrossZ(stack, 255);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif


#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/cross_45_10.tif");
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/cross_45_10_2.swc").c_str());
  tree.print();

  ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());

  path.resetPositionFromStack(stack);

  tree.print();

#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/cross_45_10.tif");
  ZSwcTree tree;
  tree.load((dataPath + "/benchmark/cross_45_10_2.swc").c_str());
  tree.print();

  ZSwcPositionAdjuster adjuster;
  ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.setSignal(stack, NeuTube::IMAGE_BACKGROUND_DARK);

  //path.resetPositionFromStack(stack);

  adjuster.adjustPosition(path);

  //path.resetPositionFromStack(stack);

  tree.print();
#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/benchmark/mouse_neuron_single/stack.tif");
  ZSwcTree tree;
  tree.load(dataPath + "/stack.Edit.swc");
  ZSwcPositionAdjuster adjuster;
  adjuster.setSignal(stack, NeuTube::IMAGE_BACKGROUND_DARK);
  //ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.adjustPosition(tree);
  tree.save((dataPath + "/test.swc").c_str());
  tree.print();
#endif

#if 0
  ZStack stack;
  stack.load(dataPath + "/biocytin/MC0509C3-2_small_small.tif");
  ZSwcTree tree;
  tree.load(dataPath + "/MC0509C3-2_small_small.Proj.Edit.swc");
  ZSwcPositionAdjuster adjuster;
  adjuster.setSignal(stack.c_stack(0), NeuTube::IMAGE_BACKGROUND_BRIGHT);
  //ZSwcPath path(tree.firstRegularRoot(), tree.firstLeaf());
  adjuster.adjustPosition(tree);
  tree.save((dataPath + "/test.swc").c_str());
  tree.print();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath + "/benchmark/432.sobj");
  std::cout << obj.getVoxelNumber() << std::endl;
  std::cout << obj.getSegmentNumber() << std::endl;
  std::cout << "Really canonized? " << obj.isCanonizedActually() << std::endl;

  obj.canonize();
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    std::cout << obj.getStripe(i).getZ() << " " << obj.getStripe(i).getY()
              << std::endl;
  }

  std::vector<ZObject3dScan> objArray = obj.getConnectedComponent();
  std::cout << objArray.size() << std::endl;

  /*
  int offset[3];
  Stack *stack = obj.toStack(offset);

  //iarray_neg(offset, 3);
  //obj.labelStack(stack, 2, offset);

  ZObject3dScan obj2;
  obj2.loadStack(stack);
  std::cout << obj2.getVoxelNumber() << std::endl;
  std::cout << obj2.getSegmentNumber() << std::endl;
  std::cout << "Really canonized? " << obj2.isCanonizedActually() << std::endl;

  obj2.translate(offset[0], offset[1], offset[2]);

  obj2.labelStack(stack, 2, offset);
  C_Stack::write(dataPath + "/test2.tif", stack);
  */
#endif

#if 0
  Stack *stack = C_Stack::readSc(dataPath + "/test.tif");
  //stack->depth = 5;
  Stack_Binarize(stack);
  C_Stack::translate(stack, GREY, 1);
  ZObject3dScan obj;
  obj.loadStack(stack);
  cout << obj.getVoxelNumber() << endl;

  obj.labelStack(stack, 2);
  C_Stack::write(dataPath + "/test2.tif", stack);
#endif

#if 0 //FIB connection
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/v1/annotations-synapse.json");
  synapseArray.printSummary();

  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session9/data_bundle.json");
  const std::vector<ZFlyEmNeuron> &neuronArray = dataBundle.getNeuronArray();
  std::set<int> bodySet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    std::cout << iter->getId() << std::endl;
    bodySet.insert(iter->getId());
  }

  ZGraph *graph = synapseArray.toGraph(bodySet);

  graph->exportDotFile(dataPath + "/test.dot");

  delete graph;
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked/29.sobj");
  //obj1.print();
  obj1.downsampleMax(4, 4, 4);

  //obj1.print();

  ZObject3dScan obj2;
  obj2.load(dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked/9942.sobj");
  obj2.downsampleMax(4, 4, 4);

  ZCuboid box1 = obj1.getBoundBox();
  ZCuboid box2 = obj2.getBoundBox();
  box1.print();
  box1.bind(box2);
  box1.print();

  Stack *stack = C_Stack::make(
        COLOR, box1.width() + 1, box1.height() + 1, box1.depth() + 1);
  obj1.translate(-box1.firstCorner().x(), -box1.firstCorner().y(),
                 -box1.firstCorner().z());
  obj2.translate(-box1.firstCorner().x(), -box1.firstCorner().y(),
                 -box1.firstCorner().z());

  obj1.drawStack(stack, 255, 0, 0, NULL);
  obj2.drawStack(stack, 0, 255, 0, NULL);

  C_Stack::write(dataPath + "/test.tif", stack);

  C_Stack::kill(stack);
#endif

#if 0
  ZStackFile stackFile;
  stackFile.import(dataPath + "/biocytin/MC0509C3-2_small_small.tif");
  ZStack *stack = stackFile.readStack();
  Stack *mask = C_Stack::make(GREY, stack->width(), stack->height(), 1);
  Zero_Stack(mask);

  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 10, 10, 0, 100, 100, 1);
  Cuboid_I_Label_Stack(&cuboid, 255, mask);

  C_Stack::write(dataPath + "/biocytin/MC0509C3-2_small_small.ROI.tif", mask);

  delete stack;
  C_Stack::kill(mask);
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load(dataPath + "/flyem/skeletonization/session3/len15/adjusted2/L2e_198.swc");
  tree1.rescale(31, 31, 40);
  tree2.load(dataPath + "/flyem/FIB/skeletonization/session9/swc/adjusted/26558.swc");

  ZSwcRangeAnalyzer rangeAnalyzer;
  rangeAnalyzer.setZStep(500);
  rangeAnalyzer.setZMargin(500);
  std::vector<Swc_Tree_Node*> nodeArray =
      rangeAnalyzer.getOutsideNode(tree1, tree2);

  tree2.setType(0);
  SwcTreeNode::setType(nodeArray.begin(), nodeArray.end(), 2);
  tree2.save(dataPath + "/test.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 2999;
  blockArray.intersect(boundBox);
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(0, 0, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  QString dirPath(
        (dataPath + "/flyem/FIB/skeletonization/session9/bodies/stacked").c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/skeletonization/session9/annotations-synapse.json");

  std::vector<int> count = synapseArray.countSynapse();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

#endif

#if 0

#endif


#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle6.json");

  const std::vector<ZFlyEmNeuron> &neuronArray =  bundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    ZSwcTree *tree = neuronArray[i].getModel();
    double ratio = ZSwcGlobalFeatureAnalyzer::computeLateralVerticalRatio(*tree);
    std::cout << neuronArray[i].getName() << " "
               << neuronArray[i].getClass() << " " << ratio << std::endl;
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(dataPath + "/flyem/FIB/skeletonization/session9/annotations-synapse.json");

  std::vector<int> count = synapseArray.countSynapse();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session13/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();


  FILE *fp = fopen((dataPath +
                   "/flyem/FIB/skeletonization/session13/bodies/bodysize.txt").c_str(),
                   "r");

  std::ofstream stream((dataPath +
                       "/flyem/FIB/skeletonization/session13/psd_count.txt").c_str());

  ZString line;
  while (line.readLine(fp)) {
    vector<int> bodySize = line.toIntegerArray();
    if (bodySize.size() == 2) {
      if (bodySize[0] < (int) count.size()) {
        stream << bodySize[0] << ' ' << bodySize[1] << ' ' << count[bodySize[0]]
            << std::endl;
      }
    }
  }

  fclose(fp);
  stream.close();
#endif

#if 0
  //Show data bundle summary
  std::map<std::string, int> classCountMap;

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath +
                      "/flyem/FIB/skeletonization/session13/len25/adjusted2/data_bundle_with_class.json");

  const std::vector<ZFlyEmNeuron> &neuronArray =  bundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    if (classCountMap.count(neuronArray[i].getClass()) == 0) {
      classCountMap[neuronArray[i].getClass()] = 1;
    } else {
      classCountMap[neuronArray[i].getClass()]++;
    }
  }

  std::cout << classCountMap.size() << " classes" << std::endl;
  for (std::map<std::string, int>::const_iterator iter = classCountMap.begin();
       iter != classCountMap.end(); ++iter) {
    std::cout << iter->first << " " << iter->second << std::endl;
  }
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
#endif

#if 0
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  //blockArray.print();

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked2";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  ofstream out((bodyDir + "/psd_orphan.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

  out.close();
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  //blockArray.print();

  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked2";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countTBar();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;

  ofstream out((bodyDir + "/tbar_orphan.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    int bodyId = fileInfo.baseName().toInt();
    if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isStitchedOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    }
  }
  ptoc();

  out.close();
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();

  boundBox.ce[2] = 4499;
  blockArray.intersect(boundBox);

  ZIntCuboidArray bottomFace = blockArray;
  boundBox.cb[2] = 4499;
  bottomFace.intersect(boundBox);

  std::cout << blockArray.getVolume() << std::endl;

  boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);
  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_6layer.swc");

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray);

  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session13";
  std::string bodyDir = sessionDir + "/bodies/stacked";
  QString dirPath(bodyDir.c_str());
  QDir dir(dirPath);
  QStringList filter;
  filter << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filter, QDir::NoFilter);

  /*
  std::cout << "Loading synapses ..." <<std::endl;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(sessionDir + "/annotations-synapse.json");

  std::vector<int> count = synapseArray.countPsd();

  int bodyCount = 0;
  for (size_t i = 0; i < count.size(); ++i) {
    if (count[i] > 0) {
      bodyCount++;
    }
  }

  std::cout << bodyCount << " bodies have synapses." << std::endl;
  */

  ofstream out((bodyDir + "/cutoff.txt").c_str());

  tic();
  std::cout << "Start checking ..." << std::endl;
  foreach (QFileInfo fileInfo, fileList) {
    //int bodyId = fileInfo.baseName().toInt();
    //if (count[bodyId] > 0) {
      ZObject3dScan obj;
      obj.load(fileInfo.absoluteFilePath().toStdString());
      if (analyzer.isExitingOrphanBody(obj)) {
        std::cout << fileInfo.baseName().toStdString() << std::endl;
        out << fileInfo.baseName().toStdString() << std::endl;
      }
    //}
  }
  ptoc();

  out.close();
#endif

#if 0
  //Calculate L1, L2 volume

  std::string outDir = dataPath + "/flyem/FIB/result1";

  const char *temL1[] = {
    "L1_209", "L1g6_439207", "L1n1_422955", "L1a_285743", "L1h7_984179",
    "L1o2_5425323", "L1b_211940", "L1i8_17802841", "L1p3_636965",
    "L1c_189938", "L1j9_984116", "L1q4_423153", "L1d_181639",
    "L1k10_5053998", "L1r5_423176", "L1e_196", "L1l11_433023",
    "L1f_285714", "L1m12_5134017"
  };

  std::ofstream outStream;

  outStream.open((outDir + "/TEM_L1_size.txt").c_str());

  for (size_t i = 0; i < sizeof(temL1) / sizeof(char*); ++i) {
    Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/" +
                                   temL1[i] + ".tif");
    Stack_Binarize(stack);
    double volume = Stack_Sum(stack) * 31 * 31 * 40;
    volume /= 0.7;
    outStream << temL1[i] << ", " << volume << std::endl;
    C_Stack::kill(stack);
  }

  outStream.close();

  const char *temL2[] = {
    "L2_212", "L2b_216003", "L2d_133765", "L2f_5858", "L2a_148554",
    "L2c_177365", "L2e_198"
  };

  outStream.open((outDir + "/TEM_L2_size.txt").c_str());

  for (size_t i = 0; i < sizeof(temL2) / sizeof(char*); ++i) {
    Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/" +
                                   temL2[i] + ".tif");
    Stack_Binarize(stack);
    double volume = Stack_Sum(stack) * 31 * 31 * 40;
    volume /= 0.7;
    outStream << temL2[i] << ", " << volume << std::endl;
    C_Stack::kill(stack);
  }

  outStream.close();


  const char *fibL1[] = {
    "206669", "195030", "203285", "216388", "231769", "222058", "198198"
  };

  outStream.open((outDir + "/FIB_L1_size.txt").c_str());
  for (size_t i = 0; i < sizeof(fibL1) / sizeof(char*); ++i) {
    ZObject3dScan obj;
    obj.load(dataPath +
             "/flyem/FIB/skeletonization/session13/bodies/stacked/" +
             fibL1[i] + ".sobj");
    double volume = obj.getVoxelNumber() * 1000;
    outStream << fibL1[i] << ", " << volume << std::endl;
  }
  outStream.close();

  const char *fibL2[] = {
    "11545", "56", "7762", "26363", "31349", "26062", "4241"
  };

  outStream.open((outDir + "/FIB_L2_size.txt").c_str());
  for (size_t i = 0; i < sizeof(fibL1) / sizeof(char*); ++i) {
    ZObject3dScan obj;
    obj.load(dataPath +
             "/flyem/FIB/skeletonization/session13/bodies/stacked/" +
             fibL2[i] + ".sobj");
    double volume = obj.getVoxelNumber() * 1000;
    outStream << fibL2[i] << ", " << volume << std::endl;
  }
  outStream.close();


  //std::cout << temVolume << ' ' << fibVolume << ' ' << temVolume / fibVolume << std::endl;

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session14/annotations-synapse.json");

  std::cout << "Psd number: " << synapseArray.getPsdNumber() << std::endl;
  std::cout << "TBar number: " << synapseArray.getTBarNumber() << std::endl;

#endif

#if 0
  ZObject3dScan obj;
  obj.load(dataPath +
           "/flyem/FIB/skeletonization/session13/bodies/stacked/206669.sobj");
  obj.downsampleMax(2, 2, 5);


  Stack *stack = C_Stack::readSc(dataPath + "/flyem/skeletonization/session3/L1_209.tif");
  //Stack_Binarize(stack);

  Stack *stack2 = C_Stack::crop(stack, 0, 0, 0, 800, 800, 495, NULL);
  int offset[3] = {0, 0, 30};
  obj.drawStack(stack2, 100, offset);

  C_Stack::write(dataPath + "/test.tif", stack2);

  C_Stack::kill(stack);
  C_Stack::kill(stack2);

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/TEM/annotations-synapse.json");

  std::vector<int> psdCount = synapseArray.countPsd();
  std::vector<int> tbarCount = synapseArray.countTBar();

  std::ofstream stream1;
  stream1.open((dataPath + "/flyem/FIB/result1/TEM_L1_Psd.txt").c_str());

  std::ofstream stream2;
  stream2.open((dataPath + "/flyem/FIB/result1/TEM_L1_TBar.txt").c_str());

  std::ofstream stream3;
  stream3.open((dataPath + "/flyem/FIB/result1/TEM_L2_Psd.txt").c_str());

  std::ofstream stream4;
  stream4.open((dataPath + "/flyem/FIB/result1/TEM_L2_TBar.txt").c_str());

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle6.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      stream1 << iter->getId() << " " << psdCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
      stream2 << iter->getId() << " " << tbarCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
    }

    if (iter->getClass() == "L2") {
      stream3 << iter->getId() << " " << psdCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
      stream4 << iter->getId() << " " << tbarCount[iter->getId()] //synapseArray.countPsd(iter->getId())
                << std::endl;
    }
  }

  stream1.close();
  stream2.close();

#endif

#if 0
  std::map<int, int>::value_type rawMap[] =
  {
    std::map<int, int>::value_type(1, 2),
    std::map<int, int>::value_type(3, 4),
    std::map<int, int>::value_type(5, 6)
  };

  std::map<int, int> testMap(rawMap, rawMap + 2);
  std::cout << testMap.size() << std::endl;
#endif

#if 0
  ZProgressReporter reporter;
  reporter.start();
  reporter.advance(0.1);

  reporter.startSubprogress(0.5);
  reporter.start();
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  reporter.end();
  reporter.endSubprogress(0.5);

  reporter.advance(0.1);

  reporter.end();


  reporter.start();
  reporter.advance(0.1);

  reporter.start(0.5);
  reporter.start();
  for (int i = 0; i < 10; ++i) {
    reporter.advance(0.1);
  }
  reporter.end();
  reporter.end(0.5);

  reporter.advance(0.1);

  reporter.end();
#endif

#if 0
  NeuTube::getMessageReporter()->report(
        "test", "error 1", neutube::EMessageType::MSG_ERROR);
  NeuTube::getMessageReporter()->report(
        "test", "warning 1", neutube::EMessageType::MSG_WARNING);
  NeuTube::getMessageReporter()->report(
        "test", "output 1", neutube::EMessageType::MSG_INFORMATION);
#endif

#if 0
  ZHistogram hist;
  hist.setInterval(2.0);
  hist.increment(0.5);
  hist.increment(1.5);
  hist.increment(1.0);
  hist.increment(2.0);
  hist.increment(3.0);
  hist.increment(-1.0);
  hist.increment(-0.99);
  hist.increment(-2.0);
  hist.print();

  std::cout << hist.getDensity(0.1) << std::endl;

#endif

#if 0

  ZObject3dScan obj;
  obj.addStripe(0, 0);
  obj.addSegment(1, 2);
  obj.addSegment(4, 5);
  ZHistogram hist = obj.getRadialHistogram(0);

  hist.print();
#endif

#if 0
  ZHistogram hist;
  //hist.increment(0.0);
  hist.increment(1.0);
  hist.increment(2.0);
  hist.increment(2.0);

  hist.print();

  Stack *stack = C_Stack::make(GREY, 5, 5, 1);
  Zero_Stack(stack);

  misc::paintRadialHistogram(hist, 2, 2, 0, stack);

  Print_Stack_Value(stack);
#endif

#if 0
  ZObject3dScan obj;
  obj.load("/groups/flyem/data/zhaot/bundle1/volume/215.sobj");

  obj.downsampleMax(3, 3, 0);

 // hist.print();
  int startZ = obj.getMinZ();
  int endZ = obj.getMaxZ();

  Stack *stack = C_Stack::make(GREY, 1000, 1000, endZ - startZ + 1);
  Zero_Stack(stack);

  for (int z = startZ; z <= endZ; ++z) {
    ZHistogram hist = obj.getRadialHistogram(z);
    misc::paintRadialHistogram(hist, 500, 500, z - startZ, stack);
  }

  stack = C_Stack::boundCrop(stack);

  C_Stack::write(dataPath + "/test.tif", stack);
#endif

#if 0
  //Load tif list

  //For each tif
    //Extract sparse object
  {
    ZObject3dScan obj;
    obj.scanArray(C_Stack::array8(input));

    int startZ = obj.getMinZ();
    int endZ = obj.getMaxZ();

    Stack *stack = C_Stack::make(GREY, 1000, 1000, endZ - startZ + 1);
    Zero_Stack(stack);

    for (int z = startZ; z <= endZ; ++z) {
      ZHistogram hist = obj.getRadialHistogram(z);
      misc::paintRadialHistogram(hist, 500, 500, z - startZ, stack);
    }

    Stack *cropped = C_Stack::boundCrop(stack);
    C_Stack::kill(stack);

    C_Stack::write(dataPath + "/test.tif", cropped);
    C_Stack::kill(cropped);
  }

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    ZObject3dScan obj;
    obj.load(neuronIter->getVolumePath());
    obj.downsampleMax(3, 3, 0);

    std::vector<ZHistogram> histArray;
    int startZ = obj.getMinZ();
    int endZ = obj.getMaxZ();
    for (int z = startZ; z <= endZ; ++z) {
      ZHistogram hist = obj.getRadialHistogram(z);
      histArray.push_back(hist);
    }

    Stack *stack = C_Stack::make(GREY, 1500, 1500, 1);
    Zero_Stack(stack);
    int cx = 750;
    misc::paintRadialHistogram2D(histArray, cx, startZ, stack);

    ZString baseName = ZString::getBaseName(neuronIter->getModelPath());
    baseName = baseName.changeExt("tif");

    std::ostringstream stream;

    stream << dataPath << "/test/hist_"
           << ZString::getBaseName(neuronIter->getModelPath()) << ".tif";
    C_Stack::write(stream.str(), stack);

    C_Stack::kill(stack);
  }

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::vector<double> voxelCount;

  std::string volumeDir = dataPath + "/flyem/skeletonization/session3";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    std::cout << neuronIter->getName() << std::endl;
    //if (ZString(neuronIter->getClass()).startsWith("L")) {

      if (ZString(neuronIter->getClass()).startsWith("Tm") ||
          ZString(neuronIter->getClass()).startsWith("Mi") ||
          ZString(neuronIter->getClass()).startsWith("C")) {

    //if (neuronIter->getClass() == "Tangential") {
      std::string volumePath = neuronIter->getVolumePath();
      if (!volumePath.empty()) {
        ZObject3dScan obj;
        obj.load(volumePath);
        std::vector<size_t> subVoxelCount = obj.getSlicewiseVoxelNumber();
        for (size_t z = 0; z < subVoxelCount.size(); ++z) {
          if (z >= voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += subVoxelCount[z];
        }
        /*
        int minZ = obj.getMinZ();
        int maxZ = obj.getMaxZ();
        for (int z = minZ; z <= maxZ; ++z) {
          if (z >= (int) voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += obj.getVoxelNumber(z);
        }
        */
      }
    }
  }

  std::ofstream stream;
  std::cout << volumeDir + "/tm_voxel_distr.txt" << std::endl;
  stream.open((volumeDir + "/tm_voxel_distr.txt").c_str());
  for (size_t i = 0; i < voxelCount.size(); ++i) {
    stream << i << " " << voxelCount[i] << std::endl;
  }
  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session18/len10/adjusted/data_bundle_with_class.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::vector<double> voxelCount;

  std::string volumeDir = dataPath + "/flyem/FIB/skeletonization/session18/bodies/stacked/";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    std::cout << neuronIter->getName() << std::endl;
/*
    if (!neuronIter->getClass().empty() &&
        neuronIter->getClass() != "Tangential" &&
        neuronIter->getClass() != "partial") {
        */
    //if (ZString(neuronIter->getClass()).startsWith("L")) {

    if (ZString(neuronIter->getClass()).startsWith("Tm") ||
        ZString(neuronIter->getClass()).startsWith("Mi") ||
        ZString(neuronIter->getClass()).startsWith("C")) {

      //std::string volumePath = neuronIter->getVolumePath();
      ZString baseName = ZString::removeFileExt(
            ZString::getBaseName(neuronIter->getModelPath()));
      std::string volumePath = ZString::fullPath(volumeDir, baseName, "sobj");

      if (!volumePath.empty()) {
        ZObject3dScan obj;
        obj.load(volumePath);
        //int minZ = obj.getMinZ();
        //int maxZ = obj.getMaxZ();
        std::vector<size_t> subVoxelCount = obj.getSlicewiseVoxelNumber();
        for (size_t z = 0; z < subVoxelCount.size(); ++z) {
          if (z >= voxelCount.size()) {
            voxelCount.resize(z + 1, 0.0);
          }
          voxelCount[z] += subVoxelCount[z];
        }
      }
    }
  }

  std::ofstream stream;
  stream.open((volumeDir + "/voxel_distr2.txt").c_str());
  for (size_t i = 0; i < voxelCount.size(); ++i) {
    stream << i << " " << voxelCount[i] << std::endl;
  }
  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::string volumeDir = dataPath + "/flyem/skeletonization/session3";
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    ZString baseName = ZString::removeFileExt(
          ZString::getBaseName(neuronIter->getModelPath()));
    std::string volumePath = ZString::fullPath(volumeDir, baseName, "tif");
    if (fexist(volumePath.c_str())) {
      Stack *stack = C_Stack::readSc(volumePath);
      ZObject3dScan obj;
      obj.loadStack(stack);
      C_Stack::kill(stack);

      std::string offsetPath =
          ZString::fullPath(volumeDir, baseName, "tif.offset.txt");
      ZString line;
      FILE *fp = fopen(offsetPath.c_str(), "r");
      line.readLine(fp);
      int zOffset = line.lastInteger();
      fclose(fp);

      obj.addZ(zOffset);

      std::vector<ZHistogram> histArray;
      int startZ = obj.getMinZ();
      int endZ = obj.getMaxZ();
      for (int z = startZ; z <= endZ; ++z) {
        ZHistogram hist = obj.getRadialHistogram(z);
        histArray.push_back(hist);
      }

      stack = C_Stack::make(GREY, 1500, 1500, 1);
      Zero_Stack(stack);
      int cx = 750;
      misc::paintRadialHistogram2D(histArray, cx, startZ, stack);

      ZString baseName = ZString::getBaseName(neuronIter->getModelPath());
      baseName = baseName.changeExt("tif");

      std::ostringstream stream;

      stream << dataPath << "/flyem/TEM/data_release/densitymap/hist_"
             << ZString::getBaseName(neuronIter->getModelPath()) << ".tif";
      C_Stack::write(stream.str(), stack);

      C_Stack::kill(stack);
    }
  }
#endif

#if 0 //Density map matching
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::string densityMapDir = dataPath + "/test";
  std::vector<Stack*> imageArray;
  for (std::vector<ZFlyEmNeuron>::const_iterator
       firstNeuronIter = neuronArray.begin();
       firstNeuronIter != neuronArray.end(); ++firstNeuronIter) {
    ZString baseName = ZString::removeFileExt(
          ZString::getBaseName(firstNeuronIter->getModelPath()));
    std::string imagePath1 = ZString::fullPath(densityMapDir, "hist_" + baseName, "swc.tif");
    Stack *stack1 = C_Stack::readSc(imagePath1);
    imageArray.push_back(stack1);
  }

  ZMatrix mat(neuronArray.size(), neuronArray.size());

  int count = 0;
  int i = 0;
  for (std::vector<ZFlyEmNeuron>::const_iterator
       firstNeuronIter = neuronArray.begin();
       firstNeuronIter != neuronArray.end(); ++firstNeuronIter, ++i) {
    Stack *stack1 = imageArray[i];
    Cuboid_I box1;
    Stack_Bound_Box(stack1, &box1);

    int j = 0;
    for (std::vector<ZFlyEmNeuron>::const_iterator
         secondNeuronIter = neuronArray.begin();
         secondNeuronIter != neuronArray.end(); ++secondNeuronIter, ++j) {
      if (i < j) {
        Stack *stack2 = imageArray[j];
        Cuboid_I box2;
        Stack_Bound_Box(stack2, &box2);

        Cuboid_I box;
        Cuboid_I_Union(&box1, &box2, &box);

        Stack *cropped1 = C_Stack::crop(stack1, box, NULL);
        Stack *cropped2 = C_Stack::crop(stack2, box, NULL);

        double corrcoef = Stack_Corrcoef(cropped1, cropped2);

        //std::cout << i << " " << j << " " << corrcoef << std::endl;

        mat.set(i, j, corrcoef);

        C_Stack::kill(cropped1);
        C_Stack::kill(cropped2);
      } else if (i > j) {
        mat.set(i, j, mat.getValue(j, i));
      }
    }

    int neuronIndex = 0;
    mat.getRowMax(i, &neuronIndex);
    if (firstNeuronIter->getClass() == neuronArray[neuronIndex].getClass()) {
      ++count;
    }

    std::cout << "Prediction: " << count << "/" << i + 1 << " - "
              << firstNeuronIter->getClass() << "(p: "
              << neuronArray[neuronIndex].getClass() << ")" << std::endl;
  }


  for (size_t i = 0; i < imageArray.size(); ++i) {
    C_Stack::kill(imageArray[i]);
  }

  mat.exportCsv(dataPath + "/test.csv");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(dataPath + "/flyem/TEM/data_release/bundle1/annotations-synapse.json");
  synapseAnnotation.exportTBar(dataPath + "/tbar.csv");
  synapseAnnotation.exportPsd(dataPath + "/psd.csv");
  synapseAnnotation.exportCsvFile(dataPath + "/synapse.csv");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;

  //synapseAnnotation.loadJson(dataPath + "/flyem/FIB/skeletonization/session13/annotations-synapse.json");
  synapseAnnotation.loadJson(dataPath + "/flyem/TEM/data_release/bundle1/annotations-synapse.json");
  ZGraph *graph = synapseAnnotation.getConnectionGraph(false);

  ZGraphCompressor compressor;
  compressor.setGraph(graph);
  compressor.compress();

  const std::vector<int> &uncompressMap = compressor.getUncompressMap();

  ofstream stream((dataPath + "/id.txt").c_str());

  for (size_t i = 0; i < uncompressMap.size(); ++i) {
    stream << uncompressMap[i] << std::endl;
  }

  stream.close();

  ZMatrix mat(graph->getVertexNumber(), graph->getVertexNumber());

  for (int i = 0; i < graph->getEdgeNumber(); ++i) {
    mat.set(graph->getEdgeBegin(i), graph->getEdgeEnd(i), graph->getEdgeWeight(i));
  }

  mat.exportCsv(dataPath + "/conn.csv");
#endif

#if 0
  Stack *stack2 = C_Stack::readSc(dataPath + "/flyem/FIB/movie/frame/00002.tif");
  Stack *stack1 = C_Stack::readSc(dataPath + "/flyem/FIB/movie/12layer1024/00500.tif");

  Stack *blend = C_Stack::make(COLOR, C_Stack::width(stack1), C_Stack::height(stack1),
                               10);


  color_t *array1 = (color_t*) C_Stack::array8(stack1);
  color_t *array2 = (color_t*) C_Stack::array8(stack2);


  size_t voxelNumber = C_Stack::voxelNumber(stack1);

  color_t *blendArray = (color_t*) C_Stack::array8(blend);

  double alpha = 0.0;
  for (int k = 0; k < 10; ++k) {
    for (size_t i =0; i < voxelNumber; ++i) {
      for (int c = 0; c < 3; ++c) {
        double v1 = array1[i][c];
        double v2 = array2[i][c];
        double v = v1 * (1.0 - alpha) + v2 * alpha;
        blendArray[i][c] = Clip_Value(v, 0, 255);
      }
    }
    blendArray += C_Stack::area(stack1);
    alpha += 0.1;
  }
  C_Stack::write(dataPath + "/test.tif", blend);
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZSwcTree *tree = neuronArray[0].getModel();

  ZFlyEmNeuronRange range;
  range.setPlaneRange(0, 100);
  range.setPlaneRange(10000, 1000);

  ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(neuronArray[0], range, 5);

  tree->setTypeByLabel();

  tree->save(dataPath + "/test.swc");

  /*
  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();
  ZSwcTree *tree = axis.toSwc(10.0);
  tree->resortId();
  tree->save(dataPath + "/test.swc");
  tree->print();
  delete tree;

  neuronArray[0].getModel()->save(dataPath + "/test2.swc");
  */
#endif

#if 0
  ZSwcTree *tree = ZSwcGenerator::createCircleSwc(0, 0, 0, 10.0);
  tree->resortId();
  tree->save(dataPath + "/test.swc");
#endif

#if 0
  ZFlyEmNeuronRange range;
  range.setPlaneRange(0, 10);
  range.setPlaneRange(30000, 10);

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();

  ZSwcTree *tree = ZSwcGenerator::createSwc(range, axis);

  tree->save(dataPath + "/test.swc");

  delete tree;

  tree = axis.toSwc(10.0);
  tree->save(dataPath + "/test2.swc");
  delete tree;

  /*
  tree = neuronArray[0].getModel();

  ZFlyEmQualityAnalyzer::labelSwcNodeOutOfRange(neuronArray[0], range, 5);

  tree->setTypeByLabel();

  tree->save(dataPath + "/test2.swc");
  */
#endif

#if 0 //Create range for type L1
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZFlyEmNeuronRange overallRange;

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getClass() == "L1") {
      std::cout << "Neuron:" << iter->getId() << std::endl;
      ZFlyEmNeuronRange range = iter->getRange();
      overallRange.unify(range);
    }
  }

  ZFlyEmNeuronAxis axis = neuronArray[0].getAxis();

  ZSwcTree *tree = ZSwcGenerator::createSwc(overallRange, axis);

  tree->save(dataPath + "/test.swc");

#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  //const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();
  ZFlyEmNeuronAxis axis = bundle.getNeuron(209)->getAxis();
  /*
  ZFlyEmNeuronRange range = bundle.getNeuron(209)->getRange(
        bundle.getImageResolution(NeuTube::X_AXIS),
        bundle.getImageResolution(NeuTube::Z_AXIS));
*/
  std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  std::set<std::string> classSet;
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    classSet.insert(iter->getClass());
  }

  std::map<std::string, ZFlyEmNeuronRange> overallRange;

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZString volumePath = dataPath + "/flyem/skeletonization/session3/";
    volumePath.appendNumber(iter->getId());
    volumePath += ".sobj";
    iter->setVolumePath(volumePath);

    std::cout << "Neuron:" << iter->getId() << std::endl;
    /*
    ZFlyEmNeuronRange range = iter->getRange(bundle.getImageResolution(NeuTube::X_AXIS),
                                             bundle.getImageResolution(NeuTube::Z_AXIS));
                                             */
    ZFlyEmNeuronRange range = iter->getRange(bundle.getSwcResolution(NeuTube::X_AXIS),
                                             bundle.getSwcResolution(NeuTube::Z_AXIS));
    if (overallRange.count(iter->getClass()) == 0) {
      overallRange[iter->getClass()] = range;
    }  else {
      overallRange[iter->getClass()].unify(range);
    }
  }

  std::string outDir = dataPath + "/flyem/skeletonization/session3/range";

  for (std::map<std::string, ZFlyEmNeuronRange>::const_iterator
       iter = overallRange.begin(); iter != overallRange.end(); ++iter) {
    iter->second.exportCsvFile(outDir + "/range_" + iter->first + ".csv");

    ZSwcTree *tree = ZSwcGenerator::createSwc(iter->second);
  //ZSwcTree *tree = axis.toSwc(10.0);
    if (tree != NULL) {
      tree->save(outDir + "/range_" + iter->first + ".swc");
    }
    delete tree;
  }

#endif

#if 0
  std::string dataPath = "/nearline/flyem/proj/zhaot/data";
  std::string sessionDir = dataPath + "/skeletonization/session_c";

  QFileInfoList fileList;
  QDir dir(sessionDir.c_str());
  QStringList filters;
  filters << "*.tif";
  fileList = dir.entryInfoList(filters);

  foreach (QFileInfo fileInfo, fileList) {
    ZString objPath = sessionDir + "/sobj/";
    objPath.appendNumber(ZString::LastInteger(fileInfo.baseName().toStdString()));
    objPath += ".sobj";

    std::cout << fileInfo.absoluteFilePath().toStdString() << std::endl;

    if (!fexist(objPath.c_str())) {
      Stack *stack = C_Stack::readSc(fileInfo.absoluteFilePath().toStdString());
      ZObject3dScan obj;
      obj.loadStack(stack);
      C_Stack::kill(stack);
      ZDoubleVector offset;
      offset.importTextFile(
            fileInfo.absoluteFilePath().toStdString() + ".offset.txt");
      if (offset.size() == 3) {
        obj.translate(offset[0] / 2, offset[1] / 2, offset[2]);
        std::cout << "Saving " << objPath << std::endl;
        obj.save(objPath);
      } else {
        std::cout << "Invalid offset. Skip." << std::endl;
      }
    }
  }
#endif


#if 0 //Regnerate all sparse volumes from downsampled stacks
  QDir dir((dataPath + "/flyem/skeletonization/session3").c_str());
  QStringList filters;
  filters << "*.tif";
  QFileInfoList list = dir.entryInfoList(
        filters, QDir::Files | QDir::NoSymLinks);

  foreach (QFileInfo fileInfo, list) {
    ZObject3dScan obj;
    obj.load(fileInfo.absoluteFilePath().toStdString());

  }

#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session18";
  QDir dir((sessionDir + "/bodies/orphan_check").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() < 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      if (qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(2598 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z() + 1490));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session19";
  QDir dir((sessionDir + "/bodies/orphan_check").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.getVoxelNumber() < 100000) {
      if (obj.isEmpty()) {
        std::cout << "Empty object: "
                  << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      }

      if (qc.isStitchedOrphanBody(obj)) {
        std::cout << "Orphan: "
                  << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                  << std::endl;
        ZVoxel voxel = obj.getMarker();
        std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        TZ_ASSERT(voxel.x() >= 0, "invalid point");

        json_array_append(arrayObj, json_integer(voxel.x()));
        json_array_append(arrayObj, json_integer(2598 - voxel.y()));
        json_array_append(arrayObj, json_integer(voxel.z() + 1490));
        json_array_append(dataObj, arrayObj);
      }
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/orphan.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  std::string sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";

#if 0
  ZObject3dScan obj;
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session18";
  obj.load(sessionDir + "/bodies/orphan_check/464300.sobj");
  ZVoxel voxel = obj.getMarker();
  ZCuboid box = obj.getBoundBox();
  voxel.translate(
        -box.firstCorner().x(), -box.firstCorner().y(), -box.firstCorner().z());

  std::ofstream stream((dataPath + "/test.swc").c_str());

  Swc_Tree_Node *tn =
      SwcTreeNode::makePointer(voxel.x(), voxel.y(), voxel.z(), 3.0);
  SwcTreeNode::dump(tn, stream);

  stream.close();
#endif

#endif

#if 0
  std::string sessionDir = dataPath+ "/flyem/FIB/skeletonization/session19";
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(sessionDir + "/annotations-synapse.json");
  std::vector<int> synapseCount = synapseAnnotation.countSynapse();
  std::cout << "Max bodyid: " << synapseCount.size() << std::endl;

  int count = 0;
  for (size_t i = 0; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      ++count;
    }
  }

  std::cout << count << " bodies have synapses" << std::endl;

  //Record body < 100000 voxels
  FILE *fp = fopen((sessionDir + "/bodies/bodysize.txt").c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<int> body = str.toIntegerArray();
    if (body.size() == 2) {
      if (body[1] >= 100000) {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = 0;
        }
      }
    } else if (!body.empty()) {
      std::cout << "Invalid value: " << str << std::endl;
    }
  }
  fclose(fp);


  QFileInfo fileInfo;
  //For each body with synapses
  for (size_t i = 1; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      std::cout << "Checking body " << i << std::endl;

      std::string bodyFilePath = QString("%1/bodies/stacked/%2.sobj").
          arg(sessionDir.c_str()).arg(i).toStdString();

      QString outPath = QString("%1/bodies/orphan_check/%2.sobj").
          arg(sessionDir.c_str()).arg(i);
      if (fexist(bodyFilePath.c_str())) {
        QFile::copy(bodyFilePath.c_str(), outPath);
        std::cout << outPath.toStdString() << " copied" << std::endl;
      } else {
        ZObject3dScan obj;
        //Load the body
        for (int z = 1490; z <= 7509; ++z) {
          fileInfo.setFile(
                QDir(QString("%1/bodies/0%2").
                     arg(sessionDir.c_str()).arg(z)), QString("%1.sobj").arg(i));
          if (fexist(fileInfo.absoluteFilePath().toStdString().c_str())) {
            ZObject3dScan objSlice;
            objSlice.load(fileInfo.absoluteFilePath().toStdString());
            obj.concat(objSlice);
          }
        }

        if (!obj.isEmpty()) {
          obj.save(outPath.toStdString());
        }
      }
    }
    //If it's an orphan body, save the marker
  }
#endif

#if 0
  std::string sessionDir = "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(sessionDir + "/annotations-synapse.json");
  std::vector<int> synapseCount = synapseAnnotation.countSynapse();
  std::cout << "Max bodyid: " << synapseCount.size() << std::endl;

  int count = 0;
  for (size_t i = 0; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      ++count;
    }
    synapseCount[i] = -synapseCount[i];
  }

  std::cout << count << " bodies have synapses" << std::endl;

  //Record body < 100000 voxels
  FILE *fp = fopen((sessionDir + "/bodies/bodysize_ext.txt").c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<int> body = str.toIntegerArray();
    if (body.size() == 2) {
      if (body[1] >= 100000) {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = 0;
        }
      } else {
        if (body[0] < (int) synapseCount.size()) {
          synapseCount[body[0]] = -synapseCount[body[0]];
        }
      }
    } else if (!body.empty()) {
      std::cout << "Invalid value: " << str << std::endl;
    }
  }
  fclose(fp);

  QFileInfo fileInfo;
  //For each body with synapses
  for (size_t i = 1; i < synapseCount.size(); ++i) {
    if (synapseCount[i] > 0) {
      std::cout << "Checking body " << i << std::endl;
      ZObject3dScan obj;
      //Load the body
      for (int z = 7094; z <= 7509; ++z) {
        fileInfo.setFile(
              QDir(QString("%1/bodies/0%2").
                   arg(sessionDir.c_str()).arg(z)), QString("%1.sobj").arg(i));
        if (fexist(fileInfo.absoluteFilePath().toStdString().c_str())) {
          ZObject3dScan objSlice;
          objSlice.load(fileInfo.absoluteFilePath().toStdString());
          obj.concat(objSlice);
        }
      }

      obj.save(QString("%1/bodies/orphan_check_ext/%2.sobj").
               arg(sessionDir.c_str()).arg(i).toStdString());
    }
    //If it's an orphan body, save the marker
  }
#endif

#if 0 //merge objects
  std::string sessionDir =
      "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  std::string stackedDir = sessionDir + "/bodies/stacked";
  std::string stackedExtDir = sessionDir + "/bodies/stacked_ext";

  std::string bodySizeFile = sessionDir + "/bodies/bodysize_merged.txt";

  std::ofstream stream(bodySizeFile.c_str());

  QDir dir(stackedExtDir.c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
  foreach (QFileInfo fileInfo, fileList) {
    QFileInfo stackedFile;
    stackedFile.setFile(QDir(stackedDir.c_str()), fileInfo.fileName());
    std::cout << stackedFile.absoluteFilePath().toStdString() << std::endl;
    if (stackedFile.exists()) {
      ZObject3dScan obj;
      obj.load(stackedFile.absoluteFilePath().toStdString());
      ZObject3dScan objExt;
      objExt.load(fileInfo.absoluteFilePath().toStdString());
      obj.concat(objExt);
      obj.canonize();
      obj.save(sessionDir + "/bodies/stacked_merged/" +
               fileInfo.fileName().toStdString());

      ZString objFileStr(fileInfo.baseName().toStdString());
      stream << objFileStr.lastInteger() << ", " << obj.getVoxelNumber() << std::endl;
    }
  }

  stream.close();

#endif

#if 0 //merge objects
  std::string sessionDir =
      "/run/media/zhaot/ATAWDC_2TB/data/skeletonization/FIB/session18";
  std::string stackedDir = sessionDir + "/bodies/orphan_check";
  std::string stackedExtDir = sessionDir + "/bodies/orphan_check_ext";

  std::string bodySizeFile = sessionDir + "/bodies/orphan_check_bodysize_merged.txt";

  std::ofstream stream(bodySizeFile.c_str());

  QDir dir(stackedExtDir.c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
  foreach (QFileInfo fileInfo, fileList) {
    QFileInfo stackedFile;
    stackedFile.setFile(QDir(stackedDir.c_str()), fileInfo.fileName());
    std::cout << stackedFile.absoluteFilePath().toStdString() << std::endl;
    if (stackedFile.exists()) {
      ZObject3dScan obj;
      obj.load(stackedFile.absoluteFilePath().toStdString());
      if (!obj.isEmpty()) {
        ZObject3dScan objExt;
        objExt.load(fileInfo.absoluteFilePath().toStdString());
        if (!objExt.isEmpty()) {
          obj.concat(objExt);
          obj.canonize();
          obj.save(sessionDir + "/bodies/orphan_check_merged/" +
                   fileInfo.fileName().toStdString());

          ZString objFileStr(fileInfo.baseName().toStdString());
          stream << objFileStr.lastInteger() << ", " << obj.getVoxelNumber() << std::endl;
        }
      }
    }
  }

  stream.close();

  json_t *obj = json_object();
  json_t *dataObj = json_array();
  json_t *arrayObj = json_array();

  json_array_append(arrayObj, json_integer(1));
  json_array_append(arrayObj, json_integer(2));
  json_array_append(arrayObj, json_integer(3));

  json_array_append(dataObj, arrayObj);
  json_object_set(obj, "data", dataObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0 //sort branch length
  std::string sessionDir =
      "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/skeletonization/session18";

  QFileInfoList fileList;
  QStringList filters;
  filters << "*.swc";

  QDir dir((sessionDir + "/len25_merged").c_str());

  fileList = dir.entryInfoList(filters);

  std::vector<std::pair<double, int> > sizeArray;

  foreach (QFileInfo fileInfo, fileList) {
    std::cout << fileInfo.baseName().toStdString() << std::endl;
    int id = ZString::lastInteger(fileInfo.baseName().toStdString());
    ZSwcTree tree;
    tree.load(fileInfo.absoluteFilePath().toStdString());
    double length = tree.length();
    sizeArray.push_back(std::pair<double, int>(length, id));
  }

  std::sort(sizeArray.begin(), sizeArray.end());

  std::ofstream stream((sessionDir + "/len25_merged/length.txt").c_str());

  for (std::vector<std::pair<double, int> >::const_reverse_iterator
       iter = sizeArray.rbegin();
       iter != sizeArray.rend(); ++iter) {
    stream << iter->second << "," << iter->first << std::endl;
  }

  stream.close();
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  bundle.exportJsonFile(dataPath + "/test.json");
#endif

#if 0
  ZFlyEmNeuronRange reference;
  reference.importCsvFile(dataPath +
        "/flyem/skeletonization/session3/range/range_Mi1.csv");
  //ZSwcTree *tree = ZSwcGenerator::createSwc(reference);

  ZFlyEmNeuronRange range;

  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (iter->getId() == 10051409) {
      ZString volumePath = dataPath + "/flyem/skeletonization/session3/";
      volumePath.appendNumber(iter->getId());
      volumePath += ".sobj";
      iter->setVolumePath(volumePath);

      range = iter->getRange(bundle.getSwcResolution(NeuTube::X_AXIS),
                             bundle.getSwcResolution(NeuTube::Z_AXIS));
    }
  }

  ZSwcTree *tree = ZSwcGenerator::createRangeCompareSwc(range, reference);
  tree->save(dataPath + "/test.swc");
#endif

#if 0
  ZSwcTree tree;
  //tree.load(dataPath + "/benchmark/diadem/golden/e1.swc");
  //tree.load(dataPath + "/benchmark/swc/dense7.swc");
  tree.load(dataPath + "/DH070313-1.Edit.swc");
  ZSwcResampler resampler;
  resampler.optimalDownsample(&tree);
  tree.save(dataPath + "/test.swc");

#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  Print_Cuboid_I(&boundBox);
#endif

#if 0
  ZFlyEmDataBundle bundle1;
  bundle1.loadJsonFile(
        dataPath + "/flyem/FIB/fib_bundle1/data_bundle.json");

  ZFlyEmDataBundle bundle2;
  bundle2.loadJsonFile(
        dataPath + "/flyem/FIB/skeletonization/session18/len10/adjusted/data_bundle_small.json");

  ZFlyEmNeuron *neuron = bundle1.getNeuron(7021);

  ZSwcTree *tree1 = neuron->getModel();

  std::vector<ZFlyEmNeuron>& neuronArray = bundle2.getNeuronArray();

  double minDist = 100.0;

  ZSwcMetric *metric = new ZSwcTerminalSurfaceMetric;

  for (std::vector<ZFlyEmNeuron>::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    //std::cout << "Checking " << iter->getId() << "..." << std::endl;
    ZSwcTree *tree2 = iter->getModel();
    if (tree2 != NULL) {
      double dist = metric->measureDistance(tree1, tree2);
      if (dist <= minDist) {
        std::cout << "Within range" << "(" << dist << "): " << " " << iter->getId() << std::endl;
      }
    }
  }
#endif

#if 0 //Laplacian map
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::map<std::string, int> classMap;
  const std::vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (classMap.count(iter->getClass()) == 0) {
      classMap[iter->getClass()] = classMap.size();
    }
  }

  ZMatrix matrix;
  matrix.importTextFile(dataPath + "/test.txt");

  ZNormColorMap colorMap;


  vector<ZVaa3dMarker> markerArray;

  for (int i = 0; i < matrix.getRowNumber(); ++i) {
    ZVaa3dMarker marker;
    int id = iround(matrix.getValue(i, 0));

    ZFlyEmNeuron *neuron = bundle.getNeuron(id);

    marker.setCenter(matrix.getValue(i, 1) * 100, matrix.getValue(i, 2) * 100,
                     matrix.getValue(i, 3) * 100);
    marker.setRadius(1.0);

    QColor color = colorMap.mapColor(classMap[neuron->getClass()]);
    marker.setColor(color.red(), color.green(), color.blue());
    marker.setType(classMap[neuron->getClass()]);
    marker.setName(neuron->getName());
    marker.setComment(neuron->getClass());

    markerArray.push_back(marker);
  }

  FlyEm::ZFileParser::writeVaa3dMakerFile(dataPath + "/test.marker", markerArray);
#endif

#if 0 //Generate layer swc
  int zStart = 1490;
  int zEnd = 7489;
  double res[3] = {10, 10, 10};

  double height = (zEnd - zStart + 1) * res[2];
  double hMarkLength = height / 20;

  double layer[10] = {0.1, 0.2, 0.3, 0.35, 0.43, 0.54, 0.66, 0.73, 0.91, 1};
  double radius = 10.0 * res[2];


  Swc_Tree_Node *tn = SwcTreeNode::makePointer(0, 0, 0, radius);
  Swc_Tree_Node *root = tn;

  Swc_Tree_Node *tn2 =
      SwcTreeNode::makePointer(hMarkLength, 0, SwcTreeNode::z(tn), radius);
  SwcTreeNode::setParent(tn2, tn);

  for (size_t i = 0; i < 10; ++i) {
    double z = height * layer[i];
    tn2 = SwcTreeNode::makePointer(0, 0, z, radius);
    SwcTreeNode::setParent(tn2, tn);
    tn = tn2;
    tn2 = SwcTreeNode::makePointer(hMarkLength, 0, z, radius);
    SwcTreeNode::setParent(tn2, tn);
  }



  ZSwcTree tree;
  tree.setDataFromNode(root);

  tree.resortId();
  tree.save(dataPath + "/test.swc");

#endif

#if 0 //Generate TEM neuron information
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::map<std::string, int> classMap;
  const std::vector<ZFlyEmNeuron> &neuronArray = bundle.getNeuronArray();
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    if (classMap.count(iter->getClass()) == 0) {
      classMap[iter->getClass()] = classMap.size();
    }
  }

  std::ofstream stream((dataPath + "/flyem/TEM/id_class.txt").c_str());

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    std::cout << iter->getId() << " " << iter->getName() << " " << iter->getClass()
              << " " << classMap[iter->getClass()] << std::endl;
    stream << iter->getId() << " " << classMap[iter->getClass()] << std::endl;
  }
  stream.close();
#endif


#if 0 //Check overlap
  QDir dir1((dataPath + "/flyem/FIB/skeletonization/session18/bodies/stacked").c_str());
  QStringList filters;
  filters << "*.sobj";
  QFileInfoList fileList1 = dir1.entryInfoList(filters);
  std::cout << fileList1.size() << " objects found in " << dir1.absolutePath().toStdString()
            << std::endl;

  QDir dir2((dataPath + "/flyem/FIB/skeletonization/session19/bodies/500k+").c_str());
  QFileInfoList fileList2 = dir2.entryInfoList(filters);
  std::cout << fileList2.size() << " objects found in " << dir2.absolutePath().toStdString()
            << std::endl;

  QVector<ZObject3dScan> objArray1(fileList1.size());
  QVector<ZObject3dScan> objArray2(fileList2.size());
  //objArray1.resize(100);
  //objArray2.resize(100);

  QVector<int> idArray1(fileList1.size());
  QVector<int> idArray2(fileList2.size());

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray1.size(); ++i) {
    objArray1[i].load(fileList1[i].absoluteFilePath().toStdString());
    std::cout << i << ": Object size: " << objArray1[i].getVoxelNumber() << std::endl;
    objArray1[i].downsample(3, 3, 3);
    objArray1[i].canonize();
    std::cout << "Object size: " << objArray1[i].getVoxelNumber() << std::endl;
    idArray1[i] = ZString::lastInteger(fileList1[i].baseName().toStdString());
  }

  std::cout << "Loading objects ..." << std::endl;
  for (int i = 0; i < objArray2.size(); ++i) {
    objArray2[i].load(fileList2[i].absoluteFilePath().toStdString());
    objArray2[i].downsample(3, 3, 3);
    objArray2[i].canonize();
    idArray2[i] = ZString::lastInteger(fileList2[i].baseName().toStdString());
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

  std::ofstream stream((dataPath + "/test.txt").c_str());

  int offset[3];
  foreach(Cuboid_I boundBox1, boundBoxArray1) {
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
    ++index1;
  }

  stream.close();

#endif

#if 0
  ZMatrix matrix;
  matrix.importTextFile(dataPath + "/flyem/FIB/overlap.txt");

  int nrow = matrix.getRowNumber();

  QSet<QPair<int, int> > bodyArray;
  for (int i = 0; i < nrow; ++i) {
    bodyArray.insert(QPair<int, int>(0, iround(matrix.at(i, 0))));
    bodyArray.insert(QPair<int, int>(1, iround(matrix.at(i, 1))));
  }

  QList<QPair<int, int> > bodyList = bodyArray.toList();
  QMap<QPair<int, int>, int> bodyMap;
  int index = 0;
  for (QList<QPair<int, int> >::const_iterator iter = bodyList.begin();
       iter != bodyList.end(); ++iter) {
    bodyMap[*iter] = index++;
  }

  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);

  for (int i = 0; i < nrow; ++i) {
    graph.addEdge(bodyMap[QPair<int, int>(0, iround(matrix.at(i, 0)))],
        bodyMap[QPair<int, int>(1, iround(matrix.at(i, 1)))],
        matrix.at(i, 2));
  }

  int *degree = graph.getDegree();

  for (int i = 0; i < graph.getVertexNumber(); ++i) {
    if (degree[i] > 1) {
      std::set<int> neighborSet = graph.getNeighborSet(i);
      std::cout << "Overlap:" << std::endl;
      QPair<int, int> body = bodyList[i];
      std::cout << body.first << " " << body.second << std::endl;
      for (std::set<int>::const_iterator iter = neighborSet.begin();
           iter != neighborSet.end(); ++iter) {
        QPair<int, int> body = bodyList[*iter];
        std::cout << "  " << body.first << " " << body.second << std::endl;
      }
    }
  }

#endif

#if 0
  ZSwcTree tree1;
  tree1.load(dataPath + "/flyem/FIB/skeletonization/session18/bodies/changed/stacked/swc/adjusted/427.swc");
  ZSwcTree tree2;
  tree2.load(dataPath + "/flyem/FIB/skeletonization/session18/bodies/changed/stacked/swc/adjusted/167091.swc");

  ZSwcTerminalAngleMetric metric;
  metric.setDistanceWeight(false);
  double dist = metric.measureDistance(&tree1, &tree2);

  std::cout << dist << std::endl;
#endif

#if 0
  ZGraph graph(ZGraph::UNDIRECTED_WITH_WEIGHT);
  graph.importTxtFile(dataPath + "/flyem/FIB/skeletonization/session19/changed.txt");


  const std::vector<ZGraph*> &subGraphList = graph.getConnectedSubgraph();
  for (std::vector<ZGraph*>::const_iterator iter = subGraphList.begin();
       iter != subGraphList.end(); ++iter) {
    ZGraph *subgraph = *iter;
    std::set<int> nodeSet;
    for (int i = 0; i < subgraph->getEdgeNumber(); ++i) {
      nodeSet.insert(subgraph->getEdgeBegin(i));
      nodeSet.insert(subgraph->getEdgeEnd(i));
    }

    for (std::set<int>::const_iterator iter = nodeSet.begin();
         iter != nodeSet.end(); ++iter) {
      std::cout << *iter << " ";
    }
    std::cout << "   ||" <<  std::endl;
  }

#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  //blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session19";
  QDir dir((sessionDir + "/bodies/500k+").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  int count = 0;
  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());
    if (obj.isEmpty()) {
      std::cout << "Empty object: "
                << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      continue;
    }

    if (qc.isOrphanBody(obj)) {
      std::cout << "Orphan " << ++count << ": "
                << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0 //Change root of the swc trees
  //Load data bundle

  //For each neuron

#endif

#if 0
  //Output a list of orphans that do not touch substack boundaries and are
  //greater than 100,000 voxels in size.
  //Please provide a list of x,y,z points along with the body ID.
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  //blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_12layer.swc");

  ZFlyEmQualityAnalyzer qc;
  qc.setSubstackRegion(blockArray);

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session22";
  QDir dir((sessionDir + "/bodies/stacked").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  std::cout << fileList.size() << " bodies loaded." << std::endl;

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *obj = json_object();
  json_t *dataObj = json_array();

  int count = 0;
  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());

    int id = ZString::lastInteger(objFile.absoluteFilePath().toStdString());
    if (obj.isEmpty()) {
      std::cout << "Empty object: "
                << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      continue;
    }

    if (qc.isOrphanBody(obj)) {
      std::cout << "Orphan " << ++count << ": "
                << ZString::lastInteger(objFile.absoluteFilePath().toStdString())
                << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(id));
      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(dataObj, arrayObj);

      ZCuboid box = obj.getBoundBox();
      ZSwcGenerator generator;
      ZSwcTree *tree = generator.createBoxSwc(box);
      tree->save((objFile.absoluteFilePath() + ".swc").toStdString());
      delete tree;
    }
  }

  json_object_set(obj, "data", dataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(obj, "metadata", metaObj);

  json_dump_file(obj, (dataPath + "/test.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/flyem/TEM/data_release/bundle1/swc/T4-11_588435.swc");
  ZSwcSubtreeAnalyzer analyzer;
  analyzer.setMinLength(1000.0);
  analyzer.decompose(&tree);
  tree.setTypeByLabel();

  Swc_Tree_Node *root = tree.root()->first_child;
  while (root != NULL) {
    Swc_Tree_Node *nextRoot = SwcTreeNode::nextSibling(root);
    ZSwcTree subtree;
    SwcTreeNode::detachParent(root);
    subtree.setDataFromNode(root);
    ZDoubleVector feature =
        ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
          subtree, ZSwcGlobalFeatureAnalyzer::NGF1);
    ZDoubleVector::print(feature);
    root = nextRoot;
  }

  //tree.save(GET_DATA_DIR + "/test.swc");
#endif


#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        dataPath + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  const std::vector<ZFlyEmNeuron>& neuronArray = bundle.getNeuronArray();

  ZSwcSubtreeAnalyzer analyzer;
  analyzer.setMinLength(10000.0);

  std::ofstream stream((GET_DATA_DIR + "/test/subtree/feature.txt").c_str());


  std::map<string, int> classMap = bundle.getClassIdMap();

  std::ofstream stream2((GET_DATA_DIR + "/test/subtree/class.txt").c_str());
  for (std::map<string, int>::const_iterator iter = classMap.begin();
      iter != classMap.end(); ++iter) {
    stream2 << iter->first << " : " << iter->second << std::endl;
  }
  stream2.close();

  int count = 0;
  for (std::vector<ZFlyEmNeuron>::const_iterator neuronIter = neuronArray.begin();
       neuronIter != neuronArray.end(); ++neuronIter) {
    const ZFlyEmNeuron &neuron = *neuronIter;
    int classId = classMap[neuron.getClass()];

    std::cout << neuronIter->getId() << std::endl;
    ZSwcTree *tree = neuronIter->getModel();

    analyzer.decompose(tree);
    tree->setTypeByLabel();
    QString path = QString("%1/%2/%3.swc").arg(GET_DATA_DIR.c_str()).arg("test/subtree").
        arg(neuronIter->getId());
    tree->save(path.toStdString());

    Swc_Tree_Node *root = SwcTreeNode::firstChild(tree->root());
    while (root != NULL) {
      Swc_Tree_Node *nextRoot = SwcTreeNode::nextSibling(root);
      ZSwcTree subtree;
      SwcTreeNode::detachParent(root);
      subtree.setDataFromNode(root);

      if (subtree.length() * 2.0 > analyzer.getMinLength()) { //ignore small subtrees
        ZString subtreePath = GET_DATA_DIR + "/test/subtree/decomposed/";
        subtreePath.appendNumber(++count, 5);
        subtreePath += "_";
        subtreePath.appendNumber(neuronIter->getId());
        subtreePath += ".swc";
        subtree.resortId();
        subtree.save(subtreePath);

        ZDoubleVector feature =
            ZSwcGlobalFeatureAnalyzer::computeFeatureSet(
              subtree, ZSwcGlobalFeatureAnalyzer::NGF1);
        ZDoubleVector::print(feature);
        stream << classId << " " << neuronIter->getId() << " ";
        for (ZDoubleVector::const_iterator iter = feature.begin();
             iter != feature.end(); ++iter) {
          stream << *iter << " ";
        }
        stream << std::endl;
      }
      root = nextRoot;
    }

  }
#endif

#if 0
  //Provide x,y,z coordinates (and a body id) for all bodies
  //between 100,000 and 500,000 in size.

  //provide x,y,z coordinate (and a body id) for all bodies
  //between 500,000 and 1,000,000 in size.

  QStringList filters;
  filters << "*.sobj";
  std::string sessionDir = dataPath + "/flyem/FIB/skeletonization/session22";
  QDir dir((sessionDir + "/bodies/stacked").c_str());
  QFileInfoList fileList = dir.entryInfoList(
        filters, QDir::Files);

  std::cout << fileList.size() << " bodies loaded." << std::endl;

  //QVector<ZObject3dScan> objList(fileList.size());

  json_t *smallObj = json_object();
  json_t *smallDataObj = json_array();

  json_t *bigObj = json_object();
  json_t *bigDataObj = json_array();

  foreach (QFileInfo objFile, fileList) {
    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan obj;
    obj.load(objFile.absoluteFilePath().toStdString());

    int id = ZString::lastInteger(objFile.absoluteFilePath().toStdString());
    if (obj.isEmpty()) {
      std::cout << "Empty object: "
                << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
      continue;
    }

    size_t volume = obj.getVoxelNumber();
    if (volume >= 100000 && volume < 500000) {
      std::cout << "100k-500k " << id << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(id));
      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(smallDataObj, arrayObj);
    } else if (volume >= 500000 && volume < 1000000) {
      std::cout << "500k-1000k " << id << std::endl;
      ZVoxel voxel = obj.getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(id));
      json_array_append(arrayObj, json_integer(voxel.x()));
      json_array_append(arrayObj, json_integer(2598 - voxel.y()));
      json_array_append(arrayObj, json_integer(voxel.z() + 1490));
      json_array_append(bigDataObj, arrayObj);
    }
  }

  json_object_set(smallObj, "data", smallDataObj);
  json_object_set(bigObj, "data", bigDataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(smallObj, "metadata", metaObj);
  json_object_set(bigObj, "metadata", metaObj);

  json_dump_file(smallObj, (dataPath + "/100k_500k.json").c_str(), JSON_INDENT(2));
  json_dump_file(bigObj, (dataPath + "/500k_100k.json").c_str(), JSON_INDENT(2));
#endif

#if 0
  ZMatrix mat;
  mat.importTextFile(GET_DATA_DIR + "/test/subtree/fuzzy_cluster_feature.txt");

  int nsample = mat.getRowNumber();
  int nfeat = mat.getColumnNumber() - 1;

  std::vector<double> hist1(nfeat);
  std::vector<double> hist2(nfeat);

  ZMatrix confusionMatrix(89, 89);

  ZMatrix simMat(nsample + 1, nsample);

  for (int i = 0; i < nsample; ++i) {
    simMat.set(0, i, mat.getValue(i, 0));
  }

  double goodCount = 0;
  for (int i = 0; i < nsample; ++i) {
    mat.copyRowValue(i, 1, nfeat, &(hist1[0]));
    int predicted = -1;
    double minDist = 0.0;
    for (int j = 0; j < nsample; ++j) {
      if (i != j) {
        mat.copyRowValue(j, 1, nfeat, &(hist2[0]));
        int classId = iround(mat.getValue(j, 0));
        double dist = ZHistogram::computeJsDivergence(hist1, hist2);
        simMat.set(i, j, 1.0 / (1.0 + dist));
        if (predicted < 0 || minDist > dist) {
          minDist = dist;
          predicted = classId;
        }
      } else {
        simMat.set(i, j, 1.0);
      }
    }

    int realClass = iround(mat.getValue(i, 0));
    if (predicted == realClass) {
      ++goodCount;
    }

    std::cout << realClass << " " << predicted << std::endl;
    confusionMatrix.addValue(realClass - 1, predicted - 1, 1);
  }

  simMat.exportCsv(GET_DATA_DIR + "/flyem/TEM/simmat_subtree.txt");
  std::cout << goodCount << " / " << nsample;
  confusionMatrix.exportCsv(GET_DATA_DIR + "/fuzzy_confmat.csv");

#endif

#if 0 //Finding holes
  QDir dir((GET_DATA_DIR + "/flyem/FIB/skeletonization/session19/bodies/500k+").c_str());
  std::string outDir = (dir.absolutePath() + "/hole").toStdString();
  QStringList nameFilters;
  nameFilters << "*.sobj";
  QFileInfoList fileList = dir.entryInfoList(nameFilters);
  std::cout << fileList.size() << " objects found." << std::endl;

  //For each object file
  foreach (QFileInfo file, fileList) {
    std::string outFile =
        outDir + "/" + file.baseName().toStdString() + ".sobj";

    if (!fexist(outFile.c_str())) {
      //Load the object
      std::cout << "Loading " << file.baseName().toStdString() << " ..." << std::endl;
      ZObject3dScan obj;
      obj.load(file.absoluteFilePath().toStdString());

      //Find the hole object
      std::cout << "Extracting holes ..." << std::endl;
      ZObject3dScan holeObj = obj.findHoleObject();

      //Save the hole object
      if (!holeObj.isEmpty()) {
        std::cout << "Saving holes ..." << std::endl;

        holeObj.save(outFile);
        std::cout << outFile << " saved." << std::endl;
      }
    }
  }
#endif

#if 0
  ZDendrogram dendrogram;

  ZMatrix Z;
  Z.importTextFile(GET_DATA_DIR + "/Z.txt");
  for (int i = 0; i < Z.getRowNumber(); ++i) {
    //dendrogram.addLink(Z.at(i, 0), Z.at(i, 1), Z.at(i, 2) - 0.5); //for matching simmat

    dendrogram.addLink(Z.at(i, 0), Z.at(i, 1), Z.at(i, 2) - 0.38);
  }
  dendrogram.loadLeafName(GET_DATA_DIR + "/flyem/TEM/neuron_name.txt");
  std::string svgString = dendrogram.toSvgString(15.0);

  //std::string svgString = dendrogram.toSvgString(10.0);//for matching simmat
  //ZSvgGenerator svgGenerator(0, 0, 800, 5000);//for matching simmat

  ZSvgGenerator svgGenerator(0, 0, 1000, 6000);

  svgGenerator.write((GET_DATA_DIR + "/test.svg").c_str(), svgString);
#endif

#if 0
  ZDendrogram dendrogram;
  dendrogram.addLink(1, 2, 1);
  dendrogram.addLink(3, 4, 1);
  dendrogram.addLink(7, 8, 2);
  dendrogram.addLink(5, 9, 3);
  dendrogram.addLink(6, 10, 4);
  std::string svgString = dendrogram.toSvgString(20.0);

  ZSvgGenerator svgGenerator;
  svgGenerator.write((GET_DATA_DIR + "/test.svg").c_str(), svgString);
#endif

#if 0
  std::string neuronNameFilePath = GET_DATA_DIR + "/flyem/TEM/class_name.txt";
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(GET_DATA_DIR + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  std::vector<ZFlyEmNeuron> neuronArray = bundle.getNeuronArray();

  std::ofstream stream(neuronNameFilePath.c_str());
  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    stream << neuron.getName() << std::endl;
  }
  stream.close();
#endif

#if 0
  ZFlyEmBodyAnalyzer bodyAnalyzer;
  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/flyem/FIB/skeletonization/session19/bodies/500k+/29.sobj");


  //obj.downsampleMax(1, 1, 0);
  //obj.save(GET_DATA_DIR + "/test.sobj");
  bodyAnalyzer.setDownsampleInterval(1, 1, 0);

  ZPointArray pts = bodyAnalyzer.computeHoleCenter(obj);
  ZCuboid box = obj.getBoundBox();
  ZPoint corner = box.firstCorner();
  corner *= -1;
  pts.translate(corner);
  pts.exportSwcFile(GET_DATA_DIR + "/test.swc", 3.0);

  pts.print();
#endif

#if 0
  ZFlyEmBodyAnalyzer bodyAnalyzer;
  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/flyem/FIB/skeletonization/session19/bodies/500k+/29.sobj");
  bodyAnalyzer.setDownsampleInterval(1, 1, 1);

  ZPointArray pts = bodyAnalyzer.computeTerminalPoint(obj);

  ZCuboid box = obj.getBoundBox();
  ZPoint corner = box.firstCorner();
  corner *= -1;
  pts.translate(corner);

  pts.exportSwcFile(GET_DATA_DIR + "/test.swc", 3.0);
#endif

#if 0
  RECORD_INFORMATION("Info test");
  RECORD_WARNING_UNCOND("Warning test");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/biocytin/bug_source1.swc");
  ZSwcResampler resampler;

  resampler.optimalDownsample(&tree);

  tree.resortId();
  //resampler.optimalDownsample(&tree);
  tree.save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/benchmark/swc/compare/compare1.swc");
  tree.setType(0);

  ZSwcNodeDistSelector selector;
  selector.setMinDistance(100);
  ZSwcTreeNodeArray nodeArray = selector.select(tree);

  nodeArray.print();

  tree.setTypeByLabel();
  tree.print();
  tree.save(GET_DATA_DIR + "/test.swc");

#endif


#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(dataPath + "/flyem/TEM/data_bundle2.json");

  ZSwcTree *tree = bundle.getModel(209);
  double length = tree->length();
  double minDist = length / 500.0;

  ZSwcNodeDistSelector selector;
  selector.setMinDistance(minDist);

  ZSwcTreeNodeArray nodeArray = selector.select(*tree);

  tree->setTypeByLabel();
  tree->save(GET_DATA_DIR + "/test.swc");

  std::cout << nodeArray.size() << " selected" << std::endl;

  double shollRange = minDist * 2.0;

  ZSwcShollFeatureAnalyzer analyzer;
  analyzer.setShollStart(shollRange / 10.0);
  analyzer.setShollEnd(shollRange);
  analyzer.setShollRadius(shollRange / 10.0);

  std::vector<double> feature;

  for (ZSwcTreeNodeArray::const_iterator iter = nodeArray.begin();
       iter != nodeArray.end(); ++iter) {
    feature = analyzer.computeFeature(*iter);
  }
#endif

#if 0
  ZObject3dScan obj;
  obj.importDvidObject(GET_DATA_DIR + "/1.dvid");

  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 1, 1, 1);
  obj.addSegment(1, 1, 0, 2);
  obj.addSegment(2, 0, 1, 1);
  obj.addSegment(2, 1, 0, 2);
  obj.addSegment(2, 2, 1, 1);
  obj.save(GET_DATA_DIR + "/benchmark/tower3.sobj");
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(1, 2, 2, 2);
  obj.addSegment(2, 2, 1, 3);
  obj.addSegment(3, 1, 2, 2);
  obj.addSegment(3, 2, 1, 3);
  obj.addSegment(3, 3, 2, 2);
  obj.save(GET_DATA_DIR + "/benchmark/tower5.sobj");
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(1, 16, 16, 16);
  obj.addSegment(2, 16, 5, 32);
  for (int y = 5; y < 16; ++y) {
    obj.addSegment(3, y, 16, 16);
  }
  obj.addSegment(3, 16, 5, 32);
  for (int y = 17; y < 33; ++y) {
    obj.addSegment(3, y, 16, 16);
  }
  obj.save(GET_DATA_DIR + "/benchmark/pile.sobj");
#endif

#if 0
  ZMatlabProcess matlabProcess;
  if (matlabProcess.findMatlab()) {
    matlabProcess.setWorkDir("/tmp/matlab");
    matlabProcess.setScript("/Users/zhaot/Work/SLAT/matlab/SLAT/run/flyem/tz_run_flyem_clustering_command.m");
    matlabProcess.run();

    matlabProcess.printOutputSummary();
  }
#endif

#if 0
  ZStackBinarizer binarizer;
  binarizer.setThreshold(8);
  binarizer.setMethod(ZStackBinarizer::BM_ONE_SIGMA);
  binarizer.setRetryCount(3);
  binarizer.setMinObjectSize(5);

  Stack *stack = C_Stack::readSc(GET_DATA_DIR + "/system/mouse_neuron_single/stack.tif");
  if (!binarizer.binarize(stack)) {
    std::cout << "Binarization failed" << std::endl;
  }
  C_Stack::write(GET_DATA_DIR + "/test.tif", stack);
#endif

#if 0
   Stack *stack = C_Stack::readSc(GET_DATA_DIR + "/system/mouse_neuron_single/stack.tif");
   std::cout << Stack_Var(stack) << std::endl;
#endif

#if 0
  ZString str("data/testffa/test.tif/");
  std::vector<std::string> parts = str.decomposePath();
  for (size_t i = 0; i < parts.size(); ++i) {
    std::cout << parts[i] << " || ";
  }
  std::cout << std::endl;
#endif

#if 0
  std::cout << ZString::relativePath(GET_DATA_DIR + "/test/test.tif", GET_DATA_DIR + "/benchmark")
               << std::endl;
#endif


#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/test.swc");
  ZStack stack(GREY, 512, 512, 60, 1);
  stack.setZero();
  tree.labelStack(stack.c_stack());
  stack.save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  Stack *stack =C_Stack::readSc(GET_DATA_DIR + "/benchmark/ball.tif");
  stack = C_Stack::boundCrop(stack, 0);
  Stack *out = misc::computeNormal(stack, NeuTube::Z_AXIS);
  C_Stack::write(GET_DATA_DIR + "/test.tif", out);
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        GET_DATA_DIR + "/flyem/TEM/data_release/bundle1/data_bundle.json");
  ZFlyEmNeuron *neuron = bundle.getNeuron(515936);
  ZFlyEmNeuronFeatureAnalyzer analyzer;
  int layer = analyzer.computeMostSpreadedLayer(*neuron);
  double radius = analyzer.computeSpreadRadius(*neuron, layer);
  std::cout << "Most spreaded layer: " << layer << std::endl;
  std::cout << "Radius: " << radius << std::endl;
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  Cuboid_I boundBox = blockArray.getBoundBox();
  std::cout << "Offset: " << boundBox.cb[0] << " " << boundBox.cb[1] << std::endl;
  blockArray.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray.translate(10, 10, 10);

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_13layer.swc");

  ZIntCuboidArray blockArray2;
  blockArray2.loadSubstackList(dataPath + "/flyem/FIB/block_layer12_13.txt");
  blockArray2.translate(-boundBox.cb[0], -boundBox.cb[1], -boundBox.cb[2]);
  blockArray2.translate(10, 10, 10);
  blockArray2.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_layer12_13.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  blockArray.print();
  calbr.calibrate(blockArray);
  //blockArray.print();
  Cuboid_I boundBox = blockArray.getBoundBox();
  Print_Cuboid_I(&boundBox);

  blockArray.exportSwc(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  std::cout << misc::computeRavelerHeight(blockArray, 10) << std::endl;
#endif

#if 0 //Get all bodies within a certain body range
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session26/100k_500k/stacked");
  std::cout << neuronArray.size() << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseAnnotation;
  synapseAnnotation.loadJson(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session26/annotations-synapse.json");
  neuronArray.setSynapseAnnotation(&synapseAnnotation);

  ZFlyEmNeuronExporter exporter;
  exporter.exportIdPosition(neuronArray, GET_DATA_DIR + "/test.json");

  /*
  for (size_t i = 0; i < neuronArray.size(); ++i) {

  }
  */
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session27/annotations-synapse.json");

  std::vector<int> psdCount = synapseArray.countPsd();
  std::vector<int> tbarCount = synapseArray.countTBar();

  size_t maxId = std::max(psdCount.size(), tbarCount.size());

  int bodyCount = 0;
  for (size_t i = 0; i < maxId; ++i) {
    if (psdCount[i] > 0 || tbarCount[i] > 0) {
      ++bodyCount;
    }
  }

  std::cout << bodyCount << " bodies have synapses" << std::endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session27/annotations-synapse.json");

  std::vector<int> psdCount = synapseArray.countPsd();
  std::vector<int> tbarCount = synapseArray.countTBar();

  std::cout << psdCount.size() << std::endl;
  std::cout << tbarCount.size() << std::endl;
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session27/100k_le_2/stacked");
  std::cout << neuronArray.size() << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session27/annotations-synapse.json");

  std::vector<int> allPsdCount = synapseArray.countPsd();
  std::vector<int> allTbarCount = synapseArray.countTBar();

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  //blockArray.print();
  //calbr.calibrate(blockArray);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  json_t *rootObj = json_object();

  json_t *neuronArrayObj = json_array();
  for (ZFlyEmNeuronArray::const_iterator
       iter = neuronArray.begin(); iter != neuronArray.end(); ++iter) {

    const ZFlyEmNeuron &neuron = *iter;
    int psdCount = 0;
    int tbarCount = 0;

    if ((size_t) neuron.getId() < allPsdCount.size()) {
      psdCount = allPsdCount[neuron.getId()];
    }
    if ((size_t) neuron.getId() < allTbarCount.size()) {
      tbarCount = allTbarCount[neuron.getId()];
    }
    if (psdCount > 0 || tbarCount > 0) {
      std::cout << "Neuron: " << neuron.getId() << ": "
                << tbarCount << " " << psdCount << std::endl;
      json_t *neuronObj = json_object();
      json_object_set_new(neuronObj, "id", json_integer(neuron.getId()));
      if (analyzer.touchingGlobalBoundary(*neuron.getBody())) {
        json_object_set_new(neuronObj, "boundary", json_true());
      } else {
        json_object_set_new(neuronObj, "boundary", json_false());
      }
      json_object_set_new(neuronObj, "tbar",
                          json_integer(tbarCount));

      json_object_set_new(neuronObj, "psd",
                          json_integer(psdCount));
      json_array_append_new(neuronArrayObj, neuronObj);
    }
  }
  json_object_set_new(rootObj, "data", neuronArrayObj);

  json_dump_file(rootObj, (GET_DATA_DIR + "/small_body.json").c_str(),
                 JSON_INDENT(2));

#endif

#if 0 //Sort large bodies
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session27/100k_5m/stacked");
  std::cout << neuronArray.size() << std::endl;

  for (ZFlyEmNeuronArray::const_iterator
       iter = neuronArray.begin(); iter != neuronArray.end(); ++iter) {
  }

  ZFlyEmNeuronExporter exporter;
  int ravelerHeight = 2599;
  exporter.setRavelerHeight(ravelerHeight);
  exporter.useRavelerCoordinate(true);

  ZFlyEmNeuronBodySizeFilter sizeFilter;
  sizeFilter.setSizeRange(100000, 1000000);
  exporter.setNeuronFilter(&sizeFilter);
  exporter.exportIdPosition(neuronArray, GET_DATA_DIR + "/100k_1m.json");

  sizeFilter.setSizeRange(1000000, 5000000);
  exporter.exportIdPosition(neuronArray, GET_DATA_DIR + "/1m_5m.json");
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session27/100k_1m/stacked");

  int ravelerHeight = 2599;
  json_t *rootObj = json_object();
  json_t *neuronArrayObj = json_array();

  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    bool isSelected = true;
    if (isSelected) {
      ZObject3dScan *obj = neuron.getBody();
      if (obj != NULL) {
        ZVoxel voxel = obj->getMarker();

        json_t *neuronObj = json_object();
        json_t *idObj = json_integer(neuron.getId());
        json_object_set_new(neuronObj, "id", idObj);


        //voxel.translate(bodyOffset[0], bodyOffset[1], bodyOffset[2]);
        //std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        json_array_append_new(arrayObj, json_integer(voxel.x()));

        int y = voxel.y();
        y = ravelerHeight - 1 - voxel.y();
        json_array_append_new(arrayObj, json_integer(y));
        json_array_append_new(arrayObj, json_integer(voxel.z()));

        json_object_set_new(neuronObj, "position", arrayObj);

        json_array_append_new(neuronArrayObj, neuronObj);
      }
      neuron.deprecate(ZFlyEmNeuron::BODY);
    }
  }

  json_object_set_new(rootObj, "data", neuronArrayObj);
  json_dump_file(rootObj, (GET_DATA_DIR + "/100k_1m.json").c_str(), JSON_INDENT(2));

  json_decref(rootObj);
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session27/1m_5m/stacked");

  int ravelerHeight = 2599;
  json_t *rootObj = json_object();
  json_t *neuronArrayObj = json_array();

  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    bool isSelected = true;
    if (isSelected) {
      ZObject3dScan *obj = neuron.getBody();
      if (obj != NULL) {
        ZVoxel voxel = obj->getMarker();

        json_t *neuronObj = json_object();
        json_t *idObj = json_integer(neuron.getId());
        json_object_set_new(neuronObj, "id", idObj);


        //voxel.translate(bodyOffset[0], bodyOffset[1], bodyOffset[2]);
        //std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
        json_t *arrayObj = json_array();

        json_array_append_new(arrayObj, json_integer(voxel.x()));

        int y = voxel.y();
        y = ravelerHeight - 1 - voxel.y();
        json_array_append_new(arrayObj, json_integer(y));
        json_array_append_new(arrayObj, json_integer(voxel.z()));

        json_object_set_new(neuronObj, "position", arrayObj);

        json_array_append_new(neuronArrayObj, neuronObj);
      }
      neuron.deprecate(ZFlyEmNeuron::BODY);
    }
  }

  json_object_set_new(rootObj, "data", neuronArrayObj);
  json_dump_file(rootObj, (GET_DATA_DIR + "/1m_5m.json").c_str(), JSON_INDENT(2));

  json_decref(rootObj);
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");

  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  blockArray.print();
  calbr.calibrate(blockArray);

  Print_Cuboid_I(&(blockArray.getBoundBox()));

  blockArray.exportSwc(dataPath + "/flyem/FIB/orphan_body_check_block_13layer.swc");
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session28/100k+/stacked");

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session28/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    if ((size_t) neuron.getId() < allSynapseCount.size()) {
      if (allSynapseCount[neuron.getId()] > 0) {
        if (analyzer.touchingSideBoundary(*neuron.getBody())) {
          selectedNeuronArray.push_back(neuron);
        }
        neuron.deprecate(ZFlyEmNeuron::BODY);
      }
    }
  }

  ZFlyEmNeuronExporter exporter;
  exporter.exportIdVolume(selectedNeuronArray, GET_TEST_DATA_DIR + "/test2.json");

#endif

#if 0
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        GET_DATA_DIR + "/flyem/FIB/data_release/test/data_bundle.json");

  std::vector<ZFlyEmNeuron>& neuronArray = dataBundle.getNeuronArray();
  ZFlyEmQualityAnalyzer analyzer;

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  analyzer.setSubstackRegion(blockArray, calbr);

  ZPointArray pointArray = analyzer.computeHotSpot(neuronArray[0]);

  std::cout << "Number of hotspots: " << pointArray.size() << std::endl;

  ZSwcTree *tree = ZSwcGenerator::createSwc(pointArray, 20);

  tree->save(GET_DATA_DIR + "/test.swc");

  delete tree;

  //misc::exportPointList(GET_DATA_DIR + "/test.json", pointArray);
#endif

#if 0
  ZPointArray pointArray;
  pointArray.append(1, 2, 3);
  pointArray.append(4, 5, 6);
  pointArray.append(4, 5, 6.5);

  std::cout << pointArray.toJsonString() << std::endl;
#endif

#if 0
  ZJsonArray jsonArray;
  jsonArray << 1 << 2 << 3.5;
  std::cout << json_dumps(jsonArray.getValue(), JSON_INDENT(2)) << std::endl;
#endif

#if 0
  ZSwcPruner pruner;
  pruner.setMinLength(18.0);

  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/fork2.swc");
  pruner.prune(&tree);
  tree.save(GET_TEST_DATA_DIR + "/test.swc");

  pruner.setMinLength(100.0);
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/compare/compare1.swc");
  pruner.prune(&tree);
  tree.save(GET_TEST_DATA_DIR + "/test.swc");

  pruner.setMinLength(5000.0);
  tree.load(GET_TEST_DATA_DIR + "/209.swc");
  pruner.prune(&tree);
  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZSquareTaskManager *taskManager = new ZSquareTaskManager;
  for (int i = 0; i < 100; ++i) {
    ZSquareTask *task = new ZSquareTask;
    task->setValue(i + 1);
    taskManager->addTask(task);
  }

  tic();
  taskManager->start();
  taskManager->waitForDone();
  ptoc();

  taskManager->deleteLater();
#endif

#if 0
  ZFlyEmNeuronMatchTaskManager *taskManager = new ZFlyEmNeuronMatchTaskManager;
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        GET_TEST_DATA_DIR + "/flyem/TEM/data_release/bundle1/data_bundle.json");

  ZFlyEmNeuron *source = bundle.getNeuron(209);
  for (std::vector<ZFlyEmNeuron>::iterator iter = bundle.getNeuronArray().begin();
       iter != bundle.getNeuronArray().end(); ++iter) {
    ZFlyEmNeuron *neuron = &(*iter);
    if (neuron != source) {
      ZFlyEmNeuronMatchTask *task = new ZFlyEmNeuronMatchTask;
      task->setSource(source);
      task->setTarget(neuron);
      taskManager->addTask(task);
    }
  }

  taskManager->start();
  taskManager->waitForDone();

  taskManager->deleteLater();
#endif

#if 0
  ZTextLineCompositer compositer;
  compositer.appendLine("Title");
  compositer.appendLine("Headline 1", 1);
  compositer.appendLine("Headline 1.1", 2);
  compositer.appendLine("Headline 2", 1);
  compositer.appendLine("Title 2");
  std::cout << compositer.toString(4) << std::endl;

  ZTextLineCompositer compositer2;
  compositer2.appendLine("All");
  compositer2.appendLine(compositer, 1);
  compositer2.setLevel(1);
  compositer2.print(2);
#endif

#if 0
  FlyEm::ZHotSpot *hotSpot =
      FlyEm::ZHotSpotFactory::createPointHotSpot(1, 2, 3);
  hotSpot->print();

  delete hotSpot;
#endif

#if 0
  ZSwcDeepAngleMetric metric;
  metric.setLevel(3);
  metric.setMinDist(100);

  ZSwcTree tree1;
  ZSwcTree tree2;

  tree1.load(GET_TEST_DATA_DIR + "/flyem/FIB/data_release/bundle5/swc/538772.swc");
  tree2.load(GET_TEST_DATA_DIR + "/flyem/FIB/data_release/bundle5/swc/622288.swc");

  //tree1.load(GET_TEST_DATA_DIR + "/benchmark/swc/dist/angle/tree3.swc");
  //tree2.load(GET_TEST_DATA_DIR + "/benchmark/swc/dist/angle/tree4.swc");

  /*
  Swc_Tree_Node *root = tree2.queryNode(1);
  SwcTreeNode::setAsRoot(root);
  tree2.setDataFromNode(root, ZSwcTree::FREE_WRAPPER);
  */

  double dist = metric.measureDistance(&tree1, &tree2);

  Print_Swc_Tree_Node(metric.getFirstNode());
  Print_Swc_Tree_Node(metric.getSecondNode());

  std::cout << "Distance: " << dist << std::endl;
#endif

#if 0
  ZSwcDeepAngleMetric metric;
  metric.setLevel(3);
  metric.setMinDist(100.0);
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(GET_TEST_DATA_DIR + "/flyem/FIB/data_release/bundle5/data_bundle.json");
  ZFlyEmNeuron *neuron = dataBundle.getNeuron(538772);
  const std::vector<ZFlyEmNeuron>& neuronArray = dataBundle.getNeuronArray();
  FlyEm::ZHotSpotArray hotSpotArray;
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    const ZFlyEmNeuron &buddyNeuron = neuronArray[i];
    if (neuron->getId() != buddyNeuron.getId()) {
      double dist =
          metric.measureDistance(neuron->getModel(), buddyNeuron.getModel());
      if (dist < 1.0) {
        const Swc_Tree_Node *tn = metric.getFirstNode();
        FlyEm::ZHotSpot *hotSpot = new FlyEm::ZHotSpot;
        FlyEm::ZPointGeometry *geometry = new FlyEm::ZPointGeometry;
        geometry->setCenter(
              SwcTreeNode::x(tn), SwcTreeNode::y(tn), SwcTreeNode::z(tn));
        hotSpot->setGeometry(geometry);
        hotSpotArray.append(hotSpot);
      }
    }
  }

  std::cout << hotSpotArray.toString() << std::endl;
#endif

#if 0 //Count orphans and boundary orphans
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session31/0_100000/stacked");
  std::cout << neuronArray.size() << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session31/annotations-synapse.json");

  std::vector<int> allPsdCount = synapseArray.countPsd();

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  //blockArray.print();
  //calbr.calibrate(blockArray);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  int totalCount = 0;
  int totalPsdCount = 0;
  int orphanCount = 0;
  int boundaryOrphanCount = 0;
  int orphanPsdCount = 0;
  int boundaryOrphanPsdCount = 0;

  for (ZFlyEmNeuronArray::const_iterator
       iter = neuronArray.begin(); iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    int psdCount = 0;

    if ((size_t) neuron.getId() < allPsdCount.size()) {
      psdCount = allPsdCount[neuron.getId()];
    }
    if (psdCount > 0) {
      ++totalCount;
      totalPsdCount += psdCount;
      if (!analyzer.touchingGlobalBoundary(*neuron.getBody())) {
        if (analyzer.isStitchedOrphanBody(*neuron.getBody())) {
          boundaryOrphanCount++;
          boundaryOrphanPsdCount += psdCount;
        }
        orphanCount++;
        orphanPsdCount += psdCount;
      }
      neuron.deprecate(ZFlyEmNeuron::ALL_COMPONENT);
    }
  }

  std::cout << "#Bodies with PSDs: " << totalCount << "; #PSDs: "
            << totalPsdCount << std::endl;
  std::cout << "#Orphans:" << orphanCount << "; #PSDs: "
            << orphanPsdCount << std::endl;
  std::cout << "#Boundary orphans:" << boundaryOrphanCount << "; #PSDs: "
            << boundaryOrphanPsdCount << std::endl;

#endif

#if 0
  ZObject3dScanArray bodyArray;
  bodyArray.importDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session31/0_100000/stacked");

  bodyArray.exportHdf5(GET_DATA_DIR + "/test.hf5");

  ZObject3dScan obj;
  obj.importHdf5(GET_DATA_DIR + "/test.hf5", "/body/1.sobj");

  obj.save(GET_DATA_DIR + "/test.sobj");

#endif

#if 0
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        GET_TEST_DATA_DIR + "/flyem/FIB/data_release/bundle5/data_bundle.json");
  dataBundle.getNeuronArray().exportBodyToHdf5(GET_DATA_DIR + "/test.hf5");
#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.importBodyFromHdf5(GET_TEST_DATA_DIR + "/test.hf5", "/bodies/9531.sobj");
  neuron.getBody()->save(GET_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        GET_DATA_DIR +
        "/flyem/FIB/data_release/bundle5/data_bundle_wo_synapse.json");

  ZFlyEmQualityAnalyzer analyzer;

#  if 0
  const std::vector<ZFlyEmNeuron>& neuronArray = dataBundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    const ZFlyEmNeuron &neuron = neuronArray[i];
    FlyEm::ZHotSpotArray &hotSpotArray = analyzer.computeHotSpotForSplit(neuron);
    if (!hotSpotArray.empty()) {
      hotSpotArray.print();
//      neuron.getUnscaledModel()->save(GET_DATA_DIR + "/test.swc");
//      break;
    }
  }
#  endif

  ZFlyEmNeuron *neuron = dataBundle.getNeuron(406309);
  FlyEm::ZHotSpotArray &hotSpotArray = analyzer.computeHotSpotForSplit(*neuron);
  hotSpotArray.print();
  neuron->getUnscaledModel()->save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  ZStackSkeletonizer skeletonizer;

  ZJsonObject config;
  config.load(GET_TEST_DATA_DIR + "/../json/skeletonize.json");
  config.print();
  skeletonizer.configure(config);
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/skeletonization/session31/5000000_/stacked/591796.sobj");
  skeletonizer.print();
  ZSwcTree *tree = skeletonizer.makeSkeleton(obj);
  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(
        GET_DATA_DIR +
        "/flyem/FIB/skeletonization/session31/500000_/len40/data_bundle.json");

  ZFlyEmQualityAnalyzer analyzer;

#  if 0
  const std::vector<ZFlyEmNeuron>& neuronArray = dataBundle.getNeuronArray();
  for (size_t i = 0; i < neuronArray.size(); ++i) {
    const ZFlyEmNeuron &neuron = neuronArray[i];
    FlyEm::ZHotSpotArray &hotSpotArray = analyzer.computeHotSpotForSplit(neuron);
    if (!hotSpotArray.empty()) {
      hotSpotArray.print();
//      neuron.getUnscaledModel()->save(GET_DATA_DIR + "/test.swc");
//      break;
    }
  }
#  else
  ZFlyEmNeuron *neuron = dataBundle.getNeuron(21894);
  FlyEm::ZHotSpotArray &hotSpotArray = analyzer.computeHotSpotForSplit(*neuron);
  hotSpotArray.print();
  //neuron->getUnscaledModel()->save(GET_DATA_DIR + "/test.swc");

  std::cout << hotSpotArray.toJsonString() << std::endl;
#  endif

#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.setId(1);
  neuron.setModelPath(GET_DATA_DIR + "/test.swc");
  ZFlyEmQualityAnalyzer analyzer;
  FlyEm::ZHotSpotArray &hotSpotArray = analyzer.computeHotSpotForSplit(neuron);
  hotSpotArray.print();

#endif

#if 0
  std::cout << misc::computeConfidence(5000, 1000, 10000) << std::endl;
#endif

#if 0
  ZFlyEmCoordinateConverter converter;
  converter.setStackSize(3150, 2599, 6500);
  converter.setVoxelResolution(10, 10, 10);
  converter.setZStart(1490);
  converter.setMargin(10);

  double x = 6600;
  double y = 10040;
  double z = 37600;

  converter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::PHYSICAL_SPACE,
                    ZFlyEmCoordinateConverter::IMAGE_SPACE);
  std::cout << x << " " << y << " " << z << std::endl;

  converter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::IMAGE_SPACE,
                    ZFlyEmCoordinateConverter::ROI_SPACE);
  std::cout << x << " " << y << " " << z << std::endl;

  converter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::ROI_SPACE,
                    ZFlyEmCoordinateConverter::PHYSICAL_SPACE);
  std::cout << x << " " << y << " " << z << std::endl;

  converter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::PHYSICAL_SPACE,
                    ZFlyEmCoordinateConverter::RAVELER_SPACE);
  std::cout << x << " " << y << " " << z << std::endl;

  converter.convert(&x, &y, &z, ZFlyEmCoordinateConverter::RAVELER_SPACE,
                    ZFlyEmCoordinateConverter::ROI_SPACE);
  std::cout << x << " " << y << " " << z << std::endl;
#endif

#if 0
  FlyEm::ZCurveGeometry geometry;
  geometry.appendPoint(0, 1, 2);
  geometry.appendPoint(3, 4, 5);
  geometry.appendPoint(6, 7, 8);
  geometry.print();


  ZPointArray curve;
  curve.append(0, 1, 2);
  curve.append(3, 4, 5);
  curve.append(6, 7, 8);

  FlyEm::ZHotSpotFactory factory;
  FlyEm::ZHotSpot *hotSpot = factory.createCurveHotSpot(curve);
  hotSpot->print();

  delete hotSpot;
  //ZJsonObject obj = hotSpot->toRavelerJsonObject(resolution, imageSize);
  //obj.print();
#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), "91a", 9000);

  std::vector<int> bodyId = reader.readBodyId(1000, 1000, 4500, 100, 100, 100);

  for (size_t i = 0; i < bodyId.size(); ++i) {
    std::cout << bodyId[i] << " ";
  }
  std::cout << std::endl;
#endif

#if 0
  ZDvidReader reader;
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), "91a", 9000);

  int sourceBodyId = 265246;
  ZSwcTree *tree = reader.readSwc(sourceBodyId);
#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), "91a", 9000);

  /*
  ZSwcPruner pruner;
  pruner.setMinLength(1000.0);
  pruner.prune(tree);
  */

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  ZDvidInfo dvidInfo;
  QString info = reader.readInfo("superpixels");
  dvidInfo.setFromJsonString(info.toStdString());

  int sourceBodyId = 117;
  ZSwcTree *tree = reader.readSwc(sourceBodyId);
  ZSwcTree *unscaledTree = tree->clone();
  tree->scale(dvidInfo.getVoxelResolution()[0],
      dvidInfo.getVoxelResolution()[1], dvidInfo.getVoxelResolution()[2]);
  ZFlyEmNeuron neuron(sourceBodyId, tree, NULL);
  neuron.setUnscaledModel(unscaledTree);

  ZSwcTreeNodeArray nodeArray =
      unscaledTree->getSwcTreeNodeArray(ZSwcTree::TERMINAL_ITERATOR);

  double margin = 50;

  std::set<int> bodySet;
  for (ZSwcTreeNodeArray::const_iterator iter = nodeArray.begin();
       iter != nodeArray.end(); ++iter) {
    ZPoint center = SwcTreeNode::pos(*iter);
    std::vector<int> bodyId = reader.readBodyId(
          center.x(), center.y(), center.z(),
          margin, margin, margin);
    std::cout << bodyId.size() << " neighbor bodies" << std::endl;
    bodySet.insert(bodyId.begin(), bodyId.end());
  }

  std::cout << "Retrieving " << bodySet.size() << " neurons ..." << std::endl;


  std::vector<ZFlyEmNeuron> neuronArray(bodySet.size());
  size_t index = 0;
  int neuronRetrievalCount = 0;
  for (std::set<int>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter, ++index) {
    int bodyId = *iter;
    ZSwcTree *tree = reader.readSwc(bodyId);
    ZFlyEmNeuron &neuron = neuronArray[index];
    if (tree != NULL) {
      neuron.setId(bodyId);
      neuron.setUnscaledModel(tree);
      ZSwcTree *tree2 = tree->clone();
      tree2->scale(dvidInfo.getVoxelResolution()[0],
          dvidInfo.getVoxelResolution()[1], dvidInfo.getVoxelResolution()[2]);
      neuron.setModel(tree2);
      ++neuronRetrievalCount;
    } else {
      neuron.setId(-1);
    }
  }

  std::cout << neuronRetrievalCount << " neurons retrieved." << std::endl;

  std::cout << "Computing hot spots ..." << std::endl;
  FlyEm::ZHotSpotArray &hotSpotArray =
      analyzer.computeHotSpot(neuron, neuronArray);
  hotSpotArray.print();
#endif

#if 0
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidReader reader;
  reader.open(eminfo.getDvidAddress().c_str(), "91a", 9000);
  QString info = reader.readInfo("superpixels");

  qDebug() << info;

  ZJsonObject obj;
  obj.decode(info.toStdString());

  obj.print();

  ZDvidInfo dvidInfo;
  dvidInfo.setFromJsonString(info.toStdString());
  dvidInfo.print();
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session33/100000_/stacked.hf5");
  std::cout << neuronArray.size() << " neurons." << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session33/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    if ((size_t) neuron.getId() < allSynapseCount.size()) {
      if (allSynapseCount[neuron.getId()] > 0) {
        if (analyzer.touchingSideBoundary(*neuron.getBody())) {
          selectedNeuronArray.push_back(neuron);
        }
        neuron.deprecate(ZFlyEmNeuron::BODY);
      }
    }
  }

  ZFlyEmNeuronExporter exporter;
  exporter.exportIdVolume(selectedNeuronArray, GET_TEST_DATA_DIR + "/test2.json");
#endif

#if 0
  ZHdf5Reader reader;
  reader.open(GET_DATA_DIR + "/flyem/FIB/skeletonization/session33/100000_/stacked.hf5");

  std::vector<std::string> nameArray = reader.getAllDatasetName("/");
  std::cout << nameArray.size() << std::endl;
  for (std::vector<std::string>::const_iterator iter = nameArray.begin();
       iter != nameArray.end(); ++iter) {
    std::cout << *iter << std::endl;
  }
#endif

#if 0
  std::vector<std::string> pathArray =
      misc::parseHdf5Path(GET_DATA_DIR +
                          "/flyem/FIB/skeletonization/session33/100000_/stacked.hf5");
  ZStringArray::print(pathArray);
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session33/100000_/stacked.hf5");
  std::cout << neuronArray.size() << " neurons." << std::endl;


#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), "339c");

  std::vector<int> bodyId = reader.readBodyId(1000000, 2000000);

  for (size_t i = 0; i < bodyId.size(); ++i) {
    std::cout << bodyId[i] << " ";
  }
  std::cout << std::endl;
#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.setId(117);
  neuron.setModelPath("http:emdata1.int.janelia.org:9000:91a");
  neuron.setVolumePath("http:emdata1.int.janelia.org:9000:91a");

  ZSwcTree *tree2 = neuron.getModel();

  tree2->save(GET_TEST_DATA_DIR + "/test2.swc");

  ZObject3dScan *obj = neuron.getBody();
  ZStackSkeletonizer skeletonizer;

  skeletonizer.configure(
        NeutubeConfig::getInstance().getPath(NeutubeConfig::SKELETONIZATION_CONFIG));
  skeletonizer.print();

  ZSwcTree *tree = skeletonizer.makeSkeleton(*obj);

  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZFlyEmDataBundle dataBundle;
  ZDvidFilter dvidFilter;
  dvidFilter.setMinBodySize(1);
  dvidFilter.setMaxBodySize(100000);
  dataBundle.loadDvid(dvidFilter);
#endif

#if 0
  ZIntCuboidFaceArray faceArray;
  Cuboid_I cuboid;
  Cuboid_I_Set_S(&cuboid, 10, 20, 30, 40, 50, 60);
  faceArray.append(&cuboid);
  ZSwcTree *tree = ZSwcGenerator::createSwc(faceArray, 5.0);
  tree->save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");

  ZIntCuboidFaceArray faceArray = blockArray.getBorderFace();
  ZSwcTree *tree = ZSwcGenerator::createSwc(faceArray, 5.0);
  tree->save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  //Provide x,y,z coordinates (and a body id) for all bodies
  //between 100,000 and 500,0000 in size.

  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session34/100000_/stacked.hf5");


  std::cout << neuronArray.size() << " bodies loaded." << std::endl;

  //QVector<ZObject3dScan> objList(fileList.size());
  json_t *smallObj = json_object();
  json_t *smallDataObj = json_array();

  //json_t *bigObj = json_object();
  //json_t *bigDataObj = json_array();

  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;

    //std::cout << objFile.absoluteFilePath().toStdString().c_str() << std::endl;
    ZObject3dScan *obj = neuron.getBody();
    //obj.load(objFile.absoluteFilePath().toStdString());

    int id = neuron.getId();
    if (obj->isEmpty()) {
      std::cout << "Empty object: " << id << std::endl;
      continue;
    }

    size_t volume = obj->getVoxelNumber();
    if (volume >= 100000 && volume <= 5000000) {
      std::cout << "100k-5M " << id << std::endl;
      ZVoxel voxel = obj->getMarker();
      std::cout << voxel.x() << " " << voxel.y() << " " << voxel.z() << std::endl;
      json_t *arrayObj = json_array();

      TZ_ASSERT(voxel.x() >= 0, "invalid point");

      json_array_append(arrayObj, json_integer(id));

      int vx = voxel.x();
      int vy = 2598 - voxel.y();
      int vz = voxel.z();

      std::cout << "(" << vx << ", " << vy << ", " << vz << ")" << std::endl;

      json_array_append(arrayObj, json_integer(vx));
      json_array_append(arrayObj, json_integer(vy));
      json_array_append(arrayObj, json_integer(vz));
      json_array_append(smallDataObj, arrayObj);
    }

    neuron.deprecate(ZFlyEmNeuron::ALL_COMPONENT);
  }

  json_object_set(smallObj, "data", smallDataObj);
  //json_object_set(bigObj, "data", bigDataObj);

  json_t *metaObj = json_object();

  json_object_set(metaObj, "description", json_string("point list"));
  json_object_set(metaObj, "file version", json_integer(1));

  json_object_set(smallObj, "metadata", metaObj);
  //json_object_set(bigObj, "metadata", metaObj);

  json_dump_file(smallObj, (dataPath + "/100k_5M.json").c_str(), JSON_INDENT(2));
  //json_dump_file(bigObj, (dataPath + "/500k_100k.json").c_str(), JSON_INDENT(2));

#endif

#if 0 //Count orphans
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_DATA_DIR + "/flyem/FIB/skeletonization/session34/100000_/stacked.hf5");
  std::cout << neuronArray.size() << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        dataPath + "/flyem/FIB/skeletonization/session34/annotations-synapse.json");

  std::vector<int> allPsdCount = synapseArray.countPsd();
  std::vector<int> allTbarCount = synapseArray.countTBar();

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  //blockArray.print();
  //calbr.calibrate(blockArray);

  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  int orphanCount = 0;
  int orphanPsdCount = 0;
  int orphanTbarCount = 0;

  for (ZFlyEmNeuronArray::const_iterator
       iter = neuronArray.begin(); iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;

    std::cout << neuron.getId() << std::endl;

    int psdCount = 0;
    int tbarCount = 0;

    if ((size_t) neuron.getId() < allPsdCount.size()) {
      psdCount = allPsdCount[neuron.getId()];
    }
    if ((size_t) neuron.getId() < allTbarCount.size()) {
      tbarCount = allTbarCount[neuron.getId()];
    }

    if (psdCount > 0 || tbarCount > 0) {
      //++totalCount;
      //totalPsdCount += psdCount;
      if (!analyzer.touchingGlobalBoundary(*neuron.getBody())) {
#if 0
        if (analyzer.isStitchedOrphanBody(*neuron.getBody())) {
          boundaryOrphanCount++;
          boundaryOrphanPsdCount += psdCount;
        }
#endif
        orphanCount++;
        orphanPsdCount += psdCount;
        orphanTbarCount += tbarCount;
      }
      neuron.deprecate(ZFlyEmNeuron::ALL_COMPONENT);
    }
  }

  std::cout << "#Orphans >= 100k: " << orphanCount << std::endl;
  std::cout << "#PSDs: " << orphanPsdCount << std::endl;
  std::cout << "#TBars: " << orphanTbarCount << std::endl;

#endif

#if 0 //rescale TEM cells
  QDir dir((GET_DATA_DIR + "/flyem/TEM/All_Cells").c_str());
  QString outputDirPath = (GET_DATA_DIR + "/flyem/TEM/All_Cells_Scaled").c_str();
  QDir outputDir(outputDirPath);

  QFileInfoList subDirList = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
  QStringList nameFilters;
  nameFilters << "*.swc";
  foreach (QFileInfo fileInfo, subDirList) {
    qDebug() << fileInfo.fileName();
    QDir subdir(fileInfo.absoluteFilePath());
    QFileInfoList swcFileInfoList = subdir.entryInfoList(
          nameFilters, QDir::Files);
    QString outputSubDir = outputDirPath + "/" + fileInfo.fileName();
    if (!outputDir.exists(fileInfo.fileName())) {
      outputDir.mkdir(fileInfo.fileName());
    }
    foreach (QFileInfo swcFileInfo, swcFileInfoList) {
      qDebug() << swcFileInfo.fileName();
      QString outputFilePath = outputSubDir + "/" + swcFileInfo.fileName();
      qDebug() << outputFilePath;

      ZSwcTree tree;
      tree.load(swcFileInfo.absoluteFilePath().toStdString());
      tree.rescale(0.031, 0.031, 0.043);
      tree.save(outputFilePath.toStdString());
    }
  }
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/100000_/stacked.hf5");
  std::cout << neuronArray.size() << " neurons." << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    std::cout << neuron.getId() << std::endl;
    if ((size_t) neuron.getId() < allSynapseCount.size()) {
      if (allSynapseCount[neuron.getId()] > 0) {
        if (analyzer.touchingSideBoundary(*neuron.getBody())) {
          selectedNeuronArray.push_back(neuron);
        }
        neuron.deprecate(ZFlyEmNeuron::BODY);
      }
    }
  }

  //ZFlyEmNeuronExporter exporter;
  //exporter.exportIdVolume(selectedNeuronArray, GET_TEST_DATA_DIR + "/side_bounary.json");
#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());

  QByteArray keyValue = reader.readKeyValue("skeletons", "1.swc");

  qDebug() << QString(keyValue);
#endif

#if 0
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZFlyEmDvidReader reader;
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());

  ZFlyEmBodyAnnotation annotation = reader.readAnnotation(117);
  annotation.print();

#endif

#if 0
  QImage image(1024, 1024, QImage::Format_Indexed8);

  tic();
  image.setColor(0, 10);
  ptoc();

  QPixmap pixmap;
  std::cout << pixmap.isDetached() << std::endl;
  pixmap.fromImage(image);
  std::cout << pixmap.isDetached() << std::endl;

  pixmap.detach();
  std::cout << pixmap.isDetached() << std::endl;
#endif

#if 0
  QTimer timer;
  timer.start();
  std::cout << timer.isActive() << std::endl;
  timer.stop();
  std::cout << timer.isActive() << std::endl;
#endif

#if 0
  ZContrastProtocol cp;
  std::cout << cp.hasNoEffect() << std::endl;
  cp.setOffset(-0.3);
  cp.setScale(5.197);
  cp.setNonlinear(ZContrastProtocol::NONLINEAR_SIGMOID);
  for (int i = 0; i < 256; ++i) {
    std::cout << i << " --> " << (int) cp.mapGrey(i) << std::endl;
  }
#endif

#if 0
  //QImage image(1024, 1024, QImage::Format_Mono);
  //QPainter painter(image);
  ZStroke2d stroke;
  stroke.setZ(0);
  stroke.append(50, 50);
  stroke.append(70, 80);
  stroke.setWidth(10);
  stroke.setLabel(200);
  Stack *stack = C_Stack::make(GREY, 100, 100, 1);
  C_Stack::setZero(stack);
  stroke.labelGrey(stack);
  C_Stack::write(GET_DATA_DIR + "/test.tif", stack);
#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session36/annotations-synapse.json");

  std::vector<int> bodyIdArray = reader.readBodyId(0, 100000);

  std::cout << bodyIdArray.size() << std::endl;

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();
  //std::vector<int> allPsdCount = synapseArray.countPsd();
  //std::vector<int> allTbarcount = synapseArray.countTBar();

  ofstream stream((GET_DATA_DIR + "/face_orphan.txt").c_str());

  int count = 0;
  for (std::vector<int>::const_iterator iter = bodyIdArray.begin();
       iter != bodyIdArray.end(); ++iter) {
    int bodyId = *iter;

    if ((size_t) bodyId < allSynapseCount.size()) {
      if (allSynapseCount[bodyId] > 0) {
        ZObject3dScan obj = reader.readBody(bodyId);
        //obj.print();
        if (analyzer.isInternalFaceOrphan(obj)) {
          std::cout << "Body ID: " << bodyId << std::endl;
          stream << bodyId << std::endl;
          ++count;
        }
      }
    }
  }

  stream.close();

  std::cout << count << " internal face orphans." << std::endl;
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/100000_/stacked.hf5");
  std::cout << neuronArray.size() << " neurons." << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    if (neuron.getId() == 605706) {
      std::cout << neuron.getId() << std::endl;
      if ((size_t) neuron.getId() < allSynapseCount.size()) {
        if (allSynapseCount[neuron.getId()] > 0) {
          neuron.getBody()->save(GET_DATA_DIR + "/test.sobj");
          if (analyzer.touchingSideBoundary(*neuron.getBody())) {
            selectedNeuronArray.push_back(neuron);
          }
          neuron.deprecate(ZFlyEmNeuron::BODY);
        }
      }
    }
  }

  //ZFlyEmNeuronExporter exporter;
  //exporter.exportIdVolume(selectedNeuronArray, GET_TEST_DATA_DIR + "/side_bounary.json");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session35/annotations-synapse.json");

  std::vector<int> allPsdCount = synapseArray.countPsd();
  std::vector<int> allTbarCount = synapseArray.countTBar();
  int psdCount = 0;
  int tbarCount = 0;

  ZString line;
  FILE *fp = fopen((GET_DATA_DIR + "/face_orphan.txt").c_str(), "r");
  while(line.readLine(fp)) {
    std::vector<int> bodyIdArray = line.toIntegerArray();
    if (!bodyIdArray.empty()) {
      int bodyId = bodyIdArray[0];
      if ((size_t) bodyId < allPsdCount.size()) {
        psdCount += allPsdCount[bodyId];
      }
      if ((size_t) bodyId < allTbarCount.size()) {
        tbarCount += allTbarCount[bodyId];
      }
    }
  }

  fclose(fp);

  std::cout << "#PSD: " << psdCount << std::endl;
  std::cout << "#Tbar: " << tbarCount << std::endl;
#endif

#if 0
  std::vector<int> bodyIdArray;

  FlyEm::Service::FaceOrphanOverlap service;
  service.loadFace(cuboidArray);
  service.markBody(bodyArray, 1);
  ZGraph *graph = service.computeOverlap();
  std::vector<ZObject3dScan> &objArray = service.getTouchRegion();

  for (size_t i = 0; i < bodyIdArray.size(); ++i) {
    int bodyId = bodyIdArray[i];
    ZObject3dScan &body = bodyArray[i];

  }

#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());

  ZStack *stack = reader.readBodyLabel(2140, 2089, 1500, 1, 500, 500);

  stack->printInfo();

  std::set<FlyEm::TBodyLabel> bodySet;

  size_t voxelNumber = stack->getVoxelNumber();

  FlyEm::TBodyLabel *labelArray =
      (FlyEm::TBodyLabel*) (stack->array8());
  for (size_t i = 0; i < voxelNumber; ++i) {
    bodySet.insert(labelArray[i]);
  }

  std::cout << bodySet.size() << std::endl;

  for (std::set<FlyEm::TBodyLabel>::const_iterator iter = bodySet.begin();
       iter != bodySet.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  return;

  std::vector<int> bodyIdArray =
      reader.readBodyId(1500, 1500, 2500, 100, 100, 10);
  for (std::vector<int>::const_iterator iter = bodyIdArray.begin();
       iter != bodyIdArray.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  Stack *scaled = Scale_Double_Stack(
        (double*) stack->array8(),
        stack->width(), stack->height(), stack->depth(), GREY16);

  C_Stack::write(GET_DATA_DIR + "/test.tif", scaled);


  stack = reader.readGrayScale(1500, 1500, 2500, 100, 100, 10);
  stack->save(GET_DATA_DIR + "/test2.tif");

  delete stack;
#endif

#if 0
  FlyEm::Service::FaceOrphanOverlap service;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidTarget dvidTarget;
  dvidTarget.set(
        eminfo.getDvidAddress(), eminfo.getDvidUuid(), eminfo.getDvidPort());
  service.setDvidTarget(dvidTarget);

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  calbr.calibrate(blockArray);

#if 0
  blockArray.resize(20);
  blockArray.exportSwc(GET_DATA_DIR + "/test.swc");
#endif
  service.loadFace(blockArray);

#  if 1
  service.markBody();

  std::vector<int> orphanBodyArray;
  orphanBodyArray.push_back(34677);
  orphanBodyArray.push_back(236315);
  orphanBodyArray.push_back(66038);
  orphanBodyArray.push_back(67948);
  orphanBodyArray.push_back(625684);
  service.loadFaceOrphanBody(orphanBodyArray);

  service.computeOverlap();

  const std::vector<ZIntPoint> &marker = service.getMarker();
  for (std::vector<ZIntPoint>::const_iterator iter = marker.begin();
       iter != marker.end(); ++iter) {
    std::cout << iter->getX() << " " << iter->getY() << " " << iter->getZ() << std::endl;
  }
#  endif
  service.print();
#endif

#if 0
  ZDvidReader reader;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());
  tic();
  ZObject3dScan obj = reader.readBody(19985);

  obj.getBoundBox().print();

  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZDvidReader reader;
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  reader.open(eminfo.getDvidAddress().c_str(), eminfo.getDvidUuid().c_str(),
              eminfo.getDvidPort());

  ZString line;
  FILE *fp = fopen((GET_DATA_DIR + "/face_orphan.txt").c_str(), "r");
  while(line.readLine(fp)) {
    std::vector<int> bodyIdArray = line.toIntegerArray();
    if (!bodyIdArray.empty()) {
      int bodyId = bodyIdArray[0];
      ZObject3dScan obj = reader.readBody(bodyId);
      if (obj.getBoundBox().lastCorner().z() < 2000) {
        std::cout << bodyId << ": ";
        obj.getBoundBox().print();
      }
    }
  }

  fclose(fp);
#endif

#if 0
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidTarget dvidTarget;
  dvidTarget.set(
        eminfo.getDvidAddress(), eminfo.getDvidUuid(), eminfo.getDvidPort());
  ZDvidReader reader;
  reader.open(dvidTarget);

  tic();
  ZObject3dScan obj = reader.readBody(9);
  //obj.canonize();
  obj.getBoundBox().print();
  obj.getMarker().print();
  ptoc();
#endif

#if 0
  FlyEm::Service::FaceOrphanOverlap service;

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidTarget dvidTarget;
  dvidTarget.set(
        eminfo.getDvidAddress(), eminfo.getDvidUuid(), eminfo.getDvidPort());
  service.setDvidTarget(dvidTarget);

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  calbr.calibrate(blockArray);

#if 0
  blockArray.resize(20);
  blockArray.exportSwc(GET_DATA_DIR + "/test.swc");
#endif
  service.loadFace(blockArray);
  service.markBody();
  service.loadSynapse(GET_DATA_DIR +
                      "/flyem/FIB/skeletonization/session36/annotations-synapse.json");


  std::vector<int> orphanBodyArray;

#  if 1
  ZString line;
  FILE *fp = fopen((GET_DATA_DIR + "/face_orphan.txt").c_str(), "r");
  while(line.readLine(fp)) {
    std::vector<int> bodyIdArray = line.toIntegerArray();
    if (!bodyIdArray.empty()) {
      int bodyId = bodyIdArray[0];
      orphanBodyArray.push_back(bodyId);
    }
  }

  fclose(fp);
#  else
  orphanBodyArray.push_back(34677);
  orphanBodyArray.push_back(236315);
  orphanBodyArray.push_back(66038);
  orphanBodyArray.push_back(67948);
  orphanBodyArray.push_back(625684);
#  endif
  service.loadFaceOrphanBody(orphanBodyArray);

  service.computeOverlap();

  const std::vector<ZIntPoint> &marker = service.getMarker();
  for (std::vector<ZIntPoint>::const_iterator iter = marker.begin();
       iter != marker.end(); ++iter) {
    std::cout << iter->getX() << " " << iter->getY() << " " << iter->getZ() << std::endl;
  }

  ZFlyEmCoordinateConverter converter;
  converter.setStackSize(3150, 2599, 6500);
  converter.setVoxelResolution(10, 10, 10);
  converter.setZStart(1490);
  converter.setMargin(10);

  service.setCoordinateConverter(converter);

  service.exportJsonFile(GET_DATA_DIR + "/test.json");
#endif

#if 0
  Mc_Stack *stack = C_Stack::make(GREY16, 5, 5, 2, 3);
  C_Stack::setOne(stack);
  C_Stack::setZero(stack, 3, 2, 1, 3, 2, 2);
  C_Stack::printValue(stack);
#endif

#if 0
  Stack *stack = C_Stack::make(GREY16, 5, 5, 2);
  C_Stack::setOne(stack);
  Stack *block = C_Stack::make(GREY16, 20, 3, 5);
  C_Stack::setZero(block);
  C_Stack::setBlockValue(stack, block, -1, -1, 0);
  C_Stack::printValue(stack);
#endif

#if 0
  FlyEm::ZSubstackRoi roi;
  roi.importJsonFile(GET_DATA_DIR + "/flyem/FIB/roi.json");

  ZIntCuboidFaceArray faceArray = roi.getCuboidArray().getSideBorderFace();
  faceArray.exportSwc(GET_DATA_DIR + "/flyem/FIB/block_13layer_chop.swc");
#endif

#if 0
  ZFlyEmNeuron neuron;
  neuron.setId(117);
  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidTarget dvidTarget;
  dvidTarget.set(eminfo.getDvidAddress(), eminfo.getDvidUuid(),
                 eminfo.getDvidPort());

  neuron.setVolumePath(dvidTarget.getBodyPath(117));
  ZObject3dScan *body = neuron.getBody();
  std::cout << body->getVoxelNumber() << std::endl;
#endif

#if 0
  FlyEm::ZSubstackRoi roi;
  roi.importJsonFile(GET_DATA_DIR + "/flyem/FIB/roi.json");

  ZIntCuboidFaceArray faceArray = roi.getCuboidArray().getSideBorderFace();

  ZFlyEmDataInfo eminfo(FlyEm::DATA_FIB25);
  ZDvidTarget dvidTarget;
  dvidTarget.set(
        eminfo.getDvidAddress(), eminfo.getDvidUuid(), eminfo.getDvidPort());
  ZDvidReader reader;
  reader.open(dvidTarget);

  ZIntSet sideBodySet;
  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
       iter != faceArray.end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    std::set<int> bodySet =
        reader.readBodyId(face.getCornerCoordinates(0),
                          face.getCornerCoordinates(3));
    std::cout << bodySet.size() << " ids." << std::endl;
    sideBodySet.insert(bodySet.begin(), bodySet.end());
  }
  std::cout << sideBodySet.size() << std::endl;

  ZIntSet largeBodySet = reader.readBodyId(100000, MAX_INT32);
  std::cout << largeBodySet.size() << std::endl;

  sideBodySet.intersect(largeBodySet);
  std::cout << sideBodySet.size() << std::endl;

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session36/annotations-synapse.json");
  std::vector<int> allSynapseCount = synapseArray.countSynapse();
  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZIntSet::const_iterator iter = sideBodySet.begin();
       iter != sideBodySet.end(); ++iter) {
    int bodyId = *iter;
    std::cout << bodyId << std::endl;
    if ((size_t) bodyId < allSynapseCount.size()) {
      if (allSynapseCount[bodyId] > 0) {
        ZFlyEmNeuron neuron;
        neuron.setVolumePath(dvidTarget.getBodyPath(bodyId));
        neuron.setId(bodyId);
        selectedNeuronArray.push_back(neuron);
      }
    }
  }

#if 0

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session34/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  ZFlyEmQualityAnalyzer::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    std::cout << neuron.getId() << std::endl;
    if ((size_t) neuron.getId() < allSynapseCount.size()) {
      if (allSynapseCount[neuron.getId()] > 0) {
        if (analyzer.touchingSideBoundary(*neuron.getBody())) {
          selectedNeuronArray.push_back(neuron);
        }
        neuron.deprecate(ZFlyEmNeuron::BODY);
      }
    }
  }
#endif

  ZFlyEmNeuronExporter exporter;
  exporter.exportIdVolume(selectedNeuronArray,
                          GET_TEST_DATA_DIR + "/side_bounary.json");
#endif

#if 0
  ZIntSet set1;
  set1.insert(1);
  set1.insert(2);
  set1.insert(3);
  /*
  ZIntSet set2;
  set2.insert(1);
  set2.insert(2);
  set1.intersect(set2);
  */

  std::vector<int> set2;
  set2.push_back(1);
  set2.push_back(2);
  set2.push_back(2);
  set2.push_back(4);
  set2.push_back(5);
  set1.intersect(set2);

  set1.print();
#endif

#if 0
  ZStroke2d stroke;
  stroke.append(10, 10);
  //stroke.append(20, 20);
  //stroke.append(21, 30);
  //stroke.append(10, 100);
  stroke.setWidth(20);
  ZStack *stack = stroke.toStack();
  stack->printInfo();

  stack->save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  Stack *stack = C_Stack::make(GREY8, 5, 5, 5);
  C_Stack::setOne(stack);
  for (size_t i = 0; i < C_Stack::voxelNumber(stack); ++i) {
    stack->array[i] = 255-i;
  }
  C_Stack::printValue(stack);

  Stack *out = C_Stack::downsampleMin(stack, 1, 1, 1);
  C_Stack::printValue(out);
#endif

#if 0
  ZStack stack;
  for (size_t i = 0; i < 10; ++i) {
    std::cout << "Loading stack " << i << " ..." << std::endl;

    stack.load(GET_DATA_DIR + "/system/diadem/diadem_e1.tif");
    Biocytin::ZStackProjector proj;
    proj.project(&stack);
  }
#endif

#if 0
  glm::mat4 Projection = glm::perspective(3.14f * 45.0f / 180.f, 1.0f, 0.1f, 100.0f);
  std::cout << Projection << std::endl;
#endif

#if 0
  ZSparseObject obj;
  obj.addSegment(0, 0, 0, 0, false);
  obj.addSegment(0, 1, 0, 1, false);
  obj.addSegment(0, 2, 0, 1, false);
  obj.addSegment(0, 3, 3, 4, false);
  obj.addSegment(0, 0, 1, 1, false);


  obj.translate(1, 2, 3);
  ZStack *stack = new ZStack(GREY, 5, 5, 5, 1);
  stack->setZero();
  stack->setValue(0, 0, 0, 0, 1);
  stack->setValue(0, 1, 0, 0, 2);
  stack->setValue(0, 2, 0, 0, 3);
  stack->setValue(1, 0, 0, 0, 4);
  stack->setValue(3, 3, 0, 0, 5);
  stack->setValue(4, 3, 0, 0, 6);
  stack->setOffset(1, 2, 3);

  obj.setVoxelValue(stack);

  std::cout << obj.getVoxelValue(1, 2, 3) << std::endl;
  std::cout << obj.getVoxelValue(1, 3, 3) << std::endl;
  std::cout << obj.getVoxelValue(1, 4, 3) << std::endl;
  std::cout << obj.getVoxelValue(2, 2, 3) << std::endl;
  std::cout << obj.getVoxelValue(4, 5, 3) << std::endl;
  std::cout << obj.getVoxelValue(5, 5, 3) << std::endl;

  /*
  std::cout << obj.getVoxelValue(0, 0, 0) << std::endl;
  std::cout << obj.getVoxelValue(0, 1, 0) << std::endl;
  std::cout << obj.getVoxelValue(0, 2, 0) << std::endl;
  std::cout << obj.getVoxelValue(1, 0, 0) << std::endl;
  */
#endif

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target = dlg.getDvidTarget();

  ZDvidReader reader;
  reader.open(target);

  qDebug() << reader.readInfo("skeletons");
  ZObject3dScan obj = reader.readBody(117);

  //tic();
  //ZStack *stack = obj.toStackObject();
  //ptoc();


  ZDvidBlockGrid grid;
  grid.setStartIndex(0, 0, 46);
  grid.setEndIndex(98, 81, 250);
  grid.setMinPoint(0, 0, 1490);
  grid.setBlockSize(32, 32, 32);

  tic();
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      int x0 = stripe.getSegmentStart(j);
      int x1 = stripe.getSegmentEnd(j);

      for (int x = x0; x <= x1; ++x) {
        ZDvidBlockGrid::Location location = grid.getLocation(x, y, z);
      }
    }
  }
  ptoc();

#if 0
  int count = 0;
  int ncount = 0;
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    if (stripe.isCanonized()) {
      ++count;
    } else {
      ++ncount;
    }
  }


  std::cout << "Canonized: " << count << std::endl;
  std::cout << "Uncanonized: " << ncount << std::endl;
#endif
  tic();
  ZObject3dScan obj2 = obj;
  ptoc();

  tic();
  obj2.downsampleMax(1, 1, 1);
  ptoc();

  tic();
  obj2.dilate();
  ptoc();



#endif

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target = dlg.getDvidTarget();

  ZDvidReader reader;
  reader.open(target);

  ZDvidInfo dvidInfo;
  dvidInfo.setFromJsonString(reader.readInfo("superpixels").toStdString());
  dvidInfo.print();

  ZObject3dScan obj = reader.readBody(117);

  ZIntPointArray blockArray = dvidInfo.getBlockIndex(obj);;

  std::cout << blockArray.size() << std::endl;

  ZStackBlockGrid grid;
  //grid.setStartIndex(0, 0, 46);
  //grid.setEndIndex(98, 81, 250);
  grid.setMinPoint(0, 0, 1490);
  grid.setBlockSize(32, 32, 32);
  grid.setGridSize(99, 82, 250 - 46 + 1);

  for (ZIntPointArray::const_iterator iter = blockArray.begin();
       iter != blockArray.end(); ++iter) {
    const ZIntPoint blockIndex = *iter - ZIntPoint(0, 0, 46);
    ZIntCuboid box = grid.getBlockBox(blockIndex);
    ZStack *stack = reader.readGrayScale(box);
    grid.consumeStack(blockIndex, stack);
  }

  tic();
  size_t volume = 0;
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      int x0 = stripe.getSegmentStart(j);
      int x1 = stripe.getSegmentEnd(j);

      for (int x = x0; x <= x1; ++x) {
        int v = grid.getValue(x, y, z);
        volume += (v > 0);
      }
    }
  }
  ptoc();

  ZStack *largeStack = grid.toStack();
  largeStack->setZero();
  largeStack->save(GET_DATA_DIR + "/test.tif");
  delete largeStack;

  std::cout << "Volume: " << volume << std::endl;
#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);

  /*
  ZObject3d *obj2 = new ZObject3d;
  obj2->append(1, 2, 3);
  obj2->append(4, 5, 6);
  obj2->append(7, 5, 6);
  obj2->append(4, 9, 6);
  obj2->append(4, 5, 16);
  obj2->append(4, 15, 6);

  obj2->setColor(255, 255, 0, 255);
  obj2->setRole(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);

  Z3DGraph *graphObj = new Z3DGraph;
  graphObj->addNode(Z3DGraphNode(0, 0, 0, 1.0));
  graphObj->addNode(Z3DGraphNode(5, 0, 0, 1.0));
  Z3DGraphEdge edge(0, 1);
  edge.setShape(GRAPH_LINE);
  edge.setWidth(5.0);

  graphObj->addEdge(edge);
  */

  ZCuboid box;
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(1000, 2000, 3000);

  Z3DMainWindow *mainWin = new Z3DMainWindow;
  mainWin->setAttribute(Qt::WA_DeleteOnClose, false);
  Z3DTabWidget *bodyViewers = new Z3DTabWidget(mainWin);
  bodyViewers->setAttribute(Qt::WA_DeleteOnClose, false);
  QSizePolicy sizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
  bodyViewers->setSizePolicy(sizePolicy);

  Z3DGraph *graphObj = Z3DGraphFactory::MakeBox(box, 10.0);

  frame->document()->addObject(graphObj);
//  frame->document()->addObject(obj2);
//  frame->document()->loadSwc(
//        (GET_TEST_DATA_DIR + "/benchmark/swc/fork.swc").c_str());
//  frame->ope;
//  ZWindowFactory::Open3DWindow(frame, Z3DWindow::INIT_EXCLUDE_VOLUME);
  ZWindowFactory *factory = new ZFlyEmBodyWindowFactory;
  factory->setDeleteOnClose(true);

  factory->setDeleteOnClose(true);
  factory->setControlPanelVisible(false);
  factory->setObjectViewVisible(false);
  factory->setVisible(Z3DWindow::LAYER_PUNCTA, false);

  Z3DWindow *window = factory->make3DWindow(frame->document());
  window->setWindowType(neutube3d::EWindowType::TYPE_SKELETON);
  window->readSettings();

  bodyViewers->addWindow(0, window, "test");

  mainWin->show();

  delete frame;
#endif

#if 0
  ZFlyEmBookmarkArray bookmarkArray;
  bookmarkArray.importJsonFile(
        GET_TEST_DATA_DIR + "/flyem/FIB/annotations-bookmarks.json");
  bookmarkArray.print();
#endif

#if 0
  ZStackFrame *frame = new ZStackFrame;
  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZStack *stack = frame->document()->getStack()->clone();
  //stack->setOffset(20, 30, 0);
  ZStackPatch *patch = new ZStackPatch(stack);
  patch->setScale(0.2, 0.2);
  patch->setFinalOffset(20, 30);
  frame->document()->addObject(patch);

  ZStackBall *circle = new ZStackBall(100, 100, 0, 10);
  circle->setColor(255, 0, 0, 255);
  frame->document()->addObject(circle, ZDocPlayer::ROLE_TMP_BOOKMARK);

  ZRect2d *rect = new ZRect2d(30, 40, 100, 200);
  rect->setPenetrating(true);
  rect->setColor(0, 255, 0);
  frame->document()->addObject(rect);
#endif

#if 0
  ZWindowFactory factory;
  factory.setWindowTitle("Test");

  ZStackDoc *doc = new ZStackDoc(NULL, NULL);
  doc->loadFile((GET_TEST_DATA_DIR + "/benchmark/em_stack.tif").c_str());
  Z3DWindow *window = factory.make3DWindow(doc);
  window->show();
  window->raise();
#endif

#if 0
  /*
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target = dlg.getDvidTarget();

  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan obj = reader.readBody(15730);
  */

  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/50.sobj");

  size_t voxelNumber =obj.getVoxelNumber();
  int intv = iround(Cube_Root((double) voxelNumber / 1000000));
  obj.downsampleMax(intv, intv, intv);

  std::cout << obj.getVoxelNumber() << std::endl;

  ZSwcTree *tree = ZSwcGenerator::createSwc(obj);
  ZWindowFactory factory;
  factory.setWindowTitle("Test");

  ZStackDoc *doc = new ZStackDoc(NULL, NULL);
  doc->addSwcTree(tree);
  Z3DWindow *window = factory.make3DWindow(doc);
  window->getSwcFilter()->setRenderingPrimitive("Sphere");
  window->show();
  window->raise();

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/obj1.sobj");

//  while (1) {
    ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj, 2);
    delete tree;
//    std::cout << "Memory usage after: " + ZFlyEmMisc::GetMemoryUsage().toStdString() << std::endl;
//  }

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/_system/big.sobj");
  QElapsedTimer timer;
  timer.start();
  ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj, 2);
  std::cout << timer.elapsed() << "ms" << std::endl;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/50.sobj");

  //size_t voxelNumber =obj.getVoxelNumber();
  //int intv = iround(Cube_Root((double) voxelNumber / 1000000));
 //obj.downsampleMax(intv, intv, intv);

  //std::cout << obj.getVoxelNumber() << std::endl;

  ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj, 2);
  ZWindowFactory factory;
  factory.setWindowTitle("Test");

  ZStackDoc *doc = new ZStackDoc(NULL, NULL);
  doc->addSwcTree(tree);
  Z3DWindow *window = factory.make3DWindow(doc);
  window->getSwcFilter()->setRenderingPrimitive("Sphere");
  window->show();
  window->raise();
#endif

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target = dlg.getDvidTarget();

  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan obj = reader.readBody(15730);

  ZStackSkeletonizer skeletonizer;
  ZJsonObject config;
  config.load(NeutubeConfig::getInstance().getApplicatinDir() +
              "/json/skeletonize.json");
  skeletonizer.configure(config);
  ZSwcTree *tree = skeletonizer.makeSkeleton(obj);

  ZDvidWriter writer;
  writer.open(target);
  writer.writeSwc(15730, tree);
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  ZStackSkeletonizer skeletonizer;
  ZJsonObject config;
  config.load(NeutubeConfig::getInstance().getApplicatinDir() +
              "/json/skeletonize.json");

  for (int i = 0; i < 1000000000; ++i) {
    skeletonizer.configure(config);
    ZSwcTree *tree = skeletonizer.makeSkeleton(obj);

    delete tree;

    std::cout << i << ": " << C_Stack::stackUsage() << std::endl;

  }
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "43f", 9000);

  ZDvidReader reader;
  reader.open(target);

  ZDvidWriter writer;
  writer.open(target);

  std::set<int> bodyIdSet = reader.readBodyId(1000000);
  std::vector<int> bodyIdArray;
  bodyIdArray.insert(bodyIdArray.begin(), bodyIdSet.begin(), bodyIdSet.end());

  ZRandomGenerator rnd;
  std::vector<int> rank = rnd.randperm(bodyIdArray.size());
  std::set<int> excluded;
  excluded.insert(16493);
  excluded.insert(8772496);

  ZStackSkeletonizer skeletonizer;
  ZJsonObject config;
  config.load(NeutubeConfig::getInstance().getApplicatinDir() +
              "/json/skeletonize.json");
  skeletonizer.configure(config);

  for (size_t i = 0; i < bodyIdArray.size(); ++i) {
    int bodyId = bodyIdArray[rank[i] - 1];
    if (excluded.count(bodyId) == 0) {
      ZSwcTree *tree = reader.readSwc(bodyId);
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
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/FIB/data_release/bundle7/thumbnails/117.tif");
  stack.printInfo();

  stack.save(GET_TEST_DATA_DIR + "/test.mraw");

  ZStack stack2;
  stack2.load(GET_TEST_DATA_DIR + "/test.mraw");
  stack2.printInfo();

  stack2.save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZDvidDialog dlg;
  dlg.loadConfig(ZString::fullPath(NeutubeConfig::getInstance().getApplicatinDir(),
                                   "json", "", "flyem_config.json"));

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b42", 9000);

  ZDvidReader reader;
  reader.open(target);

  ZStack *stack = reader.readThumbnail(117);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZJsonObject obj;
  obj.setEntry("first", 1);
  obj.setEntry("second", 2);
  obj.setEntry("third", 3);

  std::string str = obj.dumpString(0);
  std::cout << str << std::endl;
  std::string str2 = ZString(str).replace("\n", " ");
  std::cout << str2 << std::endl;
#endif

#if 0 //update annotation
  std::string annotationFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session40/annotations-body.json";
  std::string bundleFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session40/bundle.json";


  ZFlyEmNeuronArray neuronArray;
  neuronArray.importNamedBody(annotationFile);
  neuronArray.assignClass(bundleFile);

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "43f", 9000);

  ZDvidWriter writer;
  if (writer.open(target)) {
    //for each neuron, update annotation
    for (ZFlyEmNeuronArray::const_iterator iter = neuronArray.begin();
         iter != neuronArray.end(); ++iter) {
      const ZFlyEmNeuron &neuron = *iter;
      writer.writeAnnotation(neuron);
    }
  }

#endif

#if 0//update annotation
  std::string annotationFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session40/annotations-body.json";
  std::string bundleFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session40/bundle.json";


  ZFlyEmNeuronArray neuronArray;
  neuronArray.importFromDataBundle(bundleFile);
  neuronArray.assignName(annotationFile);

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "43f", 9000);

  ZDvidWriter writer;
  if (writer.open(target)) {
    //for each neuron, update annotation
    for (ZFlyEmNeuronArray::const_iterator iter = neuronArray.begin();
         iter != neuronArray.end(); ++iter) {
      const ZFlyEmNeuron &neuron = *iter;
      writer.writeAnnotation(neuron);
    }
  }

#endif

#if 0
  QDialog *dlg = ZDialogFactory::makeTestDialog();

  dlg->exec();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/50.sobj");

  ZFlyEmNeuronImageFactory factory;
  factory.setDownsampleInterval(9, 9, 9);
  tic();
  Stack *stack = factory.createSurfaceImage(obj);
  ptoc();

  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/flyem/test/roi.swc");

  //resample
  tree.resample(1.0);

  tree.save(GET_TEST_DATA_DIR + "/test.swc");

  ZSwcResampler sampler;
  sampler.optimalDownsample(&tree);
  sampler.optimalDownsample(&tree);
  tree.save(GET_TEST_DATA_DIR + "/test2.swc");
#endif

#if 0
  std::string curvePath1 = GET_TEST_DATA_DIR + "/flyem/test/roi1.swc";
  std::string curvePath2 = GET_TEST_DATA_DIR + "/flyem/test/roi2.swc";


  ZSwcTree tree1;
  tree1.load(curvePath1);
  ZSwcTree tree2;
  tree2.load(curvePath2);

  ZClosedCurve curve1 = tree1.toClosedCurve();
  ZClosedCurve curve2 = tree2.toClosedCurve();

  curve1 = curve1.resampleF(500);
  curve2 = curve2.resampleF(500);

  int shift = curve1.findMatchShift(curve2);

  double lambda = 0.5;

  ZClosedCurve curve3 = curve1.interpolate(curve2, lambda, shift);

  ZSwcTree *result = ZSwcGenerator::createSwc(curve3, 5.0);


  result->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZClosedCurve curve;
  curve.append(0, 0, 0);
  curve.append(1, 2, 3);
  ZJsonObject obj = curve.toJsonObject();
  std::cout << obj.dumpString() << std::endl;
#endif

#if 0
//http://emdata1.int.janelia.org:9000/api/node/ba959/roi_curve/0/5000

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "ba959", 9000);
  ZDvidReader reader;
  if (reader.open(target)) {
    QStringList keys = reader.readKeys("roi_curve", "0", "5000");
    qDebug() << keys;
  }
#endif

#if 0

  ZClosedCurve curve1;
  curve1.append(1, 1, 1);
  curve1.append(2, 2, 2);
  ZClosedCurve curve2;
  curve2.append(0, 0, 0);
  curve2.append(2, 2, 2);

  curve1.interpolate(curve2, 2.0, 0).print();
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/curve_test.swc");

  ZClosedCurve curve1;
  ZClosedCurve curve2;

  ZSwcTree::RegularRootIterator treeIterator(&tree);
  for (Swc_Tree_Node *tn = treeIterator.begin(); tn != NULL;
       tn = treeIterator.next()) {
    curve1.append(SwcTreeNode::pos(tn));
    curve2.append(SwcTreeNode::pos(SwcTreeNode::firstChild(tn)));
  }

  curve1.print();
  curve2.print();

  ZSwcTree *tree1 = ZSwcGenerator::createSwc(curve1, 5.0);
  ZSwcTree *tree2 = ZSwcGenerator::createSwc(curve2, 5.0);

  ZClosedCurve curve3 = curve1.interpolate(curve2, 2.0, 5);
  ZSwcTree *tree3 = ZSwcGenerator::createSwc(curve3, 5.0);


  tree.merge(tree1);
  tree.merge(tree2);
  tree.merge(tree3);

  tree.save(GET_TEST_DATA_DIR + "/test2.swc");
#endif

#if 0
  ZSwcTree tree1;
  ZSwcTree tree2;
  tree1.load(GET_TEST_DATA_DIR + "/curve1.swc");
  tree2.load(GET_TEST_DATA_DIR + "/curve2.swc");

  ZClosedCurve lowerCurve;
  ZClosedCurve upperCurve;

  Swc_Tree_Node *tn = tree1.firstRegularRoot();
  while (tn != NULL) {
    upperCurve.append(SwcTreeNode::pos(tn));
    tn = SwcTreeNode::firstChild(tn);
  }

  tn = tree2.firstRegularRoot();
  while (tn != NULL) {
    lowerCurve.append(SwcTreeNode::pos(tn));
    tn = SwcTreeNode::firstChild(tn);
  }


  int sampleNumber = imax3(100, lowerCurve.getLandmarkNumber(),
                           upperCurve.getLandmarkNumber());
  ZClosedCurve curve1 = lowerCurve.resampleF(sampleNumber);
  ZClosedCurve curve2 = upperCurve.resampleF(sampleNumber);

  if (curve1.computeDirection().dot(curve2.computeDirection()) < 0) {
    curve2.reverse();
  }
  int shift = curve1.findMatchShift(curve2);
  ZClosedCurve curve3 = curve1.interpolate(
        curve2, 2.0, shift).resampleF(50);


  ZSwcTree *tree3 = ZSwcGenerator::createSwc(curve3, 5.0);

  ZSwcTree tree;
  tree.merge(&tree1, false);
  tree.merge(&tree2, false);
  tree.merge(tree3);

  tree.save(GET_TEST_DATA_DIR + "/test2.swc");

#endif

#if 0
  QPixmap *pix = new QPixmap(500,500);
  QPainter *paint = new QPainter(pix);
  paint->setPen(*(new QColor(255,34,255,255)));
  paint->drawRect(15,15,100,100);

  pix->save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  ZStroke2d stroke;
  stroke.append(0, 0);
  stroke.append(100, 100);
  stroke.append(-100, 100);
  stroke.append(100, 20);
  stroke.setZ(10);

  ZStack *stack = ZStackFactory::makePolygonPicture(stroke);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/curve_test.swc");

  ZClosedCurve curve1;
  ZClosedCurve curve2;

  ZSwcTree::RegularRootIterator treeIterator(&tree);
  for (Swc_Tree_Node *tn = treeIterator.begin(); tn != NULL;
       tn = treeIterator.next()) {
    curve1.append(SwcTreeNode::pos(tn));
    curve2.append(SwcTreeNode::pos(SwcTreeNode::firstChild(tn)));
  }

  curve1.print();
  curve2.print();

  ZSwcTree *tree1 = ZSwcGenerator::createSwc(curve1, 5.0);
  ZSwcTree *tree2 = ZSwcGenerator::createSwc(curve2, 5.0);

  ZClosedCurve curve3 = curve1.interpolate(curve2, 2.0, 5);
  ZSwcTree *tree3 = ZSwcGenerator::createSwc(curve3, 5.0);


  tree.merge(tree1);
  tree.merge(tree2);
  tree.merge(tree3);

  tree.save(GET_TEST_DATA_DIR + "/test2.swc");
#endif

#if 0
  ZSwcTree tree1;
  tree1.load(GET_TEST_DATA_DIR + "/curve1.swc");

  ZStroke2d curve;

  Swc_Tree_Node *tn = tree1.firstRegularRoot();
  while (tn != NULL) {
    curve.append(SwcTreeNode::x(tn), SwcTreeNode::y(tn));
    tn = SwcTreeNode::firstChild(tn);
  }

  ZStack *stack = ZStackFactory::makePolygonPicture(curve);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");

  ZDvidInfo dvidInfo;
  ZDvidReader reader;

  ZDvidTarget target("emdata1.int.janelia.org", "ba959", 9000);
  if (reader.open(target)) {
    dvidInfo = reader.readGrayScaleInfo();
  }

  std::cout << obj.getVoxelNumber() << std::endl;


  ZObject3dScan blockObj = dvidInfo.getBlockIndex(obj);
  std::cout << blockObj.getVoxelNumber() << std::endl;

  blockObj.save(GET_TEST_DATA_DIR + "/test2.sobj");
#endif

#if 0
  ZDvidInfo dvidInfo;
  ZDvidReader reader;

  ZDvidTarget target("emdata1.int.janelia.org", "ba959", 9000);

  if (reader.open(target)) {
    dvidInfo = reader.readGrayScaleInfo();
  }

  ZObject3dScan blockObj;
  blockObj.load(GET_TEST_DATA_DIR + "/block.sobj");

  int z = 100;
  ZSwcTree *tree = ZSwcGenerator::createSwc(blockObj, z, dvidInfo);

  ZStackDocReader docReader;
  docReader.addSwcTree(tree);
  ZStackFrame *frame = host->createStackFrame(docReader);
  frame->open3DWindow(host);
  delete frame;
#endif

#if 0
  ZDvidInfo dvidInfo;
  ZDvidReader reader;

  ZDvidTarget target("emdata1.int.janelia.org", "ba959", 9000);
  if (reader.open(target)) {
    dvidInfo = reader.readGrayScaleInfo();
  }

  for (int z = dvidInfo.getMinZ(); z <= dvidInfo.getMaxZ(); ++z) {
    std::cout << "PLane: " << z << std::endl;
    ZStack *stack = reader.readGrayScale(
          dvidInfo.getStartCoordinates().getX(),
          dvidInfo.getStartCoordinates().getY(),
          z, dvidInfo.getStackSize()[0],
        dvidInfo.getStackSize()[1], 1);

    if (stack != NULL) {
      ZIntCuboid boundBox = ZFlyEmRoiProject::estimateBoundBox(*stack);
      delete stack;
      ZDvidWriter writer;
      if (writer.open(target)) {
        writer.writeBoundBox(boundBox, z);
      }
    }
  }
#endif

#if 0
  ZStack *stack = ZStackFactory::makeZeroStack(3, 3, 3, 3);
  for (int c = 0; c < 3; ++c) {
    for (int z = 0; z < 3; ++z) {
    for (int y = 0; y < 3; ++y) {
      for (int x = 0; x < 3; ++x) {
        stack->setIntValue(x, y, z, c, c * 100);
      }
    }
    }
  }

  stack->setIntValue(1, 1, 0, 0, 255);
  stack->setIntValue(1, 1, 0, 1, 255);
  stack->setIntValue(1, 1, 0, 2, 255);

  stack->save(GET_DATA_DIR + "/color_test2.tif");

#endif

#if 0
  QUndoStack *stack = new QUndoStack(host);
  stack->push(new QUndoCommand("1"));
  stack->push(new QUndoCommand("2"));

  qDebug() << stack->count();
  qDebug() << stack->index();

  stack->undo();;
  qDebug() << stack->count();
  qDebug() << stack->index();


  const QUndoCommand *command = stack->command(stack->index() - 1);
  qDebug() << command->text();

#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 2, 3, 5);
  obj.addSegment(0, 2, 8, 9);
  obj.addSegment(1, 2, 3, 4);

  ZJsonArray array = ZJsonFactory::makeJsonArray(obj);
  std::cout << array.dumpString(0) << std::endl;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/test.sobj");
  ZJsonArray array = ZJsonFactory::makeJsonArray(obj);
  ZJsonObject headObj;
  headObj.setEntry("data", array);
  headObj.dump(GET_DATA_DIR + "/test.json");

#endif

#if 0
  ZParameterArray paramArray;

  ZParameter *param = new ZIntParameter("value1", 1, 0, 255);
  paramArray.append(param);

  param = new ZStringParameter("value2", "first string");
  paramArray.append(param);

  ZOptionParameter<QString> *optParam = new ZOptionParameter<QString>("value3");
  optParam->addOption("first option");
  optParam->addOption("second option");
  paramArray.append(optParam);


  QDialog *dlg = ZDialogFactory::makeParameterDialog(paramArray, host);

  dlg->exec();
#endif

#if 0
  QObject *parent = new QObject();
  QPointer<QObject> child(new QObject(parent));
  delete parent;

  if (child) {
    std::cout << "object alive" << std::endl;
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_DATA_DIR + "/benchmark/swc/mouse_single_org.swc");
  double sxy = ZSwcGlobalFeatureAnalyzer::computeLateralSpan(tree);
  double sz = ZSwcGlobalFeatureAnalyzer::computeVerticalSpan(tree);

  std::cout << sxy << " " << sz << std::endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  //synapseArray.loadJson(GET_DATA_DIR + "/flyem/MB/synapses0725.json");
  synapseArray.loadJson(GET_DATA_DIR + "/flyem/FIB/FIB25/annotations-synapse.json");
  FlyEm::SynapseAnnotationConfig annotationConfig;
  annotationConfig.swcDownsample1 = 0;
  annotationConfig.swcDownsample2 = 0;
  annotationConfig.sizeScale = 10.0;

  FlyEm::SynapseDisplayConfig displayConfig;
  displayConfig.mode = FlyEm::SynapseDisplayConfig::TBAR_ONLY;

  synapseArray.exportMarkerFile(GET_DATA_DIR + "/flyem/FIB/FIB25/tbar.marker",
                                annotationConfig,
                                FlyEm::SynapseLocation::CURRENT_SPACE,
                                displayConfig);

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR + "/flyem/MB/MB6_TbarPredict_Global_fixed_y.json");

  int count = 0;
  std::string filePath = GET_DATA_DIR + "/flyem/MB/MB6_TbarPredict_Global_fixed_y.marker";
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
  } else {
    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 0;
    displayConfig.tBarColor.blue = 0;

    for (FlyEm::SynapseLocation *synapse = synapseArray.beginSynapseLocation();
         synapse != NULL; synapse = synapseArray.nextSynapseLocation()) {
      if (synapse->isTBar()) {
        stream << synapse->x() << ',' << synapse->y() << ',' << synapse->z()
               << ',' << "3,1,,,"
               << static_cast<int>(displayConfig.tBarColor.red) << ","
               << static_cast<int>(displayConfig.tBarColor.green) << ","
               << static_cast<int>(displayConfig.tBarColor.blue) << "," << endl;
        ++count;
      }
    }

    stream.close();
  }

  std::cout << count << " tbars found." << std::endl;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR + "/flyem/MB/mb06-tbar-predict-dvid_0.78.json");

  std::string roiObjPath = GET_DATA_DIR + "/flyem/MB/mb_roi.sobj";
  //Get ROI
  ZObject3dScan obj;
  obj.load(roiObjPath);

  //Get ROI stack
  ZStack *stack = obj.toStackObject();

  std::string filePath = GET_DATA_DIR + "/flyem/MB/mb06-tbar-predict-dvid_0.78.roi.marker";
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
  } else {
    FlyEm::SynapseDisplayConfig displayConfig;

    for (FlyEm::SynapseLocation *synapse = synapseArray.beginSynapseLocation();
         synapse != NULL; synapse = synapseArray.nextSynapseLocation()) {
      if (synapse->isTBar()) {
        int x = synapse->x() / 32;
        int y = synapse->y() / 32;
        int z = synapse->z() / 32;
        if (stack->getIntValue(x, y, z) > 0) {
          stream << synapse->x() << ',' << synapse->y() << ',' << synapse->z()
                 << ',' << "3,1,,,"
                 << static_cast<int>(displayConfig.tBarColor.red) << ","
                 << static_cast<int>(displayConfig.tBarColor.green) << ","
                 << static_cast<int>(displayConfig.tBarColor.blue) << "," << endl;
        }
      }
    }

    stream.close();
  }
  delete stack;
//  FlyEm::SynapseAnnotationConfig annotationConfig;
//  annotationConfig.swcDownsample1 = 0;
//  annotationConfig.swcDownsample2 = 0;
//  annotationConfig.sizeScale = 10.0;

//  FlyEm::SynapseDisplayConfig displayConfig;
//  displayConfig.mode = FlyEm::SynapseDisplayConfig::TBAR_ONLY;




/*
  synapseArray.exportMarkerFile(GET_DATA_DIR + "/flyem/MB/mb06-tbar-predict-dvid_0.78.block.marker",
                                annotationConfig,
                                FlyEm::SynapseLocation::CURRENT_SPACE,
                                displayConfig);
                                */

#endif

#if 0
  ZDvidBufferReader reader;

  qDebug() << reader.isReadable("http://emdata2.int.janelia.org:9000/api/node/43f/annotations/117");

  tic();
  reader.read("http://emdata2.int.janelia.org:9000/api/node/43f/annotations/117");
  ptoc();

  if (reader.getStatus() == ZDvidBufferReader::READ_OK) {
    std::cout << "Reading succeeded" << std::endl;
    std::cout << reader.getBuffer().size() << " bytes read in." << std::endl;
    qDebug() << reader.getBuffer();
  } else {
    std::cout << "Reading failed" << std::endl;
  }

#endif

#if 0
  ZDvidClient dvidClient;
  dvidClient.setServer("emdata2.int.janelia.org", 9000);
  dvidClient.setUuid("fa9");
  dvidClient.postRequest(ZDvidRequest::DVID_UPLOAD_SWC, 0);
#endif

#if 0
  ZMatrix mat;
  mat.importTextFile(GET_DATA_DIR + "/flyem/AL/AL_Tbars_xyz.txt");

  int count = 0;
  std::string filePath = GET_DATA_DIR + "/flyem/AL/AL_Tbars.marker";
  ofstream stream(filePath.c_str());
  if (!stream.is_open()) {
    cout << "Failed to open " << filePath << endl;
  } else {
    FlyEm::SynapseDisplayConfig displayConfig;
    displayConfig.tBarColor.red = 255;
    displayConfig.tBarColor.green = 0;
    displayConfig.tBarColor.blue = 0;

    for (int i = 0; i < mat.getRowNumber(); ++i) {
      stream << mat.at(i, 0) << ',' << mat.at(i, 1) << ',' << mat.at(i, 2)
             << ',' << "10,1,,,"
             << static_cast<int>(displayConfig.tBarColor.red) << ","
             << static_cast<int>(displayConfig.tBarColor.green) << ","
             << static_cast<int>(displayConfig.tBarColor.blue) << "," << endl;
      ++count;
    }

    stream.close();
  }

  std::cout << count << " tbars found." << std::endl;
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target("emdata.janelia.org", "bf1");
  std::cout << reader.open(target) << std::endl;
#endif

#if 0
#ifdef _ENABLE_LIBDVID_
  ZDvidTarget target =
      NeutubeConfig::getInstance().getFlyEmConfig().getDvidRepo().at(0);
  libdvid::DVIDServer server(target.getAddressWithPort());
  libdvid::DVIDNode dvidNode(server, target.getUuid());

  libdvid::BinaryDataPtr value;
  dvidNode.get("skeletons", "1.swc", value);
#endif
#endif

#if 0
  ZDvidTarget target("emdata2.int.janelia.org", "faa");
  ZDvidWriter writer;
  if (writer.open(target)) {
    writer.createData("labels64", "split");
  }
#endif

#if 0
  ZFlyEmNeuronImageFactory factory;
  factory.setDownsampleInterval(2, 2, 2);

  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/benchmark/29.sobj");
  tic();
  Stack *stack = factory.createSurfaceImage(obj);
  ptoc();

  C_Stack::write(GET_DATA_DIR + "/test.tif", stack);
#endif


#if 0
  ZStackFactory factory;
  ZPointArray ptArray;
  ptArray.importTxtFile(GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.86.txt");
  int dsScale = 5;
  for (ZPointArray::iterator iter = ptArray.begin(); iter != ptArray.end();
       ++iter) {
    *iter *= 1.0 / dsScale;
  }
//  ptArray.append(5, 5, 5);
//  ptArray.append(300, 30, 30);
  ZStack *stack = factory.makeDensityMap(ptArray, 15.0);
  /*
  Stack *stack2 = Scale_Double_Stack(
        stack->array64(), stack->width(), stack->height(), stack->depth(), GREY);
        */
  //C_Stack::write(GET_DATA_DIR + "/test.tif", stack2);

  stack->save(GET_DATA_DIR + "/test.tif");
#  if 0
  //Get grayscale
  ZStack *grayScale = factory.makeZeroStack(stack->getBoundBox());

  ZDvidTarget target("emdata2.int.janelia.org", "134", -1);
  ZDvidReader reader;
  int width = grayScale->width() * dsScale;
  int height = grayScale->height() * dsScale;
  int x0 = grayScale->getOffset().getX() * dsScale;
  int y0 = grayScale->getOffset().getY() * dsScale;

  if (reader.open(target)) {
    for (int z = 0; z < grayScale->depth(); ++z) {
      int z0 = (z + grayScale->getOffset().getZ()) * dsScale;
      ZStack *stack = reader.readGrayScale(x0, y0, z0, width, height, 1);
      stack->downsampleMin(dsScale - 1, dsScale - 1, dsScale - 1);
      stack->paste(grayScale);
    }
  }

  grayScale->save(GET_DATA_DIR + "/test2.tif");

  delete stack;
  delete grayScale;
#  endif
#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/seg_ds9.tif");
  double sigma = 5.0;
  Filter_3d *filter = Gaussian_Filter_3d(sigma, sigma, sigma);
  Stack *stack2Data = Filter_Stack(stack.c_stack(), filter);

  ZStack stack2;
  stack2.consume(stack2Data);

  Kill_FMatrix(filter);

  stack2.save(GET_DATA_DIR + "/flyem/AL/seg_ds9_smoothed.tif");
#endif

#if 0
  //Assign color
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/test/seg_ds9_smoothed_color.tif");
  //stack.setOffset(400, 1350, 450);
  stack.setOffset(357, 1233, 397);
  stack.printInfo();

  ZPointArray ptArray;
  ptArray.importTxtFile(GET_DATA_DIR + "/flyem/AL/AL_Tbars_xyz.txt");
  double dsScale = 5;
  std::vector<ZVaa3dMarker> markerArray;

  for (ZPointArray::iterator iter = ptArray.begin(); iter != ptArray.end();
       ++iter) {
    ZPoint pt = *iter;
    pt *= 1.0 / dsScale;
    ZIntPoint pos = pt.toIntPoint();
    int red = stack.getIntValue(pos.getX(), pos.getY(), pos.getZ(), 0);
    int green = stack.getIntValue(pos.getX(), pos.getY(), pos.getZ(), 1);
    int blue = stack.getIntValue(pos.getX(), pos.getY(), pos.getZ(), 2);

    ZVaa3dMarker marker;
    marker.setCenter(pt.x(), pt.y(), pt.z());
    marker.setColor(red, green, blue);
    marker.setRadius(5);
    markerArray.push_back(marker);
  }

  FlyEm::ZFileParser::writeVaa3dMakerFile(
        GET_DATA_DIR + "/test.marker", markerArray);


//    *iter *= 1.0 / dsScale;
//    stack
//  }

#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/test/densitymap.tif");
  stack.printInfo();

  ZStack grayStack;
  grayStack.load(GET_DATA_DIR + "/flyem/AL/test/grayscale.tif");

  for (int c = 0; c < stack.channelNumber(); ++c) {
    ZStack *channelStack = stack.getSingleChannel(c);
    grayStack.paste(channelStack, -1, 0.8);
    delete channelStack;
  }
  stack.save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStack *stack = ZStackFactory::makeZeroStack(ZIntCuboid(1, 2, 3, 4, 5, 6));
  stack->save(GET_DATA_DIR + "/test.tif");

  ZStack stack2;
  stack2.load(GET_DATA_DIR + "/test.tif");
  stack2.printInfo();
#endif

#if 0
  ZDvidTarget dvidTarget("emdata2.int.janelia.org", "134", -1);
  dvidTarget.setLocalFolder(GET_TEST_DATA_DIR +
                         "/Users/zhaot/Work/neutube/neurolabi/data/flyem/AL");

  ZDvidReader reader;
  if (reader.open(dvidTarget)) {
    for (size_t i = 0; i < 10000; ++i) {
      std::cout << "Iter: " << i << std::endl;
      ZStack *stack = reader.readGrayScale(0, 0, 3000, 10240, 10240, 1);
      stack->printInfo();
      ZStackDocReader docReader;
      docReader.setStack(stack);
      ZStackFrame *frame = host->createStackFrame(
            docReader, NeuTube::Document::FLYEM_ROI);
      delete frame;
    }
  }
#endif

#if 0
  ZPointArray ptArray;
  ptArray.importTxtFile(GET_DATA_DIR + "/flyem/AL/AL_Tbars_xyz.txt");

  ZGraph *graph = ptArray.computeDistanceGraph(200);

  graph->printInfo();

  graph->exportTxtFile(GET_DATA_DIR + "/flyem/AL/graph.txt");

  delete graph;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.86.json");
  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.86.txt").c_str());
  std::vector<ZPunctum*> puncta = synapseArray.toTBarPuncta(10.0);
  for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
       iter != puncta.end(); ++iter) {
    const ZPunctum *punctum = *iter;
    stream << punctum->x() << " " << punctum->y() << " " << punctum->z()
           << std::endl;
  }
  stream.close();
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_DATA_DIR + "/flyem/FIB/FIB25/annotations-synapse.json");
  ofstream stream(
        (GET_DATA_DIR + "/flyem/FIB/FIB25/annotations-synapse.csv").c_str());
  std::vector<ZPunctum*> puncta = synapseArray.toTBarPuncta(10.0);
  for (std::vector<ZPunctum*>::const_iterator iter = puncta.begin();
       iter != puncta.end(); ++iter) {
    const ZPunctum *punctum = *iter;
    stream << punctum->x() << "," << punctum->y() << "," << punctum->z()
           << std::endl;
  }
  stream.close();
#endif

#if 0
  ZPointArray ptArray;
  ptArray.importTxtFile(GET_DATA_DIR + "/flyem/AL/synapse_cleaned.txt");

  ZGraph *graph = ptArray.computeDistanceGraph(30);

  std::set<int> removeSet;
  for (int edgeIndex = 0; edgeIndex < graph->getEdgeNumber(); ++edgeIndex) {
    if (graph->getEdgeWeight(edgeIndex) < 10) {
      removeSet.insert(graph->getEdgeEnd(edgeIndex));
    }
  }

  ZPointArray newPtArray;

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/synapse_cleaned.txt").c_str());

  for (size_t i = 0; i < ptArray.size(); ++i) {
    if (removeSet.count((int) i) == 0) {
      newPtArray.push_back(ptArray[i]);
      stream << ptArray[i].x() << " " << ptArray[i].y() << " "
             << ptArray[i].z() << std::endl;
    }
  }
  stream.close();

//  graph->printInfo();

//  graph->exportTxtFile(GET_DATA_DIR + "/flyem/AL/graph.txt");

//  delete graph;
#endif

#if 0
  ZStackFactory factory;

  std::string synapseFile = GET_DATA_DIR + "/flyem/AL/synpase_labeled4.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZWeightedPointArray ptArray;

  double dsScale = 5.0;

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 4) {
      double weight = 1.0;
      if (pt[3] > 0) {
        weight = 3.0;
      }
      ptArray.append(pt[0] / dsScale, pt[1] / dsScale, pt[2] / dsScale, weight);
    }
  }

  ZStack *stack = factory.makeDensityMap(ptArray, 15.0);

  stack->save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  std::string synapseFile = GET_DATA_DIR + "/flyem/AL/synpase_labeled4.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZWeightedPointArray ptArray;

  double dsScale = 5.0;

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 4) {
      double weight = 1.0;
      if (pt[3] > 0) {
        weight = 3.0;
      }
      ptArray.append(pt[0], pt[1], pt[2], weight);
    }
  }

  fclose(fp);

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/seg_ds5_v3.tif");

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/synpase_labeled4_processed.txt").c_str());
  for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const ZPoint &pt = *iter;
    ZIntPoint dsPt = (pt * (1.0 / dsScale)).toIntPoint();
    int v = stack.getIntValue(dsPt.getX(), dsPt.getY(), dsPt.getZ());
    stream << pt.x() << " " << pt.y() << " " << pt.z() << " " << v << std::endl;
  }
  stream.close();
#endif

#if 0
  std::string annotationFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session41/annotations-body.json";
  std::string bundleFile = GET_DATA_DIR +
      "/flyem/FIB/skeletonization/session41/bundle.json";

  ZFlyEmNeuronArray neuronArray;
  neuronArray.importNamedBody(annotationFile);
//  neuronArray.assignClass(bundleFile);

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6c", -1);

  ZDvidWriter writer;
  if (writer.open(target)) {
    //for each neuron, update annotation
    for (ZFlyEmNeuronArray::const_iterator iter = neuronArray.begin();
         iter != neuronArray.end(); ++iter) {
      const ZFlyEmNeuron &neuron = *iter;
      writer.writeAnnotation(neuron);
    }
  }
#endif

#if 0
  ZFlyEmNeuronArray neuronArray;
  neuronArray.importBodyDir(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session28/100k+/stacked");

  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(
        GET_TEST_DATA_DIR +
        "/flyem/FIB/skeletonization/session28/annotations-synapse.json");

  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(GET_TEST_DATA_DIR + "/flyem/FIB/block_13layer.txt");
  FlyEm::SubstackRegionCalbration calbr;
  calbr.setBounding(true, true, false);
  calbr.setMargin(10, 10, 0);
  ZFlyEmQualityAnalyzer analyzer;
  analyzer.setSubstackRegion(blockArray, calbr);

  std::vector<int> allSynapseCount = synapseArray.countSynapse();

  ZFlyEmNeuronArray selectedNeuronArray;
  for (ZFlyEmNeuronArray::iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    ZFlyEmNeuron &neuron = *iter;
    if ((size_t) neuron.getId() < allSynapseCount.size()) {
      if (allSynapseCount[neuron.getId()] > 0) {
        if (analyzer.touchingSideBoundary(*neuron.getBody())) {
          selectedNeuronArray.push_back(neuron);
        }
        neuron.deprecate(ZFlyEmNeuron::BODY);
      }
    }
  }

  ZFlyEmNeuronExporter exporter;
  exporter.exportIdVolume(selectedNeuronArray, GET_TEST_DATA_DIR + "/test2.json");

#endif

#if 0
  FlyEm::ZSubstackRoi roi;
  roi.importJsonFile(
        GET_DATA_DIR + "/flyem/FIB/FIB25/extended_roi_substack.json");

  ZIntCuboidFaceArray faceArray = roi.getCuboidArray().getSideBorderFace();
  ZSwcTree *tree = ZSwcGenerator::createSwc(faceArray, 5.0);

  tree->save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  ZStackFactory factory;

  std::string synapseFile =
      GET_DATA_DIR + "/flyem/AL/synpase_labeled4_processed.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZWeightedPointArray ptArray;

  double dsScale = 5.0;

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 4) {
      double weight = pt[3];
      ptArray.append(pt[0] / dsScale, pt[1] / dsScale, pt[2] / dsScale, weight);
    }
  }

  ZStack *stack = factory.makeSeedStack(ptArray);

  stack->save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStackFactory factory;

  std::string synapseFile =
      GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.81.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZWeightedPointArray ptArray;

  double dsScale = 5.0;

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 3) {
      double weight = 1.0;
      ptArray.append(pt[0] / dsScale, pt[1] / dsScale, pt[2] / dsScale, weight);
    }
  }

  ZStack *stack = factory.makeDensityMap(ptArray, 15.0);
  stack->save(GET_DATA_DIR + "/test2.tif");
#endif


#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "2b6c");

  ZDvidReader reader;
  reader.open(dvidTarget);

  std::string substackPath =
      GET_DATA_DIR + "/flyem/FIB/FIB25/extended_roi_substack.json";

  FlyEm::ZSubstackRoi roi;
  roi.importJsonFile(substackPath);

  ZIntCuboidFaceArray faceArray = roi.getCuboidArray().getSideBorderFace();

//  ZSwcTree *tree = ZSwcGenerator::createSwc(faceArray, 20.0);
//  tree->save(GET_DATA_DIR + "/test.swc");

//  ZIntSet sideBodySet;
//  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
//       iter != faceArray.end(); ++iter) {
//    const ZIntCuboidFace &face = *iter;
//    std::set<int> bodySet =
//        reader.readBodyId(face.getCornerCoordinates(0),
//                          face.getCornerCoordinates(3));
//    sideBodySet.insert(bodySet.begin(), bodySet.end());
//  }

  ZIntSet sideBodySet = reader.readBodyId(ZIntPoint(5507, 2259, 1000),
                                          ZIntPoint(5507, 2758, 1499));

  std::cout << sideBodySet.size() << " bodies" << std::endl;
  for (ZIntSet::const_iterator iter = sideBodySet.begin();
       iter != sideBodySet.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

  std::cout << "Hit: " << sideBodySet.count(30155) << std::endl;
#endif

#if 0
  std::string synapseFile =
      GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.86.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZWeightedPointArray ptArray;

  double dsScale = 5.0;

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 3) {
      double weight = 1.0;
      if (pt[3] > 0) {
        weight = 3.0;
      }
      ptArray.append(pt[0], pt[1], pt[2], weight);
    }
  }

  fclose(fp);

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/al7-origroi-tbar-predict_0.86.tif");

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/cluster_predict_0.86.txt").c_str());
  for (ZWeightedPointArray::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const ZPoint &pt = *iter;
    ZIntPoint dsPt = (pt * (1.0 / dsScale)).toIntPoint();
    int v = stack.getIntValue(dsPt.getX(), dsPt.getY(), dsPt.getZ());
    stream << pt.x() << " " << pt.y() << " " << pt.z() << " " << v << std::endl;
  }
  stream.close();
#endif

#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "2b6c");

  ZFlyEmDvidReader reader;
  if (reader.open(dvidTarget)) {
    QStringList synapseList = reader.readSynapseList();
    qDebug() << synapseList;
  }
#endif

#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "2b6c");

  ZFlyEmDvidReader reader;
  if (reader.open(dvidTarget)) {
    QStringList synapseList = reader.readSynapseList();
    qDebug() << synapseList;

    ZJsonObject obj = reader.readSynapseAnnotation(synapseList[0]);
    obj.print();
  }
#endif

#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "134");

  ZDvidReader reader;
  reader.open(dvidTarget);

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

  ZSwcTree *tree;
  ZSwcGenerator generator;
  ZIntCuboid cuboid;
  cuboid.setFirstCorner(dvidInfo.getStartCoordinates());
  cuboid.setSize(dvidInfo.getStackSize()[0], dvidInfo.getStackSize()[1],
      dvidInfo.getStackSize()[2]);
  tree = generator.createBoxSwc(cuboid);

  tree->save(GET_DATA_DIR + "/test.swc");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/FIB/FIB25/annotations-synapse.json");

  ZDvidReader reader;
  ZDvidTarget target;
  target.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c");

  if (reader.open(target)) {
    ZDvidInfo info = reader.readGrayScaleInfo();
    synapseArray.convertRavelerToImageSpace(0, info.getStackSize()[1]);
    ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
    ZCuboid box = ptArray.getBoundBox();
    box.print();

    ZStackFactory factory;
    int dsScale = 10;
    for (ZWeightedPointArray::iterator iter = ptArray.begin();
         iter != ptArray.end(); ++iter) {
      *iter *= 1.0 / dsScale;
    }

    ZStack *stack = factory.makeDensityMap(ptArray, 15.0);
    stack->save(GET_DATA_DIR + "/flyem/FIB/FIB25/density_map_ds10.tif");
  }
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/FIB/FIB25/annotations-synapse.json");

  ZDvidReader reader;
  ZDvidTarget target;
  target.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c");
  double dsScale = 10.0;

  ZStack seg;
  seg.load(GET_DATA_DIR + "/flyem/FIB/FIB25/seg_ds10.tif");

  ofstream stream((GET_DATA_DIR + "/test.txt").c_str());

  if (reader.open(target)) {
    ZLabelColorTable colorTable;
    ZDvidInfo info = reader.readGrayScaleInfo();
    synapseArray.convertRavelerToImageSpace(0, info.getStackSize()[1]);
    std::vector<ZPunctum*> puncta = synapseArray.toTBarPuncta(10.0);

    std::vector<ZVaa3dMarker> markerArray;
    for (std::vector<ZPunctum*>::iterator iter = puncta.begin();
         iter != puncta.end(); ++iter) {
      ZPunctum *p = *iter;
      ZIntPoint originalCenter = p->getCenter().toIntPoint();

      p->scaleCenter(1 / dsScale, 1 / dsScale, 1 / dsScale);
      ZIntPoint center = p->getCenter().toIntPoint();
      int v = seg.getIntValue(center.getX(), center.getY(), center.getZ());

      if (v > 7) {
        v = 0;
      }

      stream << originalCenter.getX() << " " << originalCenter.getY() << " "
             << originalCenter.getZ() << " " << v << std::endl;

      //p->setSource(ZString::num2str(v));
      p->setColor(colorTable.getColor(v));
      p->scaleCenter(dsScale, dsScale, dsScale);
      markerArray.push_back(p->toVaa3dMarker());
    }

    FlyEm::ZFileParser::writeVaa3dMakerFile(GET_DATA_DIR + "/test.marker",
                                            markerArray);
  }

#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c");
  reader.open(target);
  //column string

  std::vector<std::string> columnList;
  columnList.push_back("H");
  columnList.push_back("a");
  columnList.push_back("b");
  columnList.push_back("c");
  columnList.push_back("d");
  columnList.push_back("e");
  columnList.push_back("f");
  columnList.push_back("o");

  //string->label map
  std::map<std::string, int> columnLabelMap;
  for (size_t i = 0; i < columnList.size(); ++i) {
    columnLabelMap[columnList[i]] = i + 1;
  }
  //columnLabelMap['']

  std::map<int, std::string> neuronColumnMap;

  neuronColumnMap[50] = "a";

  ZJsonObject json;
  json.load(GET_DATA_DIR + "/flyem/FIB/FIB25/test.json");

  ZJsonObject neuronGroupJson(
        json["neuron"], ZJsonValue::SET_INCREASE_REF_COUNT);
  std::map<std::string, std::vector<int> > neuronGroup;


  for (size_t i = 0; i < columnList.size(); ++i) {
    const std::string &column = columnList[i];
    ZJsonArray idJson(neuronGroupJson[column.c_str()],
                      ZJsonValue::SET_INCREASE_REF_COUNT);
    neuronGroup[column] = std::vector<int>(idJson.size());
    for (size_t k = 0; k < idJson.size(); ++k) {
      neuronGroup[column][k] = ZJsonParser::integerValue(idJson.at(k));
    }
  }


  std::vector<ZStack*> seedArray;

  for (std::map<std::string, std::vector<int> >::const_iterator
       iter = neuronGroup.begin(); iter != neuronGroup.end(); ++iter) {
    std::cout << iter->first << ": ";
    const std::vector<int> &idArray = iter->second;
    for (std::vector<int>::const_iterator idIter = idArray.begin();
         idIter != idArray.end(); ++idIter) {
      std::cout << *idIter << ", ";
    }
    std::cout << std::endl;

    int label = columnLabelMap[iter->first];

    ZObject3dScan labelObj;
    for (std::vector<int>::const_iterator idIter = idArray.begin();
         idIter != idArray.end(); ++idIter) {
      ZObject3dScan obj = reader.readBody(*idIter);
      obj.downsample(9, 9, 9);
      labelObj.concat(obj);
      //objArray.push_back(obj);
    }
    seedArray.push_back(labelObj.toStackObject(label));
  }

  ZStackWatershed engine;
  engine.setFloodingZero(false);
  ZStack signal;
  signal.load(GET_DATA_DIR + "/flyem/FIB/FIB25/density_map_ds10.tif");
  signal.binarize();

  ZStack seed;
  seed.load(GET_DATA_DIR + "/flyem/FIB/FIB25/seed_ds10.tif");
  ZStack *result = engine.run(&signal, seedArray);

  result->save(GET_DATA_DIR + "/test.tif");

//  int bodyId[] = {50, 2158, 2515, 5048, 7021, 10319, 14879, 22045, 26353,
//                  30155, 236065, 238814, 805065};

//  int bodyId[] = {50};
//  size_t nbody = sizeof(bodyId) / sizeof(int);

//  ZObject3dScanArray objArray;
//  if (reader.open(target)) {
//    for (size_t i = 0; i < nbody; ++i) {
//      ZObject3dScan obj = reader.readBody(bodyId[i]);
//      obj.downsample(9, 9, 9);
//      objArray.push_back(obj);
//    }
//  }
//  //objArray.downsample(9, 9, 9);
//  ZStack *stack = objArray.toLabelField();
//  stack->save(GET_DATA_DIR + "/flyem/FIB/FIB25/seed_ds10.tif");

#endif

#if 0
  for (int i = 0; i < 100; ++i) {
    ZStackWatershed engine;
    engine.setFloodingZero(false);
    ZStack signal;
    signal.load(GET_DATA_DIR + "/flyem/FIB/FIB25/density_map_ds10.tif");
    signal.binarize();

    ZStack seed;
    seed.load(GET_DATA_DIR + "/flyem/FIB/FIB25/seed_ds10.tif");
    ZStack *result = engine.run(&signal, &seed);
    //  engine.setFloodingZero(false);
    result->save(GET_DATA_DIR + "/test.tif");

    delete result;
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c");
  reader.open(target);

  ZJsonObject jsonObj;
  jsonObj.load(GET_DATA_DIR + "/flyem/FIB/FIB25/bookmarks-no-data.json");
  ZJsonArray bodyArrayJson(jsonObj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
  std::cout << "Processing " << bodyArrayJson.size() << " bodies." << std::endl;
  ZObject3dScan allObj;
  for (size_t i = 0; i < bodyArrayJson.size(); ++i) {
    ZJsonObject jsonObj(
          bodyArrayJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    int bodyId = ZJsonParser::integerValue(jsonObj["body ID"]);

    std::cout << "Body: " << bodyId << std::endl;

    ZObject3dScan obj = reader.readBody(bodyId);
    allObj.concat(obj);
//    ZString fileName = ZString::num2str(bodyId);
//    fileName += ".sobj";
    //obj.save(GET_DATA_DIR + "/flyem/FIB/FIB25/border_obj/" + fileName);
  }
  allObj.canonize();
  allObj.save(GET_DATA_DIR + "/flyem/FIB/FIB25/border_obj/all.sobj");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole448_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZWeightedPointArray newPtArray;

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint &pt = *iter;
    if (pt.weight() >= 0.86) {
      pt *= 1.0 / dsScale;
      if (pt.z() > 50) {
        newPtArray.append(pt);
      }
    }
  }

  ZCuboid box = newPtArray.getBoundBox();
  box.print();

  ZStackFactory factory;
  ZStack *stack = factory.makeDensityMap(newPtArray, 10.0);
  stack->save(GET_DATA_DIR +
              "/flyem/AL/al7d_whole448_tbar-predict_0.86_ds20_s10.tif");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/FIB/fib19_all_dvid_final_tbar-predict_0.74.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZWeightedPointArray newPtArray;

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint &pt = *iter;
    pt /= dsScale;
    newPtArray.append(pt);
  }

  ZCuboid box = newPtArray.getBoundBox();
  box.print();

  ZStackFactory factory;
  ZStack *stack = factory.makeDensityMap(newPtArray, 10.0);
  stack->save(GET_DATA_DIR +
              "/flyem/FIB/fib19_all_dvid_final_tbar-predict_0.74_ds20_s10.tif");
#endif

#if 0
  ZPointArray ptArray;
  ptArray.importPcdFile(GET_DATA_DIR + "/test/pc519f.pcd");

  int dsScale = 10;
  for (ZPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    *iter *= 1.0 / dsScale;
  }

  ZCuboid box = ptArray.getBoundBox();
  box.print();

  ZStackFactory factory;
  ZStack *stack = factory.makeDensityMap(ptArray, 5.0);
  stack->save(GET_DATA_DIR + "/test2.tif");

#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/benchmark/em_stack.tif");

  Stack stackView = C_Stack::sliceView(stack.c_stack(), 100, 100);
  C_Stack::write(GET_DATA_DIR + "/benchmark/em_stack_slice.tif", &stackView);
#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/label/ds20_s10_signal1_1_1_1_1_thre.tif");
  Stack_Label_Objects_N(stack.c_stack(), NULL, 1, 2, 26);
  stack.save(GET_DATA_DIR +
             "/flyem/AL/label/ds20_s10_signal1_1_1_1_1_thre_labled.tif");
#endif

#if 0
  ZFileList fileList;
  fileList.load(GET_DATA_DIR + "/flyem/AL/label", "tif");

  ZObject3dScanArray objArray;

  for (int i = 0; i < fileList.size(); ++i) {
    ZString filePath = fileList.getFilePath(i);
    std::cout << "Path: " << filePath << std::endl;
    if (filePath.startsWith(GET_DATA_DIR + "/flyem/AL/label/final_ds20_s10_label")) {
      ZStack stack;
      stack.load(filePath);
      std::map<int, ZObject3dScan*> *objList =
          ZObject3dScan::extractAllObject(
            stack.array8(), stack.width(), stack.height(), stack.depth(),
            0, NULL);

      for (std::map<int, ZObject3dScan*>::iterator iter = objList->begin();
           iter != objList->end(); ++iter) {
        if (iter->first > 0) {
          ZObject3dScan *obj = iter->second;
          obj->translate(stack.getOffset());
          objArray.push_back(*obj);
        }
        delete iter->second;
      }
    }/* else if (filePath.startsWith(GET_DATA_DIR + "/flyem/AL/label/final_ds20_s10_signal")) {
      ZStack stack;
      stack.load(filePath);
      stack.binarize();
      std::map<int, ZObject3dScan*> *objList =
          ZObject3dScan::extractAllObject(
            stack.array8(), stack.width(), stack.height(), stack.depth(),
            0, NULL);

      for (std::map<int, ZObject3dScan*>::iterator iter = objList->begin();
           iter != objList->end(); ++iter) {
        if (iter->first > 0) {
          ZObject3dScan *obj = iter->second;
          obj->translate(stack.getOffset());
          objArray.push_back(*obj);
        }
        delete iter->second;
      }
    }*/
  }
  std::cout << objArray.size() << " objects extracted." << std::endl;

  ZStack *stack = objArray.toLabelField();
  stack->save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/glomeruli/labeled_synapse_confidence.txt").c_str());
  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint pt = *iter;
    pt *= 1.0 / dsScale;
    ZIntPoint ipt = pt.toIntPoint();
    if (ipt.getZ() >= 50) {
      int label = stack.getIntValue(ipt.getX(), ipt.getY(), ipt.getZ());

      if (label > 0) {
        stream << iter->x() << " " << iter->y() << " " << iter->z() << " "
               << label << " " << pt.weight() << std::endl;
      }
    }
  }
  stream.close();
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/glomeruli/labeled_synapse_confidence_merged.txt").c_str());
  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint pt = *iter;
    pt *= 1.0 / dsScale;
    ZIntPoint ipt = pt.toIntPoint();
    if (ipt.getZ() >= 50) {
      int label = stack.getIntValue(ipt.getX(), ipt.getY(), ipt.getZ());
      if (label == 49) {
        label = 7;
      }

      if (label > 0) {
        stream << iter->x() << " " << iter->y() << " " << iter->z() << " "
               << label << " " << pt.weight() << std::endl;
      }
    }
  }
  stream.close();
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole448_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;


#endif

#if 0
  ZStack signal;
  signal.load(GET_DATA_DIR +
              "/flyem/AL/al7d_whole448_tbar-predict_0.86_ds20_s10.tif");

//  ZStack mask;
//  mask.load(GET_DATA_DIR +
//            "/flyem/AL/label/final_ds20_s10_label1_2_2_2_3_2_2_3.tif");

//  ZStack *colorStack = ZStackFactory::MakeColorStack(signal, mask, 1, 1);

  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/label/label_field.tif");

  ZStack *colorStack = ZStackFactory::MakeColorStack(signal, labelField);

  colorStack->save(GET_DATA_DIR + "/test.tif");

  delete colorStack;
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/MB/tbars_annotated_20141201T131652.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZWeightedPointArray newPtArray;

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint &pt = *iter;
    pt *= 1.0 / dsScale;
    newPtArray.append(pt);
  }

  ZCuboid box = newPtArray.getBoundBox();
  box.print();

  ZStackFactory factory;
  ZStack *stack = factory.makeDensityMap(newPtArray, 5.0);
  stack->save(GET_DATA_DIR +
              "/flyem/MB/tbars_annotated_20141201T131652_ds20_s5.tif");
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZWeightedPointArray newPtArray;

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint &pt = *iter;
    pt *= 1.0 / dsScale;
    newPtArray.append(pt);
  }

  ZCuboid box = newPtArray.getBoundBox();
  box.print();

  ZStackFactory factory;
  ZStack *stack = factory.makeDensityMap(newPtArray, 10.0);
  stack->save(GET_DATA_DIR +
              "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81_ds20_s10.tif");
#endif


#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/MB/tbars_annotated_20141201T131652.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ofstream stream((GET_DATA_DIR +
                   "/flyem/MB/tbars_annotated_20141201T131652.txt").c_str());

  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint &pt = *iter;
    stream << pt.x() << " " << pt.y() << " " << pt.z() << std::endl;
  }
  stream.close();
#endif

#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "2b6c");

  ZDvidReader reader;
  reader.open(dvidTarget);

  int dsIntv = 3;
  int dataRangeZ = 8089 / (dsIntv + 1);

  ZObject3dScan obj = reader.readBody(29);
  obj.downsampleMax(dsIntv, dsIntv, dsIntv);

//  ZObject3dScan obj;

//  for (int z = 0; z < dataRangeZ; ++z) {
//    for (int y = 0; y < 10; y += 1) {
//      obj.addSegment(z, y, 0, z % 100, false);
//    }
//  }


  ZStack *stack = obj.toStackObject(255);

  //stack->save(GET_TEST_DATA_DIR + "/test.tif");

  ZSharedPointer<ZStackDoc> academy =
      ZSharedPointer<ZStackDoc>(new ZStackDoc(NULL, NULL));
  academy->loadStack(stack);

  Z3DWindow *stage = new Z3DWindow(academy, Z3DWindow::NORMAL_INIT,
                                   false, NULL);

  stage->getVolumeRaycaster()->hideBoundBox();
  stage->getVolumeRaycasterRenderer()->setCompositeMode(
        "Direct Volume Rendering");
  stage->getAxis()->setVisible(false);

  Z3DCameraParameter* camera = stage->getCamera();
  camera->setProjectionType(Z3DCamera::Orthographic);

  glm::vec3 referenceCenter = camera->getCenter();


  double distEyeToNear = dataRangeZ * 0.5 / tan(camera->getFieldOfView() * 0.5);
  double distNearToCenter = 4000;
  double eyeDistance = distEyeToNear + distNearToCenter;

 // double eyeDistance = eyeDistance;//boundBox[3] - referenceCenter[1] + 2500;
  //double eyeDistance = 2000 - referenceCenter[1];
  glm::vec3 viewVector(0, -1, 0);

  viewVector *= eyeDistance;
  glm::vec3 eyePosition = referenceCenter - viewVector;

  referenceCenter[2] = dataRangeZ / 2;// - stack->getOffset().getZ();
  camera->setCenter(referenceCenter);
  eyePosition[2] = dataRangeZ / 2;// - stack->getOffset().getZ();
  camera->setEye(eyePosition);
  camera->setUpVector(glm::vec3(0, 0, -1));

  stage->resetCameraClippingRange();
  camera->setNearDist(distEyeToNear);

  stage->getCompositor()->setBackgroundFirstColor(0, 0, 0, 1);
  stage->getCompositor()->setBackgroundSecondColor(0, 0, 0, 1);

  stage->show();

  //std::string output = GET_DATA_DIR + "/test.tif";
  //std::cout << output << std::endl;
  //stage->takeScreenShot(output.c_str(), 2000, dataRangeZ, MonoView);
  //stage->close();
  //delete stage;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR+ "/body_1.sobj");
  obj.upSample(1, 1, 1);
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/body_0.sobj");
  std::cout << obj1.isCanonized() << std::endl;
  obj1.canonize();
  std::cout << obj1.isCanonizedActually() << std::endl;

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/body_1.sobj");
  std::cout << obj2.isCanonizedActually() << std::endl;

  ZObject3dScan remained = obj1.subtract(obj2);
  obj1.save(GET_TEST_DATA_DIR + "/test.sobj");
  remained.save(GET_TEST_DATA_DIR + "/test2.sobj");
#endif

#if 0
  std::vector<ZPointArray> synapseGroup;
  std::string synapseFile =
      GET_TEST_DATA_DIR + "/flyem/AL/label/whole_AL_synapse_labeled.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 4) {
      int label = pt[3];
      //std::cout << pt[3] << std::endl;

      if (label > 0) {
        if (label >= (int) synapseGroup.size()) {
          synapseGroup.resize(label + 1);
        }
        synapseGroup[label].push_back(ZPoint(pt[0], pt[1], pt[2]));
      }
    }
  }


  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");
  int *hist = Stack_Hist(labelField.c_stack());
  int *histArray = Int_Histogram_Array(hist);

  std::cout << synapseGroup.size() - 1 << " labels" << std::endl;
  for (size_t label = 1; label < synapseGroup.size(); ++label) {
    const ZPointArray &pt = synapseGroup[label];
    ZPoint center = pt.computeCenter();
    std::cout << label << ": " << center.toString() << " " << pt.size()
              << ", " << histArray[label] * 0.16 * 0.16 *0.16  << std::endl;
  }
#endif

#if 0
  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/glomeruli/label.tif");

  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/test.sobj");

  obj.maskStack(&labelField);
  labelField.save(GET_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/glomeruli/label.tif");

  C_Stack::shrinkBorder(labelField.c_stack(), 3);

  labelField.save(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/label_shrinked_3.tif");

#endif

#if 0
//  ZStack *stack = ZStackFactory::makeIndexStack(3, 3, 1);
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/label/label_field.tif");
//  Print_Stack_Value(stack.c_stack());
  ZGraph *graph = misc::makeCoOccurGraph(stack.c_stack(), 6);

  Graph *rawGraph = graph->getRawGraph();
  double maxWeight = 0.0;
  int root = 0;
  for (int i = 0; i < rawGraph->nedge; ++i) {
    if (maxWeight < rawGraph->weights[i]) {
      maxWeight = rawGraph->weights[i];
      root = graph->getEdgeBegin(i);
    }
    rawGraph->weights[i] = 1.0 / (1 + rawGraph->weights[i]);
  }
  graph->toMst();
  graph->print();

  std::cout << "Root: " << root << std::endl;

  graph->traverseDirect(root);
  const int *index = graph->topologicalSort();
  iarray_print2(index, graph->getVertexNumber(), 1);

  //Build real tree
  std::vector<ZTreeNode<int>*> treeNodeArray(graph->getVertexNumber() + 1);
  treeNodeArray[0] = new ZTreeNode<int>; //root
  treeNodeArray[0]->setData(0);
  for (int i = 0; i < graph->getEdgeNumber(); ++i) {
    int v1 = graph->getEdgeBegin(i);
    int v2 = graph->getEdgeEnd(i);

    //v1 is the parent of v2
    if (treeNodeArray[v1] == NULL) {
      treeNodeArray[v1] = new ZTreeNode<int>;
      treeNodeArray[v1]->setData(v1);
    }
    if (treeNodeArray[v2] == NULL) {
      treeNodeArray[v2] = new ZTreeNode<int>;
      treeNodeArray[v2]->setData(v2);
    }
    if (v1 == root) {
      treeNodeArray[v1]->setParent(treeNodeArray[0]);
      treeNodeArray[v2]->setParent(treeNodeArray[0]);
    } else {
      treeNodeArray[v2]->setParent(treeNodeArray[v1]);
    }
  }

  ZTree<int> tree;
  tree.setRoot(treeNodeArray[0]);
  ZTreeIterator<int> iterator(tree);
  while (iterator.hasNext()) {
    std::cout << iterator.next() << std::endl;
  }
#endif

#if 0
  std::vector<ZPointArray> synapseGroup;
  std::string synapseFile =
      GET_TEST_DATA_DIR + "/flyem/AL/label/whole_AL_synapse_labeled.txt";

  FILE *fp = fopen(synapseFile.c_str(), "r");

  ZString line;
  while (line.readLine(fp)) {
    std::vector<int> pt = line.toIntegerArray();
    if (pt.size() == 4) {
      int label = pt[3];
      //std::cout << pt[3] << std::endl;

      if (label > 0) {
        if (label >= (int) synapseGroup.size()) {
          synapseGroup.resize(label + 1);
        }
        synapseGroup[label].push_back(ZPoint(pt[0], pt[1], pt[2]));
      }
    }
  }
  fclose(fp);

  ZStackWatershed watershedEngine;

  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/label/label_field.tif");

  ZObject3dScan obj;
  obj.load(GET_DATA_DIR + "/flyem/AL/whole_roi_ds20.sobj");
  ZStack *signal = obj.toStackObject();

  ZStack densityMap;
  densityMap.load(GET_DATA_DIR +
                  "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81_ds20_s10.tif");
  densityMap.crop(signal->getBoundBox());

  size_t voxelNumber = signal->getVoxelNumber();
  for (size_t i = 0; i < voxelNumber; ++i) {
    if (signal->array8()[i] > 0) {
      if (densityMap.array8()[i] > 0) {
        signal->array8()[i] = densityMap.array8()[i];
      } else {
        signal->array8()[i] = 1;
      }
    }
  }

  C_Stack::shrinkBorder(labelField.c_stack(), 5);

  ZStack *result = watershedEngine.run(signal, &labelField);
  result->save(GET_DATA_DIR + "/test.tif");

  int *hist = Stack_Hist(result->c_stack());
  int *histArray = Int_Histogram_Array(hist);

  double totalVolume = 0;

  std::cout << synapseGroup.size() - 1 << " labels" << std::endl;
  for (size_t label = 1; label < synapseGroup.size(); ++label) {
    const ZPointArray &pt = synapseGroup[label];
    ZPoint center = pt.computeCenter();
    double volume = histArray[label] * 0.16 * 0.16 *0.16;
    totalVolume += volume;
    std::cout << label << ": " << center.toString() << " " << pt.size()
              << ", " << volume << std::endl;
  }

  std::cout << "Total volume: " << totalVolume << std::endl;
#endif

#if 0
  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/label/label_field.tif");
  ZObject3dArray *objArray = ZObject3dFactory::MakeRegionBoundary(
        labelField, ZObject3dFactory::OUTPUT_SPARSE);

  ZStack *stack = objArray->toStackObject();
  stack->binarize();
  ZObject3dScan obj;
  obj.loadStack(*stack);
  delete stack;

  obj.save(GET_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZJsonObject rootJson;
  ZJsonObject stackJson;
  stackJson.setEntry(
        "url", GET_DATA_DIR +
        "/flyem/AL/glomeruli/al7d_whole_wfix_tbar-predict_0.81_ds20_s5.tif");
  stackJson.setEntry("type", std::string("single"));

  rootJson.setEntry("zstack", stackJson);
//  ZJsonObject segJson;

//  ZJsonArray labelArrayJson;

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/label.tif");

  ZTree<int> *segTree = misc::buildSegmentationTree(stack.c_stack());

  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(stack);

  std::vector<ZObject3dScan*> sortedObjArray;
  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    if (obj->getLabel() >= sortedObjArray.size()) {
      sortedObjArray.resize(obj->getLabel() + 1);
    }
    sortedObjArray[obj->getLabel()] = obj;
  }

  std::vector<ZJsonObject> segJsonArray(sortedObjArray.size());

  ZTreeIterator<int> treeIter(*segTree);

  while (treeIter.hasNext()) {
    ZTreeNode<int> *node = treeIter.nextNode();
    int label = node->data();
    if (label > 0) {
      ZJsonObject &labelJson = segJsonArray[label];
      labelJson.setEntry("label", label);

      ZObject3dScan *obj = sortedObjArray[label];
      QString file = QString("_%1.sobj").arg(obj->getLabel());
      QString output = QString("%1/%2").
          arg((GET_DATA_DIR + "/flyem/AL/glomeruli/segcheck").c_str()).
          arg(file);
      obj->save(output.toStdString());
      labelJson.setEntry("source", file.toStdString());

      int parentLabel = node->parent()->data();
      std::cout << parentLabel << std::endl;
      ZJsonObject &parentJson = segJsonArray[parentLabel];
      if (parentJson.hasKey("child") == false) {
        parentJson.setEntry("child", json_array());
      }

      ZJsonArray arrayJson(
            parentJson["child"], ZJsonValue::SET_INCREASE_REF_COUNT);
      arrayJson.append(labelJson);

//      std::cout << segJsonArray[0].dumpString(2) << std::endl;
    }
  }


//  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
//       iter != objArray.end(); ++iter) {
//    ZJsonObject labelJson;


//    labelJson.setEntry("label", (int) obj->getLabel());
//    labelJson.setEntry("source", file.toStdString());
//    labelArrayJson.append(labelJson);
//  }

//  segJson.setEntry("child", segJsonArray[0]);

  rootJson.setEntry("segmentation", segJsonArray[0]);
//  std::cout << rootJson.dumpString(2) << std::endl;

  rootJson.dump(GET_DATA_DIR + "/flyem/AL/glomeruli/segcheck/seg.json");
#endif

#if 0
  ZJsonObject rootJson;
  ZJsonObject stackJson;
  std::string signalPath = GET_DATA_DIR +
      "/flyem/AL/glomeruli/al7d_whole_wfix_tbar-predict_0.81_ds20_s5.tif";

  stackJson.setEntry("url", signalPath);
  stackJson.setEntry("type", std::string("single"));

  rootJson.setEntry("zstack", stackJson);
//  ZJsonObject segJson;

//  ZJsonArray labelArrayJson;

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/label.tif");

  ZTree<int> *segTree = misc::buildSegmentationTree(stack.c_stack());

  std::vector<ZObject3dScan*> objArray =
      ZObject3dScan::extractAllObject(stack);

  std::vector<ZObject3dScan*> sortedObjArray;
  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    if (obj->getLabel() >= sortedObjArray.size()) {
      sortedObjArray.resize(obj->getLabel() + 1);
    }
    sortedObjArray[obj->getLabel()] = obj;
  }

//  std::vector<ZJsonObject> segJsonArray(sortedObjArray.size());

//  ZJsonObject segJson;
//  segJson.setEntry("child", json_array());

  ZSegmentationProject segProj;
  ZTree<ZObject3dScan> *labelTree = segProj.getLabelTree();
  ZTreeNode<ZObject3dScan> *root = new ZTreeNode<ZObject3dScan>;
  labelTree->setRoot(root);

  ZTreeNode<int> *rootNode = segTree->getRoot();
  std::cout << "#child " << rootNode->childNumber() << std::endl;
  for (int i = 0; i < rootNode->childNumber(); ++i) {
    ZTreeNode<int> *node = rootNode->getChild(i);
    ZTreeIterator<int> treeIter(node, ZTreeIterator<int>::BREADTH_FIRST);

    ZTreeNode<ZObject3dScan> *objNode = new ZTreeNode<ZObject3dScan>;

    ZObject3dScan &parentObj = objNode->data();
    std::cout << ">>>Label" << std::endl;
    while (treeIter.hasNext()) {
      ZTreeNode<int> *node = treeIter.nextNode();
      int label = node->data();
      ZObject3dScan *obj = sortedObjArray[label];
      parentObj.concat(*obj);

      std::cout << label << std::endl;

      ZTreeNode<ZObject3dScan> *childObjNode = new ZTreeNode<ZObject3dScan>;
      childObjNode->setData(*obj);
      childObjNode->setParent(objNode);
    }
    parentObj.canonize();
    parentObj.setLabel(i + 1);

    objNode->setParent(root);
  }

  ZStack *signal = new ZStack;
  signal->load(signalPath);
  segProj.setStack(signal);

  segProj.save((GET_DATA_DIR + "/flyem/AL/glomeruli/segcheck/seg.json").c_str());

//  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
//       iter != objArray.end(); ++iter) {
//    ZJsonObject labelJson;


//    labelJson.setEntry("label", (int) obj->getLabel());
//    labelJson.setEntry("source", file.toStdString());
//    labelArrayJson.append(labelJson);
//  }

//  segJson.setEntry("child", segJsonArray[0]);

//  rootJson.setEntry("segmentation", segJsonArray[0]);
//  std::cout << rootJson.dumpString(2) << std::endl;

//  rootJson.dump(GET_DATA_DIR + "/flyem/AL/label/segcheck/seg.json");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6c", -1);

  ZFlyEmDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan obj = reader.readCoarseBody(117);
    obj.setColor(QColor(0, 0, 255));
    ZStackFrame *frame = new ZStackFrame;
    tic();
    ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj);
    ptoc();

    ZStackDoc *doc = new ZStackDoc(NULL, NULL);
    doc->addObject(tree);
    tic();
    ZWindowFactory factory;
    Z3DWindow *window = factory.make3DWindow(doc);
    window->getSwcFilter()->setColorMode("Intrinsic");
    window->getSwcFilter()->setRenderingPrimitive("Sphere");
    window->show();
    window->raise();
    ptoc();
    delete frame;
  }

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/em_stack_slice.tif");
  Stack *out = C_Stack::computeGradient(stack.c_stack());
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", out);
#endif

#if 0
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/label.tif");

  ofstream stream(
        (GET_DATA_DIR + "/flyem/AL/glomeruli/al7d_whole_wfix_tbar-predict_0.81_labeled.txt").c_str());

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint pt = *iter;
    pt *= 1.0 / dsScale;
    ZIntPoint ipt = pt.toIntPoint();
    int label = stack.getIntValue(ipt.getX(), ipt.getY(), ipt.getZ());

    if (label > 0) {
      stream << iter->x() << " " << iter->y() << " " << iter->z() << " "
             << label << " " << pt.weight() << std::endl;
    }

  }

  stream.close();
#endif

#if 0
  ZDvidTarget dvidTarget("emdata2.int.janelia.org", "134");
  ZDvidReader reader;
  reader.open(dvidTarget);
  ZDvidInfo info = reader.readGrayScaleInfo();
  info.print();
#endif

#if 0
  //merging objects
  std::string dataFolder = GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck3";
  std::string outputFolder = GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck4";
  ZFileList fileList;
  fileList.load(dataFolder, "sobj");

  std::set<std::string> mergeSet;

  ZObject3dScan masterObj;
  masterObj.load(dataFolder + "/_2_46_2.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  ZObject3dScan slaveObj;
  slaveObj.load(dataFolder + "/_2_47.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));


  masterObj.load(dataFolder + "/_2_5.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_6.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_7.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_8_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_9_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-8-1 + 2-9-1
  masterObj.load(dataFolder + "/_2_8_1.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_9_1.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-53-2 + 2-49-2
  masterObj.load(dataFolder + "/_2_53_2.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_49_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-39 + 2-56
  masterObj.load(dataFolder + "/_2_39.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_56.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-45 + 2-19 + 2-20
  masterObj.load(dataFolder + "/_2_45_1.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_45_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_19.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_20.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-1 + 2-2 + 2-3 + 2-4
  masterObj.load(dataFolder + "/_2_1.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_3.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_2_4.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-25 + 1-26
  masterObj.load(dataFolder + "/_2_25.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_1_26.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  1-44 + 1-21 + 1-22 + 1-24
  masterObj.load(dataFolder + "/_1_44.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_1_21.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_1_22.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  slaveObj.load(dataFolder + "/_1_24.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-27 + 2-28
  masterObj.load(dataFolder + "/_2_27.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_28.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));

//  2-58-1 + 2-55-2
  masterObj.load(dataFolder + "/_2_58_1.sobj");
  mergeSet.insert(ZString::getBaseName(masterObj.getSource()));

  slaveObj.load(dataFolder + "/_2_55_2.sobj");
  mergeSet.insert(ZString::getBaseName(slaveObj.getSource()));
  masterObj.concat(slaveObj);

  masterObj.canonize();
  masterObj.save(outputFolder + "/" +
                 ZString::getBaseName(masterObj.getSource()));


  for (int i = 0; i < fileList.size(); ++i) {
    std::string filePath = fileList.getFilePath(i);
    if (mergeSet.count(ZString::getBaseName(filePath)) == 0) {
      std::cout << filePath << std::endl;
      masterObj.load(filePath);
      masterObj.save(outputFolder + "/" +
                     ZString::getBaseName(masterObj.getSource()));
    }
  }
#endif

#if 0
  ZSegmentationProject segProj;
  ZTree<ZObject3dScan> *labelTree = segProj.getLabelTree();
  ZTreeNode<ZObject3dScan> *root = new ZTreeNode<ZObject3dScan>;
  labelTree->setRoot(root);

  std::string dataFolder = GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck4";
  std::string outputFolder = GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck5";
  ZFileList fileList;
  fileList.load(dataFolder, "sobj");

  for (int i = 0; i < fileList.size(); ++i) {
    std::string filePath = fileList.getFilePath(i);
    ZTreeNode<ZObject3dScan> *node = new ZTreeNode<ZObject3dScan>;
    node->setParent(root);
    node->data().setLabel(i + 1);
    node->data().load(filePath);
  }

  std::string signalPath = GET_DATA_DIR +
      "/flyem/AL/glomeruli/al7d_whole_wfix_tbar-predict_0.81_ds20_s5.tif";

  ZStack *signal = new ZStack;
  signal->load(signalPath);
  segProj.setStack(signal);

  segProj.save((outputFolder + "/seg.json").c_str());
#endif

#if 0
  ZStack redStack;
  ZStack greenStack;
  ZStack blueStack;

  redStack.load(GET_TEST_DATA_DIR + "/flyem/MB/mblight.tif");
  blueStack.load(GET_TEST_DATA_DIR + "/flyem/MB/C2-alphalobealigned.tif");
  greenStack.load(GET_TEST_DATA_DIR + "/flyem/MB/C3-alphalobealigned.tif");

  greenStack.setOffset(blueStack.getOffset() - ZIntPoint(6, 4, 0));

  ZStack *stack = ZStackFactory::MakeRgbStack(redStack, greenStack, blueStack);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/MB/C3-alphalobealigned.tif");
  stack.translate(-6, -4, 0);
  stack.save(GET_TEST_DATA_DIR + "/flyem/MB/C3-alphalobealigned_adjusted.tif");

#endif

#if 0
  ZStack stack1;
  ZStack stack2;
  stack1.load(GET_TEST_DATA_DIR + "/flyem/AL/lightseg.tif");
  stack2.load(GET_TEST_DATA_DIR + "/flyem/AL/em_registered.tif");
  stack2.translate(312, 0, 0);

  ZStack *stack = ZStackFactory::CompositeForeground(stack1, stack2);
  ZIntCuboid cuboid = stack->getBoundBox();
  cuboid.setFirstZ(30);
  cuboid.setLastZ(210);
  stack->crop(cuboid);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck5/signal.tif");
  ZIntCuboid cuboid = stack.getBoundBox();
  cuboid.setFirstZ(50);
  stack.crop(cuboid);
  stack.save(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck6/signal.tif");

#endif

#if 0
  ZObject3dScan obj1;
  ZObject3dScan obj2;

  obj1.load(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck5/_39.sobj");
  obj2.load(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck5/_40.sobj");

  obj1.unify(obj2);

  obj1.save(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck6/_39.sobj");

#endif

#if 0
  ZStack labelField;
  labelField.load(GET_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");

  ZStack signal;
  signal.load(GET_DATA_DIR + "/flyem/AL/glomeruli/segcheck8/signal.tif");
  Stack_Threshold_Binarize(signal.c_stack(), 5);

  //masking
  labelField.crop(signal.getBoundBox());
  Stack_Mask(labelField.c_stack(), signal.c_stack(), labelField.c_stack());

  std::vector<ZPointArray> synapseGroup;
  FlyEm::ZSynapseAnnotationArray synapseArray;
  synapseArray.loadJson(GET_DATA_DIR +
                        "/flyem/AL/al7d_whole_wfix_tbar-predict_0.81.json");

  ZWeightedPointArray ptArray = synapseArray.toTBarConfidencePointArray();
  std::cout << ptArray.size() << " TBars" << std::endl;

  int dsScale = 20;
  for (ZWeightedPointArray::iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    ZWeightedPoint pt = *iter;
    pt *= 1.0 / dsScale;
    ZIntPoint ipt = pt.toIntPoint();
    int label = labelField.getIntValue(ipt.getX(), ipt.getY(), ipt.getZ());

    if (label > 0) {
      if (label >= (int) synapseGroup.size()) {
        synapseGroup.resize(label + 1);
      }
      synapseGroup[label].push_back(pt);
    }
  }

//  labelField.save(GET_TEST_DATA_DIR + "/test.tif");

  int *hist = Stack_Hist(labelField.c_stack());
  int *histArray = Int_Histogram_Array(hist);

  std::cout << synapseGroup.size() - 1 << " labels" << std::endl;
  for (size_t label = 1; label < synapseGroup.size(); ++label) {
    const ZPointArray &pt = synapseGroup[label];
    ZPoint center = pt.computeCenter();
    std::cout << label << ": " << center.toString() << " " << pt.size()
              << ", " << histArray[label] * 0.16 * 0.16 *0.16  << std::endl;
  }
#endif

#if 0
  ZObject3dScan obj;
//  obj.load(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/segcheck8/_31.sobj");
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZStackFrame *w1 = new ZStackFrame;
  ZStackFrame *w2 = new ZStackFrame;

  ZStackViewManager manager;
  manager.registerWindowPair(w1, w2);
  manager.registerWindowPair(w1, w2);

  manager.print();

#endif

#if 0
  FILE *fp = fopen((GET_DATA_DIR +
                   "/flyem/AL/glomeruli/labeled_synapse_confidence.txt").c_str(), "r");

  std::vector<ZPointArray> synapseGroup(60);

  ZString str;
  while (str.readLine(fp)) {
    std::vector<double> valueArray = str.toDoubleArray();
    int label = valueArray[3];
    if (label == 49) {
      label = 7;
    }
    if (valueArray.size() >= 5) {
      ZPointArray &ptArray = synapseGroup[label];
      ptArray.append(ZPoint(valueArray[0], valueArray[1], valueArray[2]));
    }
  }

  fclose(fp);

  int labelArray[] = {14, 15, 17, 18, 24, 25, 31, 33, 45, 46 };

  for (size_t i = 0; i < sizeof(labelArray) / sizeof(int); ++i) {
    int label = labelArray[i];
    ZPoint pt = synapseGroup[label].computeCenter();
    std::cout << label << ": " << pt.toIntPoint().toString() << std::endl;
  }
#endif

#if 0
  if (host != NULL) {
    ZStackFrame *frame = host->currentStackFrame();
    if (frame != NULL) {
      frame->view()->setSizeHintOption(NeuTube::SIZE_HINT_TAKING_SPACE);
      frame->resize(frame->sizeHint());
    }
  }
#endif

#if 0 //Estimate bound box
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6c", -1);

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo;
  dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  ZObject3dScan roiList[7];

  roiList[0].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_A.sobj");
  roiList[1].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_B.sobj");
  roiList[2].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_C.sobj");
  roiList[3].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_D.sobj");
  roiList[4].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_E.sobj");
  roiList[5].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_F.sobj");
  roiList[6].load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_homev2.sobj");

  ZIntCuboid box = roiList[0].getBoundBox();

  for (size_t i = 1; i < 7; ++i) {
    box.join(roiList[i].getBoundBox());
  }

  ZIntCuboid box1 = dvidInfo.getBlockBox(box.getFirstCorner());

  ZIntCuboid box2 = dvidInfo.getBlockBox(box.getLastCorner());

  std::cout << "Bound box: " << box1.getFirstCorner().toString() << " => "
            << box2.getLastCorner().toString() << std::endl;
#endif

#if 0
  ZClosedCurve curve;
  curve.append(4.5, 0.0, 0);
  curve.append(0.0, 0.0, 0);
  curve.append(0.0, 2.7, 0);

  ZObject3dScan obj;
  ZObject3dFactory::MakeFilledMask(curve, 992, &obj);

  obj.print();
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/flyem/FIB/roi/layer_roi.txt").c_str(),
                   "r");
  ZString str;
  ZObject3dScan obj[11];
  while (str.readLine(fp)) {
    std::vector<double> valueArray = str.toDoubleArray();
    if (valueArray.size() >= 4) {
      //std::cout << valueArray[1] << std::endl;
      int layer = iround(valueArray[0]);
      ZClosedCurve curve;
      for (size_t i = 2; i < valueArray.size(); i += 2) {
        curve.append(valueArray[i], valueArray[i + 1], 0);
      }
      ZObject3dScan *layerObj =
          ZObject3dFactory::MakeFilledMask(curve, valueArray[1]);
      if (layerObj != NULL) {
        obj[layer].concat(*layerObj);
      }
      delete layerObj;
    }
  }

  fclose(fp);

  for (size_t i = 1; i < 11; ++i) {
    obj[i].save(GET_TEST_DATA_DIR +
                QString("/flyem/FIB/roi/layer_roi_%1.sobj").arg(i).toStdString());
  }
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6c", -1);

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo;
  dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  ZObject3dScan obj;
  for (size_t i = 1; i < 11; ++i) {
    std::cout << "Object " << i << std::endl;
    obj.load(GET_TEST_DATA_DIR +
             QString("/flyem/FIB/roi/layer_roi_%1.sobj").arg(i).toStdString());
    ZObject3dScan blockObj = dvidInfo.getBlockIndex(obj);
    blockObj.save(GET_TEST_DATA_DIR +
                  QString("/flyem/FIB/roi/layer_roi_%1_block.sobj").
                  arg(i).toStdString());
    ZJsonArray array = ZJsonFactory::makeJsonArray(
          blockObj, ZJsonFactory::OBJECT_SPARSE);
    array.dump(GET_TEST_DATA_DIR +
               QString("/flyem/FIB/roi/layer_roi_%1.json").arg(i).toStdString());
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emrecon100.janelia.priv", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo;
  dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  ZJsonObject rootJson;
  rootJson.load(GET_TEST_DATA_DIR +
                "/flyem/FIB/hackathon/annotations-synapse.json");
  ZJsonArray synapseArrayJson(
        rootJson["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
  for (size_t i = 0; i < synapseArrayJson.size(); ++i) {
    ZJsonObject synapseJson(synapseArrayJson.at(i),
                            ZJsonValue::SET_INCREASE_REF_COUNT);
    ZJsonObject tbarJson(synapseJson["T-bar"],
        ZJsonValue::SET_INCREASE_REF_COUNT);
    ZJsonArray locationJson(tbarJson["location"],
        ZJsonValue::SET_INCREASE_REF_COUNT);
    if (locationJson.size() == 3) {
      json_t *yJson = locationJson.at(1);
      int y = ZJsonParser::integerValue(yJson);
      json_integer_set(yJson, dvidInfo.getMaxY() - y);
    }

    ZJsonArray psdJsonArray(synapseJson["partners"],
        ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t j = 0; j < psdJsonArray.size(); ++j) {
      ZJsonObject psdJson(psdJsonArray.at(j),
                          ZJsonValue::SET_INCREASE_REF_COUNT);
      ZJsonArray locationJson(psdJson["location"],
          ZJsonValue::SET_INCREASE_REF_COUNT);
      if (locationJson.size() == 3) {
        json_t *yJson = locationJson.at(1);
        int y = ZJsonParser::integerValue(yJson);
        json_integer_set(yJson, dvidInfo.getMaxY() - y);
      }
    }
  }

  ZJsonObject metadataJson(rootJson["metadata"],
      ZJsonValue::SET_INCREASE_REF_COUNT);
  metadataJson.setEntry("coordinate", std::string("DVID"));

  rootJson.dump(GET_TEST_DATA_DIR + "/test.json");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer_extended.txt");
  blockArray.exportSwc(dataPath + "/flyem/FIB/block_13layer_extended.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer_extended_surround.txt");
  blockArray.exportSwc(dataPath + "/flyem/FIB/block_13layer_extended_surround.swc");
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(
        dataPath + "/flyem/FIB/block_13layer_extended_surround.txt");

  Cuboid_I box = blockArray.getBoundBox();
  Print_Cuboid_I(&box);
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer_extended.txt");

  ZDvidTarget target;
  target.set("emrecon100.janelia.priv", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);
  QStringList swcKeys = reader.readKeys("skeletons", "0");

  int count = 0;
  ZString outputDir = GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/skeletons_original";
  foreach (const QString &swcKey, swcKeys) {

    int bodyId = ZString(swcKey.toStdString()).firstInteger();
    if (bodyId > 0) {
      ZSwcTree *tree = reader.readSwc(bodyId);

      ZSwcTree::DepthFirstIterator iter(tree);
      while (iter.hasNext()) {
        Swc_Tree_Node *tn = iter.next();
        if (SwcTreeNode::isRegular(tn)) {
          if (blockArray.hitTest(SwcTreeNode::x(tn), SwcTreeNode::y(tn),
                                 SwcTreeNode::z(tn)) >= 0) {
            qDebug() << swcKey;
            tree->save(outputDir + "/" + swcKey.toStdString());
            ++count;
            break;
          }
        }
      }

      delete tree;
      //tree->save(GET_TEST_DATA_DIR + "/test.swc");
    }
  }

  std::cout << count << "/" << swcKeys.size() << " neurons" << std::endl;

#endif

#if 0
  ZIntCuboid box;
  box.setFirstCorner(1568, 1664, 1376);
  box.setLastCorner(5183, 4223, 7807);

  size_t totalVolume = box.getVolume();
  ZObject3dScanArray objArray;
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_A.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_B.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_C.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_D.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_E.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_F.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_homev2.sobj");
  objArray.push_back(obj);

  for (size_t i = 0; i < objArray.size(); ++i) {
    const ZObject3dScan &obj = objArray[i];
    double ratio = (double) obj.getVoxelNumber() * 32 * 32 * 32 / totalVolume;

    std::cout << obj.getSource() << ": " << ratio << std::endl;
  }

#endif

#if 0
  ZObject3dScanArray objArray;
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_A.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_B.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_C.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_D.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_E.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_F.sobj");
  objArray.push_back(obj);
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/roi/roi_homev2.sobj");
  objArray.push_back(obj);


  ZStack *stack = ZStackFactory::MakeBinaryStack(objArray, 1);
  stack->printInfo();

#  if 1
  ZDvidTarget target;
  target.set("emrecon100.janelia.priv", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);
  QStringList swcKeys = reader.readKeys("skeletons", "0");


  foreach (const QString &swcKey, swcKeys) {
    qDebug() << swcKey;
  }

  std::cout << swcKeys.size() << " neurons" << std::endl;

#  endif

#  if 0
  ZFileList fileList;
  int count = 0;
  fileList.load("/Users/zhaot/Work/ConnectomeHackathon2015/skeletons", "swc");
  for (int i = 0; i < fileList.size(); ++i) {
    const char *filePath = fileList.getFilePath(i);
    ZSwcTree tree;
    tree.load(filePath);
    ZSwcTree::DepthFirstIterator iter(&tree);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      if (SwcTreeNode::isRegular(tn)) {
        int x = iround(SwcTreeNode::x(tn) / 32);
        int y = iround(SwcTreeNode::y(tn) / 32);
        int z = iround(SwcTreeNode::z(tn) / 32);
        if (stack->getIntValue(x, y, z) > 0) {
          std::cout << filePath << endl;
          ++count;
          break;
        }
      }
    }
  }

  std::cout << count << " neurons" << std::endl;

  delete stack;
#  endif

#endif

#if 0
  ZFileList fileList;
  int count = 0;
  fileList.load(GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/skeletons", "swc");

  QSet<int> excludeSet;
  excludeSet << 141559 << 14678 << 197198 << 236103 << 28639 << 316822
             << 3353 << 5027 << 532540 << 535565 << 548083 << 598789 << 601420
             << 622288 << 641004 << 91300 << 5211;

  ZString outputDir =
      GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/skeletons_processed";

//  double lengthThre = 300.0;
  for (int i = 0; i < fileList.size(); ++i) {
    const char *filePath = fileList.getFilePath(i);
    int bodyId = ZString(filePath).lastInteger();
    ZString outputPath = outputDir + "/";
    outputPath.appendNumber(bodyId);
    outputPath += ".swc";

    if (!excludeSet.contains(bodyId)) {
      ZSwcTree tree;
      tree.load(filePath);

      ZSwcForest *forest = tree.toSwcTreeArray();

      if (forest->size() > 1) {
        double maxLength = 0.0;
        ZSwcTree *masterTree = NULL;

        for (size_t i = 0; i < forest->size(); ++i) {
          ZSwcTree *subtree = (*forest)[i];
          double length = subtree->length();
          if (length > maxLength) {
            masterTree = subtree;
            maxLength = length;
          }
        }
        masterTree->save(outputPath);
        ++count;
      } else {
        tree.load(filePath);
        tree.save(outputPath);
      }

      delete forest;
    }
  }

  std::cout << count << " trees" << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);
  ZDvidTileInfo tileInfo = reader.readTileInfo("graytiles");

  tileInfo.print();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  ZDvidTile *tile = reader.readTile("graytiles", 4, 0, 0, 6000);
  tile->printInfo();
  delete tile;

#endif

#if 0
  ZFileList fileList;
//  int count = 0;
  fileList.load("/Users/zhaot/Work/ConnectomeHackathon2015/skeletons", "swc");
  ZDvidTarget target;
  target.set("hackathon.janelia.org", "2a3", -1);
  ZDvidUrl dvidUrl(target);
  for (int i = 0; i < fileList.size(); ++i) {
    const char *filePath = fileList.getFilePath(i);
    int bodyId = ZString(filePath).lastInteger();

    QString command = QString("curl -X POST %1 --data-binary @%2").
        arg(dvidUrl.getSkeletonUrl(
              bodyId, target.getBodyLabelName()).c_str()).arg(filePath);
    /*
    QString command = QString(
          "curl -X POST %1/api/node/%2/skeletons/%3.swc"
          " --data-binary @%4").arg(m_dvidClient->getServer()).
        arg(m_dvidClient->getUuid()).
        arg(bodyId).arg(tmpPath);
        */

    qDebug() << command;

    QProcess::execute(command);
  }

#endif


#if 0
  ZFileList fileList;
  int count = 0;
  fileList.load(
        GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/skeletons", "swc");

  QSet<int> excludeSet;
  excludeSet << 197198 << 236103;

  QSet<int> namedBodySet;
  namedBodySet << 14678 << 5027 << 548083 << 598789 << 2158;

  ZString outputDir =
      GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/skeletons_labeled";

  double lengthThre = 300.0;
  const int smallLabel = 20;
  const int largeLabel = 21;
  for (int i = 0; i < fileList.size(); ++i) {
    const char *filePath = fileList.getFilePath(i);
    int bodyId = ZString(filePath).lastInteger();
    ZString outputPath = outputDir + "/";
    outputPath.appendNumber(bodyId);
    outputPath += ".swc";
    ZSwcTree tree;
    tree.load(filePath);

    if (tree.regularRootNumber() == 1) {
      tree.setType(0);
      tree.save(outputPath);
    } else {
      ++count;
      ZSwcForest *forest = tree.toSwcTreeArray();

      std::string comment;
      if (excludeSet.contains(bodyId)) {
        comment = "Bad segmentation";
        for (size_t i = 0; i < forest->size(); ++i) {
          ZSwcTree *tree = forest->getSwcTree(i);
          if (tree->length() >= lengthThre) {
            tree->setType(largeLabel);
          } else {
            tree->setType(smallLabel);
          }
        }
      } else if (namedBodySet.contains(bodyId)) {
        comment = "Reliable large fragments";
        for (size_t i = 0; i < forest->size(); ++i) {
          ZSwcTree *tree = forest->getSwcTree(i);
          if (tree->length() >= lengthThre) {
            tree->setType(0);
          } else {
            tree->setType(smallLabel);
          }
        }
      } else {
        ZSwcTree *masterTree = forest->getSwcTreeWithMaxLength();
        masterTree->setType(0);
        for (size_t i = 0; i < forest->size(); ++i) {
          ZSwcTree *tree = forest->getSwcTree(i);
          if (tree != masterTree) {
            if (tree->length() >= lengthThre) {
              tree->setType(largeLabel);
            } else {
              tree->setType(smallLabel);
            }
          }
        }
      }

      ZSwcTree *labeledTree = forest->toSwcTree();
      labeledTree->setComment(comment);
      labeledTree->save(outputPath);
      delete forest;
    }
  }

  std::cout << count << " trees" << std::endl;
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj = reader.readBody(568121);

  ZIntCuboid box = obj.getBoundBox();
  std::cout << box.getFirstCorner().toString() << std::endl;
  std::cout << box.getLastCorner().toString() << std::endl;

  ZObject3dScan slice = obj.getSlice(7999);
  std::cout << slice.getVoxelNumber() << std::endl;

#endif

#if 0
  ZJsonObject neuronJson;
  neuronJson.load("/Users/zhaot/Work/ConnectomeHackathon2015/neuronsinfo.json");

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2a3", -1);
  ZDvidWriter writer;
  writer.open(target);

  const char *key;
  json_t *value;
  ZJsonObject_foreach(neuronJson, key, value) {
    ZFlyEmBodyAnnotation annotation;

    ZJsonObject bodyJson;
    bodyJson.setEntry(key, value);
    annotation.loadJsonObject(bodyJson);
    annotation.print();

    ZJsonObject obj;
    obj.setEntry("name", annotation.getName());
    obj.setEntry("class", annotation.getType());

    writer.writeAnnotation(annotation.getBodyId(), obj);
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  std::set<int> bodyIdSet = reader.readBodyId(10000000);
  std::cout << bodyIdSet.size() << " bodies" << std::endl;

  std::vector<int> bodyIdArray;
  std::vector<int> bodySizeArray;
  std::vector<size_t> boundBoxSizeArray;

  for (std::set<int>::const_iterator iter = bodyIdSet.begin();
       iter != bodyIdSet.end(); ++iter) {
    int bodyId = *iter;
    bodyIdArray.push_back(bodyId);
    ZFlyEmNeuronBodyInfo bodyInfo = reader.readBodyInfo(*iter);
    bodySizeArray.push_back(bodyInfo.getBodySize());
    boundBoxSizeArray.push_back(bodyInfo.getBoundBox().getVolume());
  }

  size_t maxSizeIndex = 0;
  size_t maxBoxIndex = 0;
  int maxBodySize= 0;
  size_t maxBoxSize = 0;
  for (size_t i = 0; i < bodyIdArray.size(); ++i) {
    if (maxBodySize < bodySizeArray[i]) {
      maxBodySize = bodySizeArray[i];
      maxSizeIndex = bodyIdArray[i];
    }
    if (maxBoxSize < boundBoxSizeArray[i]) {
      maxBoxIndex = bodyIdArray[i];
      maxBoxSize = boundBoxSizeArray[i];
    }
  }

  std::cout << "Max body size: " << maxSizeIndex << ": " << maxBodySize
            << std::endl;
  std::cout << "Max box size: " << maxBoxIndex << ": " << maxBoxSize
            << std::endl;
#endif

#if 0 //Generate thumbnail for large bodies
  ZDvidTarget target;
  target.set("hackathon.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  ZDvidWriter writer;
  writer.open(target);

  std::set<int> bodyIdSet = reader.readBodyId(1000000);
  std::cout << bodyIdSet.size() << " bodies" << std::endl;

  ZFlyEmNeuronImageFactory imageFactory;
  imageFactory.setDownsampleInterval(7, 7, 7);
  imageFactory.setSizePolicy(ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                             ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                             ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX);
  size_t index = 0;
  for (std::set<int>::const_iterator iter = bodyIdSet.begin();
       iter != bodyIdSet.end(); ++iter, ++index) {
    std::cout << index << " / " << bodyIdSet.size() << std::endl;
    int bodyId = *iter;
    ZFlyEmNeuronBodyInfo bodyInfo = reader.readBodyInfo(*iter);
    if (bodyInfo.getBodySize() == 0) {
      ZObject3dScan body;
      reader.readBody(bodyId, &body);

      Stack *stack = imageFactory.createSurfaceImage(body);
      writer.writeThumbnail(bodyId, stack);

      ZFlyEmNeuronBodyInfo bodyInfo;
      bodyInfo.setBodySize(body.getVoxelNumber());
      bodyInfo.setBoundBox(body.getBoundBox());
      writer.writeBodyInfo(bodyId, bodyInfo.toJsonObject());
    }
  }

#endif

#if 0
  ZJsonObject jsonObject;
  jsonObject.load("/Users/zhaot/Work/ConnectomeHackathon2015/neuronsinfo.json");

  const char *key;
  json_t *value;
  int count = 0;
//  std::set<std::string> typeSet;
  std::map<std::string, int> typeLabelMap;
  std::vector<int> bodyIdArray;
  std::vector<int> labelArray;
  int maxLabel = 0;
  ZJsonArray idJson;
  ZJsonArray labelJson;

  ZJsonObject_foreach(jsonObject, key, value) {
    int bodyId = ZString(key).firstInteger();
    ZJsonObject bodyJson(value, ZJsonValue::SET_INCREASE_REF_COUNT);
    std::string type = ZJsonParser::stringValue(bodyJson["Type"]);
    if (typeLabelMap.count(type) == 0) {
      typeLabelMap[type] = ++maxLabel;
    }

    int label = typeLabelMap[type];

    bodyIdArray.push_back(bodyId);
    labelArray.push_back(label);
    idJson.append(bodyId);
    labelJson.append(label);

    ++count;
  }

  ZJsonObject rootJson;
  ZJsonArray typeArray;
  for (std::map<std::string, int>::const_iterator iter = typeLabelMap.begin();
       iter != typeLabelMap.end(); ++iter) {
    ZJsonObject typeJson;
    typeJson.setEntry(iter->first.c_str(), iter->second);
    typeArray.append(typeJson);
  }

  rootJson.setEntry("id", idJson);
  rootJson.setEntry("label", labelJson);
  rootJson.setEntry("label_type", typeArray);
  std::cout << rootJson.dumpString() << std::endl;

  rootJson.dump(GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/neuron_type.json");

  std::cout << count << " named neurons." << std::endl;
#endif

#if 0
  ZMatrix matrix;
  matrix.importTextFile(GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/simmat.txt");

  std::vector<int> idArray(matrix.getColumnNumber());
  for (int i = 0; i < matrix.getColumnNumber(); ++i) {
    idArray[i] = iround(matrix.getValue(0, i));
  }

  ZMatrix simmat;
  simmat.resize(matrix.getRowNumber() - 1, matrix.getColumnNumber());
  for (int j = 0; j < matrix.getColumnNumber(); ++j) {
    for (int i = 1; i < matrix.getRowNumber(); ++i) {
      simmat.set(i - 1, j, matrix.getValue(i, j));
    }
  }

  simmat.printInfo();

  for (int j = 0; j < simmat.getColumnNumber(); ++j) {
    double maxC = matrix.getValue(j + 1, j);
    for (int i = 0; i < simmat.getRowNumber(); ++i) {
      if (i == j) {
        simmat.set(i, j, 0);
      } else {
        double maxR = matrix.getValue(i + 1, i);
        //simmat.set(i, j, simmat.getValue(i, j));
        simmat.set(i, j, simmat.getValue(i, j) / dmax2(maxC, maxR));
      }
    }
  }

  std::map<int, int> idLabelMap;
  ZJsonObject typeJson;
  typeJson.load(GET_TEST_DATA_DIR + "/flyem/FIB/hackathon/neuron_type.json");
  ZJsonArray idArrayJson(typeJson["id"], ZJsonValue::SET_INCREASE_REF_COUNT);
  ZJsonArray labelArrayJson(typeJson["label"], ZJsonValue::SET_INCREASE_REF_COUNT);
  for (size_t i = 0; i < idArrayJson.size(); ++i) {
    int id = ZJsonParser::integerValue(idArrayJson.at(i));
    int label = ZJsonParser::integerValue(labelArrayJson.at(i));
    idLabelMap[id] = label;
  }

  int count = 0;
  for (int i = 0; i < simmat.getRowNumber(); ++i) {
    int predictedIndex = 0;
    simmat.getRowMax(i, &predictedIndex);
    std::cout << i << " " << predictedIndex << std::endl;
    int trueLabel = idLabelMap[idArray[i]];
    int predictedLabel = idLabelMap[idArray[predictedIndex]];
    if (trueLabel == predictedLabel) {
      ++count;
    }
  }

  std::cout << count << std::endl;
#endif

#if 0
  ZDvidBufferReader reader;
  //reader.open("emdata2.int.janelia.org", "628");
  tic();
  reader.isReadable(
          "http://emdata2.int.janelia.org/api/node/628/segmentation011314/info");
  ptoc();
  if (reader.isReadable(
        "http://emdata2.int.janelia.org/api/node/628/segmentation011314/sparsevol/1")) {
    std::cout << "Readable" << std::endl;
  } else {
    std::cout << "NOT readable" << std::endl;
  }
#endif

#if 0
  ZFlyEmNeuronDensity d1;
  ZFlyEmNeuronDensity d2;

  d1.append(0, 1);
  d1.append(10, 2);
  d1.append(20, 3);
  d1.append(30, 3);

  d2.append(0, 1);
  d2.append(10, 1);
  d2.append(20, 2);
  d2.append(30, 3);
  d2.append(40, 5);

  ZFlyemNeuronDensityMatcher matcher;
  double score = matcher.match(d1, d2);

  std::cout << score << std::endl;
#endif

#if 0
  ZWeightedPointArray pointArray;
  ZDvidTarget target;
  target.set("hackathon.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  std::string hackathonDir =
      "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/hackathon";

  ZString str;
  FILE *fp = fopen((hackathonDir + "/simmat.txt").c_str(), "r");
  str.readLine(fp);
  fclose(fp);

  std::vector<int> bodyList = str.toIntegerArray();

  for (std::vector<int>::const_iterator iter = bodyList.begin();
       iter != bodyList.end(); ++iter) {
    int bodyId = *iter;
    ZSwcTree *tree = reader.readSwc(bodyId);
    ZSwcTree::DepthFirstIterator iter(tree);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      pointArray.append(SwcTreeNode::center(tn), 1.0);
    }
    delete tree;
  }

  ZPoint pt = pointArray.principalDirection();
  pt.print();
#endif

#if 0
  ZPoint pt;
  pt.set(-0.164321, -0.138413, 0.976647);
  double theta, psi;
  Geo3d_Normal_Orientation(-0.164321, -0.138413, 0.976647, &theta, &psi);
  Geo3d_Rotate_Coordinate(pt.xRef(), pt.yRef(), pt.zRef(), theta, psi, _TRUE_);
  pt.print();
#endif

#if 0
  std::string hackathonDir =
      "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/hackathon";

  ZString str;
  FILE *fp = fopen((hackathonDir + "/simmat.txt").c_str(), "r");
  str.readLine(fp);
  fclose(fp);

  std::vector<int> bodyList = str.toIntegerArray();
  std::vector<ZFlyEmNeuronDensity> densityList(bodyList.size());

  ZString densityDir = hackathonDir + "/density1";


  ZMatrix simmat;
  simmat.resize(bodyList.size(), bodyList.size());

  for (size_t i = 0; i < bodyList.size(); ++i) {
    int bodyId = bodyList[i];
    ZString densityPath = densityDir + "/";
    densityPath.appendNumber(bodyId);
    densityPath += ".txt";
    ZFlyEmNeuronDensity &density = densityList[i];
    density.importTxtFile(densityPath);
  }

  ZFlyemNeuronDensityMatcher matcher;
  std::cout << "Matching ..." << std::endl;
  for (size_t i = 0; i < bodyList.size(); ++i) {
    std::cout << "Neuron: " << i << std::endl;
    const ZFlyEmNeuronDensity &density1 = densityList[i];
    for (size_t j = 0; j < bodyList.size(); ++j) {
      if (i <= j) {
        const ZFlyEmNeuronDensity &density2 = densityList[j];
        simmat.set(i, j, matcher.match(density1, density2));
      } else {
        simmat.set(i, j, simmat.at(j, i));
      }
    }
  }

  fp = fopen((densityDir + "/simmat.txt").c_str(), "w");
  for (size_t i = 0; i < bodyList.size(); ++i) {
    fprintf(fp, "%d ", bodyList[i]);
  }
  fprintf(fp, "\n");
  for (size_t i = 0; i < bodyList.size(); ++i) {
    for (size_t j = 0; j < bodyList.size(); ++j) {
      fprintf(fp, "%g ", simmat.at(i, j));
    }
    fprintf(fp, "\n");
  }

  fclose(fp);

#endif

#if 0
  ZDvidTarget target;
  target.set("hackathon.janelia.org", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);

  std::string hackathonDir =
      "/Users/zhaot/Work/neutube/neurolabi/data/flyem/FIB/hackathon";

  std::string outputDir = hackathonDir + "/density1";

  ZString str;
  FILE *fp = fopen((hackathonDir + "/simmat.txt").c_str(), "r");
  str.readLine(fp);
  fclose(fp);

  std::vector<int> bodyList = str.toIntegerArray();

  for (std::vector<int>::const_iterator iter = bodyList.begin();
       iter != bodyList.end(); ++iter) {
    int bodyId = *iter;
    ZObject3dScan obj = reader.readBody(bodyId);
    const std::map<int, size_t> &sizeMap = obj.getSlicewiseVoxelNumber();

    ZString outputPath = outputDir + "/";
    outputPath.appendNumber(bodyId);
    outputPath += ".txt";

    std::ofstream outStream(outputPath.c_str());
    for (std::map<int, size_t>::const_iterator iter = sizeMap.begin();
         iter != sizeMap.end(); ++iter) {
      outStream << iter->first << " " << iter->second << std::endl;
    }
    outStream.close();
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "628");
  target.setBodyLabelName("segmentation030115");
  //std::cout << reader.hasSparseVolume(1) << std::endl;
  //  reader.has
#endif

#if 0
  ZStackFrame *frame = new ZStackFrame;
  ZStack *stack = ZStackFactory::makeVirtualStack(
        ZIntCuboid(0, 0, 0, 6445, 6642, 8089));

  frame->loadStack(stack);

  ZDvidTarget target;
  target.set("http://emrecon100.janelia.priv", "2a3", -1);

  ZDvidReader reader;
  reader.open(target);


  ZDvidTile *tile = reader.readTile(3, 0, 0, 6000);
  tile->setDvidTarget(target);
  tile->printInfo();

  frame->document()->addObject(tile);

  tile = reader.readTile(3, 0, 1, 6000);
  frame->document()->addObject(tile);

  tile = reader.readTile(3, 1, 0, 6000);
  frame->document()->addObject(tile);

  tile = reader.readTile(3, 1, 1, 6000);
  frame->document()->addObject(tile);

//  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);
#endif

#if 0
  ZObject3dScan obj;
  obj.importDvidObject("/private/var/folders/bj/5d0p7tjx4jv12yzf0458g5fxf4nm2r/T/50000003.dvid");

  std::cout << obj.getVoxelNumber() << std::endl;

  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/backup/segmentation030115_seed_10492041.sobj");

  ZObject3dScan objArray[4];
//  objArray[0].importDvidObject(GET_TEST_DATA_DIR + "/backup/10492041_50000001.dvid");
//  objArray[1].importDvidObject(GET_TEST_DATA_DIR + "/backup/10492041_50000002.dvid");
//  objArray[2].importDvidObject(GET_TEST_DATA_DIR + "/backup/10492041_50000003.dvid");
//  objArray[3].importDvidObject(GET_TEST_DATA_DIR + "/backup/10492041_50000004.dvid");

  objArray[0].load("/private/var/folders/bj/5d0p7tjx4jv12yzf0458g5fxf4nm2r/T/body_10492041_1.sobj");
  objArray[1].load("/private/var/folders/bj/5d0p7tjx4jv12yzf0458g5fxf4nm2r/T/body_10492041_2.sobj");
  objArray[2].load("/private/var/folders/bj/5d0p7tjx4jv12yzf0458g5fxf4nm2r/T/body_10492041_3.sobj");
  objArray[3].load("/private/var/folders/bj/5d0p7tjx4jv12yzf0458g5fxf4nm2r/T/body_10492041_4.sobj");


  obj.subtract(objArray[0]);
  obj.subtract(objArray[1]);
  obj.subtract(objArray[2]);
  obj.subtract(objArray[3]);


//  ZObject3dScan remained = obj.subtract(obj2);


  std::cout << obj.getVoxelNumber() << std::endl;

  std::vector<ZObject3dScan> compArray = obj.getConnectedComponent();

  std::cout << compArray.size() << " components" << std::endl;

  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

  obj.subtract(compArray[0]);

  //std::cout << compArray[0].getVoxelNumber() + compArray[1].getVoxelNumber() << std::endl;

//  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZObject3dScan wholeObj;
  wholeObj.load(GET_TEST_DATA_DIR + "/backup/segmentation030115_seed_10492041.sobj");

  ZObject3dScan wholeObj2;
  for (size_t i = 0; i < 4; ++i) {
    wholeObj2.concat(objArray[i]);
  }

  for (size_t i= 0; i < compArray.size(); ++i) {
    wholeObj2.concat(compArray[i]);
  }

  std::cout << wholeObj.getVoxelNumber() << std::endl;
  std::cout << wholeObj2.getVoxelNumber() << std::endl;

  wholeObj2.canonize();
  std::cout << wholeObj.equalsLiterally(wholeObj2) << std::endl;

//  std::cout << obj2.getVoxelNumber() << std::endl;
//  std::cout << remained.getVoxelNumber() << std::endl;
#endif

#if 0
  ZFileList fileList;
  fileList.load(GET_TEST_DATA_DIR + "/backup", "dvid");

  ZString outputDir = GET_TEST_DATA_DIR + "/backup/splits";

  for (int i = 0; i < fileList.size(); ++i) {
    std::string filePath = fileList.getFilePath(i);
    std::cout << filePath << std::endl;

    ZObject3dScan obj;
    obj.importDvidObject(filePath);

    size_t voxelNumber = obj.getVoxelNumber();
    std::vector<ZObject3dScan> objArray = obj.getConnectedComponent();

    std::cout << objArray.size() << " components" << std::endl;

    size_t checkVoxelNumber = 0;
    int index = 1;
    for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter, ++index) {
      const ZObject3dScan &subobj = *iter;
      checkVoxelNumber += subobj.getVoxelNumber();

      ZString outputPath =
          outputDir + "/" +
          ZString::removeFileExt(ZString::getBaseName(filePath)) + "_";
      outputPath.appendNumber(index, 3);
      outputPath += ".dvid";
      subobj.exportDvidObject(outputPath);
    }

    if (voxelNumber != checkVoxelNumber) {
      std::cout << voxelNumber << " " << checkVoxelNumber << std::endl;
      std::cout << "unmathced sizes" << std::endl;
      return;
    }

  }

#endif

#if 0
  ZDvidVersionDag dag;
  dag.setRoot("root");
  dag.addNode("v1", "root");
  dag.addNode("v2", "root");
  dag.addNode("v3", "v1");
  dag.addNode("v3", "v2");
  dag.addNode("v4", "v3");
  dag.addNode("v5", "v4");
  dag.addNode("v4", "v5");

  dag.print();

  std::cout << dag.getChild("root", 0) << std::endl;
  std::cout << dag.getChild("root", 1) << std::endl;
  std::cout << dag.getParentList("v3").size() << std::endl;

  ZDvidVersionDag dag2;
  dag2 = dag;
  dag2.print();

#endif

#if 0
  ZDvidTarget target("emdata4.int.janelia.org", "a21a", 8900);
  ZDvidReader reader;
  reader.open(target);

  ZDvidVersionDag dag = reader.readVersionDag();
  dag.print();
#endif

#if 0
  ZDvidBufferReader reader;
  tic();
  reader.read("http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/blocks/104_174_79/10");
  ptoc();

  tic();
  reader.read("http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/raw/0_1_2/320_32_32/3104_5568_3520");
  ptoc();

  tic();
  reader.read("http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/raw/0_1_2/320_32_32/3104_5568_3520");
  ptoc();

  tic();
  reader.read("http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/blocks/104_174_78/10");
  ptoc();


  tic();
  reader.read("http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/raw/0_1_2/320_32_32/3104_5568_3520");
  ptoc();
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "cf6e", 7000);
  ZDvidReader reader;
  reader.open(target);
  tic();
  reader.readSparseStack(8376505);
  ptoc();
#endif

#if 0
  std::string url = "http://emdata1.int.janelia.org:7000/api/node/cf6e/grayscale/raw/0_1_2/320_32_32/3104_5568_3520";
  std::cout << url << std::endl;
  std::cout << ZDvidUrl::GetEndPoint(url) << std::endl;

  ZDvidTarget target;
  target.setFromUrl(url);
  target.print();
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "628");

  ZDvidReader reader;
  reader.open(target);

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside.sobj");
  ZObject3dScan block1 = dvidInfo.getBlockIndex(obj1);
  obj1.clear();
  block1.save(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block.sobj");

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/entire_mb.sobj");
  ZObject3dScan block2 = dvidInfo.getBlockIndex(obj2);
  obj2.clear();
  block2.save(GET_TEST_DATA_DIR + "/flyem/MB/entire_mb_block.sobj");

  block1.subtract(block2);
  block1.save(GET_TEST_DATA_DIR + "/flyem/MB/ring.sobj");

  ZJsonArray array = ZJsonFactory::makeJsonArray(
        block1, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/ring.json");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/synapse_roi_test.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;


  ZJsonArray array = ZJsonFactory::makeJsonArray(
        obj1, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/synapse_roi_test.json");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block.sobj");
  std::cout << obj2.getVoxelNumber() << std::endl;


  ZJsonArray array = ZJsonFactory::makeJsonArray(
        obj2, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block.json");

  obj1.subtract(obj2);
  obj1.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;

//  ZJsonArray array = ZJsonFactory::makeJsonArray(
//        obj1, ZJsonFactory::OBJECT_SPARSE);
//  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.json");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "78b6", 7000);

  ZDvidReader reader1;
  reader1.open(target);

  ZObject3dScan obj = reader1.readBody(17160251);

  int x1 = 0;
  int x2 = 0;
  int y = 0;
  int z = 0;
  obj.getSegment(0, &z, &y, &x1, &x2);
  std::cout << x1 << " " << y << " " << z << std::endl;
  std::cout << obj.getVoxelNumber() << std::endl;


  obj = reader1.readBody(17160304);
  obj.getSegment(0, &z, &y, &x1, &x2);
  std::cout << x1 << " " << y << " " << z << std::endl;
  std::cout << obj.getVoxelNumber() << std::endl;

#endif


#if 0
  int startId = 17159585;
//  int endId = startId + 10;
  int endId = 17161095;

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "78b6", 7000);
//  target
  ZDvidReader reader1;
  reader1.open(target);

  ZDvidReader reader2;
  target.set("emdata1.int.janelia.org", "cf6e", 7000);
  reader2.open(target);

//  ZObject3dScanArray objArray;
  int x1 = 0;
  int x2 = 0;
  int y = 0;
  int z = 0;

  std::map<int, std::vector<int> > bodyMap;

  for (int id = startId; id <= endId; ++id) {
    ZObject3dScan obj = reader1.readBody(id);

    if (!obj.isEmpty()) {
      obj.getSegment(0, &z, &y, &x1, &x2);

      ZArray *array = reader2.readLabels64("labels",
                                           x1, y, z, 1, 1, 1);

      //    std::set<int> bodySet = reader2.readBodyId(x1, y, z, 1, 1, 1);
      //    int oldId = *(bodySet.begin());
      int oldId = array->getUint64Value(0);

      delete array;

      std::cout << oldId << " => " << id << std::endl;

      if (bodyMap.count(oldId) == 0) {
        bodyMap[oldId] = std::vector<int>();
        ZObject3dScan obj2 = reader1.readBody(oldId);
        if (!obj2.isEmpty()) {
          obj2.getSegment(0, &z, &y, &x1, &x2);
          ZArray *array = reader2.readLabels64("labels",
                                               x1, y, z, 1, 1, 1);
          int overlapId = array->getUint64Value(0);
          if (oldId == overlapId) {
            bodyMap[oldId].push_back(oldId);
          } else {
            std::cout << "Inconsistent ID" << std::endl;
            return;
          }
          delete array;
        }
      }

      bodyMap[oldId].push_back(id);
    }
  }

  for (std::map<int, std::vector<int> >::const_iterator iter = bodyMap.begin();
       iter != bodyMap.end(); ++iter) {
//    int oldId = iter->first;
    const std::vector<int> &idArray = iter->second;
//    std::cout << oldId << std::endl;
    for (std::vector<int>::const_iterator iter = idArray.begin();
         iter != idArray.end(); ++iter) {
      std::cout << *iter << " ";
    }
    std::cout << std::endl;
  }

#endif

#if 0
  ZDvidBufferReader reader;
  reader.read("http://emdata2.int.janelia.org/api/node/628/mbroi/roi");
  QByteArray buffer = reader.getBuffer();

  ZJsonArray array;
  array.decodeString(buffer.constData());

  ZObject3dScan obj;
  obj.importDvidRoi(array);

  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZObject3dScan obj1;
  ZJsonArray array1;
  array1.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.json");
  obj1.importDvidRoi(array1);

  ZObject3dScan obj2;
  ZJsonArray array2;
  array2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_check.json");
  obj2.importDvidRoi(array2);
  obj2.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_check.sobj");


  ZIntCuboid box = obj1.getBoundBox();
  box.join(obj2.getBoundBox());

  ZStack *stack = ZStackFactory::makeZeroStack(GREY, box, 3);
  int offset[3];
  offset[0] = -stack->getOffset().getX();
  offset[1] = -stack->getOffset().getY();
  offset[2] = -stack->getOffset().getZ();

  obj1.drawStack(stack->singleChannelStack(0)->data(), 255, offset);
  obj2.drawStack(stack->singleChannelStack(1)->data(), 255, offset);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;

  ZObject3dScan obj2;
  ZJsonArray array2;
  array2.load(GET_TEST_DATA_DIR + "/test.json");
  obj2.importDvidRoi(array2);

  ZObject3dScan obj3 = obj1;

  obj3.subtract(obj2);

  ZIntCuboid box = obj1.getBoundBox();
  box.join(obj2.getBoundBox());

  ZStack *stack = ZStackFactory::makeZeroStack(GREY, box, 3);
  int offset[3];
  offset[0] = -stack->getOffset().getX();
  offset[1] = -stack->getOffset().getY();
  offset[2] = -stack->getOffset().getZ();

  obj1.drawStack(stack->singleChannelStack(0)->data(), 255, offset);
  obj2.drawStack(stack->singleChannelStack(1)->data(), 255, offset);
  obj3.drawStack(stack->singleChannelStack(2)->data(), 255, offset);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");

  obj3.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;

  ZJsonArray array = ZJsonFactory::makeJsonArray(
        obj3, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.json");
#endif

#if 0
  ZImage image(5, 5);
  ZStTransform transform;
  transform.setOffset(-1, -2);
  image.setTransform(transform);
  ZPainter painter(&image);
//  painter.drawPoint(1, 2);
  ZObject3dScan obj;
  obj.addSegment(0, 2, 1, 2);
  obj.setColor(0, 0, 0);
  obj.display(painter, 0, ZStackObject::SOLID);

  image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  ZIntCuboidArray blockArray;
  blockArray.loadSubstackList(dataPath + "/flyem/FIB/block_13layer_extended.txt");

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6c");

  ZDvidReader reader;
  reader.open(target);

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  ZObject3dScan obj = dvidInfo.getBlockIndex(blockArray);
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
  std::cout << "Number of blocks (13layer extended): " << obj.getVoxelNumber()
            << std::endl;

  ZJsonArray jsonArray = ZJsonFactory::makeJsonArray(obj);
  jsonArray.dump(GET_TEST_DATA_DIR +
                 "/flyem/FIB/block_13layer_extended_roi.json");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.addSegment(0, 0, 0, 1);

//  obj1.interpolateSlice(1).print();

  obj1.addSegment(2, 0, 0, 10);
  obj1.interpolateSlice(1).print();


#endif


#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  stack.setOffset(10, 20, 0);

  stack.save(GET_TEST_DATA_DIR + "/test.tif");

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "cf6e", 7000);

  ZDvidReader reader;
  if (reader.open(target)) {
    std::string dataName = ZDvidData::getName(ZDvidData::ROLE_SPLIT_STATUS);
    if (reader.hasData(dataName)) {
      std::cout << dataName << " exists." << std::endl;
    } else {
      std::cout << dataName << " does not exist." << std::endl;
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "f94a", 8500);
  target.setBodyLabelName("bodies121714");

  ZDvidReader reader;
  reader.open(target);

  if (reader.hasSparseVolume(1)) {
    std::cout << "body " << 1 << " exists." << std::endl;
  } else {
    std::cout << "body check failed." << std::endl;
  }

#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");

  int dsScale = 20;
  ZPointArray ptArray;
  ptArray.append(7937, 9175, 3485);
  ptArray.append(7219,  8703, 1422);
  ptArray.append(11326, 7040, 3131);
  ptArray.append(7708, 10762, 2331);
  ptArray.append(2427,  6801, 4970);
  ptArray.append(9113,  5727, 4001);
  ptArray.append(3260 , 8181, 2982);
  ptArray.append(4030,  4335, 2222);
  ptArray.append(7974,  5245, 2691);
  ptArray.append(7816,  6988, 2723);

  for (size_t i = 0; i < ptArray.size(); ++i) {
    ZIntPoint pt = (ptArray[i] / dsScale).toIntPoint();
    int label = stack.getIntValue(pt.getX(), pt.getY(), pt.getZ());
    std::cout << "substack" << i + 1 << ": " << label << std::endl;
  }
#endif

#if 0
  FILE *fp = fopen((GET_DATA_DIR +
                   "/flyem/AL/glomeruli/labeled_synapse_confidence.txt").c_str(), "r");

//  std::vector<ZPointArray> synapseGroup(60);

  std::set<int> labelSet;

  ZString str;
  while (str.readLine(fp)) {
    std::vector<double> valueArray = str.toDoubleArray();
    if (valueArray.size() >= 5) {
      int label = iround(valueArray[3]);
      if (label == 49) {
        label = 7;
      }

      if (valueArray[2] < 3000) {
        labelSet.insert(label);
      }
    }
  }

  for (std::set<int>::const_iterator iter= labelSet.begin();
       iter != labelSet.end(); ++iter) {
    std::cout << *iter << std::endl;
  }

//      ZPointArray &ptArray = synapseGroup[label];
//      ptArray.append(ZPoint(valueArray[0], valueArray[1], valueArray[2]));


  fclose(fp);

//  int labelArray[] = {14, 15, 17, 18, 24, 25, 31, 33, 45, 46 };

//  for (size_t i = 0; i < sizeof(labelArray) / sizeof(int); ++i) {
//    int label = labelArray[i];
//    ZPoint pt = synapseGroup[label].computeCenter();
//    std::cout << label << ": " << pt.toIntPoint().toString() << std::endl;
//  }
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block_fixed.sobj");

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block.sobj");

  obj.subtract(obj2);

  obj.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_fixed.sobj");

  ZObject3dScan obj3;
  obj3.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.sobj");
  obj.subtract(obj3);
  obj.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_supp.sobj");


  ZObject3dScan obj4;
  obj4.concat(obj.getSlice(145, 148));
  obj4.concat(obj.getSlice(151, 161));
  obj4.concat(obj.getSlice(294, 301));
  obj4.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_supp_selected.sobj");

  ZJsonArray array = ZJsonFactory::makeJsonArray(
        obj4, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring_supp_selected.json");
  std::cout <<obj4.getVoxelNumber() << " blocks." << std::endl;

  obj3.unify(obj4);
  obj3.save(GET_TEST_DATA_DIR + "/flyem/MB/final_alpha_ring_fixed.sobj");
  array = ZJsonFactory::makeJsonArray(obj3, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/final_alpha_ring_fixed.json");
  std::cout << obj3.getVoxelNumber() << " blocks." << std::endl;

  ZObject3dScan oldRingObj;
  oldRingObj.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_ring.sobj");

  obj3.subtract(oldRingObj);

  std::cout << obj3.equalsLiterally(obj4) << std::endl;
//  obj3.subtract()



#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "f94a", 8500);

  tic();
  ZDvidReader reader;
  if (reader.open(target)) {
    reader.readGrayScale(2000, 7000, 2464, 1571, 3003, 1);
  }
  ptoc();
#endif

#if 0
  int width = 512;
  int height = 512;

  ZImage image(width, height);

  uint8_t *data = new uint8_t[width*height];
  int index = 0;
  tic();
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      data[index++] = 0;
    }
  }
  ptoc();

  image.setData(data, 0);

  std::vector<QPixmap> pixmapArray(20);

  size_t t = 0;
  tic();
  for (size_t i = 0; i < pixmapArray.size(); ++i) {
    pixmapArray[i].fromImage(image);
    t += toc();
  }

  std::cout << "Pixmap time: " << t << " ms." << std::endl;

#endif

#if 0
  size_t testCount = 20;
  std::vector<int> sizeArray(testCount, 0); //single dimension
  std::vector<uint64_t> initTimeArray(testCount, 0);
  std::vector<uint64_t> pixmapTimeArray(testCount, 0);

  int startSize = 512;
  for (size_t i = 1; i <= testCount; ++i) {
    int height = startSize * i;
    int width = startSize * i;
    sizeArray[i - 1] = width;

    ZImage image(width, height);

    uint8_t *data = new uint8_t[width*height];
    int index = 0;
    tic();
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        data[index++] = 0;
      }
    }
    ptoc();

    for (int j = 0; j < 10; ++j) {
      tic();
      image.setData(data, 0);
      initTimeArray[i - 1] += toc();

      tic();
      ZPixmap pixmap(width, height);
      pixmap.convertFromImage(image);
      pixmapTimeArray[i - 1] += toc();
    }

    delete data;
  }

  std::ofstream stream((GET_TEST_DATA_DIR + "/test.txt").c_str());
  for (size_t i = 0; i < testCount; ++i) {
    stream << sizeArray[i] << ", " << initTimeArray[i] << ", "
           << pixmapTimeArray[i] << std::endl;
  }
  stream.close();
#endif

#if 0
//  tic();
  ZPixmap pixmap(QSize(10000, 10000));
  pixmap.clearnUp();

  tic();
  pixmap.fill(Qt::white);
  ptoc();

  tic();
  QPixmap pixmap2 = pixmap.copy(QRect());
  std::cout << "Time elapsed: " << toc() << std::endl;

  ZImage image(10000, 10000);
  tic();
  image.fill(0);
  ptoc();

  tic();
  QPixmap pixmap3;
  pixmap3.fromImage(image);
  ptoc();

  std::cout << std::endl;
#endif

#if 0
  std::cout << new QFutureWatcher<void>(NULL) << std::endl;
#endif

#if 0
  ZPixmap pixmap(10000, 10000);

  ZPixmap pixmap2(10000, 10000);

//  ZPixmap pixmap3(10000, 10000);
  ZImage image(10000, 10000);

  tic();
  pixmap.fill(Qt::transparent);
  ptoc();


  tic();
  pixmap2.fill(Qt::transparent);
  ptoc();



//  tic();
//  pixmap3.fill(Qt::transparent);
//  ptoc();

  tic();
  pixmap.cleanUp();
  ptoc();
  std::cout << "1 inited" << std::endl;

  tic();
  pixmap2.cleanUp();
  ptoc();
  std::cout << "2 inited" << std::endl;


  tic();
  for (int i = 0; i < 100; ++i) {
//    pixmap3.fill(Qt::transparent);
    for (int j = 0; j < 100000000; ++j);

    pixmap.cleanUp();
    std::cout << "1 cleaned" << std::endl;

//    pixmap2.cleanUp();
//    std::cout << "2 cleaned" << std::endl;
  }
  ptoc();

//  pixmap3.cleanUp();

//  tic();
//  pixmap.cleanUp();

//  pixmap3.cleanUp();

//  pixmap2.cleanUp();

//  pixmap.cleanUp();

//  pixmap2.cleanUp();
//  ptoc();
//  tic();
//  QApplication::processEvents();
//  ptoc();

//  tic();
//  pixmap3.cleanUp();
//  ptoc();

  std::cout << std::endl;
#endif

#if 0
  libdvid::DVIDNodeService service("http://emdata1.int.janelia.org:8500", "ccf");
//  std::string endPoint = ZDvidUrl::GetEndPoint(url.toStdString());

  libdvid::Dims_t dims(3);
//  2968_3066_4045
  dims[0] = 512;
  dims[1] = 512;
  dims[2] = 1;

  std::vector<unsigned int> offset(3);
  offset[0] = 2968;
  offset[1] = 3066;
  offset[2] = 4045;
  std::vector<unsigned int> channels(3);
  channels[0] = 0;
  channels[1] = 1;
  channels[2] = 2;

  tic();
  try {
    libdvid::Labels3D data = service.get_labels3D(
          "labels", dims, offset, channels);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  ptoc();
  std::cout << std::endl;
#endif

#if 0
  libdvid::DVIDNodeService service("http://emdata2.int.janelia.org:8500", "b6bc");
//  std::string endPoint = ZDvidUrl::GetEndPoint(url.toStdString());

  libdvid::Dims_t dims(3);
//  2968_3066_4045
  dims[0] = 512;
  dims[1] = 512;
  dims[2] = 1;

  std::vector<int> offset(3);
  offset[0] = 1;
  offset[1] = 1;
  offset[2] = 7313;
  std::vector<unsigned int> channels(3);
  channels[0] = 0;
  channels[1] = 1;
  channels[2] = 2;

  tic();
  try {
    libdvid::BinaryDataPtr data =
        service.get_tile_slice_binary("tiles", libdvid::XY, 3, offset);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
  ptoc();
  std::cout << std::endl;

  service.custom_request("/bodies/sparsevol/200000603?minz=7313&maxz=7313",
                         libdvid::BinaryDataPtr(), libdvid::GET, true);

#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("http://emdata2.int.janelia.org:8500", "b6bc");

  target.print();

  reader.open(target);
  reader.readTileInfo("tiles");

  reader.readBody(200000603, 7313, 7313, NeuTube::Z_AXIS, NULL);
#endif



#if 0
  ZPainter painter;
  ZPixmap pixmap(10000, 10000);
  pixmap.fill(Qt::black);
  painter.begin(&pixmap);

  ZPixmap pixmap2(10000, 10000);
  tic();
  pixmap2.fill(Qt::white);
  ptoc();

  tic();
//  pixmap2.clearnUp();

  painter.drawPixmap(0, 0, pixmap2);
  painter.end();
  ptoc();

  std::cout << std::endl;

  pixmap.toImage().save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  ZPainter painter;
  ZImage image(256, 256);
  image.setScale(0.5, 0.5);
  image.setOffset(10, 20);
  painter.begin(&image);

  ZPixmap pixmap(256, 256);
  pixmap.clearnUp();
  pixmap.fill(Qt::blue);
  painter.drawPixmap(30, 40, pixmap);

  painter.end();

  image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  ZDvidTarget target("emdata1", "51d", 8500);
  target.setLabelBlockName("labels");
  target.setBodyLabelName("bodies");
  ZFlyEmDvidReader reader;
  reader.open(target);
  ZObject3dScan obj = reader.readCoarseBody(14742253);
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

#endif

#if 0
  ZDvidTarget target;
  target.set()
#endif

#if 0
  ZFlyEmBodyMerger bodyMerger;
  bodyMerger.pushMap(1, 2);
  bodyMerger.pushMap(2, 3);
  bodyMerger.pushMap(4, 3);
  bodyMerger.pushMap(5, 3);
  bodyMerger.pushMap(3, 6);
  bodyMerger.pushMap(7, 8);

  bodyMerger.print();

  ZJsonArray jsonArray = bodyMerger.toJsonArray();
  std::cout << jsonArray.dumpString(2) << std::endl;



#endif

#if 0
  ZDvidTarget target("emdata2.int.janelia.org", "79b", 7000);
  target.setBodyLabelName("m10_lo_bodies");

  /*
  std::vector<int> array;
  array.push_back(21069256);
  array.push_back(24500537);
  array.push_back(24467491);
  array.push_back(24488605);
  array.push_back(24758629);
*/
  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan obj1 = reader.readBody(24758629);
//    ZObject3dScan obj2 = reader.readBody(21054499);
//    obj1.concat(obj2);
//    obj2 = reader.readBody(10362053);
//    obj1.concat(obj2);
//    obj1.canonize();
    obj1.save(GET_TEST_DATA_DIR + "/test.sobj");
  }
#endif

#if 0
  ZIntPoint dsIntv = misc::getDsIntvFor3DVolume(29);
  std::cout << dsIntv.toString() << std::endl;
#endif

#if 0
  std::cout << qgetenv("USERNAME").data() << std::endl;
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "6f15", 8500);
  ZDvidReader reader;
  reader.open(target);

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  ZIntPoint index1 = dvidInfo.getBlockIndex(0, 0, 5100);
  std::cout << index1.getZ() << std::endl;

  ZIntPoint index2 = dvidInfo.getBlockIndex(0, 0, 9000);
  std::cout << index2.getZ() << std::endl;


  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block.sobj");

  std::cout << obj.getVoxelNumber() << std::endl;

  obj.getSlice(obj.getMinZ(), index1.getZ()).save(
        GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut1.sobj");

  obj.getSlice(index1.getZ() + 1, index2.getZ()).save(
        GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut2.sobj");

  obj.getSlice(index2.getZ() + 1, obj.getMaxZ()).save(
        GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut3.sobj");

  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut1.sobj");

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut2.sobj");

  ZObject3dScan obj3;
  obj3.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut3.sobj");

  std::cout << obj1.getVoxelNumber() + obj2.getVoxelNumber() +
               obj3.getVoxelNumber() << std::endl;
#endif

#if 0

  {
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut1.sobj");
  ZJsonArray array1 = ZJsonFactory::MakeJsonArray(
        obj1, ZJsonFactory::OBJECT_SPARSE);
  array1.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut1.json");
  }


  {
  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut2.sobj");
  ZJsonArray array2 = ZJsonFactory::MakeJsonArray(
        obj2, ZJsonFactory::OBJECT_SPARSE);
  array2.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut2.json");
  }

  {
  ZObject3dScan obj3;
  obj3.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut3.sobj");
  ZJsonArray array3 = ZJsonFactory::MakeJsonArray(
        obj3, ZJsonFactory::OBJECT_SPARSE);
  array3.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut3.json");
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target("emdata1.int.janelia.org", "9db", 8500);
  reader.open(target);

  ZObject3dScan obj = reader.readRoi("mb_subtracted");
  std::cout << obj.getVoxelNumber() << std::endl;
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  std::string filePath = GET_TEST_DATA_DIR + "/flyem/MB/aftersplit.results";
  std::string outFile = GET_TEST_DATA_DIR + "/flyem/MB/aftersplit2500.txt";

  int count = 0;

  FILE *fp = fopen(filePath.c_str(), "r");
  FILE *outFp = fopen(outFile.c_str(), "w");

  ZString str;
  while (str.readLine(fp)) {
    int bodyId = str.firstInteger();
    if (bodyId > 0) {
      fprintf(outFp, "%d\n", bodyId);
      ++count;
    }
    if (count >= 2500) {
      break;
    }
  }

  fclose(fp);
  fclose(outFp);
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "c348", 8500);
  ZDvidReader reader;
  if (reader.open(target)) {
    ZStack *stack = reader.readGrayScale(2600, 5200, 3958, 3200, 2400, 1);
    stack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/grayscale.tif");
  }

#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut1.sobj");
  std::cout << "red: " << obj1.getVoxelNumber() << std::endl;


  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut2.sobj");
  std::cout << "gree: " << obj2.getVoxelNumber() << std::endl;

  ZObject3dScan obj3;
  obj3.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_block_cut3.sobj");
  std::cout << "blue: " << obj3.getVoxelNumber() << std::endl;

  ZIntCuboid box = obj1.getBoundBox();
  box.join(obj2.getBoundBox());
  box.join(obj3.getBoundBox());

  ZStack *stack = ZStackFactory::makeZeroStack(COLOR, box);

  obj1.drawStack(stack, 255, 0, 0);
  obj2.drawStack(stack, 0, 255, 0);
  obj3.drawStack(stack, 0, 0, 255);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/grayscale.tif");

  Stack *canvas = C_Stack::translate(stack.c_stack(0), COLOR, 0);
  int offset[3];
  offset[0] = -stack.getOffset().getX();
  offset[1] = -stack.getOffset().getY();
  offset[2] = -stack.getOffset().getZ();

  ZStack *canvasStack = ZStackFactory::makeZeroStack(COLOR, stack.getBoundBox(), 1);
  C_Stack::copyValue(canvas, canvasStack->c_stack(0));

  ZDvidTarget target("emdata1.int.janelia.org", "c348", 8500);
  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan *obj = reader.readBody(7192024, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 255, 0, 0, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color1.tif");

    obj = reader.readBody(15940982, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 0, 255, 0, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color2.tif");

    obj = reader.readBody(13770783, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 0, 255, 255, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color3.tif");

    obj = reader.readBody(14571660, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 255, 0, 255, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color4.tif");

    obj = reader.readBody(12574490, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 255, 255, 0, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color5.tif");

    obj = reader.readBody(9925442, 3958, NULL);
    obj->getSlice(3958).drawStack(canvasStack->c_stack(), 0, 0, 255, 0.5, offset);
    delete obj;
    canvasStack->save(GET_TEST_DATA_DIR + "/flyem/MB/script/cast/color6.tif");
  }
#endif

#if 0
  ZStack stack;
//  stack.load(GET_TEST_DATA_DIR + "/00001.tif");

  stack.load(GET_TEST_DATA_DIR + "/bigneuron/gold166_trainingsubset79/"
             "checked6_chick_uw/DONE_09-2902-04R-01C-60x_merge_c1/"
             "09-2902-04R-01C-60x_merge_c1.v3dpbd");

  Stack *stackData = NULL;

  if (stack.kind() == COLOR) {
    stackData = C_Stack::channelExtraction(stack.c_stack(0), 1);
  } else {
    if (stack.channelNumber() > 1) {
      stackData = stack.c_stack(1);
    } else {
      stackData = stack.c_stack(0);
    }
  }

  int intensityMode = C_Stack::mode(stackData);
  int intensityMean = C_Stack::mean(stackData);

  if (intensityMode > intensityMean) {
    std::cout << "Bright field detected." << std::endl;
    Stack_Invert_Value(stackData);
  } else {
    std::cout << "Dark field detected." << std::endl;
  }

  JNeuronTracer tracer;
//  Stack *mask = tracer.makeMask(stack.c_stack());


  ZSwcTree *tree = tracer.trace(stackData);
  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/system/zjump_test.swc");

  Biocytin::SwcProcessor::breakZJump(&tree, 2.0);

  tree.save(GET_TEST_DATA_DIR + "/test.swc");

#endif

#if 0
  QDateTime time = QDateTime::currentDateTime().toLocalTime();

  qDebug() << time.toString("yyyy-MM-dd hh:mm:ss");
#endif

#if 0
  QProcess::execute(
        "curl",
        QStringList() << "-g" << "-X GET" << "http://emdata1.int.janelia.org:8500/api/help");

  QProcess process;
//  process.setWorkingDirectory("/Users/zhaot/anaconda/bin");
  process.start("curl", QStringList() << "-g" << "-X" << "GET" << "http://emdata1.int.janelia.org:8500/api/help");

  if (process.waitForStarted(-1)) {
    qDebug() << "Process started.";
  }

  if (process.waitForFinished(-1)) {
    qDebug() << process.readAllStandardOutput();
  }

#endif

#if 0
  QString str("\"MaxLabel\": {\"test\": 12433534}; other {}");
  str.remove(QRegExp("\"MaxLabel\":\\s*\\{[^{}]*\\}"));
  qDebug() << str;
#endif

#if 0
  ZFlyEmSupervisor supervisor;

  std::cout << supervisor.getMainUrl() << std::endl;

  std::cout << supervisor.getCheckinUrl("1234") << std::endl;
  std::cout << supervisor.getCheckoutUrl("1234") << std::endl;

  std::cout << supervisor.getCheckinUrl("1234", 100) << std::endl;
  std::cout << supervisor.getCheckoutUrl("1234", 100) << std::endl;

  ZDvidTarget target("emdata1.int.janelia.org", "abcd", 8500);
  supervisor.setDvidTarget(target);

  std::cout << supervisor.getCheckinUrl(100) << std::endl;
  std::cout << supervisor.getCheckoutUrl(100) << std::endl;

//  std::cout << supervisor.checkIn(100) << std::endl;
  std::cout << supervisor.checkOut(100) << std::endl;



#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/flyem/MB/proofread/fix1";
  ZDvidTarget target2("emdata1.int.janelia.org", "0f33", 8500);
  ZDvidReader reader;
  reader.open(target2);
  ZObject3dScan wholeObj =reader.readBody(13707636);
  wholeObj.canonize();
  wholeObj.save(dataDir + "/13707636_s.sobj");
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "c0a5", 8500);

  std::string dataDir = GET_TEST_DATA_DIR + "/flyem/MB/proofread/fix1";

  ZDvidReader reader;


  ZDvidTarget target2("emdata1.int.janelia.org", "0f33", 8500);
  reader.open(target2);
  ZObject3dScan wholeObj =reader.readBody(13707636);
//  wholeObj.save(dataDir + "/13707636_s.sobj");

#  if 1
  ZDvidWriter writer;
  writer.open(target2);

  if (reader.open(target)) {
    FILE *fp = fopen((dataDir  + "/body_splits.csv").c_str(), "r");
    ZString str;

    int label = 1;
    std::set<uint64_t> bodySet;

    while (str.readLine(fp)) {
      std::vector<int> coords = str.toIntegerArray();
      if (coords.size() == 3) {
        uint64_t bodyId = reader.readBodyIdAt(coords[0], coords[1], coords[2]);
        std::cout << "ID: " << bodyId << std::endl;
        bodySet.insert(bodyId);
      }
    }
    fclose(fp);

    for (std::set<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter) {
      uint64_t bodyId = *iter;
      ZObject3dScan obj = reader.readBody(bodyId);
      obj.canonize();
      QString outputPath = QString("%1/body_%2.dvid").arg(dataDir.c_str()).
          arg(bodyId);
      obj.exportDvidObject(outputPath.toStdString());

      ZObject3dScan intersected = obj.intersect(wholeObj);
      intersected.exportDvidObject(
            QString("%1/body_%2_intersected.dvid").arg(dataDir.c_str()).
            arg(bodyId).toStdString());

      writer.writeSplit(intersected, 13707636, label++);
    }
  } else {
    std::cout << "Failed to open " << target.getSourceString() << std::endl;
  }
#  endif
#endif

#if 0
  ZDvidBufferReader reader;
  reader.read("http://emdata2.int.janelia.org:9100/state/ee7dc");
  const QByteArray &buffer = reader.getBuffer();

  ZJsonArray array;
  array.decodeString(buffer.data());

  array.print();

  ZFlyEmSupervisor supervisor;
  supervisor.setDvidTarget(ZDvidTarget("emdata1.int.janelia.org", "ee7dc", 8500));
  supervisor.setUserName("changl");

  for (size_t i = 0; i < array.size(); ++i) {
    ZJsonObject obj(array.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    std::string userName = ZJsonParser::stringValue(obj["Client"]);
    if (userName == "takemurasa") {
      supervisor.setUserName(userName);
      std::cout << ZJsonParser::integerValue(obj["Label"]) << std::endl;
      supervisor.checkIn(ZJsonParser::integerValue(obj["Label"]));
    }
  }
#endif

#if 0
  ZString text = "split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=split <username=<username=zhaot>";
  if (text.contains("<username=")) {
    std::string::size_type pos = text.rfind("<username=") +
        std::string("<username=").size();
    std::string::size_type lastPos = text.find_first_of(">", pos);
    ZString userName = text.substr(pos, lastPos - pos);
    userName.trim();
    std::cout << userName << std::endl;
  }

#endif

#if 0
  ZWindowFactory factory;
  factory.setWindowTitle("Test");

  ZFlyEmBody3dDoc *doc = new ZFlyEmBody3dDoc;

  doc->updateFrame();

  ZDvidTarget dvidTarget("emdata1.int.janelia.org", "86e1", 8500);
  doc->setDvidTarget(dvidTarget);
  doc->updateFrame();

//  doc->loadFile((GET_TEST_DATA_DIR + "/benchmark/em_stack.tif").c_str());

  Z3DWindow *window = factory.make3DWindow(doc);
  window->setYZView();

  window->show();
  window->raise();

  doc->addEvent(ZFlyEmBody3dDoc::BodyEvent::ACTION_ADD, 12596838);
  doc->addEvent(ZFlyEmBody3dDoc::BodyEvent::ACTION_ADD, 13890100);
//  doc->addEvent(ZFlyEmBody3dDoc::BodyEvent::ACTION_REMOVE, 12596838);

//  doc->addBody(12596838);
//  doc->addBody(13890100);
#endif

#if 0
  ZFlyEmBody3dDoc::BodyEvent event1(
        ZFlyEmBody3dDoc::BodyEvent::ACTION_ADD, 1200);

  ZFlyEmBody3dDoc::BodyEvent event2(
        ZFlyEmBody3dDoc::BodyEvent::ACTION_REMOVE, 1200);

  event1.mergeEvent(event2, NeuTube::DIRECTION_FORWARD);
  event1.print();

  event2.mergeEvent(event1, NeuTube::DIRECTION_FORWARD);
  event2.print();
#endif

#if 0
  ZFlyEmBody3dDoc doc;
  std::vector<uint64_t> bodyIdArray;
  bodyIdArray.push_back(1);
  bodyIdArray.push_back(2);

  doc.addBodyChangeEvent(bodyIdArray.begin(), bodyIdArray.end());
  doc.printEventQueue();
#endif

#if 0
  ZString str = "232435232-53634643637-423422222222893";
  std::vector<uint64_t> array = str.toUint64Array();
  for (size_t i = 0; i < array.size(); ++i) {
    std::cout << array[i] << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("http://emdata1.int.janelia.org", "1f62", 8500);

  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan obj = reader.readRoi("mbroi");
    obj.canonize();
    obj.save(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/mbroi.sobj");

    ZObject3dScan obj2;
    obj2.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block_fixed.sobj");
//    obj2.save(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/merged.sobj");

    obj2.subtract(obj);
    obj2.save(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/mb_subtracted.sobj");

    if (obj.hasOverlap(obj2)) {
      std::cout << "WARNING: mbroi and mb_subtracted has overlap." << std::endl;
    } else {
      std::cout << "mbroi and mb_subtracted has no overlap." << std::endl;
    }

    obj.unify(obj2);
    obj.save(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/merged.sobj");
  }

#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/mb_subtracted.sobj");
  std::cout << obj1.getVoxelNumber() << std::endl;


  ZJsonArray array = ZJsonFactory::MakeJsonArray(
        obj1, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/mb_subtracted.json");
#endif

#if 0
  ZDvidTarget target;
  target.set("http://emdata1.int.janelia.org", "1f62", 8500);

  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan obj = reader.readRoi("mbroi");
    obj.canonize();

    ZDvidReader reader2;
    target.set("http://emdata1.int.janelia.org", "d6959", 8500);
    reader2.open(target);

    ZObject3dScan obj2 = reader2.readRoi("mb_subtracted_v3");

    if (obj.hasOverlap(obj2)) {
      std::cout << "WARNING: mbroi and mb_subtracted has overlap." << std::endl;
    } else {
      std::cout << "mbroi and mb_subtracted has no overlap." << std::endl;
    }

    obj.unify(obj2);
    obj.save(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/merged.sobj");
  }

#endif

#if 0
  ZNeuronTracerConfig::getInstance().print();
#endif

#if 0 //Move body annotations
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "66ba", 8500);

  ZDvidReader reader;
  reader.open(target);
  QStringList keyList = reader.readKeys("annotations");

  ZDvidTarget target2;
  target2.set("emdata1.int.janelia.org", "66ba", 8500);
  target2.setBodyLabelName("labels3");
  target2.setLabelBlockName("bodies3");

  ZDvidWriter writer;
  writer.open(target2);
  ZDvidUrl dvidUrl(target2);

  foreach (const QString key, keyList) {
    std::cout << key.toStdString() << std::endl;
    QByteArray data = reader.readKeyValue("annotations", key);
    ZJsonValue obj;
    obj.decodeString(data.constData());

    writer.writeJson("bodies3_annotations", key.toStdString(), obj);
  }


#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  frame->view()->highlightPosition(129, 120, 0);
#endif

#if 0
  QDir autoSaveDir(NeutubeConfig::getInstance().getPath(
        NeutubeConfig::AUTO_SAVE).c_str());
  QString mergeFolder = "neutu_proofread_backup";

  QDir mergeDir(autoSaveDir.absoluteFilePath(mergeFolder));
  qDebug() << mergeDir.absoluteFilePath("neutu_merge_opr.json");
#endif


#if 0
  ZDvidReader reader;
  ZDvidTarget target("emdata1.int.janelia.org", "eafc", 8500);
  target.setLabelBlockName("labels3");
  reader.open(target);
  for (int i = 0; i < 200; ++i) {
    reader.readLabels64(4327, 5443, 6341 + i, 512, 512, 1);
  }
#endif

#if 0
  ZDvidBufferReader reader;

  for (int i = 0; i < 10; ++i) {
    tic();
    reader.read("http://emdata1.int.janelia.org:8500/api/node/86e1/labels/raw/0_1_2/512_512_1/4327_5443_7341");
    std::cout << toc() << " ms" << std::endl;
  }
#endif

#if 0
  ZPixmap image(2056, 2056);
//  ZStTransform transform;
//  transform.setOffset(-1, -2);
//  image.setTransform(transform);
  ZPainter painter(&image);
//  painter.drawPoint(1, 2);
  ZObject3dScan obj;
  for (int y = 0; y < 2056; ++y) {
    obj.addSegment(0, y, 0, 2055, false);
  }
//  obj.addSegment(0, 2, 1, 2);
  obj.setColor(255, 255, 255, 255);

  tic();
  obj.display(painter, 0, ZStackObject::SOLID);
  std::cout << toc() << "ms passed" << std::endl;

//  image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
#endif

#if 0
  ZImage image(2056, 2056);
  ZObject3dScan obj;
  for (int y = 0; y < 2056; ++y) {
    obj.addSegment(0, y, 0, 2055, false);
  }
  tic();
  image.setData(obj);
  QPixmap pixmap;
  pixmap.fromImage(image);
  std::cout << toc() << "ms passed" << std::endl;
#endif

#if 0
  ZDvidWriter writer;

//  ZDvidReader reader;
  ZDvidTarget target("emdata2.int.janelia.org", "6cac", 7000);
  target.setLabelBlockName("labels0802");
  target.setBodyLabelName("bodies0802");

  writer.open(target);

  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB25/20150802/annotations-body.json");

  ZJsonArray annoArray(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);

  for (size_t i = 0; i < annoArray.size(); ++i) {
    ZJsonObject obj(annoArray.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
    uint64_t bodyId = ZJsonParser::integerValue(obj["body ID"]);
    if (bodyId > 0) {
      writer.writeAnnotation(bodyId, obj);
    }
  }

  /*
  reader.open(target);
  for (int i = 0; i < 200; ++i) {
    reader.readLabels64(4327, 5443, 6341 + i, 512, 512, 1);
  }
  */
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "b0f7d", 8500);
  target.setLabelBlockName("labels3");
  target.setBodyLabelName("bodies3");

  ZDvidReader reader;
  reader.open(target);

  std::vector<uint64_t> bodyArray;
  bodyArray.push_back(1248607);
  bodyArray.push_back(1413886);
  bodyArray.push_back(6503274);
  bodyArray.push_back(8067132);

  std::vector<ZIntPoint> posArray;

  for (std::vector<uint64_t>::const_iterator iter = bodyArray.begin();
       iter != bodyArray.end(); ++iter) {
    uint64_t bodyId = *iter;
    ZObject3dScan body = reader.readBody(bodyId);
    ZVoxel voxel = body.getMarker();
    std::cout << bodyId << ": (" << voxel.x() << ", " << voxel.y() << ", "
              << voxel.z() << ")" << std::endl;
    posArray.push_back(ZIntPoint(voxel.x(), voxel.y(), voxel.z()));
  }

  for (size_t i = 0; i < bodyArray.size(); ++i) {
    std::cout << bodyArray[i] << ": " << posArray[i].toString() << std::endl;
  }
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "7872", 8500);
  target.setLabelBlockName("labels3");
  target.setBodyLabelName("bodies3");

  ZDvidReader reader;
  reader.open(target);

  std::vector<uint64_t> bodyArray;

  FILE *fp = fopen((GET_TEST_DATA_DIR + "/body.txt").c_str(), "r");
  ZString str;
  while (str.readLine(fp)) {
    std::vector<uint64_t> bodyIdArray = str.toUint64Array();
    for (std::vector<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      bodyArray.push_back(bodyId);
    }
  }

  fclose(fp);


  std::vector<ZIntPoint> posArray;

  for (std::vector<uint64_t>::const_iterator iter = bodyArray.begin();
       iter != bodyArray.end(); ++iter) {
    uint64_t bodyId = *iter;
    ZObject3dScan body = reader.readBody(bodyId);
    ZVoxel voxel = body.getMarker();
    std::cout << bodyId << ": (" << voxel.x() << ", " << voxel.y() << ", "
              << voxel.z() << ")" << std::endl;
    posArray.push_back(ZIntPoint(voxel.x(), voxel.y(), voxel.z()));
  }

  for (size_t i = 0; i < bodyArray.size(); ++i) {
    std::cout << bodyArray[i] << ": " << posArray[i].toString() << std::endl;
  }
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "86e1", 8500);
  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj1 = reader.readBody(15165220);
  ZObject3dScan obj2 = reader.readBody(200010814);

  if (obj1.isAdjacentTo(obj2)) {
    std::cout << "The two objects are adjacent." << std::endl;
  } else {
    std::cout << "The two objects are NOT adjacent." << std::endl;
  }

#endif

#if 0
  Swc_Tree_Node *tn = SwcTreeNode::makePointer(1, 0, 0, 0, 0, 1, -1);

  ZSwcTreeNodeSelector selector;
  selector.setSelection(tn, true);
  selector.setSelection(tn, false);
  selector.setSelection(tn, true);

  selector.print();

  selector.reset();

  selector.print();

  Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(2, 0, 0, 0, 0, 1, 1);
  Swc_Tree_Node *tn3 = SwcTreeNode::makePointer(3, 0, 0, 0, 0, 1, 2);

  std::set<Swc_Tree_Node*> selected;
  selected.insert(tn);
  selected.insert(tn2);

  std::set<Swc_Tree_Node*> prevSelected;
  prevSelected.insert(tn3);
  prevSelected.insert(tn2);

  selector.reset(selected, prevSelected);
  selector.print();
#endif

#if 0
  ZSharedPointer<ZStackObject> obj = ZSharedPointer<ZStackObject>(new ZSwcTree);
  std::cout << obj.use_count() << std::endl;

//  ZSharedPointer<ZSwcTree> obj2 =
//      ZSharedPointer<ZSwcTree>(dynamic_cast<ZSwcTree*>(obj.get()));
//  std::cout << obj.use_count() << std::endl;

  ZSharedPointer<ZSwcTree> obj2 = Shared_Dynamic_Cast<ZSwcTree>(obj);
  std::cout << obj.use_count() << std::endl;
  std::cout << obj2.use_count() << std::endl;

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/pile.sobj");

  ZObject3dScan subobj = obj.subobject(ZIntCuboid(16, 16, 1, 16, 16, 1));
  subobj.print();

  subobj = obj.subobject(ZIntCuboid(5, 5, 1, 15, 15, 3));
  subobj.print();

  subobj = obj.subobject(ZIntCuboid(15, 15, 1, 17, 17, 3));
  subobj.print();

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  ZObject3dScan subobj = obj.subobject(ZIntCuboid(261, 603, 224, 441, 754, 272));
  subobj.save(GET_TEST_DATA_DIR + "/test.sobj");

#endif

#if 0
  Stack *stack = C_Stack::readSc(
        GET_TEST_DATA_DIR + "/benchmark//binary/2d/btrig2_skel.tif");
  ZStackProcessor::ShrinkSkeleton(stack, 10);

  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/mouse_single_org.swc");
//  ZSwcBranch *branch = tree.extractFurthestBranch();
//  branch->print();

  ZSwcResampler sampler;
  sampler.radiusResample(&tree);

  tree.print();

  tree.save(GET_TEST_DATA_DIR + "/test.swc");
  /*
  std::vector<ZWeightedPoint> ptArray = branch->radiusSamplePoint();
  for (std::vector<ZWeightedPoint>::const_iterator iter = ptArray.begin();
       iter != ptArray.end(); ++iter) {
    const ZWeightedPoint &pt = *iter;
    std::cout << pt << std::endl;
  }
  */

//  branch->radiusResample();
//  branch->print();

#endif

#if 0
  std::string dataDir =
      GET_TEST_DATA_DIR + "/flyem/MB/light/2015alphalobe/neurons_affine";
  std::string baseName = "MBON-a2sc_affreg";

  ZSwcTree tree;
  tree.load(dataDir + "/" + baseName + ".swc");

#if 0
  ZSwcTree::DepthFirstIterator treeIter(&tree);
  while (treeIter.hasNext()) {
    Swc_Tree_Node *tn = treeIter.next();
    if (SwcTreeNode::isRegular(tn)) {
      SwcTreeNode::setZ(tn, 570 - SwcTreeNode::z(tn));
    }
  }
#endif

  tree.rescale(20, 20, 20, false);
  tree.changeRadius(0, 20);

//  ZSwcResampler sampler;
//  sampler.radiusResample(&tree);

  tree.save(dataDir + "/" + baseName + "_scaled.swc");

#endif

#if 0
  std::string dataDir =
      GET_TEST_DATA_DIR + "/flyem/MB/light/2015alphalobe/KCandMBON/MBON/affine";

  QList<std::string> baseNameList;
  baseNameList << "MBON-b1-a_affreg";
//  baseNameList << "KC_abs_aligned";
//  baseNameList << "MBON-a_3_affreg";
//  baseNameList << "MBON-a_3_aligned";
//  baseNameList << "MBON-a2sc_affreg";
//  baseNameList << "MBON-a2sc_aligned";

  foreach (const std::string &baseName, baseNameList) {
    ZSwcTree tree;
    tree.load(dataDir + "/" + baseName + ".swc");

    tree.rescale(20, 20, 20, false);
    tree.changeRadius(0, 20.0);

    ZSwcResampler sampler;
    sampler.radiusResample(&tree);

    tree.save(dataDir + "/" + baseName + "_scaled.swc");
  }
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "9875", 8500);
  target.setBodyLabelName("bodies3");
  target.setLabelBlockName("labels3");

  writer.open(target);

  ZJsonObject jsonObj;
  jsonObj.load("/Users/zhaot/Work/neutube/neurolabi/json/skeletonize_mb.json");

  writer.writeJson("bodies3_skeletons", "config.json", jsonObj);

#endif

#if 0
  ZObject3dScan oldRoi;
  oldRoi.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block.sobj");

  ZObject3dScan newRoi;
  newRoi.load(GET_TEST_DATA_DIR + "/flyem/MB/deeper_roi.sobj");

  ZObject3dScan patchRoi = oldRoi.getSlice(351);
  patchRoi.concat(newRoi.getSlice(353));


  patchRoi = patchRoi.interpolateSlice(352);
  patchRoi.save(GET_TEST_DATA_DIR + "/flyem/MB/deeper_patch_roi.sobj");

  ZJsonArray array = ZJsonFactory::MakeJsonArray(
        patchRoi, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/deeper_patch_roi.json");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/sample.swc");
  ZSwcResampler resampler;
  resampler.optimalDownsample(&tree);

  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc;
  doc->setDvidTarget(ZDvidTarget("emdata1.int.janelia.org", "005a", 7000));
  std::cout << "Valid: " << doc->isDataValid("test") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_1") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_2") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_3") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_4") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_5") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_6") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_7") << std::endl;
  std::cout << "Valid: " << doc->isDataValid("segmentation_8") << std::endl;
#endif

#if 0
  ZRandomGenerator rand;

  ZDvidSynapseEnsemble se;
  while (1) {
    ZDvidSynapse synapse;
    int x = rand.rndint(0, 100);
    int y = rand.rndint(0, 100);
    int z = rand.rndint(0, 1000);

    synapse.setPosition(x, y, z);
    se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

    x = rand.rndint(0, 100);
    y = rand.rndint(0, 100);
    z = rand.rndint(0, 1000);
    se.removeSynapse(x, y, z, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

    x = rand.rndint(0, 100);
    y = rand.rndint(0, 100);
    z = rand.rndint(0, 1000);
    ZDvidSynapse &synapse2 =
        se.getSynapse(x, y, z, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);
    synapse2.getX();
  }
#endif

#if 0
  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc;
  doc->setDvidTarget(ZDvidTarget("emdata1.int.janelia.org", "86e1", 8500));

  doc->downloadSynapseFunc();

  ZDvidReader reader;

//  reader.open(doc->getDvidTarget());
//  ZObject3dScan body = reader.readBody(12918474);
  tic();
//  std::vector<ZPunctum*> puncta = doc->getTbar(body);
  std::vector<ZPunctum*> puncta = doc->getTbar(12918474);
  std::cout << toc() << "ms" << std::endl;

  std::cout << puncta.size() << "tbar found" << std::endl;

  delete doc;
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/test.swc");
  ZSwcResampler resampler;
  resampler.ignoreInterRedundant(true);
  resampler.optimalDownsample(&tree);

//  resampler.denseInterpolate(&tree);

  tree.save(GET_TEST_DATA_DIR + "/test2.swc");
#endif

#if 0
  ZSwcTree tree;
  Swc_Tree_Node *root = SwcTreeNode::makePointer(ZPoint(0, 1, 0), 1);
  Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(10, 0, 3.9), 2);
  SwcTreeNode::setParent(tn, root);

  tree.setDataFromNode(root);

  ZSwcResampler resampler;
  resampler.denseInterpolate(&tree);

  tree.resortId();
  tree.print();

  tree.save(GET_TEST_DATA_DIR + "/test.swc");
//  tree.load(GET_TEST_DATA_DIR + "/benchmark/");
#endif

#if 0
//  ZSwcTree tree;
  Swc_Tree_Node *root = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
  Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(2, 0, 0), 1);
  SwcTreeNode::setParent(tn, root);
  Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(-2, 0, 0), 1);
  SwcTreeNode::setParent(tn2, tn);
  Swc_Tree_Node *tn3 = SwcTreeNode::makePointer(ZPoint(6, 2, 0), 1);
  SwcTreeNode::setParent(tn3, tn2);
  Swc_Tree_Node *tn4 = SwcTreeNode::makePointer(ZPoint(8, 3, 4), 1);
  SwcTreeNode::setParent(tn4, tn3);

  std::cout << "Bending energy: " << SwcTreeNode::maxBendingEnergy(root) << std::endl;
  std::cout << "Bending energy: " << SwcTreeNode::maxBendingEnergy(tn) << std::endl;
  std::cout << "Bending energy: " << SwcTreeNode::maxBendingEnergy(tn2) << std::endl;
  std::cout << "Bending energy: " << SwcTreeNode::maxBendingEnergy(tn3) << std::endl;
  std::cout << "Bending energy: " << SwcTreeNode::maxBendingEnergy(tn4) << std::endl;

//  tree.setDataFromNode(root);


#endif

#if 0
  QColor color(255, 153, 0);
  std::cout << color.hueF() << std::endl;
  std::cout << color.saturationF() << std::endl;

  color.setHsv(color.hue(), color.saturation()/2, color.value());
  std::cout << color.red() << " " << color.green() << " " << color.blue() << std::endl;

#endif

#if 0
  ZStackDoc doc;
  ZSwcTree *tree = new ZSwcTree;
  std::cout << "Tree: " << tree << std::endl;
  Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
  tree->setDataFromNode(tn);

  tree->selectNode(tn, true);
  std::cout << "  Node: " << tn << std::endl;

  doc.addObject(tree);

  //Add another tree
  tree = new ZSwcTree;
  std::cout << "Tree: " << tree << std::endl;
  tn = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
  tree->setDataFromNode(tn);
  tree->selectNode(tn, true);
  std::cout << "  Node: " << tn << std::endl;
  Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
  SwcTreeNode::setParent(tn2, tn);

  tn2 = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
  SwcTreeNode::setParent(tn2, tn);
  tree->selectNode(tn2, true);
  std::cout << "  Node: " << tn2 << std::endl;

  doc.addObject(tree);

  QMap<const Swc_Tree_Node *, const ZSwcTree *> swcMap =
      doc.getSelectedSwcNodeMap();

  if (swcMap.isEmpty()) {
    std::cout << "Empty swc map" << std::endl;
  }
  for (QMap<const Swc_Tree_Node *, const ZSwcTree *>::const_iterator
       iter = swcMap.begin(); iter != swcMap.end(); ++iter) {
    std::cout << iter.key() << " " << iter.value() << std::endl;
  }

#endif

#if 0
  tic();
  std::vector<int> vec;
  vec.resize(10000000);
//  int *vec = new int[10000000];
  for (int i = 0; i < 5000000; ++i) {
    vec[i] = i;
  }
  std::cout << vec.size() << " elements" << std::endl;
  ptoc();
#endif

#if 0
  tic();
  Int_Arraylist *arrayList = Int_Arraylist_New(0, 0);
  for (int i = 0; i < 5000000; ++i) {
    Int_Arraylist_Add(arrayList, i);
  }
  std::cout << arrayList->length << " elements" << std::endl;
  ptoc();
#endif

#if 0
  tic();
  std::vector<int> vec;
  vec.reserve(10000000);
  for (int i = 0; i < 5000000; ++i) {
    vec.push_back(i);
  }
  std::cout << vec.size() << " elements" << std::endl;
  ptoc();
#endif

#if 0
//  tic();
  ZDvidTarget dvidTarget("emdata1.int.janelia.org", "86e1", 8500);
  ZDvidReader reader;
  ZObject3dScan obj1;
  if (reader.open(dvidTarget)) {
    reader.readBody(14307133, &obj1);

    reader.readBody(15933492);
  }

  tic();
  ZSwcTree *tree = ZSwcFactory::CreateSurfaceSwc(obj1);
  std::cout << tree->size() << " nodes." << std::endl;
  ptoc();

  delete tree;
//  ptoc();
#endif

#if 0
  Biocytin::SwcProcessor processor;
  ZResolution resolution;
  resolution.setVoxelSize(1, 1, 8);
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/test.swc");
  processor.setResolution(resolution);
  processor.breakZJump(&tree);
  tree.save(GET_TEST_DATA_DIR + "/test2.swc");
#endif

#if 0
  JNeuronTracer tracer;
  ZResolution resolution;
  resolution.setVoxelSize(1, 1, 8);
  tracer.setResolution(resolution);
//  Stack *mask = tracer.makeMask(stack.c_stack());

  std::string filePath = GET_TEST_DATA_DIR +
      "/biocytin/DH_7-6-13-2_100x/DH070613C2X100-36.tif";

  ZStack stack;
  stack.load(filePath);

  Stack *stackData = stack.c_stack(1);

  int intensityMode = C_Stack::mode(stackData);
  int intensityMean = C_Stack::mean(stackData);

  if (intensityMode > intensityMean) {
    std::cout << "Bright field detected." << std::endl;
    Stack_Invert_Value(stackData);
  } else {
    std::cout << "Dark field detected." << std::endl;
  }

  Stack *mask = C_Stack::readSc(
        GET_TEST_DATA_DIR +
        "/biocytin/DH_7-6-13-2_100x/DH070613C2X100-36.Proj.Mask.tif");
  Stack_Binarize(mask);
  tracer.setMask(mask);

  ZSwcTree *tree = tracer.trace(stackData);
  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR + "/benchmark/fork_2d.tif");
  ZIntHistogram *hist = C_Stack::hist(stack, NULL);
  hist->print();

  std::cout << "Mode: " << hist->getMode() << std::endl;
  std::cout << "Mode started from 38: " << hist->getMode(38, hist->getMaxValue())
            << std::endl;
  std::cout << "Uc: " << hist->getUpperCount(220) << std::endl;
  std::cout << "Lc: " << hist->getLowerCount(40) << std::endl;

  delete hist;
#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_TEST_DATA_DIR + "/benchmark/fork_2d.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZStroke2d *stroke = new ZStroke2d;
  stroke->setLabel(1);
  stroke->append(10, 10);
  stroke->setWidth(5);
  stroke->setRole(ZStackObjectRole::ROLE_SKELETON_MASK);
  stroke->setZ(0);

  ZStroke2d *eraser = new ZStroke2d;
  eraser->setEraser(true);
  eraser->append(11, 11);
  eraser->setWidth(5);
  eraser->setRole(ZStackObjectRole::ROLE_SKELETON_MASK);
  eraser->setZ(0);

  ZStroke2d *stroke2 = new ZStroke2d;
  stroke2->setLabel(1);
  stroke2->append(12, 12);
  stroke2->setWidth(5);
  stroke2->setRole(ZStackObjectRole::ROLE_SKELETON_MASK);
  stroke2->setZ(0);

  frame->document()->addObject(stroke);
  frame->document()->addObject(eraser);
  frame->document()->addObject(stroke2);

  ZBiocytinProjMaskFactory maskFactory;

  ZStack *stack = maskFactory.MakeMask(frame->view(), 1);

  ZStackFrame *frame2 = ZStackFrame::Make(NULL);
  frame2->loadStack(stack);
  host->addStackFrame(frame2);
  host->presentStackFrame(frame2);

//  stack->save(GET_TEST_DATA_DIR + "/test.tif");

#  if 0
  ZStack *stack = frame->document()->getStack()->clone();
  //stack->setOffset(20, 30, 0);
  ZStackPatch *patch = new ZStackPatch(stack);
  patch->setScale(0.2, 0.2);
  patch->setFinalOffset(20, 30);
  frame->document()->addObject(patch);

  ZRect2d *rect = new ZRect2d(30, 40, 100, 200);
  rect->setPenetrating(true);
  rect->setColor(0, 255, 0);
  frame->document()->addObject(rect);
#  endif
#endif

#if 0
  ZDvidBufferReader reader;

  ZJsonArray queryObj;

  ZJsonArray coordObj;
  coordObj.append(10);
  coordObj.append(20);
  coordObj.append(30);

  ZJsonArray coordObj2;
  coordObj2.append(3970);
  coordObj2.append(5450);
  coordObj2.append(7313);

  queryObj.append(coordObj);
  queryObj.append(coordObj2);

  QString queryForm = queryObj.dumpString(0).c_str();

  std::cout << "Payload: " << queryForm.toStdString() << std::endl;

  QByteArray payload;
  payload.append(queryForm);

  reader.read("http://emdata1.int.janelia.org:8500/api/node/86e1/labels/labels",
              payload, true);

  QString labels = QString(reader.getBuffer());
  std::cout << labels.toStdString() << std::endl;
#endif

#if 0
  ZDvidTarget dvidTarget("emdata1.int.janelia.org", "86e1", 8500);

  ZDvidReader reader;
  if (reader.open(dvidTarget)) {
    std::vector<ZIntPoint> ptArray;
    ptArray.push_back(ZIntPoint(3970, 5450, 7313));
    ptArray.push_back(ZIntPoint(4281, 5003, 7313));

    std::vector<uint64_t> bodySet = reader.readBodyIdAt(ptArray);
    for (std::vector<uint64_t>::const_iterator iter = bodySet.begin();
         iter != bodySet.end(); ++iter) {
      std::cout << *iter << std::endl;
    }
  }
#endif

#if 0
  ZDvidTarget target("emdata2.int.janelia.org", "86e1", 7100);
  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan bs = reader.readBody(15950033);
  bs.save(GET_TEST_DATA_DIR + "/split.sobj");
#endif

#if 0
  ZDvidTarget target("emdata2.int.janelia.org", "86e1", 7100);

  ZDvidReader reader;
  reader.open(target);

  uint64_t bodyId = 14316509;
  ZObject3dScan bf = reader.readBody(bodyId);

  ZDvidWriter writer;
  if (writer.open(target)) {
    ZObject3dScan obj;
    obj.load(GET_TEST_DATA_DIR + "/split.sobj");
//    ZObject3dScan obj = ZObject3dFactory::MakeObject3dScan(
//          ZIntCuboid(ZIntPoint(4128, 5370, 7731), ZIntPoint(4160, 5399, 7768)));
//    obj.addSegment(7725, 5387, 4139, 4145);
    ZObject3dScan bm = bf;
    bm.subtract(obj);
    tic();
    uint64_t newBodyId = writer.writePartition(bm, obj, bodyId);
//    uint64_t newBodyId = writer.writeSplit(obj, bodyId, 1);
//    uint64_t newBodyId = writer.writeSplitMultires(bf, obj, bodyId);
    ptoc();
    std::cout << "New body: " << newBodyId << std::endl;
  }

  std::cout << "Status code: " << writer.getStatusCode() << std::endl;
#endif

#if 0
  ZObject3dScan bf;
  bf.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  ZObject3dScan *bs = bf.subobject(ZIntCuboid(ZIntPoint(276, 895, 400),
                                              ZIntPoint(821, 1091, 642)));

  tic();
  bf.subtractSliently(*bs);
  ptoc();

  delete bs;

#endif

#if 0
  ZObject3dScan bf;
  bf.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  ZObject3dScan bf2 = bf;

  ZDvidInfo dvidInfo;
  dvidInfo.setStackSize(10000, 10000, 10000);
  dvidInfo.setStartBlockIndex(0, 0, 0);
  dvidInfo.setEndBlockIndex(300, 300, 300);

  /*
  ZDvidReader reader;
  ZDvidTarget target("emdata1.int.janelia.org", "86e1", 8500);
  if (reader.open(target)) {
    dvidInfo = reader.readGrayScaleInfo();
  }
  */

  tic();

  ZObject3dScan Bbf = dvidInfo.getBlockIndex(bf);
  std::cout << "Full object block count: " << Bbf.getVoxelNumber() << std::endl;
//  Bbf.print();
//  Bbf.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZObject3dScan bs;
  bs = bf.getSlice(419, 642);

  ZObject3dScan dbs = bs;
  dbs.dilate();
  ZObject3dScan Bdbs = dvidInfo.getBlockIndex(dbs);
  ZObject3dScan Bbs = dvidInfo.getBlockIndex(bs);

  ZObject3dScan Bbb = Bdbs;
  Bbb.subtractSliently(Bbs);

  Bbb.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZObject3dScan Bsc = Bbs;
  Bsc.subtractSliently(Bbb);



  /*
  ZObject3dScan Bbs = dvidInfo.getBlockIndex(bs);
  ZObject3dScan Bbf_bs = Bbf;
  Bbf_bs.subtract(Bbs);

  Bbf_bs.concat(Bbf.intersect(Bbs));
  */
//  Bbf_bs.print();

//  bf.subobject(ZIntCuboid(ZIntPoint(201, 824, 335), ZIntPoint(324, 938, 470)), &bs);
//  bs.addSegment(1, 1, 1, 1);
//  bs.addSegment(2, 16, 5, 32);


  /*
  ZObject3dScan bf_bs = bf;
  bf_bs.subtract(bs);
  ZObject3dScan Bbf_bs = dvidInfo.getBlockIndex(bf_bs);
  */

//  std::cout << "B(bf-bs) size: " << Bbf_bs.getVoxelNumber() << std::endl;

//  ZObject3dScan Bsc = Bbf;
//  Bsc.subtract(Bbf_bs);

  ZObject3dScan bBsc = Bsc;
  bBsc.translate(-dvidInfo.getStartBlockIndex());
  bBsc.upSample(dvidInfo.getBlockSize().getX() - 1,
                dvidInfo.getBlockSize().getY() - 1,
                dvidInfo.getBlockSize().getZ() - 1);
  bBsc.translate(dvidInfo.getStartCoordinates());

  std::cout << "Block subtraction size: " << bBsc.getVoxelNumber() << std::endl;
  bf.subtract(bBsc);

  ZObject3dScan bsr = bs;
  bsr.subtract(bBsc);

  bf.subtract(bsr);

  ptoc();

//  bf.save(GET_TEST_DATA_DIR + "/test.sobj");

  bf2.subtract(bs);


  bf.canonize();
  bf2.canonize();

  if (bf.equalsLiterally(bf2)) {
    std::cout << "Testing passed." << std::endl;
  } else {
    std::cout << "Testing failed: Object inconsistent." << std::endl;
  }
//  obj.subtract(ZObject3dFactory::MakeObject3dScan(
//  block.subtract(
#endif

#if 0
  ZObject3dScan bf;
  bf.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  QByteArray payload = bf.toDvidPayload();
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/test.dvid").c_str(), "wb");

  fwrite(payload.constData(), payload.size(), 1, fp);

  fclose(fp);

  ZObject3dScan bf2;
  bf2.importDvidObject(GET_TEST_DATA_DIR + "/test.dvid");
  if (bf.equalsLiterally(bf2)) {
    std::cout << "Testing passed." << std::endl;
  } else {
    std::cout << "Testing failed: Object inconsistent." << std::endl;
  }
#endif

#if 0
  ZDvidTarget dvidTarget("emdata2.int.janelia.org", "c077", 7000);
  dvidTarget.setBodyLabelName("segmentation-labelvol");
  dvidTarget.setLabelBlockName("segmentation");

  ZDvidReader reader;
  if (reader.open(dvidTarget)) {
    ZDvidWriter writer;
    writer.open(dvidTarget);
    QByteArray out = reader.readKeyValue("annotations", "body_synapses");
    ZJsonObject obj;
    obj.decodeString(out.constData());

    if (obj.hasKey("data")) {
      ZJsonArray dataJson(obj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);
      std::cout << dataJson.size() << " annotations found." << std::endl;
      for (size_t i = 0; i < dataJson.size(); ++i) {
        ZJsonObject annoJson(dataJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
        uint64_t bodyId = ZJsonParser::integerValue(annoJson["body ID"]);
        std::string name = ZJsonParser::stringValue(annoJson["name"]);
        std::string comment = ZJsonParser::stringValue(annoJson["comment"]);
        std::string user = ZJsonParser::stringValue(annoJson["user"]);


        std::cout << bodyId << " " << name << std::endl;

        ZFlyEmBodyAnnotation anno = reader.readBodyAnnotation(bodyId);
        if (anno.getName().empty() || anno.getName() == "name") {
          anno.setName(name);
          anno.setComment(comment);
          anno.setUser(user);
          writer.writeBodyAnntation(anno);
        }
      }
    }
  }

#endif

#if 0
  std::string dataDir =
      GET_TEST_DATA_DIR +
      "/flyem/MB/light/2015alphalobe/SS01240/GMR_SS01240-20140801_32_F1/align";
  std::string baseName = "C2-Aligned63xScale-EM";

  ZSwcTree tree;
  tree.load(dataDir + "/" + baseName + ".swc");

  tree.rescale(20, 20, 20, false);
  tree.changeRadius(0, 20);

//  ZSwcResampler sampler;
//  sampler.radiusResample(&tree);

  tree.save(dataDir + "/" + baseName + "_scaled.swc");

#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "86e1", 7100);
  reader.open(target);
#endif

#if 0
#if defined(_ENABLE_LIBDVIDCPP_)
  libdvid::DVIDNodeService service("emdata1.int.janelia.org:8500", "b6bc");
  std::cout << "Reading tiles ..." << std::endl;

  ZSleeper sleeper;

  int count = 0;
  while (1) {
    QElapsedTimer timer;
    timer.start();
    std::vector<std::vector<int> > tile_locs_array(6);
    for (size_t i = 0; i < tile_locs_array.size(); ++i) {
      std::vector<int> loc(3);
      loc[0] = 3 + i;
      loc[1] = 3 + i;
      loc[2] = 9259;
      tile_locs_array[i] = loc;
    }

    try {
      const std::vector<libdvid::BinaryDataPtr> &data = get_tile_array_binary(
            service, "tiles", libdvid::XY, 0,
            tile_locs_array);
      std::cout << data.size() << "x tile reading time: "
                << timer.elapsed() << std::endl;
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
//      break;
    }

    std::cout << "#Reading: " << ++count << std::endl;
    if (count % 2600 == 0) {
      sleeper.sleep(60);
    }
  }
#endif
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "86e1", 7100);

  ZDvidTileEnsemble ensemble;
  ensemble.setDvidTarget(target);

  std::vector<ZDvidTileInfo::TIndex> tileIndices;
  tileIndices.push_back(ZDvidTileInfo::TIndex(1, 2));
  tileIndices.push_back(ZDvidTileInfo::TIndex(0, 2));
  tileIndices.push_back(ZDvidTileInfo::TIndex(2, 2));
  tileIndices.push_back(ZDvidTileInfo::TIndex(1, 1));

  while (1) {
    ensemble.update(tileIndices, 0, 9259);
  }
#endif

#if 0
  ZObject3dScan wholeRoi;
  wholeRoi.importDvidRoi(GET_TEST_DATA_DIR + "/flyem/AL/whole_roi.json");

  std::cout << "Whole size: " << wholeRoi.getVoxelNumber() << std::endl;

  ZObject3dScan glomRoi;
  glomRoi.importDvidRoi(GET_TEST_DATA_DIR + "/flyem/AL/glomerulus_roi.json");

  std::cout << "Glom size: " << glomRoi.getVoxelNumber() << std::endl;

  wholeRoi.subtractSliently(glomRoi);

  ZJsonFactory factory;
  ZJsonArray json = factory.MakeJsonArray(wholeRoi);

  json.dump(GET_TEST_DATA_DIR + "/flyem/AL/whole_sub_glomerulus_roi.json");
#endif

#if 0
  ZJsonObject jsonObj;
  jsonObj.load(GET_TEST_DATA_DIR +
               "/flyem/FIB/fib25/annotations-synapse-shinya1-13_20151104-dvid.json");

  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "fa60", 7000);
  target.setBodyLabelName("bodies1104");
  target.setLabelBlockName("labels1104");

  ZDvidUrl url(target);

  if (writer.open(target)) {
    writer.writeJson(url.getSynapseAnnotationUrl(), jsonObj);
  }

#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "fa60", 7000);
  target.setBodyLabelName("bodies1104");
  target.setLabelBlockName("labels1104");


  if (writer.open(target)) {
    ZJsonObject jsonObj;
    jsonObj.load(GET_TEST_DATA_DIR +
                 "/flyem/FIB/fib25/20151104/annotations-body.json");

    ZJsonArray arrayJson(jsonObj["data"], ZJsonValue::SET_INCREASE_REF_COUNT);

    for (size_t i = 0; i < arrayJson.size(); ++i) {
      ZJsonObject annoJson(arrayJson.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);

      ZFlyEmBodyAnnotation anno;
      anno.loadJsonObject(annoJson);
      writer.writeBodyAnntation(anno);
    }
  }

#endif

#if 0
  ZWindowFactory factory;
  factory.setWindowTitle("Test");

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "86e1", 7100);

  ZDvidWriter writer;
  if (writer.open(target)) {
    writer.removeBodyAnnotation(12587667);
  }
#endif

#if 0
  ZObject3dStripe s1;
  s1.addSegment(0, 5);
  s1.addSegment(7, 9);
  s1.addSegment(11, 13);
  s1.addSegment(17, 19);
  s1.addSegment(25, 29);

  ZObject3dStripe s2;
  s2.addSegment(-5, -3);
  s2.addSegment(-1, 0);
  s2.addSegment(2, 3);
  s2.addSegment(5, 7);
  s2.addSegment(9, 13);
  s2.addSegment(20, 25);

  ZObject3dStripe s = s1 - s2;
  s.print();

#endif

#if 0
  ZObject3dScan bf;
  bf.load(GET_TEST_DATA_DIR + "/flyem/MB/large_outside_block.sobj");

  ZObject3dScan surfaceObj = bf.getSurfaceObject();
  surfaceObj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZObject3dScan bf;
  bf.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  ZObject3dScan *bs = bf.subobject(ZIntCuboid(ZIntPoint(276, 895, 400),
                                              ZIntPoint(821, 1091, 642)));

  tic();
  ZObject3dScan diff = bf - *bs;
//  bf.subtractSliently(*bs);
  ptoc();

  std::cout << diff.isCanonizedActually() << std::endl;
  std::cout << bf.isCanonizedActually() << std::endl;
  std::cout << bs->isCanonizedActually() << std::endl;

  tic();
  bf.subtractSliently(*bs);
  ptoc();

  std::cout << diff.getVoxelNumber() << std::endl;
  std::cout << bf.getVoxelNumber() << std::endl;

  std::cout << bf.equalsLiterally(diff) << std::endl;

  bf.subtractSliently(diff);
//  bf.print();
  bf.save(GET_TEST_DATA_DIR + "/test.sobj");

  delete bs;

#endif

#if 0
  FlyEm::ZSynapseAnnotationArray sa;
  sa.loadJson(GET_TEST_DATA_DIR + "/synapse.json");

  ZJsonArray dvidSynapseJson;
  for (FlyEm::ZSynapseAnnotationArray::const_iterator iter = sa.begin();
       iter != sa.end(); ++iter) {
    const FlyEm::ZSynapseAnnotation annotation = *iter;
//    std::cout << annotation.toDvidSynapseElementJson().dumpString(2)
//              << std::endl;

    std::vector<ZJsonObject> objArray = annotation.toDvidSynapseElementJson();
    for (std::vector<ZJsonObject>::iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      dvidSynapseJson.append(*iter);
    }
  }

  dvidSynapseJson.dump(GET_TEST_DATA_DIR + "/test.json");
#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZDvidSynapse *synapse = new ZDvidSynapse;
  synapse->setPosition(30, 30, 30);
  synapse->setKind(ZDvidSynapse::KIND_PRE_SYN);
  synapse->setDefaultRadius();
  synapse->setDefaultColor();

  synapse->addPartner(100, 100, 100);

  frame->document()->addObject(synapse);
#endif

#if 0
  QStringList strings;
  strings << "This" << "is" << "a" << "test";
  QStringList result =
      QtConcurrent::blockingMapped(strings, &QString::toLower);

  qDebug() << result;
#endif

#if 0
  QList<Stack*> stackList;

  for (int i = 0; i < 5; ++i) {
    Stack *stack = C_Stack::make(GREY, 100, 100, 10);
    C_Stack::setZero(stack);
    stackList.append(stack);
  }

  ZFlyEmBody3dDoc *doc = new ZFlyEmBody3dDoc;

  Z3DWindow *window = factory.make3DWindow(doc);
  window->setYZView();

  window->show();
  window->raise();
#endif

#if 0
  std::cout << "message testing ..." << std::endl;
  qDebug() << "Debug message test";
  qWarning() << "Warning message test";
#endif

#if 0
  ZDvidSynapse synapse;
  synapse.setPosition(30, 30, 30);
  synapse.setKind(ZDvidSynapse::KIND_PRE_SYN);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();


  ZDvidSynapseEnsemble se;
  se.addSynapse(synapse);

  synapse.setPosition(1, 2, 3);
  se.addSynapse(synapse);

  synapse.setPosition(1, 2, 30);
  se.addSynapse(synapse);

  synapse.setPosition(-1, 2, 30);
  se.addSynapse(synapse);

  synapse.setPosition(-1, 2, -1);
  se.addSynapse(synapse);

  synapse.setPosition(-1, -2, -1);
  se.addSynapse(synapse);

  std::cout << se;

#endif

#if 0
  ZObject3dScan obj1;
  obj1.load(GET_TEST_DATA_DIR + "/flyem/MB/roi_fix/mbroi.sobj");

  ZObject3dScan obj2;
  obj2.load(GET_TEST_DATA_DIR + "/roi_test.sobj");

  obj2.subtractSliently(obj1);

  ZObject3dScan obj = obj2.getSlice(336, 421);

  obj.save(GET_TEST_DATA_DIR + "/flyem/MB/alpha_add.sobj");
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha_add.sobj");

  ZJsonArray objJson = ZJsonFactory::MakeJsonArray(obj);
  objJson.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha_add.json");
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  std::vector<ZObject3dScan> objArray =
      obj.getConnectedComponent(ZObject3dScan::ACTION_CANONIZE);
  size_t volume = 0;
  for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    const ZObject3dScan &subobj = *iter;
    volume += subobj.getVoxelNumber();
  }
#endif

#if 0
  QElapsedTimer timer;
  timer.start();
  ZRandomGenerator::SetSeed(0);
  for (int i = 0; i < 10; ++i) {
    ZObject3dScan obj = ZObject3dFactory::MakeRandomObject3dScan(
          ZIntCuboid(0, 0, 0, 200, 200, 200));
    obj.save(GET_TEST_DATA_DIR + "/test.sobj");

    if (!obj.isCanonizedActually()) {
      std::cout << "Bug found: Not canonized." << std::endl;
      break;
    }

//    std::cout << "Canonized: " << obj.isCanonizedActually() << std::endl;

    std::vector<ZObject3dScan> objArray =
        obj.getConnectedComponent(ZObject3dScan::ACTION_CANONIZE);
    size_t volume = 0;
    for (std::vector<ZObject3dScan>::const_iterator iter = objArray.begin();
         iter != objArray.end(); ++iter) {
      const ZObject3dScan &subobj = *iter;
      volume += subobj.getVoxelNumber();
    }

    if (volume != obj.getVoxelNumber()) {
      std::cout << "Bug found: Unmatched volume." << std::endl;
      break;
    }

    for (size_t i = 0; i < objArray.size(); i += 10) {
      for (size_t j = i + 1; j < objArray.size(); j += 10) {
        if (i != j) {
          ZObject3dScan &obj1 = objArray[i];
          ZObject3dScan &obj2 = objArray[j];
          if (obj1.isAdjacentTo(obj2)) {
            std::cout << "Bug found: Adjacent disjoints." << std::endl;
            i = objArray.size();
            break;
          }
        }
      }
    }
  }
  std::cout << timer.elapsed() << "ms" << std::endl;
#endif

#if 0
  ZDvidSynapse synapse;
  synapse.setPosition(1, 2, 3);
  synapse.setKind(ZDvidSynapse::KIND_PRE_SYN);
  synapse.addPartner(4, 5, 6);
  synapse.addPartner(7, 8, 9);
  synapse.addTag("test");
  synapse.addTag("test2");

  std::cout << synapse.toJsonObject().dumpString(2) << std::endl;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/roi_ME_cell_body.sobj");



  ZObject3dScan slice = obj.getSlice(112);
  ZStack *stack = slice.toStackObject();

  Stack *filled = Stack_Fill_2dhole(stack->c_stack(), NULL, 1, 1);
  ZStack filledStack;
  filledStack.consume(filled);
  filledStack.setOffset(stack->getOffset());

  slice.loadStack(filledStack);

  slice.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  if (host != NULL) {
    host->testFlyEmProofread();
  }
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "059e", 7000);
  writer.open(target);
  writer.deleteKey(QString("bodies0802_skeletons"), QString("0_swc"),
                   QString("9_swc"));
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "e402", 7000);
  writer.open(target);

  ZJsonObject obj;
  obj.load(GET_APPLICATION_DIR + "/json/skeletonize_fib25_len40.json");
  writer.writeJson("bodies1104_skeletons", "config.json", obj);
#endif

#if 0
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("Mi15-O") << std::endl;
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("Mi3-like_13852") << std::endl;
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("Dm158-lik") << std::endl;
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("Y3/Y24 O-like") << std::endl;
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("Y3/Y24-like") << std::endl;
  std::cout << ZFlyEmNeuronInfo::GuessTypeFromName("TmY4-like-0") << std::endl;
#endif

#if 0
  std::string dataFolder =
      GET_TEST_DATA_DIR + "/flyem/FIB/FIB25/20151104/neuromorpho";

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "e402", 7000);
  target.setBodyLabelName("bodies1104");
  ZDvidReader reader;
  reader.open(target);

  //Load neuron list
  int count = 0;
  int swcCount = 0;

  std::set<std::string> excludedType;
  std::set<uint64_t> excludedId;

  ZJsonObject configJson;
  configJson.load(dataFolder + "/config.json");
  if (configJson.hasKey("excluded")) {
    ZJsonObject excludedJson(configJson.value("excluded"));
    if (excludedJson.hasKey("type")) {
      ZJsonArray excludeTypeJson(excludedJson.value("type"));
      for (size_t i = 0; i < excludeTypeJson.size(); ++i) {
        excludedType.insert(ZJsonParser::stringValue(excludeTypeJson.at(i)));
      }
    }
    if (excludedJson.hasKey("id")) {
      ZJsonArray excludeIdJson(excludedJson.value("id"));
      for (size_t i = 0; i < excludeIdJson.size(); ++i) {
        excludedId.insert(ZJsonParser::integerValue(excludeIdJson.at(i)));
      }
    }
  }

  std::vector<uint64_t> emptyBody;

  ZString line;
  FILE *fp = fopen((dataFolder + "/neuron.csv").c_str(), "r");
  while (line.readLine(fp)) {
    line.trim();
    std::vector<std::string> fieldArray = line.tokenize(',');
    if (fieldArray.size() != 5) {
      std::cout << line << std::endl;
    } else {
      ZString type = fieldArray[2];
      type.replace("\"", "");
      //      type.replace("/", "_");
      ZString name = fieldArray[1];
      if (!type.empty() && !name.contains("?") && !type.contains("/") &&
          !type.endsWith("like") && !type.startsWith("Mt") &&
          excludedType.count(type) == 0) {
        ZString bodyIdStr(fieldArray[0]);
        uint64_t bodyId = bodyIdStr.firstUint64();
        if (excludedId.count(bodyId) == 0) {
          std::cout << bodyId << ": " << fieldArray[2] << std::endl;
          QDir dataDir((dataFolder + "/swc").c_str());
          if (!dataDir.exists(type.c_str())) {
            std::cout << "Making directory "
                      << dataDir.absolutePath().toStdString() + "/" + type
                      << std::endl;
            dataDir.mkdir(type.c_str());
          }
#  if 1
          ZSwcTree *tree = reader.readSwc(bodyId);
          if (tree != NULL) {
            if (!tree->isEmpty()) {
              tree->addComment("");
              tree->addComment("Imaging: FIB SEM");
              tree->addComment("Unit: um");
              tree->addComment("Neuron: fly medulla, " + type);
              tree->addComment("Reference: Takemura et al. PNAS 112(44) (2015): 13711-13716.");

              tree->rescale(0.01, 0.01, 0.01);
              tree->setType(3);
              name.replace("\"", "");
              name.replace("?", "_");
              name += "_";
              name.appendNumber(bodyId);
              tree->save(dataFolder + "/swc/" + type + "/" + name + ".swc");
              ++swcCount;
            } else {
              emptyBody.push_back(bodyId);
              std::cout << "WARING: empty tree" << std::endl;
            }
          } else {
            emptyBody.push_back(bodyId);
            std::cout << "WARING: null tree" << std::endl;
          }
#  endif

          ++count;
        }
      }
    }
  }
  fclose(fp);

  std::cout << count << " neurons valid." << std::endl;
  std::cout << swcCount << " neurons saved." << std::endl;
  std::cout << "Missing bodies:" << std::endl;
  reader.setVerbose(false);
  for (std::vector<uint64_t>::const_iterator iter = emptyBody.begin();
       iter != emptyBody.end(); ++iter) {
    ZObject3dScan body = reader.readBody(*iter);
    std::cout << "  " << *iter << " " << body.getVoxelNumber()
              << std::endl;
  }

#endif

#if 0
  ZJsonArray jsonArray;

  ZDvidSynapse::AddRelation(jsonArray, ZIntPoint(1, 2, 3), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonArray, ZIntPoint(1, 2, 3), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonArray, ZIntPoint(3, 4, 5), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonArray, ZIntPoint(3, 4, 5), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonArray, ZIntPoint(3, 4, 9), "UnknownRelationship");

  std::cout << jsonArray.dumpString(2) << std::endl;
#endif

#if 0
  ZJsonArray jsonArray;

  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(1, 2, 3), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(1, 2, 3), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 5), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 5), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 9), "UnknownRelationship"));

  ZDvidSynapse::RemoveRelation(jsonArray, ZIntPoint(1, 2, 3));
  ZDvidSynapse::RemoveRelation(jsonArray, ZIntPoint(3, 4, 5));
  ZDvidSynapse::RemoveRelation(jsonArray, ZIntPoint(3, 4, 9));

  std::cout << jsonArray.dumpString(2) << std::endl;
#endif

#if 0
  ZJsonObject jsonObj;
  ZDvidSynapse::AddRelation(jsonObj, ZIntPoint(1, 2, 3), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonObj, ZIntPoint(1, 2, 3), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonObj, ZIntPoint(3, 4, 5), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonObj, ZIntPoint(3, 4, 5), "PreSynTo");
  ZDvidSynapse::AddRelation(jsonObj, ZIntPoint(3, 4, 9), "UnknownRelationship");

  std::cout << jsonObj.dumpString(2) << std::endl;
#endif

#if 0
  ZJsonObject jsonArray;

  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(1, 2, 3), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(1, 2, 3), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 5), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 5), "PreSynTo"));
  ZDvidSynapse::AddRelation(
        jsonArray, ZDvidSynapse::MakeRelJson(ZIntPoint(3, 4, 9), "UnknownRelationship"));

  std::cout << jsonArray.dumpString(2) << std::endl;
#endif

#if 0
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata1.int.janelia.org", "86e1", 8500);
  dvidTarget.setSynapseName("synapse");

  ZDvidReader reader;
  if (reader.open(dvidTarget)) {
    std::vector<ZIntPoint> ptArray;
    ptArray.push_back(ZIntPoint(4572, 6097, 7313));
    ptArray.push_back(ZIntPoint(4562, 6115, 7313));
    ptArray.push_back(ZIntPoint(4583, 6113, 7313));
    ZJsonArray jsonArray = reader.readSynapseJson(ptArray.begin(), ptArray.end());
    std::cout << jsonArray.dumpString(2) << std::endl;
  }
#endif

#if 0
  FILE *fp = fopen((GET_TEST_DATA_DIR + "/benchmark/swc/breadth_first.swc").c_str(), "r");
  Swc_Node node;
  std::cout << Swc_Node_Fscan(fp, &node) << std::endl;
  Print_Swc_Node(&node);

  std::cout << Swc_Node_Fscan(fp, &node) << std::endl;
  Print_Swc_Node(&node);

  fclose(fp);
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/test.swc");
  tree.print();
#endif

#if 0
  ZDvidSynapse synapse;
  synapse.setPosition(30, 30, 30);
  synapse.setKind(ZDvidSynapse::KIND_PRE_SYN);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();


  ZDvidSynapseEnsemble se;
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  synapse.setPosition(31, 30, 30);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  synapse.setPosition(31, 30, 29);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  synapse.setPosition(31, 30, 29);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  synapse.setPosition(31, 28, 29);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  synapse.setPosition(-1, -2, -3);
  se.addSynapse(synapse, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL);

  std::cout << se << std::endl;

  //Check consistensy
  for (int z = se.getMinZ(); z <= se.getMaxZ(); ++z) {
    const ZDvidSynapseEnsemble::SynapseSlice &slice = se.getSlice(z);
    if (!slice.isEmpty()) {
      std::cout << slice << std::endl;
    }
  }


  std::cout << se.getSynapse(0, 0, 0, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL)
            << std::endl;

  std::cout << se.getSynapse(-1, -2, -3, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL)
            << std::endl;

  std::cout << se.getSynapse(31, 28, 29, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL)
            << std::endl;
  std::cout << se.getSynapse(31, 28, 28, ZDvidSynapseEnsemble::EDataScope::DATA_LOCAL)
            << std::endl;

#endif

#if 0
  ZFlyEmBookmark bookmark;
  bookmark.setCenter(1, 2, 3);
  bookmark.setBodyId(1);
  bookmark.setCustom(true);
  bookmark.setBookmarkType(ZFlyEmBookmark::EBookmarkType::TYPE_FALSE_MERGE);
  bookmark.setUser("zhaot");
  bookmark.addUserTag();

  std::cout << bookmark.toDvidAnnotationJson().dumpString(2) << std::endl;

  ZFlyEmBookmark bookmark2;
  bookmark2.loadDvidAnnotation(bookmark.toDvidAnnotationJson());
  std::cout << bookmark2.toDvidAnnotationJson().dumpString(2) << std::endl;
#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZLineSegmentObject *line = new ZLineSegmentObject;
  line->setStartPoint(10, 10, 10);
  line->setEndPoint(70, 70, 70);
  line->setColor(QColor(255, 0, 0));
//  line->setVisualEffect(NeuTube::Display::Line::VE_LINE_FADING_PROJ);
  line->addVisualEffect(NeuTube::Display::Line::VE_LINE_PROJ);

  frame->document()->addObject(line);
#endif


#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  host->addStackFrame(frame);
  ZStack *stack = new ZStack(GREY, 10, 10, 1, 1);
  stack->setOne();
  stack->setIntValue(0, 0, 0, 0, 2);
//  stack->setIntValue(0, 0, 0, 0, 5);
  frame->loadStack(stack);

  host->presentStackFrame(frame);
#endif

#if 0
  ZObject3dScan obj;
  for (int i = 0; i < 50; ++i) {
    int z = 100 + i;
    int y = 100 + i * 2;
    int x1 = 30 + i;
    int x2 = 30 + i * 2;
    obj.addSegment(z, y, x1, x2);
  }

  obj.save(GET_TEST_DATA_DIR + "/benchmark/obj2.sobj");

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "3ca7", 8500);
  target.setSynapseName("synapses");


//  ZStackDoc *doc = new ZStackDoc(NULL);
//  doc->loadFile(GET_TEST_DATA_DIR + "/system/diadem/diadem_e1.tif");

  ZFlyEmOrthoMvc *stackWidget = ZFlyEmOrthoMvc::Make(target, NeuTube::Z_AXIS);
  stackWidget->getCompleteDocument()->updateStack(ZIntPoint(4085, 5300, 7329));
  //ZStackFrame *stackWidget = ZStackFrame::Make(
  //      NULL, ZSharedPointer<ZStackDoc>(doc));

  //stackWidget->setWindowFlags(Qt::Widget);

  //stackWidget->consumeDocument(doc);

  //stackWidget->load(GET_TEST_DATA_DIR + "/benchmark/ball.tif");
  ZTestDialog *testDlg = new ZTestDialog(host);
  testDlg->getMainLayout()->addWidget(stackWidget);
  testDlg->show();
  testDlg->raise();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);
  target.setSynapseName("synapses");

  ZFlyEmOrthoWindow *window = new ZFlyEmOrthoWindow(target, NULL);
  window->updateData(ZIntPoint(4085, 5300, 7329));

  window->show();
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "3ca7", 8500);
  target.setSynapseName("synapses");

  QDialog *dlg = new QDialog(host);
  QGridLayout *layout = new QGridLayout(dlg);
  dlg->setLayout(layout);

  ZSharedPointer<ZFlyEmOrthoDoc> sharedDoc =
      ZSharedPointer<ZFlyEmOrthoDoc>(new ZFlyEmOrthoDoc);
  sharedDoc->setDvidTarget(target);

  ZFlyEmOrthoMvc *xyWidget =
      ZFlyEmOrthoMvc::Make(NULL, sharedDoc, NeuTube::Z_AXIS);
//  xyWidget->setDvidTarget(target);
  xyWidget->getCompleteDocument()->updateStack(ZIntPoint(4085, 5300, 7329));

  ZFlyEmOrthoMvc *yzWidget =
      ZFlyEmOrthoMvc::Make(NULL, sharedDoc, NeuTube::X_AXIS);
//  yzWidget->setDvidTarget(target);

  ZFlyEmOrthoMvc *xzWidget =
      ZFlyEmOrthoMvc::Make(NULL, sharedDoc, NeuTube::Y_AXIS);
//  xzWidget->setDvidTarget(target);


  layout->addWidget(xyWidget, 0, 0);
  layout->addWidget(yzWidget, 0, 1);
  layout->addWidget(xzWidget, 1, 0);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setHorizontalSpacing(0);
  layout->setVerticalSpacing(0);

  dlg->show();
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "3ca7", 8500);
  target.setSynapseName("synapses");
  if (reader.open(target)) {
    tic();
    ZStack *stack = reader.readGrayScale(4085, 5300, 7329,
                                         256, 256, 256);
    std::cout << "Stack reading time: " << toc() << "ms" << std::endl;

    QDialog *dlg = new QDialog(host);
    QGridLayout *layout = new QGridLayout(dlg);
    dlg->setLayout(layout);
    ZStackDoc *doc = new ZStackDoc(NULL);

    doc->loadStack(stack);

    {
      ZDvidLabelSlice *slice = new ZDvidLabelSlice;
      slice->setDvidTarget(target);
      slice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      doc->addObject(slice);
    }

    {
      ZDvidLabelSlice *slice = new ZDvidLabelSlice;
      slice->setSliceAxis(NeuTube::X_AXIS);
      slice->setDvidTarget(target);
      slice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      doc->addObject(slice);
    }

    {
      ZDvidLabelSlice *slice = new ZDvidLabelSlice;
      slice->setSliceAxis(NeuTube::Y_AXIS);
      slice->setDvidTarget(target);
      slice->setRole(ZStackObjectRole::ROLE_ACTIVE_VIEW);
      doc->addObject(slice);
    }

    ZSharedPointer<ZStackDoc> sharedDoc(doc);

    ZStackMvc *xyWidget =
        ZStackMvc::Make(NULL, sharedDoc, NeuTube::Z_AXIS);
    xyWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
    xyWidget->getView()->setContentsMargins(0, 0, 0, 0);
    xyWidget->getView()->hideThresholdControl();
    {
      ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
      se->setDvidTarget(target);
      se->attachView(xyWidget->getView());
      doc->addObject(se);
    }

    ZStackMvc *yzWidget =
        ZStackMvc::Make(NULL, sharedDoc, NeuTube::X_AXIS);
    yzWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
    yzWidget->getView()->setContentsMargins(0, 0, 0, 0);
    yzWidget->getView()->hideThresholdControl();
    {
      ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
      se->setDvidTarget(target);
      se->setSliceAxis(NeuTube::X_AXIS);
      se->attachView(yzWidget->getView());
      se->setRange(stack->getBoundBox());
      doc->addObject(se);
    }

    ZStackMvc *xzWidget =
        ZStackMvc::Make(NULL, sharedDoc, NeuTube::Y_AXIS);
    xzWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
    xzWidget->getView()->setContentsMargins(0, 0, 0, 0);
    xzWidget->getView()->hideThresholdControl();
    {
      ZDvidSynapseEnsemble *se = new ZDvidSynapseEnsemble;
      se->setDvidTarget(target);
      se->setSliceAxis(NeuTube::Y_AXIS);
      se->attachView(xzWidget->getView());
      doc->addObject(se);
    }

    layout->addWidget(xyWidget, 0, 0);
    layout->addWidget(yzWidget, 0, 1);
    layout->addWidget(xzWidget, 1, 0);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setHorizontalSpacing(0);
    layout->setVerticalSpacing(0);

    dlg->show();
  }
#endif

#if 0
  QDialog *dlg = new QDialog(host);
  QGridLayout *layout = new QGridLayout(dlg);
  dlg->setLayout(layout);

  ZStackDoc *doc = new ZStackDoc(NULL);
  doc->loadFile(GET_TEST_DATA_DIR + "/system/emstack2.tif");

  ZSharedPointer<ZStackDoc> sharedDoc(doc);

  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/obj2.tif");

  {
    ZObject3dScan *obj = new ZObject3dScan;
    obj->setSliceAxis(NeuTube::X_AXIS);
    obj->loadStack(stack);
    obj->setColor(255, 0, 0);
    doc->addObject(obj);
  }

  {
    ZObject3dScan *obj = new ZObject3dScan;
    obj->setSliceAxis(NeuTube::Y_AXIS);
    obj->loadStack(stack);
    obj->setColor(255, 0, 0);
    doc->addObject(obj);
  }

  {
    ZObject3dScan *obj = new ZObject3dScan;
    obj->setSliceAxis(NeuTube::Z_AXIS);
    obj->loadStack(stack);
    obj->setColor(255, 0, 0);
    doc->addObject(obj);
  }

  /*
  for (int i = 0; i < 50; ++i) {
    int z = 100 + i;
    int y = 100 + i;
    int x1 = 30 + i;
    int x2 = 30 + i * 2;
    obj->addSegment(z, y, x1, x2);
  }

  */
//  obj->setSliceAxis(NeuTube::X_AXIS);
//

//  obj->save(GET_TEST_DATA_DIR + "/benchmark/obj1.sobj");





  ZStackMvc *xyWidget =
      ZStackMvc::Make(NULL, sharedDoc, NeuTube::Z_AXIS);
  xyWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  xyWidget->getView()->setContentsMargins(0, 0, 0, 0);
  xyWidget->getView()->hideThresholdControl();
  ZStackMvc *yzWidget =
      ZStackMvc::Make(NULL, sharedDoc, NeuTube::X_AXIS);
  yzWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  yzWidget->getView()->setContentsMargins(0, 0, 0, 0);
  yzWidget->getView()->hideThresholdControl();
  ZStackMvc *xzWidget =
      ZStackMvc::Make(NULL, sharedDoc, NeuTube::Y_AXIS);
  xzWidget->getView()->layout()->setContentsMargins(0, 0, 0, 0);
  xzWidget->getView()->setContentsMargins(0, 0, 0, 0);
  xzWidget->getView()->hideThresholdControl();

  layout->addWidget(xyWidget, 0, 0);
  layout->addWidget(yzWidget, 0, 1);
  layout->addWidget(xzWidget, 1, 0);

  layout->setContentsMargins(0, 0, 0, 0);
  layout->setHorizontalSpacing(0);
  layout->setVerticalSpacing(0);

  dlg->show();

#endif

#if  0
  ZStackDoc *doc = new ZStackDoc(NULL);
  doc->loadFile(GET_TEST_DATA_DIR + "/system/diadem/diadem_e1.tif");

    ZStackMvc *stackWidget =
        ZStackMvc::Make(NULL, ZSharedPointer<ZStackDoc>(doc), NeuTube::X_AXIS);
  //ZStackFrame *stackWidget = ZStackFrame::Make(
  //      NULL, ZSharedPointer<ZStackDoc>(doc));

  //stackWidget->setWindowFlags(Qt::Widget);

  //stackWidget->consumeDocument(doc);

  //stackWidget->load(GET_TEST_DATA_DIR + "/benchmark/ball.tif");
  ZTestDialog *testDlg = new ZTestDialog(host);
  testDlg->getMainLayout()->addWidget(stackWidget);
  testDlg->show();
  testDlg->raise();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1", "3ca7", 8500);
  target.setSynapseName("mb6_synapses");
  ZDvidReader reader;
  reader.open(target);
  std::vector<ZDvidSynapse> synapseArray =
      reader.readSynapse(1, FlyEM::LOAD_PARTNER_LOCATION);
  for (std::vector<ZDvidSynapse>::const_iterator iter = synapseArray.begin();
       iter != synapseArray.end(); ++iter) {
    const ZDvidSynapse &synapse = *iter;
    std::cout << synapse.toJsonObject().dumpString(2) << std::endl;
  }
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test.sobj");
  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZObject3dScan obj;
  tic();
  obj.importDvidObject("/Users/zhaot/Downloads/513414-6");
  ptoc();
  obj.canonize();
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

#endif

#if 0
  ZObject3dScan bm;
  bm.importDvidObject(GET_TEST_DATA_DIR + "/test_bm.dvid");

  ZObject3dScan bs;
  bs.importDvidObject(GET_TEST_DATA_DIR + "/test_bs.dvid");

  ZDvidInfo dvidInfo;
  ZDvidReader reader;
  ZDvidTarget dvidTarget;
  dvidTarget.set("emdata2.int.janelia.org", "4ad1", 7000);
  if (reader.open(dvidTarget)) {
    dvidInfo = reader.readGrayScaleInfo();
  } else {
    LERROR() << "DVID connection error.";
  }

  ZObject3dScan Bsc = dvidInfo.getBlockIndex(bs);
  ZObject3dScan Bbf_bs = dvidInfo.getBlockIndex(bm);

//  Bsc.subtractSliently(Bbf_bs);

  Bsc.exportDvidObject(GET_TEST_DATA_DIR + "/test_Bsc.dvid");
  Bbf_bs.exportDvidObject(GET_TEST_DATA_DIR + "/test_Bbf_bs.dvid");

  Bsc.subtractSliently(Bbf_bs);

  Bsc.exportDvidObject(GET_TEST_DATA_DIR + "/test_Bsc_sub.dvid");

#endif


#if 0
  ZDvidReader reader;
  reader.open("emdata2.int.janelia.org", "059e", 7000);
  ZObject3dScan obj = reader.readRoi("seven_column_roi");

  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
  ZIntCuboid box = obj.getBoundBox();
  int xMax = (box.getFirstCorner().getX() + box.getLastCorner().getX()) / 2;

  ZObject3dScan::ConstSegmentIterator iter(&obj);

  ZObject3dScan newObj;

  while (iter.hasNext()) {
    const ZObject3dScan::Segment &seg = iter.next();
    if (seg.getStart() <= xMax) {
      int end = std::min(xMax, seg.getEnd());
      newObj.addSegment(seg.getZ(), seg.getY(), seg.getStart(), end);
    }
  }

//  newObj.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZJsonArray array = ZJsonFactory::MakeJsonArray(newObj);

  array.dump(GET_TEST_DATA_DIR + "/test.json");

#endif

#if 0
  ZDvidReader reader;
  reader.open("emdata2.int.janelia.org", "e402", 7000);
  ZObject3dScan obj = reader.readRoi("seven_column_roi");

  ZObject3dScan obj2 = reader.readRoi("half_seven_column_roi");

//  obj.subtractSliently(obj2);

  ZObject3dScan obj3 = obj - obj2;
  obj3.save(GET_TEST_DATA_DIR + "/test3.sobj");

  if (obj2.hasOverlap(obj3)) {
    std::cout << "Bad object subtraction." << std::endl;
  }
  obj2.concat(obj3);

  obj.canonize();
  obj2.canonize();
  if (obj2.equalsLiterally(obj)) {
    std::cout << "Good object subtraction." << std::endl;
    ZJsonArray array = ZJsonFactory::MakeJsonArray(obj3);

    array.dump(GET_TEST_DATA_DIR + "/test3.json");
  } else {
    std::cout << "Bad object subtraction." << std::endl;
  }
#endif

#if 0
  Stack *stack = C_Stack::readSc(GET_TEST_DATA_DIR + "/benchmark/block3.tif");

  double x = 1.4;
  double y = 2.3;
  double z = 3.2;

  double v = misc::SampleStack(stack, x, y, z, misc::SAMPLE_STACK_NN);
  std::cout << v << std::endl;

  v = misc::SampleStack(stack, x, y, z, misc::SAMPLE_STACK_AVERAGE);
  std::cout << v << std::endl;

  v = misc::SampleStack(stack, x, y, z, misc::SAMPLE_STACK_UNIFORM);
  std::cout << v << std::endl;

#endif

#if 0
  ZStack stack;
//  stack.load(GET_TEST_DATA_DIR + "/benchmark/block3.tif");
  stack.load(
        GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/new_label_field.tif");

  ZIntCuboid box = stack.getBoundBox();

  int ix0 = box.getFirstCorner().getX();
  int iy0 = box.getFirstCorner().getY();
  int iz0 = box.getFirstCorner().getZ();

  double ratio = 20.0 / 32.0;
  double inverseRatio = 32.0 / 20.0;

  box.setFirstCorner((box.getFirstCorner().toPoint() * ratio).toIntPoint());
  box.setLastCorner((box.getLastCorner().toPoint() * ratio).toIntPoint());

  ZStack newStack(GREY, box, 1);

  size_t offset = 0;
  box = newStack.getBoundBox();

  int x0 = box.getFirstCorner().getX();
  int y0 = box.getFirstCorner().getY();
  int z0 = box.getFirstCorner().getZ();

  int x1 = box.getLastCorner().getX();
  int y1 = box.getLastCorner().getY();
  int z1 = box.getLastCorner().getZ();

  Stack *oldStack = stack.c_stack();
  uint8_t *array = newStack.array8();


  for (int z = z0; z <= z1; ++z) {
    for (int y = y0; y <= y1; ++y) {
      for (int x = x0; x <= x1; ++x) {
        double v = misc::SampleStack(
              oldStack, x * inverseRatio - ix0,
              y * inverseRatio - iy0,
              z * inverseRatio - iz0,
              misc::SAMPLE_STACK_NN);
        if (std::isnan(v)) {
          v = 0;
        }

        array[offset] = iround(v);
        offset++;
      }
    }
  }

  newStack.save(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/new_label_field_block.tif");

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/new_label_field_block.tif");

  std::vector<ZObject3dScan*> objArray = ZObject3dScan::extractAllObject(stack);
  for (std::vector<ZObject3dScan*>::iterator iter = objArray.begin();
       iter != objArray.end(); ++iter) {
    ZObject3dScan *obj = *iter;
    ZJsonArray objJson = ZJsonFactory::MakeJsonArray(*obj);
    QString outFile = QString("roi_%1.json").arg(obj->getLabel());
    objJson.dump(GET_TEST_DATA_DIR + "/flyem/AL/glomeruli/roi_json/" +
                 outFile.toStdString());
  }

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/gaussians.tif");
  ZStackProcessor::SubtractBackground(&stack, 0.5, 3);
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/system/slice15_L11.tif");
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/body_150.sobj");

  ZFlyEmNeuronImageFactory factory;
  factory.setSizePolicy(ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                        ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX,
                        ZFlyEmNeuronImageFactory::SIZE_BOUND_BOX);
  factory.setDownsampleInterval(7, 7, 7);
  Stack *stack = factory.createSurfaceImage(obj);
  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", stack);

  C_Stack::kill(stack);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "739f", 6300);
  target.setBodyLabelName("bodies121714");

  ZDvidWriter writer;
  ZDvidReader reader;
//  ZDvidUrl url(target);
  if (writer.open(target) && reader.open(target)) {
    std::string dataName = ZDvidData::GetName(ZDvidData::ROLE_THUMBNAIL, ZDvidData::ROLE_BODY_LABEL,
                                              target.getBodyLabelName());
    QStringList keyList = reader.readKeys(dataName.c_str());
    foreach(const QString &key, keyList) {
      writer.deleteKey(dataName, key.toStdString());
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "739f", 6300);
  target.setBodyLabelName("bodies121714");

  ZDvidReader reader;
//  ZDvidUrl url(target);
  if (reader.open(target)) {
    ZJsonObject config = reader.readSkeletonConfig();
    std::cout << config.dumpString(2) << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);
  target.setBodyLabelName("bodies");

  ZDvidReader reader;
  if (reader.open(target)) {
    std::cout << "Has body: " << reader.hasBody(8116) << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);
  target.setBodyLabelName("bodies");

  ZDvidWriter writer;
  writer.open(target);

  writer.deleteSkeleton(10001209);
  writer.deleteBodyAnnotation(10011641);

  std::cout << writer.getStatusCode() << std::endl;

#endif


#if 0
  ZDvidBufferReader reader;
  reader.readPartial(
        "http://emdata1.int.janelia.org:8500/api/node/372c/bodies/sparsevol/8116",
        12, true);
  QByteArray byteArray = reader.getBuffer();

  std::cout << byteArray.size() << std::endl;
  std::cout << *((uint32_t*) (byteArray.data() + 8)) << std::endl;
#endif

#if 0
  try {
    libdvid::DVIDNodeService service("emdata1.int.janelia.org:8500", "372c");
    service.custom_request("bodies/sparsevol/101", libdvid::BinaryDataPtr(), libdvid::HEAD);
  } catch (libdvid::DVIDException &e) {
    std::cout << e.getStatus() << std::endl;
    std::cout << e.what() << std::endl;
  }

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/new_ROI_LOP.sobj");

  ZJsonArray array = ZJsonFactory::MakeJsonArray(obj);

  array.dump(GET_TEST_DATA_DIR + "/new_ROI_LOP.json");
//  std::cout << array.dumpString(0) << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);

  ZDvidWriter writer;
  writer.open(target);

  writer.createData("annotation", "todotest2");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "bdca", 7000);
  target.setBodyLabelName("segmentation-labelvol");
  target.setLabelBlockName("segmentation2");
  ZDvidWriter writer;
  writer.open(target);

  writer.syncAnnotationToLabel("segmentation-labelvol_todo", "replace=true");
#endif

#if 0
  ZDvidTarget target;
  target.set("zhaot-ws1", "0751", 6300);
  target.setBodyLabelName("bodies121714");
  target.setLabelBlockName("segmentation121714");
  ZDvidWriter writer;
  writer.open(target);

  writer.syncAnnotationToLabel("bodies121714_todo");
#endif

#if 0
  //Create pixmap
  ZPixmap pixmap(256, 256);
  pixmap.setOffset(0, 0);
  pixmap.setScale(0.5, 0.5);


  //Paint
  ZPainter painter(&pixmap);
  painter.setPen(QColor(255, 0, 0));

  ZStroke2d stroke;
  stroke.append(164, 241);
  stroke.display(painter, 0, ZStackObject::SOLID, NeuTube::Z_AXIS);
//  painter.drawLine(100, 100, 200, 200);

  qDebug() << painter.getTransform();

  //Save
  pixmap.save((GET_TEST_DATA_DIR + "/test.tif").c_str());

#endif

#if 0
  QUrl url("http://emdata1.int.janelia.org:8500/api/node");
  qDebug() << url.host();
  qDebug() << url.port();
  qDebug() << url.path();
#endif

#if 0
  ZDvidWriter writer;
  writer.del("http://emdata1.int.janelia.org:8500/api/node/372c/skeletons/key/1_swc");
  std::cout << writer.getStatusCode() << std::endl;
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget dvidTarget("emdata1.int.janelia.org", "372c", 8500);
  writer.open(dvidTarget);

  ZJsonObject annotation;
  annotation.setEntry("status", "test");
  writer.writeAnnotation(1, annotation);
#endif

#if 0
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/server_test.json");

  ZDvidWriter writer;
  writer.post("http://zhaot-ws1:8080/update_body", obj);
#endif

#if 0
  ZNeutuService service("http://zhaot-ws1:8080");
  ZDvidTarget target("emdata1.int.janelia.org", "372c", 8500);

  service.requestBodyUpdate(target, 1, ZNeutuService::UPDATE_DELETE);
#endif

#if 0
  ZNeutuService service("http://zhaot-ws1:8080");
  ZDvidTarget target("emdata1.int.janelia.org", "372c", 8500);

  std::cout << "Service normal?: " << service.isNormal() << std::endl;

  service.updateStatus();
  std::cout << "Service normal?: " << service.isNormal() << std::endl;

#endif

#if 0
#if defined(_ENABLE_LIBDVIDCPP_)
  libdvid::DVIDNodeService service("emdata1.int.janelia.org:8500", "372c");
  std::cout << "Reading grayscale ..." << std::endl;

  std::vector<int> blockCoords;
  blockCoords.push_back(100);
  blockCoords.push_back(100);
  blockCoords.push_back(100);


  libdvid::GrayscaleBlocks blocks = service.get_grayblocks("grayscale", blockCoords, 2);

  std::cout << blocks.get_num_blocks() << std::endl;
#endif

#endif


#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo;
  dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  std::vector<ZStack*> stackArray =
      reader.readGrayScaleBlock(ZIntPoint(100, 200, 300), dvidInfo, 2);

  stackArray[0]->save(GET_TEST_DATA_DIR + "/test.tif");
  stackArray[1]->save(GET_TEST_DATA_DIR + "/test2.tif");

#endif

#if 0
  ZSwcTree tree;
  tree.load("/Users/zhaot/Work/neutube/neurolabi/data/00544-ngc.0.swc");
  tree.updateIterator();
  for (Swc_Tree_Node *tn = tree.begin(); tn != NULL; tn = tree.next()) {
    tn->node.x *= -1;
    tn->node.y *= -1;
    tn->node.z *= -1;
  }

  tree.save("/Users/zhaot/Work/neutube/neurolabi/data/00544-ngc.0.flip.swc");
#endif

#if 0
  ZJsonArray objJson1;
  objJson1.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha1_roi.json");

  std::cout << objJson1.size() << std::endl;

  ZJsonArray objJson2;
  objJson2.load(GET_TEST_DATA_DIR + "/flyem/MB/alpha1_below_roi.json");
  std::cout << objJson2.size() << std::endl;

  objJson1.concat(objJson2);
  objJson1.dump(GET_TEST_DATA_DIR + "/flyem/MB/alpha1_new.json");
  std::cout << objJson1.size() << std::endl;
#endif

#if 0
  ZObject3dScan obj;

  ZDvidReader reader;
  ZDvidTarget target("emdata1.int.janelia.org", "425d", 8500);
  reader.open(target);

  ZObject3dScan obj1 = reader.readRoi("alpha3_roi");
  ZObject3dScan obj2 = reader.readRoi("alpha2_roi");
  ZObject3dScan obj3 = reader.readRoi("alpha1_roi");

  obj1.concat(obj2);
  obj1.concat(obj3);
  obj1.canonize();

  obj1.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  ZIntPoint index1 = dvidInfo.getBlockIndex(0, 0, 5120);
  std::cout << index1.getZ() << std::endl;

  ZIntPoint index2 = dvidInfo.getBlockIndex(0, 0, 8736);
  std::cout << index2.getZ() << std::endl;

  ZObject3dScan alpha3 = obj1.getSlice(obj1.getMinZ(), index1.getZ() - 1);
  ZObject3dScan alpha2 = obj2.getSlice(index1.getZ(), index2.getZ() - 1);
  ZObject3dScan alpha1 = obj3.getSlice(index2.getZ(), obj1.getMaxZ());

  {
    ZJsonArray alpha1Json =
        ZJsonFactory::MakeJsonArray(alpha1, ZJsonFactory::OBJECT_SPARSE);
    alpha1Json.dump(GET_TEST_DATA_DIR + "/flyem/MB/roi/alpha1.json");
  }

  {
    ZJsonArray alpha2Json =
        ZJsonFactory::MakeJsonArray(alpha2, ZJsonFactory::OBJECT_SPARSE);
    alpha2Json.dump(GET_TEST_DATA_DIR + "/flyem/MB/roi/alpha2.json");
  }

  {
    ZJsonArray alpha3Json =
        ZJsonFactory::MakeJsonArray(alpha3, ZJsonFactory::OBJECT_SPARSE);
    alpha3Json.dump(GET_TEST_DATA_DIR + "/flyem/MB/roi/alpha3.json");
  }
#endif

#if 0
  std::string name = "@MB6";
  std::string rootNode = GET_FLYEM_CONFIG.getDvidRootNode(name);

  std::cout << "Root node for " << name << ": " << rootNode << std::endl;

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@MB6", 8500);
  target.setSynapseName("mb6_synapses");
  target.setBodyLabelName("bodies3");
  target.setLabelBlockName("labels3");
  target.setGrayScaleName("grayscale");

  ZDvidReader reader;
  if (reader.open(target)) {
    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

    ZWeightedPointArray ptArray;

    std::set<uint64_t> bodyIdArray = reader.readAnnnotatedBodySet();
    for (std::set<uint64_t>::const_iterator iter = bodyIdArray.begin();
         iter != bodyIdArray.end(); ++iter) {
      uint64_t bodyId = *iter;
      ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);
      if (ZString(annotation.getName()).startsWith("KC-s") ||
          ZString(annotation.getName()).startsWith("KC-p") ||
          ZString(annotation.getName()).startsWith("KC-any") ||
          ZString(annotation.getName()).startsWith("KC-c")) {
        std::vector<ZDvidSynapse> synapseArray = reader.readSynapse(bodyId);
        for (std::vector<ZDvidSynapse>::const_iterator
             iter = synapseArray.begin(); iter != synapseArray.end(); ++iter) {
          const ZDvidSynapse &synapse = *iter;
          ZIntPoint blockIndex = dvidInfo.getBlockIndex(synapse.getPosition());
          ptArray.append(blockIndex.toPoint(), 1.0);
        }
      }
    }

    std::cout << ptArray.size() << " synapses" << std::endl;

    ZStackFactory factory;
    ZStack *stack = factory.makeDensityMap(ptArray, 5.0);
    stack->save(GET_DATA_DIR + "/flyem/MB/kc_synapse.tif");
  }

#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/flyem/MB/kc_synapse.tif");

  stack.binarize(30);

  ZObject3dScan obj = ZObject3dFactory::MakeObject3dScan(stack);

  obj.save(GET_DATA_DIR + "/flyem/MB/kc_synapse_t30.sobj");

  ZJsonArray array =
      ZJsonFactory::MakeJsonArray(obj, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_TEST_DATA_DIR + "/flyem/MB/roi/kc_synapse_t30_roi.json");
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "ad5d", 8500);

  ZDvidReader reader;
  reader.open(target);



  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();

  ZIntCuboid box =
      dvidInfo.getBlockBox(dvidInfo.getBlockIndex(3200, 1600, 12960));
  box.setSize(352, 384, 352);
  ZArray *label = reader.readLabels64(box);
//  label->setValue((uint64_t) 16573243);

//  label->setValue(100);
  ZDvidWriter writer;
  writer.open(target);
  writer.writeLabel(*label);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);

  ZDvidReader reader;
  reader.open(target);

  ZDvidWriter writer;
  writer.open(target);

  writer.refreshLabel(ZIntCuboid(4099, 5018, 10343,
                                 4099 + 99, 5018 + 99, 10343 + 99), 1);
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);

  ZDvidReader reader;
  reader.open(target);

  reader.readTile(0, 0, 0, 0);

  ZArray *label = reader.readLabels64Lowtis(4099, 5018, 10343, 512, 512);

  reader.readLabels64Lowtis(4099, 5018, 10344, 512, 512);

#endif


#if 0
  ZFlyEmBookmark bookmark;
  bookmark.setComment("test");

  bookmark.getPropertyJson().setEntry("comment", "test2");
  bookmark.getPropertyJson().setEntry("prop1", "test2");
  bookmark.getPropertyJson().setEntry("prop2", "test3");
  bookmark.setLocation(1, 2, 3);
  bookmark.setBodyId(10);

  std::cout << bookmark.toDvidAnnotationJson().dumpString() << std::endl;

  ZFlyEmBookmark bookmark2;
  bookmark2.loadDvidAnnotation(bookmark.toDvidAnnotationJson());
  std::cout << bookmark2.toDvidAnnotationJson().dumpString() << std::endl;

#endif

#if 0
  ZSharedPointer<int> sharedPtr;
  sharedPtr = ZSharedPointer<int>(new int);
//  sharedPtr.reset();

  if (sharedPtr == NULL){
    std::cout << "NUll pointer" << std::endl;
  } else {
    std::cout << "NOT NULL" << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@FIB19", 7000);

  ZDvidWriter writer;
  writer.open(target);

  writer.writeMasterNode("3e179");

  std::vector<std::string> nodeList = ZDvidReader::ReadMasterList(target);
  for (std::vector<std::string>::const_iterator iter = nodeList.begin();
       iter != nodeList.end(); ++iter) {
    std::cout << "  " << *iter << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@MB6", 8500);

  ZDvidWriter writer;
  writer.open(target);

  writer.writeMasterNode("7abee");

  std::vector<std::string> nodeList = ZDvidReader::ReadMasterList(target);
  for (std::vector<std::string>::const_iterator iter = nodeList.begin();
       iter != nodeList.end(); ++iter) {
    std::cout << "  " << *iter << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@MB6", 8500);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj = reader.readRoi("kc_alpha_roi");

  std::cout << obj.getVoxelNumber() * (obj.getDsIntv().getX() + 1) * 8 *
               (obj.getDsIntv().getY() + 1) * 8 *
               (obj.getDsIntv().getZ() + 1) * 8 / 1000 / 1000 /1000
            << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj = reader.readRoi("roi_new_segmentation_LO_LOP_x1");

  size_t v1 = obj.getVoxelNumber() * (obj.getDsIntv().getX() + 1) * 8 *
      (obj.getDsIntv().getY() + 1) * 8 *
      (obj.getDsIntv().getZ() + 1) * 8 / 1000 / 1000 /1000;


  obj = reader.readRoi("roi_new_segmentation_LO_LOP_x2");

  size_t v2 = obj.getVoxelNumber() * (obj.getDsIntv().getX() + 1) * 8 *
      (obj.getDsIntv().getY() + 1) * 8 *
      (obj.getDsIntv().getZ() + 1) * 8 / 1000 / 1000 /1000;

  obj = reader.readRoi("roi_new_segmentation_LO_LOP_x3");

  size_t v3 = obj.getVoxelNumber() * (obj.getDsIntv().getX() + 1) * 8 *
      (obj.getDsIntv().getY() + 1) * 8 *
      (obj.getDsIntv().getZ() + 1) * 8 / 1000 / 1000 /1000;

  std::cout << v1 + v2 + v3 << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@FIB19", 7000);

  ZDvidReader reader;
//  reader.open(target);
  std::vector<std::string> nodeList = ZDvidReader::ReadMasterList(target);
  for (std::vector<std::string>::const_iterator iter = nodeList.begin();
       iter != nodeList.end(); ++iter) {
    std::cout << "  " << *iter << std::endl;
  }

#endif

#if 0
  QDateTime time;
  std::cout << time.currentTime().elapsed() << std::endl;
#endif

#if 0
  ZJsonArray myList;
  ZJsonObject myMap;

  // workaround

//  myList.remove(0);

  // now this works:
  myMap.setEntry("key", myList);

  myList.append(12345);

  std::cout << myMap.dumpString() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);

  ZDvidReader reader;

  if (reader.open(target)) {
    ZDvidRoi roi;
    reader.readRoi("ROI_LOP_15", &roi);

    std::cout << roi.getRoiRef()->getVoxelNumber() << std::endl;
    std::cout << roi.getBlockSize().toString() << std::endl;
    std::cout << roi.getName() << std::endl;

    std::cout << roi.contains(8781, 5368, 14824) << std::endl;
    std::cout << roi.contains(5778, 5564, 15442) << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);

  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan roi;
    roi.load(GET_TEST_DATA_DIR +
             "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP.sobj");

    ZIntCuboid roiBox = roi.getBoundBox();

    ZIntCuboid cropBox = roiBox;

    ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
    ZIntPoint blockIndex1 = dvidInfo.getBlockIndex(7314, 0, 0);
    ZIntPoint blockIndex2 = dvidInfo.getBlockIndex(12958, 0, 0);

    cropBox.setLastX(blockIndex1.getX());

    ZObject3dScan *roi1 = roi.subobject(cropBox, NULL);
    roi1->save(GET_TEST_DATA_DIR +
               "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x1.sobj");

    ZJsonArray array = ZJsonFactory::MakeJsonArray(*roi1);
    array.dump(GET_TEST_DATA_DIR +
               "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x1.json");

    cropBox.setFirstX(blockIndex1.getX() + 1);
    cropBox.setLastX(blockIndex2.getX());
    ZObject3dScan *roi2 = roi.subobject(cropBox, NULL);
    roi2->save(GET_TEST_DATA_DIR +
               "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x2.sobj");
    array = ZJsonFactory::MakeJsonArray(*roi2);
        array.dump(GET_TEST_DATA_DIR +
                   "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x2.json");


    cropBox.setFirstX(blockIndex2.getX() + 1);
    cropBox.setLastX(roiBox.getLastCorner().getX());
    ZObject3dScan *roi3 = roi.subobject(cropBox, NULL);
    roi3->save(GET_TEST_DATA_DIR +
               "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x3.sobj");

    array = ZJsonFactory::MakeJsonArray(*roi3);
        array.dump(GET_TEST_DATA_DIR +
                   "/flyem/FIB/FIB19/roi/roi_new_segmentation_LO_LOP_x3.json");


    if (roi1->getVoxelNumber() + roi2->getVoxelNumber() + roi3->getVoxelNumber()
        != roi.getVoxelNumber()) {
      std::cout << "WARNING: Inconsistent voxel number" << std::endl;
    }
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("segmentation-labelvol");
  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan obj = reader.readCoarseBody(51017317967);
    obj.save(GET_TEST_DATA_DIR + "/test.sobj");
  }
#endif

#if 0
  const std::vector<ZDvidTarget> &repoList = GET_FLYEM_CONFIG.getDvidRepo();

  std::cout << repoList.size() << std::endl;

  std::set<std::string> userNameSet;
  std::set<std::string> uuidSet;
  std::vector<ZDvidTarget> targetList;

  for (std::vector<ZDvidTarget>::const_iterator iter = repoList.begin();
       iter != repoList.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    userNameSet.insert(
          target.getUserNameSet().begin(), target.getUserNameSet().end());
    if (ZString(target.getUuid()).startsWith("@")) {
      if (uuidSet.count(target.getUuid()) == 0) {
        uuidSet.insert(target.getUuid());
        targetList.push_back(target);
      }
    }
  }

  std::cout << userNameSet.size() << " users." << std::endl;


  for (std::vector<ZDvidTarget>::const_iterator iter = targetList.begin();
       iter != targetList.end(); ++iter) {
    const ZDvidTarget &target = *iter;
    ZDvidReader reader;
    reader.open(target);
    //      ZDvidUrl url(target);
    for (std::set<std::string>::const_iterator iter = userNameSet.begin();
         iter != userNameSet.end(); ++iter) {
      const std::string &user = *iter;
      ZJsonArray jsonArray = reader.readAnnotation(
            target.getBookmarkName(), "user:" + user);
      std::cout << jsonArray.size() << " bookmarks for " << target.getUuid()
                << ":" << user << std::endl;
      for (size_t i = 0; i < jsonArray.size(); ++i) {
        ZJsonObject jsonObj(jsonArray.value(i));
        ZFlyEmBookmark bookmark;
        bookmark.loadDvidAnnotation(jsonObj);
        if (bookmark.getComment().contains("split")) {
          std::cout << bookmark.getCenter().toString() << ":"
                    << bookmark.getComment().toStdString() << std::endl;
        }
      }
    }
  }

#endif
#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "5cb3", 8700);

  ZDvidWriter writer;
  writer.open(target);

  ZJsonObject obj;
  obj.setEntry("nonlinear", true);
  obj.setEntry("offset", 0.0);
  obj.setEntry("scale", 1.2);
  writer.writeJson("neutu_config", "contrast", obj);

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "27b2", 7000);

  ZDvidWriter writer;
  writer.open(target);

  writer.createData("keyvalue", "branches", false);
#endif

#if 0
  lowtis::DVIDLabelblkConfig config;
  config.username = "test";
  config.dvid_server = "emdata1.int.janelia.org:8500";
  config.dvid_uuid = "372c";
  config.datatypename = "labels";

  lowtis::ImageService service(config);

  int width = 512;
  int height = 512;
  std::vector<int> offset(3, 0);
  offset[0] = 4099;
  offset[1] = 5018;
  offset[2] = 10343;

  char *buffer = new char[width * height * 8];
  service.retrieve_image(width, height, offset, buffer);

#endif

#if 0
#if defined(_ENABLE_LOWTIS_)
  lowtis::DVIDLabelblkConfig config;
  config.username = "test";
  config.dvid_server = "emdata2.int.janelia.org:9000";
  config.dvid_uuid = "2ad1";
  config.datatypename = "groundtruth";

  lowtis::ImageService service(config);

  int width = 512;
  int height = 512;
  std::vector<int> offset(3, 0);
  offset[0] = 2357;
  offset[1] = 2216;
  offset[2] = 4053;

  char *buffer = new char[width * height * 8];
  service.retrieve_image(width, height, offset, buffer, 2);


  ZStack *stack = new ZStack(GREY, 512, 512, 1, 1);

  uint64_t *dataArray = (uint64_t*) buffer;
  uint8_t *stackArray = stack->array8();

  int index = 0;
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      stackArray[index] = dataArray[index] % 255;
      ++index;
    }
  }

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "372c", 8500);


  lowtis::DVIDLabelblkConfig config;
  config.username = "test";
  config.dvid_server = "emdata1.int.janelia.org:8500";
  config.dvid_uuid = "372c";
  config.datatypename = "labels";
  //config.refresh_rate = 0;
  srand(5);
  lowtis::ImageService service(config);

  config.bytedepth = 0;
  for (int i = 0; i < 5; i++) {
    ZDvidReader reader;
    reader.open(target);

    int width = 800;
    int height = 600;
    std::vector<int> offset(3, 0);
    int xoff = rand() % 1000 + 4000;
    int yoff = rand() % 1000 + 4000;
    int zoff = rand() % 1000 + 4000;

    mylib::Dimn_Type arrayDims[3];
    arrayDims[0] = width;
    arrayDims[1] = height;
    arrayDims[2] = 1;
    ZArray *array = new ZArray(mylib::UINT64_TYPE, 3, arrayDims);

    //  4570 4303 4439

    //   offset[0] = 4570;
    //   offset[1] = 4303;
    //   offset[2] = 4439;

    offset[0] = xoff;
    offset[1] = yoff;
    offset[2] = zoff;

    std::cout << i << " " << xoff << " " << yoff << " " << zoff <<std::endl;

    array = reader.readLabels64Lowtis(offset[0], offset[1], offset[2],
        width, height);
//    service.retrieve_image(width, height, offset, array->getDataPointer<char>());

     ZObject3dScanArray m_objArray;
    ZObject3dFactory::MakeObject3dScanArray(
          *array, NeuTube::Z_AXIS, true, &m_objArray);

    delete array;
    //          char *buffer = new char[width * height * 8];

    //          service.retrieve_image(width, height, offset, buffer);

    //          delete []buffer;
  }
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/system/split_test/test.sobj");
  obj.getSlice(2742, 2813).save(GET_TEST_DATA_DIR + "/system/split_test/body.sobj");
#endif

#if 0
  ZPixmap pixmap(512, 512);
  ZStTransform transform;
  transform.setOffset(-50, -100);
  transform.setScale(0.5, 0.5);
  pixmap.setTransform(transform);

  ZPainter painter(&pixmap);

  painter.setPen(QColor(255, 0, 0));

  painter.drawLine(100, 200, 300, 500);

  pixmap.save((GET_TEST_DATA_DIR + "/test.tif").c_str());

#endif

#if 0
  ZDvidTarget target;
  target.setFromSourceString("http:hackathon.janelia.org:-1:2a3");
#endif

#if 0
  ZDvidSynapse synapse;
  synapse.setPosition(30, 30, 30);
  synapse.setKind(ZDvidSynapse::KIND_PRE_SYN);
  synapse.setDefaultRadius();
  synapse.setDefaultColor();

  ZJsonObject json = synapse.toJsonObject();

  ZDvidAnnotation::AddProperty(json, "user", "test");
  ZDvidSynapse::SetConfidence(json, 1.0);

  std::cout << json.dumpString(2) << std::endl;

  ZJsonObject obj;
  obj.setEntry("conf", "0.5");
  ZDvidAnnotation::SetProperty(json, obj);
  std::cout << json.dumpString(2) << std::endl;

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "0c7e", 7000);
  target.setBodyLabelName("segmentation-labelvol");
  target.setLabelBlockName("segmentation-labelvol");
  target.setSynapseName("annot_synapse_060616");

  ZDvidWriter writer;
  writer.open(target);
  writer.createSynapseLabelsz();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "ab89", 7000);
  target.setBodyLabelName("segmentation-labelvol");
  target.setLabelBlockName("segmentation-labelvol");
  target.setSynapseName("annot_synapse_083116");

  ZDvidReader reader;
  reader.open(target);
  ZJsonArray array = reader.readSynapseLabelsz(10, ZDvid::ELabelIndexType::INDEX_ALL_SYN);
  std::cout << array.dumpString(2);

  /*
  ZDvidWriter writer;
  writer.open(target);
  writer.createSynapseLabelsz();
  */
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "0c7e", 7000);
  target.setBodyLabelName("segmentation-labelvol");
  target.setLabelBlockName("segmentation-labelvol");
  target.setSynapseName("annot_synapse_060616");

  ZDvidReader reader;
  reader.open(target);

  ZJsonArray json = reader.readSynapseLabelsz(10, ZDvid::ELabelIndexType::INDEX_PRE_SYN);
  std::cout << json.dumpString(2) << std::endl;
#endif

#if 0
  for (int i = 0; i < 66; ++i) {
    std::cout << i << ": " << BIT_FLAG(i) << std::endl;
  }
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 1, 2);
  obj.addSegment(0, 1, 5, 7);
  obj.addSegment(0, 2, 0, 10);
  obj.addSegment(0, 3, 1, 2);
  obj.addSegment(0, 3, 4, 6);

  ZObject3dScan slice = obj.getPlaneSurface(0);

  slice.print();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  obj.getPlaneSurface().save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/flyem/AL/lightseg.tif");

  Stack slice = C_Stack::sliceView(stack.c_stack(), 128, 128);
  C_Stack::write(GET_TEST_DATA_DIR + "/misc/segtest2.tif", &slice);
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/system/emstack2.tif");

  ZImage image(stack.width(), stack.height(), QImage::Format_ARGB32);

  image.setData(stack.array8(0));
  image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());

  image.setData(stack.array8(0), 150);
  image.save((GET_TEST_DATA_DIR + "/test2.tif").c_str());

  image.setData(stack.array8(0), 2, 1, 150);
  image.save((GET_TEST_DATA_DIR + "/test3.tif").c_str());


#endif

#if 0
  int *array = new int[100];
  delete [] array;
  int x = array[0];  // BOOM
#endif

#if 0
  Mc_Stack *testStack = (Mc_Stack*) malloc(sizeof(Mc_Stack));

  free(testStack);

  ZStack *stack = new ZStack;
  Stack *stackData = C_Stack::make(GREY, 3, 3, 3);
  stack->load(stackData);

  delete stack;

  stack = NULL;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/432.sobj");

  ZDvidInfo dvidInfo;
  dvidInfo.setBlockSize(32, 32, 32);
  tic();
  ZObject3dScan obj2 = dvidInfo.getBlockIndex(obj);
  ptoc();

  tic();
  obj.downsampleMax(31, 31, 31);
  ptoc();
  obj.setDsIntv(0, 0, 0);

//  obj.save(GET_TEST_DATA_DIR + "/test.sobj");


//  obj2.load(GET_TEST_DATA_DIR + "/benchmark/sparsevol/test2.sobj");

  std::cout << obj.equalsLiterally(obj2) << std::endl;



#endif

#if 0
  tic();
  ZPixmap pixmap(QSize(1000, 1000));
  pixmap.fill(Qt::transparent);

  QPainter painter(&pixmap);
  painter.setPen(QColor(0, 128, 0, 128));

  ZImage image(1000, 1000);
  int w = image.width();
  int h = image.height();

  for (int j = 0; j < h; j++) {
    uchar *line = image.scanLine(j);
    for (int i = 0; i < w; i++) {
      *line++ = 0;
      *line++ = 128;
      *line++ = 0;
      *line++ = 128;
    }
  }

  pixmap.fromImage(image);

  std::cout << "Painting time: " << toc() << std::endl;
#endif


#if 0
  ZPixmap pixmap(512, 512);

  ZStTransform transform;
  transform.setOffset(100, 0);
//  transform.setScale(0.5, 0.5);
  pixmap.setTransform(transform);

  ZPainter painter(&pixmap);
  QPen pen(QColor(0, 255, 0));
  pen.setWidth(2);
  painter.setPen(pen);

  painter.drawLine(100, 100, 200, 300);

  ZPixmap pixmap2(512, 512);


  transform.setScale(0.5, 0.3);
  pixmap2.setTransform(transform);

  ZPainter painter2(&pixmap2);
  pen.setColor(QColor(255, 0, 0, 128));
  painter2.setPen(pen);
  painter2.drawLine(100, 100, 200, 300);

  pixmap2.matchProj();

  painter.drawPixmap(pixmap2);
  pixmap.save((GET_TEST_DATA_DIR + "/test.tif").c_str());

#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target("emdata1.int.janelia.org", "eafc", 8500);
  target.setLabelBlockName("labels3");
  reader.open(target);
  ZArray *array = reader.readLabels64(4327, 5443, 6341, 1024, 1024, 1);

  tic();
  ZObjectColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);

  ZImage image(1024, 1024);
  std::set<uint64_t> selected;
  selected.insert(1);
  image.drawLabelField(array->getDataPointer<uint64_t>(),
                       colorScheme.getColorTable(), 128, selected);

//  ZPixmap pixmap;
//  pixmap.fromImage(image);

  std::cout << "Painting time: " << toc() << std::endl;

  image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());

  delete array;
#endif

#if 0
  ZDvidReader reader;

  ZDvidTarget target("emdata2.int.janelia.org", "2ad1", 9000);
  if (reader.open(target)) {
    ZIntPoint pt = reader.readRoiBlockSize("seven_column");
    std::cout << pt.toString() << std::endl;
  }

#endif

#if 0
  int statusCode;
  std::string url = "http://emdata2.int.janelia.org:9000/api/repo/2ad2";
  ZDvid::MakeHeadRequest(url, statusCode);
  std::cout << statusCode << std::endl;
#endif

#if 0
  ZDvidTarget target("emdata2.int.janelia.org", "3c1d", 9000);

  ZDvidReader reader;
  reader.open(target);

  switch (reader.getNodeStatus()) {
  case ZDvid::ENodeStatus::NODE_INVALID:
    std::cout << "Invalid node";
    break;
  case ZDvid::ENodeStatus::NODE_LOCKED:
    std::cout << "Locked node";
    break;
  case ZDvid::ENodeStatus::NODE_NORMAL:
    std::cout << "Normal node";
    break;
  case ZDvid::ENodeStatus::NODE_OFFLINE:
    std::cout << "Node cannot be connected";
    break;
  }

  std::cout << std::endl;
#endif

#if 0
  ZFlyEmRoiProject proj("test");

  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/flyem/test/roi_test.swc");

  proj.importRoiFromSwc(&tree, false);
  proj.printSummary();

  std::cout << "Appending" << std::endl;
  tree.translate(0, 0, 1);
  proj.importRoiFromSwc(&tree, true);

  proj.printSummary();

  std::cout << "Restting" << std::endl;
  proj.resetRoi();
  proj.printSummary();

  std::cout << "Clearing" << std::endl;
  proj.clear();
  proj.printSummary();
#endif

#if 0
  ZFlyEmSupervisor supervisor;
  supervisor.setSever("emdata1.int.janelia.org:9100");
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "a0310", 7000);
  supervisor.setDvidTarget(target);

  FILE *fp = fopen((GET_TEST_DATA_DIR + "/flyem/lockbodies.log").c_str(), "r");

  std::set<uint64_t> idSet;
  ZString str;
  while (str.readLine(fp)) {
    std::vector<std::string> wordArray = str.toWordArray(" ");
    if (wordArray.size() > 3) {
      ZString idStr = wordArray[wordArray.size() - 2];
      std::vector<uint64_t> idArray = idStr.toUint64Array();
      if (idArray.size() == 1) {
        idSet.insert(idArray[0]);
      }
    }
  }

  ZString extIdStr = "391 53031432314 51008689050 275 51026028840 51008697839 "
      "53031432315 53031432313 53031432312 53031432310 53031432309 53031432307 "
      "53031422579  53031432311 "
      "53031432316 51027140199 53031432308 53031432306";
  std::vector<uint64_t> idArray = extIdStr.toUint64Array();

  idSet.insert(idArray.begin(), idArray.end());

  ZDvidReader reader;
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("segmentation-labelvol");
  reader.open(target);
  std::cout << "Locking " << idSet.size() << " bodies" << std::endl;
  idArray.resize(idSet.size());
  std::vector<size_t> voxelCountArray(idSet.size());
  size_t index = 0;
  for (std::set<uint64_t>::const_iterator iter = idSet.begin();
       iter != idSet.end(); ++iter, ++index) {
    uint64_t bodyId = *iter;
    std::cout << "  " << bodyId << std::endl;

//    ZObject3dScan obj = reader.readCoarseBody(bodyId);
////    std::cout << "  " << bodyId <<  " #" << obj.getVoxelNumber() << std::endl;
//    idArray[index] = bodyId;
//    voxelCountArray[index] = obj.getVoxelNumber();
    supervisor.checkIn(bodyId);
//    supervisor.checkOut(bodyId);
  }

  std::cout << idArray.size() << " bodies" << std::endl;
  for (size_t i = 0; i < idArray.size(); ++i) {
    if (voxelCountArray[i] > 0) {
      std::cout << idArray[i] << " " << voxelCountArray[i] << std::endl;
    }
  }

  for (size_t i = 0; i < idArray.size(); ++i) {
    if (voxelCountArray[i] > 0) {
      std::cout << idArray[i] << " ";
    }
  }

  std::cout << std::endl;
#endif

#if 0
//  ZString idStr = "170 275 ";
  ZString idStr = "391 39168 39778 44509 555931 922828 1590792 "
      "3860200 128434283 131179769 51001411287 51002028701 51002423651 "
      "51002431213 51002432799 51002924100 51002962389 51003258636 "
      "51003288814 51003610271 51003950675 51003998269 51007802197 "
      "51008111673 51008181411 51008689050 51008697212 51019820724 "
      "51019822625 51019822853 51025529191 51025530145 51025961260 "
      "51026028840 51027140199 53031422579 53031422587 53031432307 53031432314";

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "a0310", 7000);
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("segmentation-labelvol");

  ZDvidWriter writer;
  writer.open(target);

  std::vector<uint64_t> idArray = idStr.toUint64Array();

  for (std::vector<uint64_t>::const_iterator iter = idArray.begin();
       iter != idArray.end(); ++iter) {
    uint64_t bodyId = *iter;
    std::cout << "  " << bodyId << std::endl;
    uint64_t newBodyId = writer.rewriteBody(bodyId);
    std::cout << "  " << newBodyId << std::endl;

    if (newBodyId == 0) {
      std::cout << "WARNING: rewite falied!!!" << std::endl;
    }
  }



#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("http://emdata2.int.janelia.org:8500", "b6bc");

  ZDvidWriter writer;
  writer.open(target);
//  reader.open(target);

  uint64_t bodyId = 200013154;
  uint64_t newBodyId =writer.rewriteBody(bodyId);
  std::cout << newBodyId << std::endl;

#if 0
  ZObject3dScan obj = reader.readBody(bodyId);


  if (!obj.isEmpty()) {
    uint64_t newBodyId = writer.writeSplit(obj, bodyId, 0);
    std::cout << newBodyId << std::endl;

    if (newBodyId > 0) {
      ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);

      if (!annotation.isEmpty()) {
        writer.deleteBodyAnnotation(bodyId);
        annotation.setBodyId(newBodyId);
        writer.writeBodyAnntation(annotation);
      }
    }
  }
#endif

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "a0310", 7000);
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("segmentation-labelvol");

  ZDvidReader reader;
  reader.open(target);


  ZString idStr = "53031432101 53031432096 53031432001 53031431446 53031431447 "
      "53031430965 53031430964 53031430949 53031430932 53031430836 53031430833 "
      "53031430802 53031430800 53031430438 53031430437 53031430434 53031430435 "
      "53031430436 "
      "53031429567 53031429562 53031429465 53031429466 53031429467 53031429468 "
      "53031429469 53031429470 53031429471 53031429472 53031429473 53031429474 "
      "53031429475 53031429476 53031429477 53031429478 53031429479 53031429480 "
      "53031429481 53031429482 53031429483 53031429484 53031429485 53031429486 "
      "53031429487 53031429488 53031429489 53031429490 53031429491 53031429492 "
      "53031429493 53031429494 53031429495 53031429496 53031429497 53031429498 "
      "53031429499 53031429500 53031429501 53031429502 53031429503 53031429504 "
      "53031429505 53031429506 53031429507 53031429508 53031429509 53031429510 "
      "53031429511 53031429512 53031429513 53031429514 53031429515 53031429516 "
      "53031429517 53031429518 53031429519 53031429520 53031429521 53031429522 "
      "53031429523 53031429524 53031429525 53031429526 53031429527 53031429528 "
      "53031429529 53031429530 53031429531 53031429532 53031429533 53031429534 "
      "53031429535 53031429536 53031429537 53031429538 53031429539 53031429540 "
      "53031429541 53031429542 53031429543 53031429544 53031429545 53031429546 "
      "53031429547 53031429548 53031429549 53031429550 53031429551 53031429552 "
      "53031429553 53031429554 53031429555 53031429463 53031429459 53031429453"
      "53031431448 53031431449 53031431450 53031431451 53031431452 53031431453 "
      "53031430663 53031430658 53031430649 53031430646 53031430639 53031430585"
      "53031432042 53031432043 53031432044 53031432045 53031432046 53031432047 "
      "53031432048 53031432049 53031432050 53031432051 53031432052 53031432053 "
      "53031432054 53031432055 53031432056 53031432057 53031432058 53031432059 "
      "53031432060 53031432061 53031432062 53031432063 53031432064 53031432065 "
      "53031432066 53031432067 53031432068 53031432069 53031432070 53031432071 "
      "53031432072";

  ZDvidWriter writer;
  writer.open(target);

  std::vector<uint64_t> idArray = idStr.toUint64Array();

  for (std::vector<uint64_t>::const_iterator iter = idArray.begin();
       iter != idArray.end(); ++iter) {
    uint64_t bodyId = *iter;
    std::cout << "  " << bodyId << std::endl;
    ZFlyEmBodyAnnotation annotation = reader.readBodyAnnotation(bodyId);

    if (annotation.isEmpty()) {
      annotation.setBodyId(bodyId);
      annotation.setComment("from external split");
      annotation.setUser(NeuTube::GetCurrentUserName());
      annotation.setName("glia");
      writer.writeAnnotation(bodyId, annotation.toJsonObject());
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "a0310", 7000);
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("segmentation-labelvol");
  target.setSynapseName("annot_synapse_083116");

  ZDvidReader reader;
  reader.open(target);

  ZString idStr = "53031432101 53031432096 53031432001 53031431446 53031431447 "
      "53031430965 53031430964 53031430949 53031430932 53031430836 53031430833 "
      "53031430802 53031430800 53031430438 53031430437 53031430434 53031430435 "
      "53031430436 "
      "53031429567 53031429562 53031429465 53031429466 53031429467 53031429468 "
      "53031429469 53031429470 53031429471 53031429472 53031429473 53031429474 "
      "53031429475 53031429476 53031429477 53031429478 53031429479 53031429480 "
      "53031429481 53031429482 53031429483 53031429484 53031429485 53031429486 "
      "53031429487 53031429488 53031429489 53031429490 53031429491 53031429492 "
      "53031429493 53031429494 53031429495 53031429496 53031429497 53031429498 "
      "53031429499 53031429500 53031429501 53031429502 53031429503 53031429504 "
      "53031429505 53031429506 53031429507 53031429508 53031429509 53031429510 "
      "53031429511 53031429512 53031429513 53031429514 53031429515 53031429516 "
      "53031429517 53031429518 53031429519 53031429520 53031429521 53031429522 "
      "53031429523 53031429524 53031429525 53031429526 53031429527 53031429528 "
      "53031429529 53031429530 53031429531 53031429532 53031429533 53031429534 "
      "53031429535 53031429536 53031429537 53031429538 53031429539 53031429540 "
      "53031429541 53031429542 53031429543 53031429544 53031429545 53031429546 "
      "53031429547 53031429548 53031429549 53031429550 53031429551 53031429552 "
      "53031429553 53031429554 53031429555 53031429463 53031429459 53031429453"
      "53031431448 53031431449 53031431450 53031431451 53031431452 53031431453 "
      "53031430663 53031430658 53031430649 53031430646 53031430639 53031430585"
      "53031432042 53031432043 53031432044 53031432045 53031432046 53031432047 "
      "53031432048 53031432049 53031432050 53031432051 53031432052 53031432053 "
      "53031432054 53031432055 53031432056 53031432057 53031432058 53031432059 "
      "53031432060 53031432061 53031432062 53031432063 53031432064 53031432065 "
      "53031432066 53031432067 53031432068 53031432069 53031432070 53031432071 "
      "53031432072";

  std::vector<uint64_t> idArray = idStr.toUint64Array();

  std::ofstream output;
  output.open((GET_TEST_DATA_DIR + "/test.txt").c_str());
  for (std::vector<uint64_t>::const_iterator iter = idArray.begin();
       iter != idArray.end(); ++iter) {
    uint64_t bodyId = *iter;
    std::cout << "  " << bodyId << std::endl;
    std::vector<ZDvidSynapse> synapseArray = reader.readSynapse(bodyId);
    if (!synapseArray.empty()) {
      output << bodyId << std::endl;
    }
  }

#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "0c7e", 7000);
  writer.open(target);
  writer.deleteKey("annotations", "body_synapses");

#endif

#if 0
#ifdef _ENABLE_LIBDVIDCPP_
  libdvid::DVIDNodeService service("emdata2.int.janelia.org:8500", "b6bc");
  service.custom_request("bodies/sparsevol/200013118", libdvid::BinaryDataPtr(), libdvid::GET, true);
  service.custom_request("bodies/sparsevol-coarse/200013118", libdvid::BinaryDataPtr(), libdvid::GET, false);
#endif
  /*
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "b6bc", 8500);
  reader.open(target);
//  reader.readBody(200013118);
  reader.readCoarseBody(200013118);
*/
#endif

#if 0
  QProcess p;
  p.start("sysctl", QStringList() << "kern.version" << "hw.memsize");
  p.waitForFinished();
  QString system_info = p.readAllStandardOutput();
  p.close();

  qDebug() << system_info;
#endif

#if 0
  struct rusage usage;
  getrusage(RUSAGE_SELF, &usage);

  std::cout << "PID: " << getpid() << std::endl;
  std::cout << "memory usage: " << usage.ru_maxrss << std::endl;
  std::cout << "  shared memory: " << usage.ru_ixrss << std::endl;
  std::cout << "  unshared data: " << usage.ru_isrss << std::endl;
  std::cout << "  unshared stack: " << usage.ru_idrss << std::endl;

#endif

#if 0
  qDebug() << getpid();

  qDebug() << ZFlyEmMisc::GetMemoryUsage();
  QProcess p;
  p.start(QString("ps v -p %1").arg(getpid()));
  p.waitForFinished();
  qDebug() << p.readAllStandardOutput();
#endif

#if 0
  //crash test
  int a[1];
  a[1] = 1;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "a031", 7000);

  ZDvidReader reader;
  reader.open(target);
  std::cout << "Master node: " << reader.readMasterNode() << std::endl;

  std::cout << "Node history:" << std::endl;
  std::vector<std::string> nodeList = reader.readMasterList();
  for (std::vector<std::string>::const_iterator iter = nodeList.begin();
       iter != nodeList.end(); ++iter) {
    std::cout << "  " << *iter << std::endl;
  }

  target.setUuid("@FIB19");
  std::cout << "Master node: " << ZDvidReader::ReadMasterNode(target) << std::endl;

  nodeList = reader.ReadMasterList(target);
  for (std::vector<std::string>::const_iterator iter = nodeList.begin();
       iter != nodeList.end(); ++iter) {
    std::cout << "  " << *iter << std::endl;
  }

  ZJsonObject obj = reader.readDefaultDataSetting(ZDvidReader::EReadOption::READ_CURRENT);
  obj.print();

  obj = reader.readDefaultDataSetting(ZDvidReader::READ_TRACE_BACK);
  obj.print();

  target.setUuid("e2f0");
  ZDvidReader reader2;
  reader2.open(target);
  obj = reader2.readDefaultDataSetting(ZDvidReader::EReadOption::READ_CURRENT);
  obj.print();
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  reader.open(target);

  target.loadDvidDataSetting(
        reader.readDefaultDataSetting(ZDvidReader::EReadOption::READ_CURRENT));
  ZDvidReader reader2;
  reader2.open(target);

  ZObject3dScan obj = reader2.readCoarseBody(4833432549);
  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
//  target.set("emdata2.int.janelia.org", "d35f", 7000);
  target.useDefaultDataSetting(true);
  target.setBodyLabelName("bodies_test");
  target.setSynapseName("synapses_test");
  target.setLabelBlockName("label_test");

  ZDvidReader reader;
  reader.open(target);

  reader.getDvidTarget().toDvidDataSetting().print();


  ZJsonObject obj = reader.readDefaultDataSetting(ZDvidReader::EReadOption::READ_CURRENT);

  target = reader.getDvidTarget();
  target.loadDvidDataSetting(obj);
  target.setSynapseName("annot_synapse_010417");
  target.setLabelBlockName("segmentation2");
  target.print();

  ZDvidWriter writer;
  writer.open(target);
  writer.writeDefaultDataSetting();
#endif

#if 0
  ZStack stack;
  Stack *desstack = C_Stack::make(GREY, 32, 32, 1);
  stack.load(GET_TEST_DATA_DIR + "/system/diadem/diadem_e2_proj.tif");

  int width = stack.width();
  int height = stack.height();
  for (int left = 0; left < width - 32; left += 16) {
    for (int top = 0; top < height - 32; top += 16) {
      C_Stack::crop(stack.c_stack(), left, top, 0, 32, 32, 1, desstack);
      ZString output = GET_TEST_DATA_DIR + "/system/substruct/general/diadem_e2_";
      output.appendNumber(left);
      output += "_";
      output.appendNumber(top);
      if (C_Stack::mean(desstack) > 30) {
        C_Stack::write(output, desstack);
      }
    }
  }


#endif

#if 0
  std::cout << sizeof(ZDvidAnnotation) << std::endl;
  std::cout << sizeof(ZJsonObject) << std::endl;
  std::cout << sizeof(ZIntPoint) << std::endl;
  std::cout << sizeof(std::vector<ZIntPoint>) << std::endl;
  std::cout << sizeof(std::vector<std::string>) << std::endl;
#endif

#if 0
  ZDvidTarget target;
//  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.set("emdata2.int.janelia.org", "40c2", 7000);
  ZDvidWriter writer;
  writer.open(target);

  ZJsonObject obj = writer.getDvidReader().readDataMap();

  ZJsonObject labelszObj(obj.value("roi_synapse_labelsz"));

//  labelszObj.setEntry("ROI_LOP_15", "annot_synapse_010417_ROI_LOP_15");
//  labelszObj.setEntry("ROI_LOP_40", "annot_synapse_010417_ROI_LOP_40");
  labelszObj.setEntry("ROI_chiasm_body2", "annot_synapse_ROI_chiasm_body2");

//  ZJsonObject obj;
//  obj.setEntry("roi_synapse_labelsz", labelszObj);

  writer.writeDataMap(obj);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);

  ZDvidReader reader;
  reader.open(target);

  ZJsonObject obj = reader.readDataMap();
  obj.print();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "3b54", 7000);
  target.setBodyLabelName("bodies1104");
  target.setSynapseName("annot_synapse_08162016");
  target.setLabelBlockName("labels1104");
  target.setGrayScaleName("grayscale");

  ZDvidReader reader;
  reader.open(target);

  ZDvidWriter writer;
  writer.open(reader.getDvidTarget());
  writer.writeDefaultDataSetting();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "005a", 7000);
  target.setBodyLabelName("gtpruned-bodies");
  target.setLabelBlockName("groundtruth_pruned");


  ZDvidReader reader;
  reader.open(target);

  ZJsonObject jsonObj = reader.readInfo();

  jsonObj.print();

  reader.updateMaxLabelZoom();

  std::cout << "Level: " << reader.getDvidTarget().getMaxLabelZoom() << std::endl;
#endif

#if 0
  ZFlyEmSplitUploadOptionDialog dlg;
  ZDvidTarget target;
  target.set("http://emdata2.int.janelia.org:8500", "b6bc");
  dlg.setDvidTarget(target);

  std::cout << "No passing" << std::endl;
  dlg.setPassingAnnotation(false);
  dlg.getAnnotation(14634755, 1).print();

  std::cout << "No passing, with comment" << std::endl;
  dlg.setNewComment(true);
  dlg.setComment("split from xxx");
  dlg.getAnnotation(14634755, 1).print();

  std::cout << "Passing, no new comment" << std::endl;
  dlg.setNewComment(false);
  dlg.setPassingAnnotation(true);
  dlg.getAnnotation(14634755, 1).print();

  std::cout << "Passing, with new comment" << std::endl;
  dlg.setNewComment(true);
  dlg.getAnnotation(14634755, 1).print();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");

  ZObject3dScan remain;
  ZObject3dScan subobj;

  obj.chopY(750, &remain, &subobj);
  subobj.save(GET_TEST_DATA_DIR + "/test.sobj");
  remain.save(GET_TEST_DATA_DIR + "/test2.sobj");

#endif

#if 0
  host->testFlyEmProofread();
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/system/slice15_L11.tif");

  stack.crop(ZIntCuboid(ZIntPoint(463, 382, 49), ZIntPoint(669, 547, 67)));

  stack.save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZStressTestOptionDialog *dlg = new ZStressTestOptionDialog(host);
  if (dlg->exec()) {
    switch (dlg->getOption()) {
    case ZStressTestOptionDialog::OPTION_BODY_MERGE:
      std::cout << "Testing body merge" << std::endl;
      break;
    case ZStressTestOptionDialog::OPTION_BODY_SPLIT:
      std::cout << "Testing body split" << std::endl;
      break;
    case ZStressTestOptionDialog::OPTION_CUSTOM:
      std::cout << "Custom testing" << std::endl;
      break;
    case ZStressTestOptionDialog::OPTION_OBJECT_MANAGEMENT:
      std::cout << "Testing multithread object" << std::endl;
      break;
    case ZStressTestOptionDialog::OPTION_BODY_3DVIS:
      std::cout << "Testing 3D body visualization" << std::endl;
      break;
    }
  }
#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  ZObject3dScan *obj = new ZObject3dScan;
  obj->load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  ZStack *stack = ZStackFactory::makeVirtualStack(obj->getBoundBox());
  frame->loadStack(stack);

  frame->document()->addObject(obj);
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  /*
  ZObject3dScan slice = obj->getSlice(343);
  ZStroke2d* stroke = ZFlyEmMisc::MakeSplitSeed(slice, 1);

  frame->document()->addObject(stroke);

  slice = obj->getSlice(344);
  frame->document()->addObject(ZFlyEmMisc::MakeSplitSeed(slice, 2));
  */

  std::vector<ZStroke2d*> seedList = ZFlyEmMisc::MakeSplitSeedList(*obj);
  for (std::vector<ZStroke2d*>::iterator iter = seedList.begin();
       iter != seedList.end(); ++iter) {
    frame->document()->addObject(*iter);
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/benchmark/swc/breadth_first.swc");
  tree.addComment("Test");
  tree.save(GET_TEST_DATA_DIR + "/test.swc");

  std::cout << tree.toString() << std::endl;
#endif

#if 0
  ZProofreadWindow *proofreadWindow = host->startProofread();
  ZDvidTarget target("emdata1.int.janelia.org", "b6bc", 8500);
  target.setLabelBlockName("labels");
  target.setGrayScaleName("grayscale");

  proofreadWindow->getMainMvc()->setDvidTarget(target);
  std::vector<uint64_t> bodyIdArray;
  bodyIdArray.push_back(1);
  bodyIdArray.push_back(7265504);

  proofreadWindow->getMainMvc()->exportNeuronScreenshot(
        bodyIdArray, 512, 512, GET_TEST_DATA_DIR.c_str());
#endif

#if 0
  std::cout << -5 % 2 + 2 << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB25", 7000);
  target.setLabelBlockName("labels1104");

  ZDvidWriter writer;
  writer.open(target);
  writer.getDvidTarget().print();

  writer.syncAnnotationToLabel("bodies1104_todo");
#endif

#if 0
  ZNeuronTracer tracer;
  tracer.test();
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.setGrayScaleName("grayscale");
//  target.setGrayScaleName("google_grayscale");

  ZDvidReader reader;
  reader.open(target);
  ZDvidUrl url(reader.getDvidTarget());
  QList<ZDvidGrayscaleReadTask*> taskList;

  for (int k = 0; k < 10; ++k) {
    ZDvidGrayscaleReadTask *task = new ZDvidGrayscaleReadTask(NULL);
    task->setDvidTarget(target);
    taskList.append(task);
  }



  for (int z = 0; z < 10; ++z) {
    tic();
    for (int k = 0; k < 10; ++k) {
      taskList[k]->setRange(512, 512, 6511, 5473, 9540 + z * 10 + k);
      taskList[k]->setFormat("jpg");
    }
    QtConcurrent::blockingMap(taskList, &ZDvidGrayscaleReadTask::ExecuteTask);
    ptoc();
//    reader.getBufferReader().read(
//          url.getGrayscaleUrl(1024, 1024, 6511, 5473, 9540 + z).c_str(), false);
//    reader.readGrayScale(6511, 5473, 9540 + z, 1024, 1024, 1);
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "43f", 9000);
  target.setSupervisorServer("emdata1.int.janelia.org:9000");
  std::cout << target.toJsonObject().dumpString(2);
#endif

#if 0
  ZSwcTree tree1;
  tree1.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_segments.swc");

  ZSwcTree tree2;
  tree2.load(GET_TEST_DATA_DIR + "/flyem/MB/apl.swc");


  tree2.setType(0);
  ZSwcForest *forest = tree1.toSwcTreeArray();
  int type = 2;
  for (ZSwcForest::const_iterator iter = forest->begin();
       iter != forest->end(); ++iter) {
    const ZSwcTree *tree = *iter;
    std::vector<Swc_Tree_Node*> nodeArray = ZSwc::FindOverlapNode(*tree, tree2);
    for (std::vector<Swc_Tree_Node*>::iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      SwcTreeNode::setType(*iter, type);
    }
    type++;
  }

  tree2.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZSwcTree tree1;
  tree1.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_segments.swc");

  ZSwcTree tree2;
  tree2.load(GET_TEST_DATA_DIR + "/flyem/MB/apl.swc");

  ZSwc::Subtract(&tree2, &tree1);
  tree2.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_bk3.swc");
  tree.forceVirtualRoot();
  Swc_Tree_Node *root = tree.root();
  std::vector<Swc_Tree_Node *> nodeArray = tree.getSwcTreeNodeArray();
  for (std::vector<Swc_Tree_Node *>::iterator iter = nodeArray.begin();
       iter != nodeArray.end(); ++iter) {
    Swc_Tree_Node *tn = *iter;
    if (!SwcTreeNode::isRoot(tn)) {
      if (SwcTreeNode::type(tn) != SwcTreeNode::type(SwcTreeNode::parent(tn))) {
        SwcTreeNode::setParent(tn, root);
      }
    }
  }
  tree.save(GET_TEST_DATA_DIR + "/flyem/MB/apl_bk3.swc");
#endif

#if 0
  ZSwcTree tree2;
  tree2.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_original.swc");

  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_allseg.swc");

  Swc_Tree_Subtract(tree2.data(), tree.data());

  tree2.save(GET_TEST_DATA_DIR + "/flyem/MB/apl_sub.swc");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/flyem/MB/apl_original.swc");

  for (int i = 1; i <= 13; ++i) {
    QString path = QString("%1/%2.swc").
        arg((GET_TEST_DATA_DIR + "/flyem/MB/paper/movie8/cast").c_str()).arg(i);
    std::cout << "Processing " << path.toStdString() << std::endl;
    ZSwcTree tree2;
    tree2.load(path.toStdString());
    std::vector<Swc_Tree_Node*> nodeArray = ZSwc::FindOverlapNode(tree2, tree);
    for (std::vector<Swc_Tree_Node*>::iterator iter = nodeArray.begin();
         iter != nodeArray.end(); ++iter) {
      Swc_Tree_Node *tn = *iter;
      if (SwcTreeNode::radius((tn)) >= 1.0) {
        SwcTreeNode::setRadius(tn, SwcTreeNode::radius((tn)) - 0.1);
      } else {
        SwcTreeNode::setRadius(tn, SwcTreeNode::radius((tn))*0.9);
      }
    }
  }

  tree.save(GET_TEST_DATA_DIR + "/flyem/MB/paper/movie8/cast/14.swc");
#endif

#if 0
  Stack *stack = C_Stack::make(GREY, 3, 3, 3);
  C_Stack::setOne(stack);
//  C_Stack::setPixel(stack, 0, 0, 0, 0, 0);
//  C_Stack::setPixel(stack, 2, 0, 0, 0, 0);

  size_t voxelCount = C_Stack::voxelNumber(stack);
  long int *label = new long int[voxelCount];
  tic();
  Stack *out = C_Stack::Bwdist(stack, NULL, label);
  ptoc();

  for (size_t i = 0; i < voxelCount; ++i) {
    std::cout << i << " " << label[i] << std::endl;
  }

  C_Stack::kill(out);
#endif

#if 0
  ZStack *stack = ZStackFactory::makeOneStack(3, 3, 3);
  stack->setOffset(1, 2, 3);
  stack->setIntValue(3, 4, 5, 0, 0);
  ZIntPoint pt = FlyEm::FindClosestBg(stack, 1, 2, 3);

  std::cout << pt.toString() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  std::cout << reader.getDvidTarget().toJsonObject().dumpString(2) << std::endl;

  ZIntPoint pt = reader.readPosition(4699477314, 9255, 5397, 11259);
  std::cout << pt.toString() << std::endl;

  std::cout << reader.readBodyIdAt(pt) << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setLabelBlockName("labels");
  ZDvidReader reader;
  reader.open(target);
  ZIntPoint pt = reader.readPosition(200002591, 4080, 5648, 7088);

  std::cout << pt.toString() << std::endl;

  std::cout << reader.readBodyIdAt(pt) << std::endl;
#endif

#if 0
  ZStackDoc *doc = new ZStackDoc;
  doc->loadFile(GET_TEST_DATA_DIR + "/system/diadem/diadem_e1.tif");
  doc->test();

  doc->test();
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/misc/larva/3.tif");

  Stack stack1 = C_Stack::sliceView(stack.c_stack(), 0);
  Stack stack2 = C_Stack::sliceView(stack.c_stack(), 1);

  Stack *out = Stack_Sub(&stack1, &stack2, NULL);

  std::vector<int> zArray;
  zArray.push_back(0);
  for (int z = 1; z < stack.depth(); ++z) {
    stack2 = C_Stack::sliceView(stack.c_stack(), z);
    Stack_Sub(&stack2, &stack1, out);
    double diff = C_Stack::sum(out);
    if (diff> 0) {
//      std::cout << z << " " << C_Stack::sum(out) << std::endl;
      zArray.push_back(z);
    }
    stack1 = stack2;
  }

  Stack *newStack = C_Stack::make(GREY, stack.width(), stack.height(), zArray.size());
  for (size_t i = 0; i < zArray.size(); ++i) {
    stack1 = C_Stack::sliceView(stack.c_stack(), zArray[i]);
    C_Stack::copyPlaneValue(newStack, C_Stack::array8(&stack1), i);
  }

  C_Stack::write(GET_TEST_DATA_DIR + "/test.tif", newStack);

//  std::cout << C_Stack::sum(out) << std::endl;

  C_Stack::kill(out);
  C_Stack::kill(newStack);

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  uint64_t bodyId = 1321847559;
  tic();
  ZObject3dScan *obj1 = reader.readBody(bodyId, true, NULL);
  ptoc();
  tic();
  ZObject3dScan *obj2 = reader.readBodyWithPartition(bodyId, NULL);
  ptoc();

  std::cout << "Equal: " << obj1->equalsLiterally(*obj2) << std::endl;
  std::cout << obj1->getMinZ() << " " << obj1->getMaxZ() << " "
            << obj1->getVoxelNumber() << std::endl;
  std::cout << obj2->getMinZ() << " " << obj2->getMaxZ() << " "
            << obj2->getVoxelNumber() << std::endl;

//  Q_ASSERT(obj1->equalsLiterally(*obj2));

  delete obj1;
  delete obj2;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  uint64_t bodyId = 115901398;

  tic();
//  ZObject3dScan *obj2 = reader.readBodyWithPartition(bodyId, NULL);
  ZObject3dScan *obj2 = reader.readBody(bodyId, true, NULL);
  ptoc();

  std::cout << obj2->getMinZ() << " " << obj2->getMaxZ() << " "
            << obj2->getVoxelNumber() << std::endl;
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/split_test2.tif");
  stack.setOffset(0, 0, 0);
  stack.save(GET_TEST_DATA_DIR + "/split_test3.tif");

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "df58", 9000);
//  target.setBodyLabelName("segmentation-labelvol");
//  target.setLabelBlockName("segmentation2");
  ZDvidWriter writer;
  writer.open(target);

  writer.syncData("gtpruned-bodies_2", "groundtruth_pruned_2", "replace=true");
  writer.syncData("gtpruned-bodies_3", "groundtruth_pruned_3", "replace=true");
  writer.syncData("gtpruned-bodies_4", "groundtruth_pruned_4", "replace=true");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "df58", 9000);
  target.setBodyLabelName("gtpruned-bodies");
  target.setLabelBlockName("groundtruth_pruned");
  ZDvidReader reader;
  reader.open(target);
  tic();
  reader.updateMaxLabelZoom();
  ptoc();
  std::cout << reader.getDvidTarget().getMaxLabelZoom() << std::endl;

  ZObject3dScan *obj = reader.readMultiscaleBody(244264, 4, true, NULL);
  obj->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "7abe", 8500);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan newRoiAlpha3 = reader.readRoi("alpha3_roi_0217");
  newRoiAlpha3.printInfo();

  std::cout << newRoiAlpha3.getVoxelNumber() * (32.0*32*32*8*8*8/1000/1000/1000)
            << " um^3" << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "2b6d7", 7000);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan roi = reader.readRoi("seven_column_roi");
  int z = 4600/32;
  {
    ZObject3dScan obj = roi.getSlice(0, z);
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save(GET_FLYEM_DATA_DIR + "/roi/FIB25/distal.obj");
  }

  {
    ZObject3dScan obj = roi.getSlice(z+1, roi.getMaxZ());
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save(GET_FLYEM_DATA_DIR + "/roi/FIB25/proximal.obj");
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "7abe", 8500);

  ZDvidReader reader;
  reader.open(target);


  {
    ZObject3dScan newRoiAlpha1 = reader.readRoi("alpha1_roi_0217");
    newRoiAlpha1.printInfo();
    ZMesh *mesh = ZMeshFactory::MakeMesh(newRoiAlpha1);
    mesh->save(GET_FLYEM_DATA_DIR + "/roi/MB/alpha1.obj");
  }

  {
    ZObject3dScan newRoiAlpha2 = reader.readRoi("alpha2_roi_0217");
    newRoiAlpha2.printInfo();
    ZMesh *mesh = ZMeshFactory::MakeMesh(newRoiAlpha2);
    mesh->save(GET_FLYEM_DATA_DIR + "/roi/MB/alpha2.obj");
  }

  {
    ZObject3dScan newRoiAlpha3 = reader.readRoi("alpha3_roi_0217");
    newRoiAlpha3.printInfo();
    ZMesh *mesh = ZMeshFactory::MakeMesh(newRoiAlpha3);
    mesh->save(GET_FLYEM_DATA_DIR + "/roi/MB/alpha3.obj");
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "7abe", 8500);

  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan roiAlpha = reader.readRoi("kc_alpha_roi");
  roiAlpha.printInfo();

  ZObject3dScan roiAlpha1 = reader.readRoi("alpha1_roi");
  roiAlpha1.printInfo();

  ZObject3dScan roiAlpha2 = reader.readRoi("alpha2_roi");
  roiAlpha2.printInfo();

  ZObject3dScan roiAlpha3 = reader.readRoi("alpha3_roi");
  roiAlpha3.printInfo();

  ZObject3dScan newRoiAlpha1 = reader.readRoi("alpha1_roi_0217");
  newRoiAlpha1.printInfo();

  ZObject3dScan newRoiAlpha2 = reader.readRoi("alpha2_roi_0217");
  newRoiAlpha2.printInfo();

  ZObject3dScan newRoiAlpha3 = reader.readRoi("alpha3_roi_0217");
  newRoiAlpha3.printInfo();

  ZObject3dScan newRoi;
  newRoi.concat(newRoiAlpha3);
  newRoi.concat(newRoiAlpha2);
  newRoi.concat(newRoiAlpha1);
  newRoi.setCanonized(true);
  newRoi.setDsIntv(31, 31, 31);

  newRoi.printInfo();
  std::cout << newRoi.equalsLiterally(roiAlpha) << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "d0b7", 8700);
  target.setBodyLabelName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_vol");
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded");
  target.setGrayScaleName("grayscale");
  target.setMultiscale2dName("tiles");

  ZDvidWriter writer;
  writer.open(target);
  writer.writeDefaultDataSetting();

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "89ec", 8700);
  target.setBodyLabelName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol");
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setGrayScaleName("grayscale");
  target.setMultiscale2dName("tiles");

  ZDvidWriter writer;
  writer.open(target);
  writer.writeDefaultDataSetting();

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "89ec", 8700);
  target.setBodyLabelName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol");
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setGrayScaleName("grayscale");
  target.setMultiscale2dName("tiles");

  ZDvidWriter writer;
  writer.open(target);

  writer.syncAnnotationToLabel(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol_todo");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "1165", 8700);

  ZDvidReader reader;
  reader.open(target);
  ZJsonObject obj = reader.readDefaultDataSetting();

  target.loadDvidDataSetting(obj);

  std::cout << target.toJsonObject().dumpString(2) << std::endl;

  std::cout << "Todo data: " << reader.hasData(target.getTodoListName())
            << std::endl;
  std::cout << "Label block: " << reader.hasData(target.getLabelBlockName())
            << std::endl;
  std::cout << "Labelvol: " << reader.hasData(target.getBodyLabelName())
            << std::endl;
  std::cout << "Body annotation: " << reader.hasData(target.getBodyAnnotationName())
            << std::endl;
  std::cout << "Skeleton: " << reader.hasData(target.getSkeletonName())
            << std::endl;
  std::cout << "Grayscale: " << reader.hasData(target.getGrayScaleName())
            << std::endl;
  std::cout << "Tile: " << reader.hasData(target.getMultiscale2dName())
            << std::endl;

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@CX", 8700);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  reader.getDvidTarget().setSynapseName("synapses_20170307_highprec_reload");
  ZDvidWriter writer;
  writer.open(reader.getDvidTarget());
  writer.writeDefaultDataSetting();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  std::cout << "Updating ..." << std::endl;
  reader.getDvidTarget().setSynapseName("annot_synapse_prod");
  reader.getDvidTarget().toJsonObject().print();

  ZDvidWriter writer;
  reader.getDvidTarget().useDefaultDataSetting(false);
  writer.open(reader.getDvidTarget());
  writer.writeDefaultDataSetting();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "7abe", 8500);

  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan roiAlpha = reader.readRoi("kc_alpha_roi");
  roiAlpha.printInfo();

  ZObject3dScan roiAlpha1 = reader.readRoi("alpha1_roi_0217");
  roiAlpha1.printInfo();

  ZObject3dScan roiAlpha2 = reader.readRoi("alpha2_roi_0217");
  roiAlpha2.printInfo();

  ZObject3dScan roiAlpha3 = reader.readRoi("alpha3_roi_0217");
  roiAlpha3.printInfo();

  std::cout << roiAlpha1.getVoxelNumber() + roiAlpha2.getVoxelNumber() +
               roiAlpha3.getVoxelNumber() << std::endl;

  ZObject3dScan newRoiAlpha3 =
      roiAlpha.getSlice(roiAlpha.getMinZ(), 159);
  newRoiAlpha3.printInfo();

  ZObject3dScan newRoiAlpha2 =
      roiAlpha.getSlice(160, 281);
  newRoiAlpha2.printInfo();

  ZObject3dScan newRoiAlpha1 =
      roiAlpha.getSlice(282, roiAlpha.getMaxZ());
  newRoiAlpha1.printInfo();

  std::cout << newRoiAlpha1.getVoxelNumber() + newRoiAlpha2.getVoxelNumber() +
               newRoiAlpha3.getVoxelNumber() << std::endl;

  ZObject3dScan newRoi;
  newRoi.concat(newRoiAlpha3);
  newRoi.concat(newRoiAlpha2);
  newRoi.concat(newRoiAlpha1);
  newRoi.setCanonized(true);
  newRoi.setDsIntv(31, 31, 31);

  newRoi.printInfo();
  std::cout << newRoi.equalsLiterally(roiAlpha) << std::endl;

  ZJsonArray array1 =
      ZJsonFactory::MakeJsonArray(newRoiAlpha1, ZJsonFactory::OBJECT_SPARSE);
  array1.dump(GET_TEST_DATA_DIR + "/alpha1.json");

#if 0
  ZDvidWriter writer;
  writer.open(target);
  newRoiAlpha1.printInfo();
  writer.writeRoi(newRoiAlpha1, "alpha1_roi_0217");
  writer.writeRoi(newRoiAlpha2, "alpha2_roi_0217");
  writer.writeRoi(newRoiAlpha3, "alpha3_roi_0217");
#endif

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "d0b7", 8700);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  int missing = reader.checkProofreadingData();
  if (missing > 0) {
    std::cout << missing << " data are missing for proofreading." << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "6a1b", 8700);
//  target.set("emdata2.int.janelia.org", "e2f0", 7000);
  target.setGrayScaleName("grayscale_lcn");
  ZDvidReader reader;
  reader.open(target);
  ZIntPoint blockIndex(53, 13, 54);
  int blockNumber = 4;

  ZDvidInfo grayscaleInfo = reader.readGrayScaleInfo();
  tic();
  std::vector<ZStack*> stackArray = reader.readGrayScaleBlock(
        blockIndex, grayscaleInfo, blockNumber);
  ptoc();
#endif

#if 0
  ZDvidTarget target;

  target.set("emdata1.int.janelia.org", "89ec", 8700);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  ZDvidWriter writer;
  writer.open(reader.getDvidTarget());
  writer.createSplitLabel();
//  writer.createKeyvalue("neutu_config");
  writer.createKeyvalue(reader.getDvidTarget().getBodyInfoName());
//  writer.createKeyvalue(reader.getDvidTarget().get);

  int missing = reader.checkProofreadingData();
  if (missing > 0) {
    std::cout << missing << " data are missing for proofreading." << std::endl;
  } else {
    std::cout << "The node looks good for proofreading." << std::endl;
  }
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "6a1b", 8700);
//  target.set("emdata2.int.janelia.org", "e2f0", 7000);
  target.setGrayScaleName("grayscale_lcn");
#endif

#if 0
  target.set("emdata2.int.janelia.org", "e2f0", 7000);
  target.setSynapseName("annot_synapse_010417");
  target.setSynapseLabelszName("annot_synapse_010417_ROI_LOP_15");
  ZDvidReader reader;
  reader.open(target);

  int count = reader.readSynapseLabelszBody(80, ZDvid::ELabelIndexType::INDEX_PRE_SYN);
  std::cout << "Pre count: " << count << std::endl;

  count = reader.readSynapseLabelszBody(80, ZDvid::ELabelIndexType::INDEX_POST_SYN);
  std::cout << "Post count: " << count << std::endl;

  count = reader.readSynapseLabelszBody(80, ZDvid::ELabelIndexType::INDEX_ALL_SYN);
  std::cout << "All count: " << count << std::endl;
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "@FIB19", 7000);
  target.useDefaultDataSetting(true);


  ZDvidReader reader;
  reader.open(target);

  reader.getDvidTarget().print();

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();

  ZIntCuboid box = dvidInfo.getDataRange();

  ZStack *stack = reader.readGrayScale(
        box.getFirstCorner().getX(), 6100, box.getFirstCorner().getZ(),
        box.getWidth(), 1, box.getDepth());

  stack->reshape(stack->width(), stack->depth(), 1);
  stack->setOffset(stack->getOffset().getX(), stack->getOffset().getZ(),
                   stack->getOffset().getY());
  stack->printInfo();

//  stack->downsampleMin(7, 7, 7);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");

  delete stack;
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/test.tif");

  stack.downsampleMin(7, 7, 7);
  stack.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/slice_ds8.tif");

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/90_shift.swc");
    tree.rescale(0.125, 0.125, 0.125);
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/90_shift_ds8.swc");
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/139_shift.swc");
    tree.rescale(0.125, 0.125, 0.125);
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/139_shift_ds8.swc");
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/9829583889_shift.swc");
    tree.rescale(0.125, 0.125, 0.125);
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/9829583889_shift_ds8.swc");
  }
#endif

#if 0
  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/90.swc");

    ZSwcTree::DepthFirstIterator iter(&tree);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      std::swap(tn->node.y, tn->node.z);
    }
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/90_shift.swc");
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/139.swc");

    ZSwcTree::DepthFirstIterator iter(&tree);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      std::swap(tn->node.y, tn->node.z);
    }
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/139_shift.swc");
  }

  {
    ZSwcTree tree;
    tree.load(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/9829583889.swc");

    ZSwcTree::DepthFirstIterator iter(&tree);
    while (iter.hasNext()) {
      Swc_Tree_Node *tn = iter.next();
      std::swap(tn->node.y, tn->node.z);
    }
    tree.save(GET_TEST_DATA_DIR + "/flyem/FIB/FIB19/9829583889_shift.swc");
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6", 8700);
  target.setGrayScaleName("grayscalejpeg");
  ZStack *stack = reader.readGrayScaleLowtis(2610, 2354, 3197, 512, 512, 1);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6", 8700);
  target.setGrayScaleName("grayscale");

  ZDvidReader reader;
  reader.open(target);

  reader.updateMaxGrayscaleZoom();

  std::cout << reader.getDvidTarget().getMaxGrayscaleZoom() << std::endl;

  ZStack* stack = reader.readGrayScale(2610, 2354, 3197, 512, 512, 1, 2);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete stack;

#endif

#if 0
  ZDvidGraySlice slice;

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6", 8700);
  target.setGrayScaleName("grayscale");

  ZDvidReader reader;
  reader.open(target);

  reader.updateMaxGrayscaleZoom();

  slice.setDvidTarget(reader.getDvidTarget());

  ZStackViewParam param;
  param.setViewPort(2610, 2354, 2610 + 1024, 2354 + 1024);
  param.setZ(3197);

  slice.update(param);
  slice.saveImage(GET_TEST_DATA_DIR + "/test.tif");
  slice.savePixmap(GET_TEST_DATA_DIR + "/test2.tif");

#endif

#if 0
  ZStack *stack1 = ZStackFactory::makeZeroStack(GREY, 3, 3, 1, 1);
  ZStack *stack2 = ZStackFactory::makeOneStack(3, 3, 1, 1);
  C_Stack::setConstant(stack2->data(), 255);

  ZStack *out = ZStackProcessor::Intepolate(stack1, stack2, 0.5);

  C_Stack::printValue(out->c_stack());

#endif

#if 0
  ZStack *stack1 = ZStackFactory::makeZeroStack(GREY, 7, 5, 1, 1);
  ZStack *stack2 = ZStackFactory::makeOneStack(7, 5, 1, 1);
  C_Stack::setConstant(stack2->data(), 255);

  ZStack *out = ZStackProcessor::IntepolateFovia(stack1, stack2, 3, 3, 4, 1, 6, 1);

  C_Stack::printValue(out->c_stack());
//  out->save(GET_TEST_DATA_DIR + "/test.tif");

#endif

#if 0
  ZStack *stack1 = ZStackFactory::makeZeroStack(GREY, 512, 256, 1, 1);
  ZStack *stack2 = ZStackFactory::makeOneStack(512, 256, 1, 1);
  C_Stack::setConstant(stack2->data(), 255);

  ZStack *out = ZStackProcessor::IntepolateFovia(stack1, stack2, 128, 128, 2, 0, 2, 1);

//  C_Stack::printValue(out->c_stack());
  out->save(GET_TEST_DATA_DIR + "/test.tif");

#endif

#if 0
#  if defined(_ENABLE_LOWTIS_)
  lowtis::DVIDGrayblkConfig lowtisConfigGray;

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6c", 8700);
  target.setGrayScaleName("grayscalejpeg");

  int cx = 256;
  int cy = 256;
  bool centerCut = false;

  lowtisConfigGray.username = NeuTube::GetCurrentUserName();
  lowtisConfigGray.dvid_server = target.getAddressWithPort();
  lowtisConfigGray.dvid_uuid = target.getUuid();
  lowtisConfigGray.datatypename = target.getGrayScaleName();
  lowtisConfigGray.centercut = std::tuple<int, int>(cx, cy);

  ZSharedPointer<lowtis::ImageService> lowtisServiceGray =
      ZSharedPointer<lowtis::ImageService>(
        new lowtis::ImageService(lowtisConfigGray));
  int width = 250;
  int height = 256;
  std::vector<int> offset(3);
  int x0 = offset[0] = 2960;
  int y0 = offset[1] = 3319;
  int z0 = offset[2] = 3433;
  int zoom = 1;
  int scale = 1;

  ZIntCuboid box;
  box.setFirstCorner(x0 / scale, y0 / scale, z0 / scale);
  box.setWidth(width);
  box.setHeight(height);
  box.setDepth(1);


  ZStack *stack = new ZStack(GREY, box, 1);

  for (int i = 0; i < 100; ++i) {
    offset[2] += 1;
    lowtisServiceGray->retrieve_image(
          width, height, offset, (char*) stack->array8(), zoom, centerCut);
  }

#  endif
#endif

#if 0
  std::cout << misc::GetZoomScale(1) << std::endl;
  std::cout << misc::GetZoomScale(10) << std::endl;

  tic();
  int n = 100000000;
  for (int i = n; i >= 0; --i) {
    int x = i * i;
  }
  ptoc();

  tic();
  int i = 100000000;
  while (i--) {
    int x = i * i;
  }
  ptoc();

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "1cea", 8800);
  target.setBodyLabelName("labels");
  target.setLabelBlockName("labels");

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan *obj = reader.readBody(51000860214, true, NULL);
  std::cout << "Voxel count: " << obj->getVoxelNumber() << std::endl;

  obj->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "@CX", 8700);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan roi = reader.readRoi("CXPB-ROI-2");
  ZIntPoint blockSize = reader.readRoiBlockSize("CXPB-ROI-2");

  size_t v = 0;

  ZObject3dScan::ConstVoxelIterator iter(&roi);
  while (iter.hasNext()) {
    ZIntPoint pt = iter.next();
    ZIntPoint corner = pt * blockSize;
    uint64_t label = reader.readBodyIdAt(corner);
    if (label == 0) {
      ZIntPoint center = corner + blockSize / 2;
      label = reader.readBodyIdAt(center);
    }
    if (label > 0) {
      ++v;
    }
  }

  std::cout << "Volume: " << double(v) *
//            << (double) roi.getVoxelNumber() *
               blockSize.getX() * blockSize.getY() * blockSize.getZ()
               * 0.008 * 0.008 *0.008 << std::endl;
#endif

#if 0
  std::string source =
      ZStackObjectSourceFactory::MakeFlyEmBodySource(123, 1, FlyEM::BODY_FULL);
  std::cout << source << std::endl;

  source =
      ZStackObjectSourceFactory::MakeFlyEmBodySource(123, 0, FlyEM::BODY_FULL);
  std::cout << source << std::endl;

  source =
      ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(source);
  std::cout << source << std::endl;

  source = ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(
        "#.FlyEmBody#123_1#.full");
  std::cout << source << std::endl;

  source = ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(
        "#.FlyEmBody.Agree#123_1#.full");
  std::cout << source << std::endl;

  source = ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(
        "#.FlyEmBody.Split#123_1#.full");
  std::cout << source << std::endl;

  source = ZStackObjectSourceFactory::ExtractBodyStrFromFlyEmBodySource(
        "#.FlyEmBody.Merge#123_1#.full");
  std::cout << source << std::endl;
#endif

#if 0
  QRect rect(0, 0, 5, 6);
  qDebug() << rect.width() << rect.height();
  qDebug() << rect.topLeft();
  qDebug() << rect.bottomRight();

  QRectF rect2(0, 0, 5.5, 6);
  qDebug() << rect2.width() << rect2.height();
  qDebug() << rect2.topLeft();
  qDebug() << rect2.bottomRight();

  //Float conversion preserves size instead of corners
  QRectF rect3(rect);
  qDebug() << rect3.width() << rect3.height();
  qDebug() << rect3.topLeft();
  qDebug() << rect3.bottomRight();

#endif

#if 0
  ZViewProj viewProj;

  viewProj.setCanvasRect(QRect(0, 0, 100, 200));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));
  viewProj.setZoom(2);

  QPoint pt = viewProj.mapPointBack(QPointF(10, 10));
  qDebug() << pt;

  viewProj.setZoomWithFixedPoint(1, QPoint(10, 20), QPointF(50, 80));
  QPointF pt2 = viewProj.mapPoint(QPoint(10, 20));
  pt = viewProj.mapPointBack(QPointF(50, 80));
  qDebug() << pt2;
  qDebug() << pt;
#endif

#if 0

  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/block.tif");

  std::vector<ZSwcTree*> treeArray = ZSwcFactory::CreateLevelSurfaceSwc(
        stack, 1, ZIntPoint(1, 1, 1));

  ZSwcTree *tree = new ZSwcTree;
  int index = 1;
  for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
       iter != treeArray.end(); ++iter) {
    ZSwcTree *subtree = *iter;
    if (subtree != NULL) {
      subtree->setType(index);
      tree->merge(subtree, true);
    }
    ++index;
  }

  tree->save(GET_TEST_DATA_DIR + "/test.swc");

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/rice_label.tif");
  ZObject3dScanArray *objArray =
      ZObject3dFactory::MakeObject3dScanArray(stack, NeuTube::Z_AXIS, true, NULL);

  ZStack *stack2 =
      ZStackFactory::makeZeroStack(GREY8, stack.getBoundBox());

  for (ZObject3dScanArray::const_iterator iter = objArray->begin();
       iter !=  objArray->end(); ++iter) {
    const ZObject3dScan &obj = *iter;
    obj.addStack(stack2, obj.getLabel());
//    obj.addStack(stack2, obj.getLabel());
  }

  stack2->save(GET_TEST_DATA_DIR + "/test.tif");
  std::cout << stack.equals(*stack2) << std::endl;
  delete stack2;
  delete objArray;
#endif

#if 0
  ZObject3dScan obj1;
  ZObject3dScan obj2;

//  obj1.addSegment(0, 0, 0, 2);
//  obj2.addSegment(0, 0, 1, 3);

  obj1.load(GET_TEST_DATA_DIR + "/benchmark/29.sobj");
  obj2 = obj1.getSlice(100, 500);

  std::vector<ZSwcTree*> treeArray =
      ZSwcFactory::CreateDiffSurfaceSwc(obj1, obj2);

//  ZSwcTree *tree = new ZSwcTree;

  int index = 1;
  for (std::vector<ZSwcTree*>::iterator iter = treeArray.begin();
       iter != treeArray.end(); ++iter) {
    ZSwcTree *subtree = *iter;
    if (subtree != NULL) {
      subtree->setType(index);
      QString output = QString("%1/test%2.swc").
          arg(GET_TEST_DATA_DIR.c_str()).arg(index);
      subtree->save(output.toStdString());
//      tree->merge(subtree, true);
    }
    ++index;
  }
//  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZComboEditDialog *dlg = new ZComboEditDialog(host);

  std::vector<std::string> strList;
  strList.push_back("test1");
  strList.push_back("test2");
  dlg->setStringList(strList);
  dlg->exec();
#endif

#if 0
  ZComboEditDialog *dlg = new ZComboEditDialog(host);

  std::vector<std::string> strList;
  strList.push_back("test1");
  strList.push_back("test2");
  dlg->setStringList(strList);
  dlg->exec();
#endif

#if 0
  ZFlyEmBodyComparisonDialog *dlg = new ZFlyEmBodyComparisonDialog(host);
  dlg->exec();

  std::cout << dlg->getSegmentation() << std::endl;
#endif

#if 0
#ifdef _ENABLE_LIBDVIDCPP_
  ZDvidNeuronTracer tracer;

  ZDvidTarget target;
  target.set("localhost", "ae53", 8000);
  target.setGrayScaleName("grayscale");

  ZDvidReader reader;
  reader.open(target);

//  ZStack *stack = reader.readGrayScale(484, 1060, 60, 128, 128, 64);
//  stack->save(GET_TEST_DATA_DIR + "/test.tif");

//  delete stack;

  tracer.setDvidTarget(target);
  int x = 478;
  int y = 1113;
  int z = 60;
  ZStack *stack = tracer.readStack(x, y, z, 32);
//  stack->save(GET_TEST_DATA_DIR + "/test.tif");

//  delete stack;

  Local_Neuroseg *locseg = New_Local_Neuroseg();
  double r = 2.0;
  Set_Local_Neuroseg(
        locseg,  r + r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);

  double score = tracer.optimize(locseg);

  std::cout << score << std::endl;

  Print_Local_Neuroseg(locseg);

  ZLocalNeuroseg *segObj = new ZLocalNeuroseg(locseg);
  segObj->setColor(Qt::red);

  tracer.trace(x, y, z, 2.0);
  ZSwcTree *tree = tracer.getResult();

  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->loadStack(stack);
  frame->document()->addObject(segObj);
  frame->document()->addObject(tree);
  host->addStackFrame(frame);
  host->presentStackFrame(frame);
#endif

#endif

#if 0
  ZDvidTarget target;
//  target.set("emdata2.int.janelia.org", "e2f0", 7000);
//  target.setLabelBlockName("segmentation2");

  target.set("emdata1.int.janelia.org", "d693", 8700);
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(157976);

  ZIntCuboid box = spStack->getBoundBox();

  int minZ = box.getFirstCorner().getZ();
  int maxZ = box.getLastCorner().getZ();
  int x0 = box.getFirstCorner().getX();
  int y0 = box.getFirstCorner().getY();

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_DEFAULT);

  minZ = 1500;
  maxZ = 2500;
  x0 = 3700;
  y0 = 2200;
  int width = 800;
  int height = 800;
//  box.getWidth(), box.getHeight()
  for (int z = minZ; z <= maxZ; ++z) {
    ZStack *stack = spStack->getSlice(z, x0, y0, width, height);
    if (stack != NULL) {
      std::cout << z << "/" << maxZ << std::endl;
      ZString numStr;
      numStr.appendNumber(z, 5);

      writer.write(GET_TEST_DATA_DIR + "/test/series/" + numStr + ".tif", stack);
//      stack->save(GET_TEST_DATA_DIR + "/test/series/" + numStr + ".tif");
      delete stack;
    }
  }
//  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif


#if 0
  ZFileList fileList;
  fileList.load(
        "/Users/zhaot/Work/neutube/neurolabi/data/test/goodseg", "tif");
  for (int i = 0; i < fileList.size(); ++i) {
    const char *filePath = fileList.getFilePath(i);
    Stack *stack = C_Stack::readSc(filePath);
    C_Stack::write(std::string(filePath) + ".tif", stack);
    C_Stack::kill(stack);
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "d693", 8700);
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyIdArray;
//  bodyIdArray << 4595099 << 4616071 << 4902587 << 4951619 << 4954921 << 5256538
//              << 5286415 << 5615875 << 5616186 << 5950178;
  bodyIdArray << 4283540 << 4595099 << 4616071 << 4902587 << 4945695 << 4951619
              << 4954921 << 5256538 << 5280102 << 5286415 << 5305549 << 5305950
              << 5615875 << 5616186 << 5950178;

  ZObject3dScanArray objArray;
  objArray.resize(bodyIdArray.size());
  int index = 0;
  foreach (uint64_t bodyId, bodyIdArray) {
    ZObject3dScan &obj = objArray[index];
    reader.readBody(bodyId, false, &obj);
    index++;
  }

  ZIntCuboid box = objArray.getBoundBox();
  int dsIntv = misc::getIsoDsIntvFor3DVolume(box, NeuTube::ONEGIGA, false);
  std::cout << "Downsampling:" << dsIntv << std::endl;
  objArray.downsample(dsIntv, dsIntv, dsIntv);
  ZStack *stack = objArray.toLabelField();

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);
  writer.write(GET_TEST_DATA_DIR + "/test/d693/merge2.tif", stack);

  delete stack;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "d693", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  /*
  ZDvidSparseStack *spStack = reader.readDvidSparseStack(
        401430, ZIntCuboid(ZIntPoint(2807, 202, 3240),
                           ZIntPoint(3214, 605, 3540)));
                           */
  ZDvidSparseStack *spStack = reader.readDvidSparseStack(401430);
  spStack->shakeOff();

  ZStack *stack = spStack->makeIsoDsStack(NeuTube::ONEGIGA + NeuTube::ONEGIGA / 2);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete spStack;
  delete stack;

#endif

#if 0
  std::string dataDir = "/Users/zhaot/Work/neutube/neurolabi/data/system/split/data8";
  ZJsonObject obj;
  obj.load(dataDir + "/" + "data.json");
  ZStack *stack = ZFlyEmMisc::GenerateExampleStack(obj);
  stack->setOffset(0, 0, 0);
  stack->save(dataDir + "/test.tif");

  delete stack;

#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/benchmark/binary/3d/diadem_e1.tif");

  size_t voxelCount = stack.getVoxelNumber();
  for (size_t i = 0; i < voxelCount; ++i) {
    if (stack.value8(i) == 1) {
      stack.setValue(i, 0, 8);
    }
  }

  stack.save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  std::string dataDir = "/Users/zhaot/Work/neutube/neurolabi/data/system/split/data7";
  ZStack stack;
  stack.load(dataDir + "/test.tif");
  stack.setOffset(0, 0, 0);
  stack.save(dataDir + "/test.tif");

  ZStack stack2;
  stack2.load(dataDir + "/seg.tif");
  stack2.setOffset(0, 0, 0);
  stack2.save(dataDir + "/seg.tif");
#endif

#if 0
  ZDvidTarget target;
//  target.set("emdata2.int.janelia.org", "e2f0", 7000);
//  target.setLabelBlockName("segmentation2");

  target.set("emdata1.int.janelia.org", "d693", 8700);
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyIdArray;
  bodyIdArray << 159860 << 156067;

//  bodyIdArray << 401430 << 157909 << 157976
//              << 180022;
//  bodyIdArray << 468892 << 156107 << 11334 << 181790 << 181594 << 112963
//              << 516025;
//  bodyIdArray << 87679;
//  bodyIdArray << 111928 << 184586;

//  bodyIdArray << 1887367;

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);

  foreach (uint64_t bodyId, bodyIdArray) {

    ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(bodyId);

    //  ZIntCuboid box = spStack->getBoundBox();

    ZStack *stack = spStack->makeIsoDsStack(NeuTube::ONEGIGA);

//    ZStack *stack = spStack->makeStack(ZIntCuboid());
//    ZStack *stack = spStack->getStack();

    ZString numStr;
    numStr.appendNumber(bodyId);
    writer.write(GET_TEST_DATA_DIR + "/test/d693/" + numStr + ".tif", stack);

//    delete stack;
    delete spStack;
  }

#endif

#if 0
  ZIntCuboid box;
  box.setWidth(1024);
  box.setHeight(1024);
  box.setDepth(1024);
  int dsIntv = misc::getIsoDsIntvFor3DVolume(box, NeuTube::ONEGIGA);

  std::cout << "Ds intv:" << dsIntv << std::endl;

  box.setWidth(box.getWidth() / (dsIntv + 1));
  box.setHeight(box.getHeight() / (dsIntv + 1));
  box.setDepth(box.getDepth() / (dsIntv + 1));

  std::cout << "Volume:" << box.getVolume() << std::endl;

#endif

#if 0
  QUrl url("mock://emdata4.int.janelia.org:8900?"
           "uuid=52a1&segmentation=segmentation&admintoken=xxx");
  qDebug() << url.scheme();
  qDebug() << url.host();
  qDebug() << url.port();

  QUrlQuery query(url);
  qDebug() << query.queryItemValue("segmentation", QUrl::FullyDecoded);
  qDebug() << query.queryItemValue("admintoken", QUrl::FullyDecoded);

#endif

#if 0
//  QUrl url("dvid://emdata1.int.janelia.org:8000/api/node/uuid/dataname?x1=1&y1=1&z1=1");
  QUrl url("/test/test");
  qDebug() << url.scheme();
  qDebug() << url.path();
//  qDebug() << url.allQueryItemValues("x1");
  qDebug() << url.host();
  qDebug() << url.port();
  qDebug() << url.toLocalFile();

//  qDebug() << ZDvidUrl::GetEndPoint(url.path().toStdString());
#endif

#if 0
  QString path =
      "http://emdata1.int.janelia.org:8100/api/node/ef20/dataname?x1=1&y1=1&z1=1";


//  ZDvidTarget target = dvid::MakeTargetFromUrl(path);

//  qDebug() << target.toJsonObject().dumpString(2);
#endif

#if 0
  ZDvidResultService service;
  QByteArray data = ZDvidResultService::ReadData(
        "http://localhost:8000/api/node/4d3e3c206a314236b73800bc23a1f2d7/result/key/54b0c58c7ce9f2a8b551351102ee0938");
  qDebug() << data;

  ZDvidResultService::ReadData(
          "http://localhost:8000/api/node/4d3e3c206a314236b73800bc23a1f2d7/result/key/54b0c58c7ce9f2a8b551351102ee0938");
#endif

#if 0
  QByteArray data("this is a new test");
  ZDvidResultService::WriteData(
        "http://localhost:8000/api/node/4d3e3c206a314236b73800bc23a1f2d7/result/key/test",
        data);

  QByteArray data2("this is a new test2");
  ZDvidResultService::WriteData(
        "http://localhost:8000/api/node/4d3e3c206a314236b73800bc23a1f2d7/result/key/test",
        data2);
#endif

#if 0
  ZJsonObject obj;
  obj.addEntry("signal", GET_BENCHMARK_DIR + "/em_slice.tif");

  ZJsonArray seedArrayJson;
  obj.addEntry("seeds", seedArrayJson);

  {
    ZStroke2d stroke;
    stroke.setLabel(1);
    stroke.append(2901, 4085);
    stroke.setWidth(6);
    stroke.setZ(3201);

    ZJsonObject seedJson;
    seedJson.addEntry("type", "ZStroke2d");
    ZJsonObject strokeJson = stroke.toJsonObject();
    seedJson.addEntry("data", strokeJson);

    seedArrayJson.append(seedJson);
  }

  {
    ZStroke2d stroke;
    stroke.setLabel(2);
    stroke.append(3034, 4093);
    stroke.setWidth(6);
    stroke.setZ(3201);

    ZJsonObject seedJson;
    seedJson.addEntry("type", "ZStroke2d");
    ZJsonObject strokeJson = stroke.toJsonObject();
    seedJson.addEntry("data", strokeJson);

    seedArrayJson.append(seedJson);
  }

  obj.dump(GET_TEST_DATA_DIR + "/test.json");
#endif

#if 0
  ZStackReader reader;
  ZStack *stack = reader.read(
        "dvid://emdata1.int.janelia.org:8700/186a/"
        "grayscale?x0=2606&y0=2248&z0=3200&width=512&height=512&depth=1");
  ZStackWriter writer;
  writer.write(GET_TEST_DATA_DIR + "/test.tif", stack);
#endif


#if 0
  ZStroke2d stroke1;
  stroke1.setZ(3200);
  stroke1.append(2681, 2593);
  stroke1.append(2688, 2677);
  ZObject3dScan obj1 = ZObject3dFactory::MakeObject3dScan(stroke1);
  obj1.save(GET_TEST_DATA_DIR + "/system/command_test/body_split/seed1.sobj");


  ZStroke2d stroke2;
  stroke2.setZ(3200);
  stroke2.append(2977, 2399);
  stroke2.append(3015, 2462);
  ZObject3dScan obj2 = ZObject3dFactory::MakeObject3dScan(stroke2);
  obj2.save(GET_TEST_DATA_DIR + "/system/command_test/body_split/seed2.sobj");

#endif

#if 0
  QUrl url("file:///Users/dir/?prefix=test");
  qDebug() << url.scheme();
  qDebug() << url.path();
  qDebug() << url.queryItems();
#endif

#if 0
  std::string dirPath =
      GET_TEST_DATA_DIR +
      "/_misc/MHb Z-Stacks for neuTube Analysis/ZSeries-01102017-1158-024";
  QDir dir(dirPath.c_str());
  QFileInfoList fileList =
      dir.entryInfoList(QStringList() << "ZSeries-01102017-1158-024_Cycle00001_Ch*.ome.tif");

  foreach (const QFileInfo &fileInfo, fileList) {
    qDebug() << fileInfo.absoluteFilePath();
  }
#endif

#if 0
  ZStackReader reader;
  ZStack *stack =
      reader.read("file://" + GET_BENCHMARK_DIR + "/series?prefix=image&suffix=.tif");
  stack->printInfo();
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("MB_Test");

  reader->getDvidTarget().print();

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("MB_Test");

  writer->getDvidTarget().print();
//  reader->readSwc(1);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("MB_Test");
  ZSwcTree *tree = reader->readSwc(16827408);
  tree->save(GET_TEST_DATA_DIR + "/+test.swc");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("MB_Test");
  writer->changeLabel(ZIntCuboid(4186, 5377, 7367, 4248, 5422, 7391),
                      16433436, 164334367);
#endif

#if 0
  ZIntCuboid box(1252, 3961, 3396, 1979, 4344, 3676);
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "1897", 8700);
  target.useDefaultDataSetting(true);
  ZDvidWriter writer;
  writer.open(target);
  writer.getDvidTarget().print();

  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo = reader.readLabelInfo();
  ZIntCuboid blockCuboid;
  blockCuboid.setFirstCorner(dvidInfo.getBlockIndex(box.getFirstCorner()));
  blockCuboid.setLastCorner(dvidInfo.getBlockIndex(box.getLastCorner()));

  for (int z = blockCuboid.getFirstCorner().getZ();
       z <= blockCuboid.getLastCorner().getZ(); ++z) {
    for (int y = blockCuboid.getFirstCorner().getY();
          y<= blockCuboid.getLastCorner().getY(); ++y) {
      for (int x = blockCuboid.getFirstCorner().getX();
           x <= blockCuboid.getLastCorner().getX(); ++x) {
        writer.refreshLabel(dvidInfo.getBlockBox(x, y, z), 2596484);
      }
    }
  }
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/swc/multi_branch.swc");

  std::vector<std::vector<double> > allDirection =
      tree.computeAllTerminalDirection();
  ZDoubleVector::Print(allDirection);
#endif

#if 0
  ZWeightedPointArray ptArray;
  ptArray.append(ZPoint(0, 0, 0), 1.0);
  ptArray.append(ZPoint(1, 0, 0), 1.0);
  ptArray.append(ZPoint(2, 2, 0), 2.0);
  ptArray.append(ZPoint(5, 2, 0), 3.0);

  ZPoint direction = SwcTreeNode::weightedDirection(ptArray);

  direction.print();
  std::cout << direction.length() << std::endl;
#endif

#if 0
  const char *data = "this is a test";

  ZDvidEndPoint endPoint;
  qDebug() << endPoint.GetResultEndPoint(QByteArray(data));

  ZDvidWriter writer;
  ZDvidTarget target;
  target.setNullLabelBlockName();
  target.set("localhost", "4d3e", 8000);
  if (writer.open(target)) {
    writer.writeServiceResult(QByteArray(data));
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("localhost", "4d3e", 8000);

  if (reader.open(target)) {
    QByteArray buffer = reader.readServiceResult(
          "54b0c58c7ce9f2a8b551351102ee0938");
    qDebug() << buffer;
  }
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  QByteArray byteArray = obj.toDvidPayload();


  ZDvidWriter writer;
  ZDvidTarget target;
  target.setNullLabelBlockName();
  target.set("localhost", "4d3e", 8000);
  if (writer.open(target)) {
    writer.writeServiceResult(byteArray);
  }
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("localhost", "4d3e", 8000);
  target.setLabelBlockName("*");

  if (writer.open(target)) {
    writer.createKeyvalue("task_split");
    writer.createKeyvalue("result_split");
  }
#endif

#if 0
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("localhost", "4d3e", 8000);
  target.setLabelBlockName("*");

  ZJsonObject task;
  task.load(GET_BENCHMARK_DIR + "/split.json");

  if (writer.open(target)) {
    writer.writeServiceTask("split", task);
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("localhost", "4d3e", 8000);

  if (reader.open(target)) {
    QByteArray buffer = reader.readServiceResult(
          "79674142a7bf5bf37b217dedcb9437e7");
//    qDebug() << buffer;
    ZObject3dScan obj;
    obj.importDvidObjectBuffer(buffer.data(), buffer.size());
    obj.print();
  }
#endif

#if 0
  for (int i = 0; i < 1000; ++i) {
//    Mc_Stack *stack = Read_Mc_Stack((GET_BENCHMARK_DIR + "/../_system/emstack2.tif").c_str(), 0);
//    Mc_Stack *stack = C_Stack::read(GET_BENCHMARK_DIR + "/em_slice.tif");
//    Mc_Stack *stack = C_Stack::make(GREY, 512, 512, 1, 1);
//    C_Stack::kill(stack);

    ZStack stack;
    stack.load(GET_BENCHMARK_DIR + "/em_slice.tif");
    ZStackWatershedContainer container(&stack);
    ZObject3dScan obj;
    obj.addSegment(3201, 4102, 2900, 2905);
    obj.setLabel(1);

    ZObject3dScan obj2;
    obj2.addSegment(3201, 4114, 3010, 3015);
    obj2.setLabel(2);

    container.addSeed(obj);
    container.addSeed(obj2);

    ZStack *result = container.run();

//    result->save(GET_TEST_DATA_DIR + "/test.tif");

    delete result;
  }

  while (1) {}
#endif

#if 0
  ZStack stack;
  stack.load(GET_BENCHMARK_DIR + "/em_slice.tif");
  stack.setDsIntv(1, 1, 0);

  ZStackWatershedContainer container(&stack);
  ZObject3dScan obj;
  obj.addSegment(3201, 4102 * 2, 2900 * 2, 2905 * 2);
  obj.setLabel(1);

  ZObject3dScan obj2;
  obj2.addSegment(3201, 4114 * 2, 3010 * 2, 3015 * 2);
  obj2.setLabel(2);

  ZObject3dScan obj3;
  obj3.setLabel(3);
  obj3.addSegment(3201, 4185 * 2, 2904 * 2, 3000 * 2);
//  container.setRange(2890, 4095, 3201, 3100, 4173, 3201);
  container.addSeed(obj);
  container.addSeed(obj2);
  container.addSeed(obj3);

  ZStroke2d stroke;
  stroke.append(3089 * 2, 4173 * 2);
  stroke.setZ(3201);
  stroke.setWidth(5);
  stroke.setLabel(3);

  container.addSeed(stroke);

  ZObject3d obj4;
  obj4.append(2938 * 2, 4143 * 2, 3201);
  obj4.append(2939 * 2, 4144 * 2, 3201);
  obj4.append(2940 * 2, 4146 * 2, 3201);
  obj4.setLabel(5);
  container.addSeed(obj4);

  container.exportMask(GET_TEST_DATA_DIR + "/test.tif");
//  ZStack *result = container.run();

//  result->save(GET_TEST_DATA_DIR + "/test.tif");

//  delete result;

#endif

#if 0
  ZSparseStack stack;
  stack.load(GET_TEST_DATA_DIR + "/_system/test.zss");
  stack.getObjectMask()->canonize();
  ZStackWatershedContainer container(&stack);
  container.setFloodFillingZero(false);
  container.useSeedRange(true);
//  container.setRange(3707, 4142, 4681, 4235, 5849, 9793);


  ZObject3dScan obj1;
  obj1.addSegment(9693, 4542, 3807, 3817);
  obj1.setLabel(1);

  ZObject3dScan obj2;
  obj2.addSegment(4781, 5749, 4035, 4046);
  obj2.setLabel(200);

  container.addSeed(obj1);
  container.addSeed(obj2);

  container.run();
//  container.exportSource(GET_TEST_DATA_DIR + "/test2.tif");
//  container.exportMask(GET_TEST_DATA_DIR + "/test.tif");

  container.getResultStack()->save(GET_TEST_DATA_DIR + "/test.tif");

  ZObject3dScanArray *split = container.makeSplitResult(2, NULL);
  std::cout << split->size() << " splits" << std::endl;
  std::cout << split->getVoxelNumber() << " voxels" << std::endl;

  int index = 1;
  for (ZObject3dScanArray::const_iterator iter = split->begin();
       iter != split->end(); ++iter) {
    const ZObject3dScan &obj = **iter;
    ZString path = GET_TEST_DATA_DIR + "/test";
    path.appendNumber(index++);
    obj.save(path + ".sobj");

    stack.getObjectMask()->subtractSliently(obj);
  }

//  stack.getObjectMask()->print();

  delete split;

  container.printState();


//  delete result;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/test1.sobj");
  obj.upSample(3, 3, 1);
  obj.save(GET_TEST_DATA_DIR + "/test3.sobj");
#endif

#if 0
  ZStack stack;
  stack.load(GET_BENCHMARK_DIR + "/em_slice.tif");

  ZStackWatershedContainer container(&stack);
  ZObject3dScan obj;
  obj.addSegment(3201, 4102, 2900, 2905);
  obj.setLabel(1);

  ZObject3dScan obj2;
  obj2.addSegment(3201, 4114, 3010, 3015);
  obj2.setLabel(2);

  ZObject3dScan obj3;
  obj3.setLabel(3);
  obj3.addSegment(3201, 4185, 2904, 3000);
//  container.setRange(2890, 4095, 3201, 3100, 4173, 3201);
  container.addSeed(obj);
  container.addSeed(obj2);
  container.addSeed(obj3);

  ZStroke2d stroke;
  stroke.append(3089, 4173);
  stroke.setZ(3201);
  stroke.setWidth(5);
  stroke.setLabel(3);

  container.addSeed(stroke);

  ZObject3d obj4;
  obj4.append(2938, 4143, 3201);
  obj4.append(2939, 4144, 3201);
  obj4.append(2940, 4146, 3201);
  obj4.setLabel(5);
  container.addSeed(obj4);

  container.exportMask(GET_TEST_DATA_DIR + "/test.tif");
//  ZStack *result = container.run();

//  result->save(GET_TEST_DATA_DIR + "/test.tif");

//  delete result;

#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(3201, 4102, 2900, 2905);
  obj.exportHdf5(GET_TEST_DATA_DIR + "/test.hf5", "mask");

  ZObject3dScan obj2;
  obj2.importHdf5(GET_TEST_DATA_DIR + "/test.hf5", "mask");
  obj2.print();
#endif


#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_BENCHMARK_DIR + "/em_slice.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  QList<ZObject3dScan*> objList = ZDvidResultService::ReadSplitResult(
        "http://localhost:8000/api/node/4d3e3c206a314236b73800bc23a1f2d7/"
        "result/key/242ad92398d751344cfafbf1cc91e383");
  qDebug() << objList.size();

  ZColorScheme colorScheme;
  colorScheme.setColorScheme(ZColorScheme::UNIQUE_COLOR);
  foreach(ZObject3dScan *obj, objList) {
    obj->setColor(colorScheme.getColor(obj->getLabel()));
    frame->document()->addObject(obj);
    obj->save(GET_TEST_DATA_DIR + "/test.sobj");
  }
  frame->document()->notifyObjectModified();
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(11);
  stripe.setZ(20);
  stripe.addSegment(1, 10);

  std::ofstream stream((GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  stripe.write(stream);
  stream.close();

  ZObject3dStripe stripe2;
  std::ifstream stream2((GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  stripe2.read(stream2);
  stream2.close();

  stripe2.print();
#endif

#if 0
  ZIntPoint pt(1, 2, 3);
  std::ofstream stream(
        (GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  pt.write(stream);
  stream.close();

  ZIntPoint pt2;
  std::ifstream stream2(
        (GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  pt2.read(stream2);
  stream2.close();

  std::cout << pt2.toString() << std::endl;

#endif

#if 0
  std::cout << GET_FLYEM_CONFIG.getTaskServer() << std::endl;
#endif

#if 0
  QList<ZJsonObject> taskList = ZDvidResultService::ReadSplitTaskList(
        GET_FLYEM_CONFIG.getTaskServer().c_str());
  foreach (const ZJsonObject &task, taskList) {
    task.print();
  }

#endif

#if 0
  ZDvidTarget target;
  target.setFromUrl("http://localhost:8000/api/node/uuid/segname/sparsevol/12345");
  target.print();

  std::cout << ZDvidUrl::GetBodyId(
                 "http://localhost:8000/api/node/uuid/segname/sparsevol/12345")
            << std::endl;
#endif

#if 0
  ZIntPoint pt(1, 2, 3);
  std::stringstream stream;

  pt.write(stream);
//  stream.seekp(0);

//  std::cout << std::hex << pt.getX() << std::endl;

  QByteArray byteArray;
  byteArray.setRawData(stream.str().data(), stream.str().size());
//  std::cout << stream.str().data() << std::endl;

  std::stringstream stream2;
  stream2.write(byteArray.constData(), byteArray.size());
//  stream2.seekp(0);

  ZIntPoint pt2;
  pt2.read(stream2);
  std::cout << pt2.toString() << std::endl;
#endif

#if 0
  ZStack *stack = ZStackFactory::makeIndexStack(2, 3, 4);
  stack->setOffset(10, 20, 30);
  std::ofstream stream(
        (GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  stack->write(stream);
  stream.close();

  ZStack stack2;
  std::ifstream stream2(
        (GET_TEST_DATA_DIR + "/test.bin").c_str(), std::ios_base::binary);
  stack2.read(stream2);
  stack2.printInfo();
  C_Stack::printValue(stack2.data());
#endif

#if 0
  ZSparseStack spStack;

  ZObject3dScan *obj = new ZObject3dScan;
  obj->addStripe(0, 0);
  obj->addSegment(0, 1);
  spStack.setObjectMask(obj);

  ZStackBlockGrid *stackGrid = new ZStackBlockGrid;
  stackGrid->setBlockSize(2, 2, 2);
  stackGrid->setGridSize(3, 3, 3);

  ZStack *stack = ZStackFactory::MakeUniformStack(2, 2, 2, 255);
  stackGrid->consumeStack(ZIntPoint(0, 0, 0), stack);

  stack = ZStackFactory::MakeUniformStack(2, 2, 2, 255);
  stack->setOffset(2, 2, 2);
  stackGrid->consumeStack(ZIntPoint(1, 1, 1), stack);

  spStack.setGreyScale(stackGrid);
  spStack.save(GET_TEST_DATA_DIR + "/test.zss");
#endif

#if 0
  ZSparseStack *spStack = new ZSparseStack;
  spStack->load(GET_TEST_DATA_DIR + "/test.zss");

  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->document()->setSparseStack(spStack);
//  frame->load(GET_TEST_DATA_DIR + "/benchmark/em_stack.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

//  frame->view()->highlightPosition(129, 120, 0);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "876d", 8700);
//  target.set("emdata2.int.janelia.org", "d35f", 7000);
  target.useDefaultDataSetting(true);

  ZDvidReader reader;
  reader.open(target);

  reader.getDvidTarget().toDvidDataSetting().print();

  ZDvidWriter writer;
  target = reader.getDvidTarget();
  target.useDefaultDataSetting(false);
  target.setLabelBlockName("pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setBodyLabelName("");
  target.setTodoListName("");
  target.toDvidDataSetting().print();

  writer.open(target);

  writer.writeDefaultDataSetting();
#endif

#if 0
  ZSparseStack *spStack = new ZSparseStack;
  spStack->load(GET_TEST_DATA_DIR + "/test.zss");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyIdArray;
  bodyIdArray << 851657;
  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);
  foreach (uint64_t bodyId, bodyIdArray) {

    ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(bodyId);

    //  ZIntCuboid box = spStack->getBoundBox();

    ZStack *stack = spStack->makeIsoDsStack(NeuTube::ONEGIGA);

//    ZStack *stack = spStack->makeStack(ZIntCuboid());
//    ZStack *stack = spStack->getStack();

    ZString numStr;
    numStr.appendNumber(bodyId);
    writer.write(GET_TEST_DATA_DIR + "/test.tif", stack);

//    delete stack;
    delete spStack;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyIdArray;
  bodyIdArray << 851657 << 180022 << 181594 << 157909 << 156067 << 114452
              << 87839 << 371523 << 401430 << 110180;

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);

  foreach (uint64_t bodyId, bodyIdArray) {

    ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(bodyId);

    //  ZIntCuboid box = spStack->getBoundBox();

    ZStack *stack = spStack->makeIsoDsStack(NeuTube::ONEGIGA);

//    ZStack *stack = spStack->makeStack(ZIntCuboid());
//    ZStack *stack = spStack->getStack();

    ZString numStr;
    numStr.appendNumber(bodyId);
    writer.write(GET_TEST_DATA_DIR + "/_test/93e8/" + numStr + ".tif", stack);

//    delete stack;
    delete spStack;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(851657);

  ZIntCuboid box = spStack->getBoundBox();

  int minZ = box.getFirstCorner().getZ();
  int maxZ = box.getLastCorner().getZ();
  int x0 = box.getFirstCorner().getX();
  int y0 = box.getFirstCorner().getY();
  int width = box.getWidth();
  int height = box.getHeight();

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_DEFAULT);

  for (int z = minZ; z <= maxZ; ++z) {
    ZStack *stack = spStack->getSlice(z, x0, y0, width, height);
    if (stack != NULL) {
      std::cout << z << "/" << maxZ << std::endl;
      ZString numStr;
      numStr.appendNumber(z, 5);

      writer.write(GET_TEST_DATA_DIR + "/_test/series/" + numStr + ".tif", stack);
      delete stack;
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");

  ZDvidReader reader;
  reader.open(target);

  ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(2932287);
  spStack->getSparseStack()->save(GET_TEST_DATA_DIR + "/2932287.zss");
  delete spStack;
#endif

#if 0
  QList<ZJsonObject> objList = ZDvidResultService::ReadSplitTaskList(
        GET_FLYEM_CONFIG.getTaskServer().c_str());

  foreach (const ZJsonObject &obj, objList) {
    obj.print();
  }


  ZDvidTarget bodySource;
  bodySource.setFromUrl("http://emdata1.int.janelia.org:8500/api/node/b6bc/bodies/sparsevol/13123419");
  QList<ZObject3dScan*> segList = ZDvidResultService::ReadSplitResult(
        GET_FLYEM_CONFIG.getTaskServer().c_str(), bodySource, 11707760);
  std::cout << segList.size() << std::endl;
//  segList[0]->save(GET_TEST_DATA_DIR + "/test1.sobj");
//  segList[1]->save(GET_TEST_DATA_DIR + "/test2.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.setFromUrl("http://localhost:8001/api/node/4d3e");
  target.setBodyLabelName("*");
  ZDvidReader *reader = ZGlobal::GetDvidReader(target);
  std::cout << reader << std::endl;
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.setFromUrl("http://localhost:8000/api/node/4d3e");
  target.setBodyLabelName("*");
  std::cout << "Opening " << target.getSourceString() << std::endl;
  std::cout << (reader.open(target) ? "Opened successfully." : "Cannot open to read.")
            << std::endl;
#endif

#if 0

  std::string signalPath=
        "http://emdata1.int.janelia.org:8500/api/node/b6bc/bodies/sparsevol/"
        "13334275";
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReaderFromUrl(signalPath);
  ZDvidSparseStack *spStack =
      reader->readDvidSparseStackAsync(ZDvidUrl::GetBodyId(signalPath));

#endif
  
#if 0
  std::cout << ZDvidData::GetName(ZDvidData::ROLE_SPLIT_GROUP) << std::endl;
  qDebug() << ZDvidData::GetName<QString>(ZDvidData::ROLE_SPLIT_GROUP);
  qDebug() << ZDvidData::GetName<QString>(ZDvidData::ERole::ROLE_SPLIT_RESULT_KEY);

  qDebug() << ZDvidPath::GetResultPath("test", "data", true);
  qDebug() << ZDvidPath::GetResultKeyPath("test", "testkey");
#endif

#if 0
  ZStack *stack = ZStackFactory::makeIndexStack(3, 4, 5);
  stack->setOffset(1, 2, 3);
  stack->setDsIntv(0, 5, 7);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");

  ZStack stack2;
  stack2.load(GET_TEST_DATA_DIR + "/test.tif");
  std::cout << stack2.getOffset().toString() << std::endl;
  std::cout << stack2.getDsIntv().toString() << std::endl;
#endif

#if 0
  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_DEFAULT);

  ZStack *stack = ZStackFactory::makeIndexStack(3, 4, 5);
  stack->setOffset(1, 0, 0);
  stack->setDsIntv(0, 5, 7);
  writer.write(GET_TEST_DATA_DIR + "/test.tif", stack);

  ZStack stack2;
  stack2.load(GET_TEST_DATA_DIR + "/test.tif");
  std::cout << stack2.getOffset().toString() << std::endl;
  std::cout << stack2.getDsIntv().toString() << std::endl;
#endif

#if 0
  ZStack stack2;
  stack2.load(GET_TEST_DATA_DIR + "/tmp/15363212.tif");
  std::cout << stack2.getOffset().toString() << std::endl;
  std::cout << stack2.getDsIntv().toString() << std::endl;
#endif

#if 0
  ZStack *stack = ZStackFactory::MakeZeroStack(GREY, 256, 256, 1, 1);
//  stack->setOffset(100, 100, 0);
  ZSwcTree tree;
  Swc_Tree_Node *tn = SwcTreeNode::makePointer(1, 1, ZPoint(127, 127, 0), 3, -1);
  tree.addRegularRoot(tn);

  Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(2, 1, ZPoint(100, 127, 0), 3, -1);
  SwcTreeNode::setParent(tn2, tn);

  Swc_Tree_Node *tn3 = SwcTreeNode::makePointer(2, 2, ZPoint(127, 100, 0), 3, -1);
  SwcTreeNode::setParent(tn3, tn);

  tree.labelStackByType(stack);

  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZJsonObject seedJson;
  seedJson.load(
        "/Users/zhaot/Work/neutube/neurolabi/data/_flyem/CX/test_061317.json");

  ZSwcTree tree;

  ZJsonArray pointArrayJson(
        ZJsonObject(ZJsonArray(seedJson.value("meshReview")).value(0)).
        value("pointMarkers"));
  for (size_t i = 0; i < pointArrayJson.size(); ++i) {
    ZJsonObject pointJson(pointArrayJson.value(i));
    int label = ZJsonParser::integerValue(pointJson["color"]);
    double x = ZJsonParser::numberValue(pointJson["x"]);
    double y = ZJsonParser::numberValue(pointJson["y"]);
    double z = ZJsonParser::numberValue(pointJson["z"]);
    Swc_Tree_Node *tn =
        SwcTreeNode::makePointer(1, label + 1, x + 376, y + 128, z + 16, 1.5, -1);
    tree.addRegularRoot(tn);
  }

  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  std::shared_ptr<int> ptr;
  if (ptr) {
    std::cout << "*ptr=" << *ptr << "\n";
  } else {
    std::cout << "ptr is not a valid pointer.\n";
  }

  ptr = std::make_shared<int>(7);

  if (ptr) {
    std::cout << "*ptr=" << *ptr << "\n";
  } else {
    std::cout << "ptr is not a valid pointer.\n";
  }
#endif

#if 0
  struct Foo {
    Foo() { std::cout << "Foo ...\n"; }
    ~Foo() { std::cout << "~Foo...\n\n"; }
  };

  Foo *foo = new Foo();
  std::unique_ptr<Foo> up(foo);
  std::cout << "hash(up): " << std::hash<std::unique_ptr<Foo>>()(up) << '\n';
  std::cout << "hash(foo): " << std::hash<Foo*>()(foo) << '\n';

#endif

#if 0
  //lambda expression: [capture](parameters)->return-type {body}
  char s[] = "Hello World!";
  int uppercase = 0;
  for_each(s, s + sizeof(s), [&uppercase](char c) {
    if (isupper(c))
      uppercase++;
  });
  cout << uppercase << " uppercase letters in: " << s << endl;

#endif

#if 0
  auto x = 0;
  typedef decltype(x) CIT;
  CIT y = 1;
  std::cout << y << std::endl;

#endif

#if 0
  std::cout << (nullptr == 0) << std::endl;
#endif

#if 0
  ZDoubleVector v = {1.0, 2.0, 3.0};
  std::cout << v.mean() << std::endl;
#endif

#if 0
  ZTextEdit *widget = new ZTextEdit(NULL);
  widget->show();
#endif

#if 0
  StringListDialog *dlg = new StringListDialog(host);
  dlg->exec();
#endif

#if 0
  ZBodyListWidget *widget = new ZBodyListWidget(NULL);
  widget->show();
#endif

#if 0
  CrashTest();
#endif

#if 0
  QJsonObject json;
  json["test"] = QJsonArray();

  QJsonArray array = json["test"].toArray();
  array << 1 << 2 << 3;
  json["test"] = array;
  qDebug() << json;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setBodyLabelName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol");

  QJsonObject taskJson;

  QFile file((GET_TEST_DATA_DIR + "/_flyem/test/VR_Practice_Assignment_smithc_dvid.json").c_str());
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll());

  QJsonArray rootObj = jsonDoc.array();
  foreach (QJsonValue v, rootObj) {
    QJsonObject obj = v.toObject();
    QJsonArray markerJson = obj.value("pointMarkers").toArray();
    if (!markerJson.isEmpty()) {
      uint64_t bodyId =
          ZString(obj.value("file").toString().toStdString()).firstUint64();
      ZFlyEmMisc::SetSplitTaskSignalUrl(taskJson, bodyId, target);

      foreach (QJsonValue marker, markerJson) {
        ZStroke2d stroke = ZFlyEmMisc::SyGlassSeedToStroke(marker.toObject());
//        stroke.print();
        ZFlyEmMisc::AddSplitTaskSeed(taskJson, stroke);
      }

      break;
    }
  }

  QFile output((GET_TEST_DATA_DIR + "/test.json").c_str());
  output.open(QIODevice::WriteOnly);

  output.write(QJsonDocument(taskJson).toJson());
  output.close();
//  qDebug() << taskJson;

//  qDebug() << rootObj[0];


#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "3f7a", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setBodyLabelName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol");

  ZJsonObject docJson;
  docJson.load((GET_TEST_DATA_DIR +
                "/_flyem/test/VR_Practice_Assignment_smithc_dvid.json"));

  ZJsonArray rootObj(docJson.value("meshReview"));
//  QJsonArray rootObj = jsonDoc.array();

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());
  ZDvidUrl dvidUrl(target);

  for (size_t i = 0; i < rootObj.size(); ++i) {
    ZJsonObject obj(rootObj.value(i));
    ZJsonArray markerJson(obj.value("pointMarkers"));
    if (!markerJson.isEmpty()) {
      uint64_t bodyId =
          ZString(obj.value("file").toString()).firstUint64();

      if (bodyId > 0) {
        ZJsonObject taskJson;
//        ZIntPoint offset(298, 732, 544);
//        ZIntPoint dsIntv(2, 2, 2);
        ZFlyEmMisc::SetSplitTaskSignalUrl(taskJson, bodyId, target);

        for (size_t i = 0; i < markerJson.size(); ++i) {
          ZJsonObject markerObj(markerJson.value(i));
          ZStroke2d stroke =
              ZFlyEmMisc::SyGlassSeedToStroke(markerObj);
          //        stroke.print();
          ZFlyEmMisc::AddSplitTaskSeed(taskJson, stroke);
        }
        std::string location = writer->writeServiceTask("split", taskJson);

        ZJsonObject entryJson;
        entryJson.setEntry(NeuTube::Json::REF_KEY, location);
        QString taskKey = dvidUrl.getSplitTaskKey(bodyId).c_str();
        writer->writeSplitTask(taskKey, taskJson);
      }
    }
  }

//  taskJson.dump(GET_TEST_DATA_DIR + "/test.json");

//  qDebug() << taskKey;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "93e8", 8700);
  target.setLabelBlockName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz");
  target.setBodyLabelName(
        "pb26-27-2-trm-eroded32_ffn-20170216-2_celis_cx2-2048_r10_0_seeded_64blksz_vol");

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
        GET_FLYEM_CONFIG.getTaskServer());
   ZDvidUrl dvidUrl(target);


#endif

#if 0
   std::vector<ZPoint> ptArray = ZGeometry::LineShpereIntersection(
         ZPoint(0, 0.5, -2), ZPoint(0, 0, 1), ZPoint(0, 0, 0), 1);

   std::cout << ptArray.size() << " intersections: " << std::endl;
   for (std::vector<ZPoint>::const_iterator iter = ptArray.begin();
        iter != ptArray.end(); ++iter) {
     const ZPoint &pt = *iter;
     std::cout << pt.toString() << std::endl;
   }
#endif

#if 0
   ZStack stack;
   stack.load(GET_TEST_DATA_DIR + "/test.tif");
   stack.printInfo();
#endif

#if 0
   ZMesh mesh;
   ZMeshIO::instance().load(
         "/Users/zhaot/Work/vol2mesh/test.tif.smooth.obj", mesh);
   mesh.swapXZ();
   mesh.translate(404, 480, 180);
   mesh.scale(8, 8, 8);
   ZMeshIO::instance().save(
        mesh, (GET_TEST_DATA_DIR + "/test.obj").c_str(), "obj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setBodyLabelName("bodies");
  target.setLabelBlockName("labels");

  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyIdArray;
  bodyIdArray << 1;

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);

  foreach (uint64_t bodyId, bodyIdArray) {

    ZDvidSparseStack *spStack = reader.readDvidSparseStackAsync(bodyId);

    //  ZIntCuboid box = spStack->getBoundBox();

    ZStack *stack = spStack->makeIsoDsStack(NeuTube::ONEGIGA);

//    ZStack *stack = spStack->makeStack(ZIntCuboid());
//    ZStack *stack = spStack->getStack();

    ZString numStr;
    numStr.appendNumber(bodyId);
    writer.write(GET_TEST_DATA_DIR + "/test.tif", stack);

//    delete stack;
    delete spStack;
  }
#endif

#if 0
   ZMesh mesh;
   ZMeshIO::instance().load(
         "/Users/zhaot/Work/vol2mesh/test.tif.smooth.obj", mesh);
   mesh.swapXZ();

   ZStack stack;
   stack.load(GET_TEST_DATA_DIR + "/test.tif");
   mesh.translate(stack.getOffset().getX(), stack.getOffset().getY(),
                  stack.getOffset().getZ());
   mesh.scale(stack.getDsIntv().getX() + 1, stack.getDsIntv().getY() + 1,
              stack.getDsIntv().getZ() + 1);
   ZMeshIO::instance().save(
        mesh, (GET_TEST_DATA_DIR + "/test.obj").c_str(), "obj");
#endif

#if 0
   QList<uint64_t> bodyList;

   std::string dataDir = GET_TEST_DATA_DIR + "/_test/93e8";
   ZFileList fileList;
   fileList.load(dataDir, "tif");
   for (int i = 0; i < fileList.size(); ++i) {
     ZString path = fileList.getFilePath(i);
     uint64_t bodyId = path.lastUint64();
     bodyList << bodyId;
   }

   qDebug() << bodyList;
   foreach (uint64_t bodyId, bodyList) {
     std::cout << "Processing " << bodyId << "..." << std::endl;
     ZMesh mesh;
     QString path = QString("%1/meshes/%2.tif.smooth.obj").
         arg(dataDir.c_str()).arg(bodyId);
     ZMeshIO::instance().load(path, mesh);
     mesh.swapXZ();
     ZStack stack;
     stack.load(QString("%1/%2.tif").arg(dataDir.c_str()).arg(bodyId).toStdString());
     mesh.translate(stack.getOffset().getX(), stack.getOffset().getY(),
                    stack.getOffset().getZ());
     mesh.scale(stack.getDsIntv().getX() + 1, stack.getDsIntv().getY() + 1,
                stack.getDsIntv().getZ() + 1);
     ZMeshIO::instance().save(
          mesh, QString("%1/meshes/%2.dvid.obj").arg(dataDir.c_str()).arg(bodyId), "obj");
   }

#endif

#if 0
   ZMesh mesh;
   ZMeshIO::instance().load(
         (GET_TEST_DATA_DIR + "/test2.obj").c_str(), mesh);
   mesh.translate(404, 480, 180);
   mesh.scale(8, 8, 8);
   mesh.swapXZ();
   ZMeshIO::instance().save(
        mesh, (GET_TEST_DATA_DIR + "/test3.obj").c_str(), "obj");
#endif

#if 0
   ZMesh mesh;
   ZMeshIO::instance().load(
         "/Users/zhaot/Work/vol2mesh/test.tif.smooth.obj", mesh);

   vtkSmartPointer<vtkPolyData> poly = meshToVtkPolyData(mesh);

   vtkSmartPointer<vtkOBBTree> obbTree =
       vtkSmartPointer<vtkOBBTree>::New();
   obbTree->SetDataSet(poly);
   obbTree->BuildLocator();

   vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
   double a0[] = {0, 352.376, 54.7405};
   double a1[] = {10, 352.376, 54.7405};
   obbTree->IntersectWithLine(a0, a1, points, NULL);

   std::cout << "#Intersections: " << points->GetNumberOfPoints() << std::endl;

   points->GetPoint(0, a0);
   points->GetPoint(1, a1);

   std::cout << a0[0] << " " << a0[1] << " " << a0[2] << std::endl;
   std::cout << a1[0] << " " << a1[1] << " " << a1[2] << std::endl;
#endif

#if 0
   std::vector<std::pair<int, int>> line = LineToPixel(0, -10, 18, 0);
   for (const std::pair<int, int> &pt : line) {
     std::cout << pt.first << " " << pt.second << std::endl;
   }
   std::cout << endl;
#endif

#if 0
   ZDvidTarget target;
   target.set("emdata3.int.janelia.org", "aed4", 8000);
//   target.setBodyLabelName("labels-v3");
   target.setLabelBlockName("labels-v3");

   ZDvidReader reader;
   reader.open(target);

   std::cout << reader.getDvidTarget().usingLabelArray() << std::endl;
   reader.updateMaxLabelZoom();
   std::cout << reader.getDvidTarget().getMaxLabelZoom() << std::endl;

   ZObject3dScan *obj = reader.readMultiscaleBody(274766007, 6, true, NULL);
   obj->save(GET_TEST_DATA_DIR + "/test.sobj");
   delete obj;
#endif

#if 0
   ZStack *stack = ZStackFactory::MakeZeroStack(GREY, 3, 3, 3, 1);
   stack->setValue(1, 1, 1, 0, 255);
   stack->setOffset(1, 1, 1);
   stack->setDsIntv(1, 1, 1);

   stack->save(GET_BENCHMARK_DIR + "/dot.tif");
#endif

#if 0
   ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriterFromUrl(
         "http://emdata1.int.janelia.org:8500/api/node/b6bc/bodies2/sparsevol/1");
   writer->getDvidTarget().print();
#endif

#if 0
   ZDvidTarget target;
   target.setFromUrl("http://emdata1.int.janelia.org:8500/api/node/b6bc/bodies2/sparsevol/1");

//   target.setFromSourceString("http:emdata2.int.janelia.org:-1:2b6c:bodies2");
   target.print();
#endif

#if 0

  std::string dataDir = GET_DATA_DIR + "/_system/split/data9";
  ZJsonObject obj;
  obj.load(dataDir + "/" + "data.json");
  ZStack *stack = ZFlyEmMisc::GenerateExampleStack(obj);
  stack->setOffset(0, 0, 0);
  stack->save(dataDir + "/test.tif");

  delete stack;
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/swc/color.swc");
  ZStack *stack = tree.toTypeStack();
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete stack;
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 0);
  ZDvidInfo dvidInfo;
  dvidInfo.setBlockSize(32, 32, 32);
  ZCubeArray *cubeArray = ZFlyEmMisc::MakeRoiCube(
        obj, dvidInfo, QColor(255, 0, 0), 0);
  std::cout << "#cubes: " << cubeArray->size() << std::endl;
  ZMesh mesh = ZMesh::FromZCubeArray(*cubeArray);
  std::cout << "#vertices: " << mesh.numVertices() << std::endl;
  std::cout << "#Indices: " << mesh.indices().size() << std::endl;
  ZDebugPrintArrayG(mesh.indices(), 0, 36);
  mesh.save((GET_TEST_DATA_DIR + "/test.obj").c_str());
  delete cubeArray;
#endif

#if 0
   ZDvidTarget target;
   target.set("emdata3", "bf6e", 8000);
   target.setLabelBlockName("segmentation");
   ZDvidReader reader;
   reader.open(target);

   reader.checkProofreadingData();
#endif

#if 0
   ZDvidReader reader;
   ZDvidTarget target("emdata3.int.janelia.org", "c0ab", 8000);
   target.setLabelBlockName("segmentation-from-bricks");
   target.setGrayScaleSource(ZDvidNode("127.0.0.1", "9b07", 8000));
   target.setGrayScaleName("grayscalejpeg");
   reader.open(target);

   ZDvidSparseStack *dss = reader.readDvidSparseStack(290523653);
   ZSparseStack *ss = dss->getSparseStack();
   ss->save(GET_TEST_DATA_DIR + "/test.zss");
//   ss->getStackGrid()->toStack()->save(GET_TEST_DATA_DIR + "/test.tif");

//   ss.setDvidTarget(target);
//   ss.

//   std::vector<ZStack*> stackArray = reader.readGrayScaleBlock(
//         ZIntPoint(52, 57, 212), reader.readGrayScaleInfo(), 2);
//   ZStack *stack = stackArray[1];
////   ZStack *stack = reader.readGrayScale(3189, 3850, 13970, 100, 100, 100);
//   stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
   ZRandomGenerator g;
   while (true) {
     ZObject3dScan obj1;
     ZObject3dScan obj2;
     int z = g.rndint(1000);
     for (int i = 0; i < 100; ++i) {
       obj1.addSegment(
             z, g.rndint(100), g.rndint(100),
             g.rndint(1000));
       obj2.addSegment(
             z, g.rndint(100), g.rndint(100),
             g.rndint(1000));
     }

     std::cout << obj1.getVoxelNumber() << " ";
     obj1.subtractSliently(obj2);
     std::cout << obj1.getVoxelNumber() << std::endl;
     obj1.downsampleMax(1, 1, 1);
     ZSwcTree *tree = ZSwcGenerator::createSurfaceSwc(obj1, 3);
     delete tree;
   }

#endif


#if 0
   ZDvidReader reader;
   ZDvidTarget target("emdata1.int.janelia.org", "babd", 8500);
   target.setBodyLabelName("bodies3");
   reader.open(target);

   ZObject3dScan *obj = reader.readBody(43752, true, NULL);
   obj->exportImageSlice(GET_TEST_DATA_DIR + "/tmp/43752");
   delete obj;

#endif

#if 0
   ZObject3dScan obj;
   obj.addSegment(0, 0, 0, 0);
   obj.addSegment(1, 1, 2, 3);
   obj.exportImageSlice(0, 1, GET_TEST_DATA_DIR + "/tmp/slice");


#endif

#if 0
   ZIntCuboid box;
   box.setFirstCorner(0, 0, 0);
   box.setLastCorner(10, 20, 30);

   std::vector<bool> visible;
   visible.resize(6, true);
   visible[0] = false;
   visible[2] = false;
   visible[4] = false;

   ZMesh mesh = ZMesh::CreateCuboidFaceMesh(box, visible, QColor(255, 0, 0));
   std::cout << "#vertices: " << mesh.numVertices() << std::endl;
   std::cout << "#Indices: " << mesh.indices().size() << std::endl;
//   ZDebugPrintArrayG(mesh.indices(), 0, 36);

#endif

#if 0
  ZObject3dScan obj;
//  obj.load(GET_TEST_DATA_DIR + "/_flyem/MB/large_outside_block.sobj");
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 2);
  ZDvidInfo dvidInfo;
  dvidInfo.setBlockSize(32, 32, 32);
  obj.setDsIntv(31);
//  ZMesh *mesh = ZFlyEmMisc::MakeRoiMesh(obj, QColor(255, 0, 0), 1);
  ZMesh *mesh = ZFlyEmMisc::MakeRoiMesh(
        obj, dvidInfo, QColor(255, 0, 0), 1);

  std::cout << "#vertices: " << mesh->numVertices() << std::endl;
  std::cout << "#Indices: " << mesh->indices().size() << std::endl;

//  mesh->save((GET_TEST_DATA_DIR + "/_test.obj").c_str());


  delete mesh;
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "c0ab", 8000);
  target.setLabelBlockName("segmentation-from-bricks");

  reader.open(target);
  tic();
  std::cout << reader.readBodyBlockCount(296188589) << std::endl;
  std::cout << reader.readBodyBlockCount(296612416) << std::endl;
  ptoc();
#endif

#if 0
  std::map<QString, int/*,QStringNaturalCompare*/> map1;
  map1["#.FlyEMSynapse.Psd#43"] = 1;
  map1["#.FlyEMSynapse.Psd#18110738494"] = 2;
  map1["#.FlyEMSynapse.TBar#43"] = 3;
  map1["#.FlyEMSynapse.TBar#18110738494"] = 4;

  std::map<QString, std::string/*,QStringNaturalCompare*/> map2;
  map2["#.FlyEMSynapse.Psd#43"] = "1";
  map2["#.FlyEMSynapse.TBar#18110738494"] = "2";

  std::map<QString, int> map3;
  std::set_difference(map1.begin(), map1.end(), map2.begin(), map2.end(),
                      std::inserter(map3, map3.end()), QStringKeyNaturalLess());

  std::cout << "Wrong set difference" << std::endl;
  std::cout << map3.size() << std::endl;
  for (const auto &m : map3) {
    qDebug() << m.first << m.second;
  }
#endif

#if 0
  std::set<QString, QStringNaturalCompare> set1;
  set1.insert("#.FlyEMSynapse.Psd#43");
  set1.insert("#.FlyEMSynapse.Psd#18110738494");
  set1.insert("#.FlyEMSynapse.TBar#43");
  set1.insert("#.FlyEMSynapse.TBar#18110738494");

  std::map<QString, std::string, QStringNaturalCompare> map2;
  map2["#.FlyEMSynapse.Psd#43"] = "1";
  map2["#.FlyEMSynapse.TBar#18110738494"] = "2";
  map2["#.FlyEMSynapse.TBar#18"] = "3";

  std::set<QString, QStringNaturalCompare> set3;
  std::set_difference(set1.begin(), set1.end(), map2.begin(), map2.end(),
                      std::inserter(set3, set3.end()), QStringKeyNaturalLess());

  std::cout << set3.size() << std::endl;
  for (const auto &m : set3) {
    qDebug() << m;
  }
#endif

#if 0
  ZDvidTarget target;
//  target.set("emdata3.int.janelia.org", "0312", 8000);
//  target.setLabelBlockName("segmentation-agglomerated");
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setLabelBlockName("labels");

  ZDvidReader reader;
  reader.open(target);
  std::cout << reader.readCoarseBodySize(1) << std::endl;

  ZObject3dScan obj = reader.readCoarseBody(1);
  std::cout << obj.getVoxelNumber() << std::endl;
#endif

#if 0
  ZFlyEmBodyMergeProject project;
  project.test();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "3b54", 7000);
  target.setLabelBlockName("labels1104");
  ZDvidReader reader;
  reader.open(target);

  QList<uint64_t> bodyList =
      QList<uint64_t>() << 22351 << 151837 << 618595
                        << 597861 << 568121 << 571372 << 598856;

  foreach (uint64_t bodyId, bodyList) {
    ZSwcTree *tree = reader.readSwc(bodyId);
    ZString outputPath = GET_TEST_DATA_DIR + "/_flyem/FIB/FIB25/neuromorpho_20171103/";
    outputPath.appendNumber(bodyId);
    outputPath += ".swc";
    tree->rescale(0.01, 0.01, 0.01);
    tree->save(outputPath);
  }
#endif

#if 0
  ZObject3dScan obj;
//  obj.load(GET_TEST_DATA_DIR + "/_flyem/MB/large_outside_block.sobj");
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 2);
  obj.addSegment(1, 0, 1, 2);
  ZDvidInfo dvidInfo;
  dvidInfo.setBlockSize(32, 32, 32);
//  obj.setDsIntv(31);
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj, 0);
  std::cout << "#vertices: " << mesh->numVertices() << std::endl;
  std::cout << "#Indices: " << mesh->indices().size() << std::endl;

  mesh->generateNormals();
  mesh->setColor(255, 0, 0);
  mesh->pushObjectColor();

  mesh->save((GET_TEST_DATA_DIR + "/test.obj").c_str());
  delete mesh;
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/_system/test.sobj");
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);

  std::cout << "#vertices: " << mesh->numVertices() << std::endl;
  std::cout << "#Indices: " << mesh->indices().size() << std::endl;

  mesh->generateNormals();
  mesh->save((GET_TEST_DATA_DIR + "/test.obj").c_str());
  delete mesh;
#endif

#if 0
  ZStroke2d stroke;
  stroke.setWidth(3);
  stroke.append(0, 0);
  stroke.append(1, 1.5);
  stroke.append(0, 2);
  stroke.append(0, 3);
  stroke.append(3, 3);
  stroke.decimate();

  stroke.print();
#endif

#if 0
  ZObject3dScan *obj = new ZObject3dScan;
  obj->addStripe(0, 0);
  obj->addSegment(0, 1, false);
  obj->addSegment(0, 5, false);
  obj->addSegment(0, 8, false);
  obj->addStripe(0, 2);
  obj->addSegment(3, 3, false);
  obj->addSegment(0, 1, false);
  obj->addStripe(0, 1);
  obj->addSegment(5, 7, false);

//  obj->sort();
//  obj->sortedCanonize();
//  obj->isEmpty();
  obj->canonize();
#endif

#if 0
  ZObject3dStripe stripe;
  stripe.setY(0);
  stripe.setZ(0);
  stripe.addSegment(0, 2);
  stripe.addSegment(4, 6);
  stripe.addSegment(8, 10);
  stripe.remove(5, 8);
  stripe.print();
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_BENCHMARK_DIR + "/29.sobj");
  ZIntCuboid box;
  box.set(ZIntPoint(276, 784, 182), ZIntPoint(293, 798, 186));
  obj.remove(box);

  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setBodyLabelName("labels");

  ZFlyEmProofDoc *doc = new ZFlyEmProofDoc(host);
  doc->setDvidTarget(target);

  TaskProtocolWindow *window = new TaskProtocolWindow(doc, NULL, host);
  window->test();

#endif

#if 0
  ZDvidTarget target;
  target.set("127.0.0.1", "0f59", 8000);
  target.setGrayScaleName("grayscalejpeg");
  ZDvidReader reader;
  reader.open(target);
  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  std::cout << "Bound box: " << dvidInfo.getDataRange().toString() << std::endl;

  ZStack *stack = reader.readGrayScale(10000, 3000, 10000, 256, 256, 256, 0);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");

  ZStack *stack2 = reader.readGrayScale(10000, 3000, 10000, 256, 256, 256, 1);
  stack2->save(GET_TEST_DATA_DIR + "/test2.tif");

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_BENCHMARK_DIR + "/29.sobj");
  ZObject3dScan obj2 = obj;
  tic();
  ZObject3dScan obj3 = obj - obj2;
  ptoc();
#endif

#if 0
  ZStack *stack1 = ZStackFactory::MakeZeroStack(2, 3, 1, 1);
  stack1->setDsIntv(1, 1, 0);

  stack1->setIntValue(0, 0, 0, 0, 1);
  stack1->setIntValue(0, 1, 0, 0, 1);
  stack1->setIntValue(0, 2, 0, 0, 1);

  stack1->setIntValue(1, 0, 0, 0, 2);
  stack1->setIntValue(1, 1, 0, 0, 2);
  stack1->setIntValue(1, 2, 0, 0, 2);

  ZStackArray stackArray;
  stackArray.append(stack1);

  ZStack *stack2 = ZStackFactory::MakeZeroStack(4, 6, 1, 1);
  stack2->setIntValue(1, 1, 0, 0, 1);
  stack2->setIntValue(0, 2, 0, 0, 1);
  stack2->setIntValue(0, 3, 0, 0, 1);
  stack2->setIntValue(1, 4, 0, 0, 1);

  stack2->setIntValue(2, 1, 0, 0, 2);
  stack2->setIntValue(1, 2, 0, 0, 2);
  stack2->setIntValue(1, 3, 0, 0, 2);
  stack2->setIntValue(2, 4, 0, 0, 2);
  stackArray.append(stack2);

//  ZStack *stack2 = ZStackFactory::MakeZeroStack(GREY, 2, 3, 1);
  ZObject3dScanArray *objArray =
      ZObject3dFactory::MakeObject3dScanArray(stackArray);

  for (size_t i = 0; i < objArray->size(); ++i) {
    ZObject3dScan *obj = (*objArray)[i];
    std::cout << "Label: " << obj->getLabel() << std::endl;
    obj->print();
  }

#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
  frame->load(GET_BENCHMARK_DIR + "/em_slice.tif");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZStackWatershedContainer container(frame->document()->getStack());
  ZStroke2d seed1;
  seed1.set(2959, 4101);
  seed1.setZ(3201);
  seed1.setWidth(10);
  seed1.setLabel(1);
  container.addSeed(seed1);

  ZStroke2d seed2;
  seed2.set(3038, 4085);
  seed2.setZ(3201);
  seed2.setWidth(10);
  seed2.setLabel(2);
  container.addSeed(seed2);

  container.run();
  ZObject3dScanArray *objArray = container.makeSplitResult(1, NULL);
  for (ZObject3dScan *obj : *objArray) {
    frame->document()->addObject(obj);
  }
  objArray->shallowClear();
  delete objArray;

#endif

#if 0
  for (int i = 1; i < 1000; i *= 2) {
    int s = misc::getIsoDsIntvFor3DVolume(i, true);
    std::cout << i << " <= " <<  s << "^3 = " << (s+1)*(s+1)*(s+1) << std::endl;
  }
//  std::cout <<  misc::getIsoDsIntvFor3DVolume(512, true);
#endif

#if 0
  ZStack *stack = new ZStack;
  stack->load(GET_BENCHMARK_DIR + "/em_slice.tif");
  ZStackWatershedContainer container(stack);
  ZStroke2d *seed1 = new ZStroke2d;
  seed1->set(3930, 6265);
  seed1->setZ(6044);
  seed1->setWidth(10);
  seed1->setLabel(2);
  seed1->setPenetrating(false);
  container.addSeed(dynamic_cast<ZStackObject*>(seed1));

  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/swc/breadth_first.swc");
  tree.setLabel(1);
  container.addSeed(dynamic_cast<ZStackObject*>(&tree));

  std::cout << "#seeds: " << container.getSeedArray().size() << std::endl;
#endif

#if 0
  ZSparseStack sp;
  sp.load(GET_TEST_DATA_DIR + "/_system/split_test/body5.zss");
  ZStack *stack = sp.makeStack(ZIntCuboid(), true);
  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZObject3dScan obj;
//  obj.addSegment(0, 0, 0, 1);
//  obj.addSegment(0, 0, 3, 4);
//  ZObject3dScan border = obj.downsampleBorderMask(1, 1, 1);
//  border.print();

//  obj.addSegment(0, 0, 0, 1);
//  obj.addSegment(0, 0, 3, 4);
//  obj.downsampleMin(1, 1, 1);
//  obj.print();



  obj.load(GET_TEST_DATA_DIR + "/_system/split_test/body7.zss");


//  std::cout << obj.getBoundBox().toString() << std::endl;
//  ZObject3dScan comp = obj.getComplementObject();
//  comp.downsampleMax(1, 1, 1);
//  comp.print();
//  comp.save(GET_TEST_DATA_DIR + "/test.sobj");

//  ZObject3dScan obj = *this;
//  obj.downsampleMax(xintv, yintv, zintv);

//  ZObject3dScan border = obj.downsampleBorderMask(1, 1, 1);
//  border.save(GET_TEST_DATA_DIR + "/test.sobj");

//  obj.downsampleMax(1, 1, 1);
//  (obj - border).save(GET_TEST_DATA_DIR + "/test2.sobj");

#  if 0
  obj.downsampleMin(1, 1, 1);
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");

  obj.load(GET_TEST_DATA_DIR + "/_system/split_test/body.sobj");
  obj.downsampleMax(1, 1, 1);
  obj.save(GET_TEST_DATA_DIR + "/test2.sobj");
#  endif

#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
//  frame->load(GET_BENCHMARK_DIR + "/test.zss");
//  frame->load(GET_TEST_DATA_DIR + "/test.zss");
  frame->load(GET_TEST_DATA_DIR + "/_system/split_test/body7.zss");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZStackWatershedContainer container(frame->document()->getSparseStack());

  ZJsonObject taskJson;
  taskJson.load(GET_TEST_DATA_DIR + "/_system/split_test/body7.json");
  QList<ZStackObject*> seedList = ZFlyEmMisc::LoadSplitTask(taskJson);
  foreach (ZStackObject *seed, seedList) {
    container.addSeed(seed);
    frame->document()->addObject(seed);
  }

#  if 1
  container.setRefiningBorder(true);
  container.run();

  ZObject3dScanArray *objArray = container.makeSplitResult(1, NULL);
  for (ZObject3dScan *obj : *objArray) {
    frame->document()->addObject(obj);
  }

//  (*objArray)[0]->save(GET_TEST_DATA_DIR + "/test.sobj");

  objArray->shallowClear();
  delete objArray;
#  endif

#endif

#if 0
  ZStackFrame *frame = ZStackFrame::Make(NULL);
//  frame->load(GET_BENCHMARK_DIR + "/test.zss");
  frame->load(GET_TEST_DATA_DIR + "/_system/split_test/body2.zss");
  host->addStackFrame(frame);
  host->presentStackFrame(frame);

  ZStackWatershedContainer container(frame->document()->getSparseStack());
  ZStroke2d *seed1 = new ZStroke2d;
  seed1->set(3930, 6265);
  seed1->setZ(6044);
  seed1->setWidth(10);
  seed1->setLabel(2);
  seed1->setPenetrating(false);
  container.addSeed(*seed1);

  ZStroke2d *seed2 = new ZStroke2d;
//  seed2->set(4295, 4496);
//  seed2->setZ(7264);
  seed2->set(3898, 6279);
  seed2->setZ(6044);
  seed2->setWidth(10);
  seed2->setLabel(1);
  seed2->setPenetrating(false);
  container.addSeed(*seed2);

  ZStroke2d *seed3 = new ZStroke2d;
  seed3->set(3837, 5700);
  seed3->setZ(7289);
  seed3->setWidth(10);
  seed3->setLabel(3);
  seed3->setPenetrating(false);
  container.addSeed(*seed3);

  ZStroke2d *seed4 = new ZStroke2d;
  seed4->set(3883, 5739);
  seed4->setZ(7289);
  seed4->setWidth(10);
  seed4->setLabel(4);
  seed4->setPenetrating(false);
  container.addSeed(*seed4);

  frame->document()->addObject(seed1);
  frame->document()->addObject(seed2);
  frame->document()->addObject(seed3);
  frame->document()->addObject(seed4);

//  container.setMaxVolume(100*100*100);
  container.setRefiningBorder(true);
  container.run();

#  if 0
  std::vector<ZObject3d*> seedArray =
      ZStackWatershedContainer::MakeBorderSeed(*(container.getResultStack()));

  for (ZObject3d *obj : seedArray) {
    if (obj != NULL) {
//      obj->print();
      obj->setColor(ZStroke2d::GetLabelColor(obj->getLabel()));
      frame->document()->addObject(obj);
    }
  }
#  endif

#  if 1
  ZObject3dScanArray *objArray = container.makeSplitResult(1, NULL);
  for (ZObject3dScan *obj : *objArray) {
    frame->document()->addObject(obj);
  }
  objArray->shallowClear();
  delete objArray;
#  endif
#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_BENCHMARK_DIR + "/29.sobj");

  ZObject3dScan obj2 = obj.getSlice(190, 250);
  ZObject3dScan subtracted = obj.subtract(obj2);
  subtracted.save(GET_TEST_DATA_DIR + "/test.sobj");
  obj.save(GET_TEST_DATA_DIR + "/test2.sobj");

#endif

#if 0
  ZJsonArray arrayJson;
  ZJsonValue value(C_Json::makeInteger(1), ZJsonValue::SET_AS_IT_IS);
  std::cout << "Ref count: " << value.getRefCount() << std::endl;
  arrayJson.append(value);

  ZJsonArray arrayJson2 = arrayJson;

  std::cout << "Ref count after array appending: " << value.getRefCount() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "43f", 9000);
  target.setLabelBlockName("segmentation");
  target.setBodyLabelName("bodies");
//  target.setGrayScaleName("grayscalejpeg");
  target.setGrayScaleSource(ZDvidNode("emdata1.int.janelia.org", "231f", 9001));
  ZJsonArray seedJson;
  ZJsonArray roiJson;
  ZIntCuboid box(1, 2, 3, 10, 20, 30);
  roiJson = box.toJsonArray();

  ZStroke2d stroke;
  stroke.append(1, 1);
  stroke.setLabel(1);
  ZJsonObject splitJson = ZFlyEmMisc::MakeSplitSeedJson(stroke);
  seedJson.append(splitJson);


  ZJsonObject taskJson = ZFlyEmMisc::MakeSplitTask(target, 1, seedJson, roiJson);
  std::cout << taskJson.dumpString(2) << std::endl;
#endif

#if 0
  ZStackDocCommand::SwcEdit::MoveSwcNode command(new ZStackDoc());
  command.test();
#endif

#if 0
  ZStackObjectInfo objInfo;
  objInfo.setTarget(ZStackObject::ETarget::TARGET_OBJECT_CANVAS);
  objInfo.print();

  ZStackObjectInfoSet infoSet;
  infoSet.add(objInfo);
  infoSet.add(ZStackObject::ETarget::TARGET_DYNAMIC_OBJECT_CANVAS);
  infoSet.add(ZStackObject::ETarget::TARGET_OBJECT_CANVAS);
  infoSet.add(ZStackObjectRole::ROLE_3DGRAPH_DECORATOR);
  infoSet.add(ZStackObject::TYPE_CUBE);
  infoSet.add(ZStackObject::TYPE_CUBE, ZStackObjectInfo::STATE_ADDED);
  ZSwcTree tree;
  infoSet.add(tree);
  infoSet.add(tree);

  tree.setRole(ZStackObjectRole::ROLE_3DPAINT);
  infoSet.add(tree);

  infoSet.print();
#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "1d1d", 8000);
  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan obj = reader.readRoi("FB");
  ZObject3dScan slice = obj.makeZProjection(687, 687, 688);
  obj.unify(slice);
  ZObject3dScan obj2 = reader.readRoi("FB2");
  ZObject3dScan obj3 = reader.readRoi("FB3");
  obj.unify(obj2);
  obj.unify(obj3);
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  mesh->save((GET_TEST_DATA_DIR + "/test.obj").c_str());
#endif


#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "1d1d", 8000);
  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj = reader.readRoi("PB");
  ZObject3dScan obj2 = reader.readRoi("PB2");
  obj.unify(obj2);
  ZObject3dScan obj3 = reader.readRoi("PB3");
  ZObject3dScan slice = obj3.makeZProjection(464, 464, 463);
  obj3.unify(slice);
//  slice = obj.makeZProjection(464, 464, 462);
//  obj3.unify(slice);
  obj.unify(obj3);
  ZObject3dScan obj4 = reader.readRoi("PB4");
  obj.unify(obj4);

  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  mesh->save((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/PB.obj").c_str());
#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "1d1d", 8000);
  ZDvidReader reader;
  reader.open(target);
  {
    ZObject3dScan obj = reader.readRoi("PB");
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/PB1.obj").c_str());
  }

  {
    ZObject3dScan obj = reader.readRoi("PB2");
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/PB2.obj").c_str());
  }

  {
    ZObject3dScan obj = reader.readRoi("PB3");
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/PB3.obj").c_str());
  }

  {
    ZObject3dScan obj = reader.readRoi("PB4");
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/PB4.obj").c_str());
  }
#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "877d", 8300);
  ZDvidWriter writer;
  writer.open(target);

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/FB_dec.obj", "FB");
#endif

#if 0
//  ZDvidTarget target("emdata2.int.janelia.org", "fb02", 8900);
  ZDvidTarget target("emdata3.int.janelia.org", "0716", 8900);
  ZDvidWriter writer;
  writer.open(target);

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_02655_cut.obj",
        "hkcut_34");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_05251_cut.obj",
        "hkcut_32-33");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_07920_cut.obj",
        "hkcut_31-32");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_10600_cut.obj",
        "hkcut_30-31");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_13229_cut.obj",
        "hkcut_29-30");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_15895_cut.obj",
        "hkcut_28-29");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_18489_cut.obj",
        "hkcut_27-28");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_21204_cut.obj",
        "hkcut_26-27");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_24041_cut.obj",
        "hkcut_25-26");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_26853_cut.obj",
        "hkcut_24-25");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_29743_cut.obj",
        "hkcut_23-24");

  writer.uploadRoiMesh(
        GET_TEST_DATA_DIR +
        "/_flyem/FIB/hemibrain/Hemi_Brain_Hot_Knife_Cut_Meshes/hb_hk_32360_cut.obj",
        "hkcut_22-23");
#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "1d1d", 8000);
  ZDvidWriter writer;
  writer.open(target);
  std::vector<std::string> keyList;
  keyList.push_back("PB");
  keyList.push_back("PB2");
  keyList.push_back("PB3");
  keyList.push_back("PB4");

  writer.writeRoiRef("PB", keyList, "roi");
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "1d1d", 8100);
  ZDvidWriter writer;
  writer.open(target);

  writer.writeRoiRef("pb-lm", "pb-lm", "roi");
#endif

#if 0
  ZJsonObject roiJson;
  roiJson.load(GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/roi.json");
  ZDvidTarget target;
  target.loadJsonObject(ZJsonObject(roiJson.value("dvid")));


  ZDvidWriter writer;
  writer.open(target);

  ZJsonArray roiArray(roiJson.value("rois"));
  for (size_t i = 0; i < roiArray.size(); ++i) {
    ZJsonObject roiJson(roiArray.value(i));
    std::string name = ZJsonParser::stringValue(roiJson["name"]);
    std::string type = ZJsonParser::stringValue(roiJson["type"]);

    if (ZJsonParser::isArray(roiJson["key"])) {
      std::vector<std::string> keyList;
      ZJsonArray keyJson(roiJson.value("key"));
      for (size_t j = 0; j < keyJson.size(); ++j) {
        keyList.push_back(ZJsonParser::stringValue(keyJson.at(j)));
      }
      writer.writeRoiRef(name, keyList, type);
    } else {
      std::string key = ZJsonParser::stringValue(roiJson["key"]);
      writer.writeRoiRef(name, key, type);
    }
  }

#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "1d1d", 8000);
  ZDvidWriter writer;
  writer.open(target);
  writer.deleteKey("rois", "hkcut_34");
#endif

#if 0
  QProcess process;

  QStringList args;
  args << "python";
  process.start("which", args);

  process.waitForFinished();
  qDebug() << process.readAllStandardOutput();
#endif

#if 0
  ZPythonProcess python;

  python.runScript("/Users/zhaot/Work/neutube/neurolabi/python/testjson.py");
  python.printOutputSummary();
#endif

#if 0
  ZBrowserOpener bo;
  bo.updateChromeBrowser();
  qDebug() << bo.getBrowserPath();
//  bo.open("emdata2.int.janelia.org");
  bo.open("emdata2.int.janelia.org/neuroglancer/#!{'layers':{'grayscale':{'type':'image'_'source':'dvid://http://127.0.0.1:8000/a89e/grayscalejpeg'}}_'navigation':{'pose':{'position':{'voxelSize':[8_8_8]_'voxelCoordinates':[12105_7894_11286]}}_'zoomFactor':8}_'perspectiveOrientation':[-0.3254986107349396_0.3294858932495117_-0.10295828431844711_0.8802781701087952]_'perspectiveZoom':64}");

//  bo.open("emdata2.int.janelia.org/neuroglancer/#!{'\"'\"'layers'\"'\"':{'\"'\"'grayscale'\"'\"':{'\"'\"'type'\"'\"':'\"'\"'image'\"'\"'_'\"'\"'source'\"'\"':'\"'\"'dvid://http://emdata2.int.janelia.org:80/e932474289bc4b048b001d2aa0b4219c/grayscale'\"'\"'}}_'\"'\"'navigation'\"'\"':{'\"'\"'pose'\"'\"':{'\"'\"'position'\"'\"':{'\"'\"'voxelSize'\"'\"':[8_8_8]_'\"'\"'voxelCoordinates'\"'\"':[9280_5408_11264]}}_'\"'\"'zoomFactor'\"'\"':8}_'\"'\"'perspectiveOrientation'\"'\"':[-0.12320887297391891_0.2175416201353073_-0.00949245784431696_0.9681968092918396]_'\"'\"'perspectiveZoom'\"'\"':64}");
#endif

#if 0
  ZDvidTarget target("emdata3.int.janelia.org", "877d", 8300);
  target.setLabelBlockName("prelim_001_8nm-base");
  ZDvidReader reader;
  if (reader.open(target)) {
    ZObject3dScan *obj = reader.readMultiscaleBody(1319932995, 0, true, NULL);
    obj->save(GET_TEST_DATA_DIR + "/test.sobj");
    delete obj;
  }
#endif

#if 0

  ZSparseStack spStack;
  spStack.load(GET_BENCHMARK_DIR + "/test.zss");

  ZSparseStack *stack2 = spStack.downsample(1, 1, 1);
  stack2->save(GET_TEST_DATA_DIR + "/test.zss");

  ZObject3d seed;
  ZStackWatershedContainer container2(stack2);
  ZObject3dFactory::MakeBoxObject3d(
        ZIntCuboid(4266, 4484, 7264, 4267, 4486, 7265), &seed);
  seed.setLabel(1);
  container2.addSeed(seed);

  ZObject3dFactory::MakeBoxObject3d(
        ZIntCuboid(4289, 4490, 7264, 4290, 4491, 7265), &seed);
  seed.setLabel(2);
  container2.addSeed(seed);

  container2.run();
  ZObject3dScanArray *result = container2.makeSplitResult(1, NULL);

  ZStack *labelStack = result->toColorField();
  labelStack->save(GET_TEST_DATA_DIR + "/test2.tif");
  delete labelStack;


  ZStackWatershedContainer container(&spStack);
  ZObject3dFactory::MakeBoxObject3d(
        ZIntCuboid(4266, 4484, 7264, 4267, 4486, 7265), &seed);
  seed.setLabel(1);
  container.addSeed(seed);

  ZObject3dFactory::MakeBoxObject3d(
        ZIntCuboid(4289, 4490, 7264, 4290, 4491, 7265), &seed);
  seed.setLabel(2);
  container.addSeed(seed);

  container.addResult(container2.getResult());

  container.refineBorder();
  container.makeSplitResult(1, result);


  labelStack = result->toColorField();
  labelStack->save(GET_TEST_DATA_DIR + "/test.tif");
  delete labelStack;


#endif

#if 0
  ZStack stack;
//  stack.load(GET_BENCHMARK_DIR + "/sphere_bw.tif");
  stack.load(GET_BENCHMARK_DIR + "/binary/3d/diadem_e1.tif");

  ZMesh mesh;
  ZMarchingCube::March(stack, &mesh);

  mesh.save((GET_TEST_DATA_DIR + "/test.obj").c_str());
#endif

#if 0
  ZIntCuboid box;
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(10, 20, 30);

  std::vector<bool> visible;
  visible.resize(6, true);
  visible[0] = false;
  visible[2] = false;
  visible[4] = false;

  ZMesh mesh = ZMesh::CreateCuboidFaceMesh(box, visible, QColor(255, 0, 0));

  std::cout << "#vertices: " << mesh.numVertices() << std::endl;
  std::cout << "#Indices: " << mesh.indices().size() << std::endl;
//  ZDebugPrintArrayG(mesh.indices(), 0, 36);
//  mesh.save((GET_TEST_DATA_DIR + "/test.obj").c_str());

  QByteArray data = mesh.writeToMemory("obj");
  std::cout << data.toStdString() << std::endl;

  std::ofstream stream((GET_TEST_DATA_DIR + "/test.obj").c_str());
  stream.write(data.constData(), data.size());
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setLabelBlockName("labels");
  ZDvidWriter writer;
  writer.open(target);

  ZIntCuboid box;
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(10, 20, 30);

  std::vector<bool> visible;
  visible.resize(6, true);
  visible[0] = false;
  visible[2] = false;
  visible[4] = false;

  ZMesh mesh = ZMesh::CreateCuboidFaceMesh(box, visible, QColor(255, 0, 0));

  writer.writeMesh(mesh, 2, 0);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setLabelBlockName("labels");
  target.setBodyLabelName("bodies");

  ZFlyEmMisc::RemoveSplitTask(target, 1);
#endif

#if 0

#if defined(_USE_WEBENGINE_)
  QWebEngineView *view = new QWebEngineView(NULL);
//  view->setUrl(QUrl("https://janeliascicomp.github.io/SharkViewer/"));
  QUrl url("http://emdata1.int.janelia.org:8500/neuroglancer/#!"
           "{'layers':{'grayscale':{"
           "'type':'image'_'source':"
           "'dvid://http://emdata1.int.janelia.org:8500/babdf6dbc23e44a69953a66e2260ff0a/grayscale'}_"
           "'labels3':{'type':'segmentation'_"
           "'source':'dvid://http://emdata1.int.janelia.org:8500/"
           "babdf6dbc23e44a69953a66e2260ff0a/labels3'_'segments':['100032978448']}}"
           "_'navigation':{'pose':{'position':{'voxelSize':[8_8_8]_'voxelCoordinates':[4032_5600_7936]}}_'zoomFactor':8}_'perspectiveOrientation':[-0.12320887297391891_0.2175416201353073_-0.00949245784431696_0.9681968092918396]_'perspectiveZoom':64}");
  view->setUrl(url);
  view->show();
#endif

#if defined(_USE_WEBKIT_)
  QWebView view;
  QUrl url("http://emdata1.int.janelia.org:8500/neuroglancer/#!{'layers':{'grayscale':{'type':'image'_'source':'dvid://http://emdata1.int.janelia.org:8500/babdf6dbc23e44a69953a66e2260ff0a/grayscale'}_'labels3':{'type':'segmentation'_'source':'dvid://http://emdata1.int.janelia.org:8500/babdf6dbc23e44a69953a66e2260ff0a/labels3'}}_'navigation':{'pose':{'position':{'voxelSize':[8_8_8]_'voxelCoordinates':[4032_5600_7936]}}_'zoomFactor':8}_'perspectiveOrientation':[-0.12320887297391891_0.2175416201353073_-0.00949245784431696_0.9681968092918396]_'perspectiveZoom':64}");
  view->setUrl(url);
  view->show();
#endif


#endif

#if 0
  lowtis::DVIDLabelblkConfig config;

  config.username = "sample";
  config.dvid_server = "emdata2.int.janelia.org:8300";
  config.dvid_uuid = "1236";
  config.datatypename = "base20180227_8nm_watershed_fixed";

  // create service for 2D image fetching
  lowtis::ImageService service(config);


  // fetch image from 0,0,0 into buffer
  int width = 1024; int height = 1024;
  std::vector<int> offset(3,0);
  offset[0] = 17152;
  offset[1] = 22592;
  offset[2] = 19328;

  char* buffer = new char[width*height*8];
  service.retrieve_image(width, height, offset, buffer);
#endif

#if 0
  // create dvid config
  lowtis::DVIDLabelblkConfig config;
  config.username = "sample";
  config.dvid_server = "emdata1.int.janelia.org:8500";
  config.dvid_uuid = "b6bc";
  config.datatypename = "labels";
  //config.centercut = std::tuple<int, int>(256, 256);
  // create service for 2D image fetching
  lowtis::ImageService service(config);
  std::vector<int> center(3,0);
  center[0] = 4003;
  center[1] = 5607;
  center[2] = 7312;
  std::vector<double> dim1(3,0);
  std::vector<double> dim2(3,0);
  dim1[0] = 1;
  dim2[1] = 1;
  int width = 1024; int height = 1024;
  char* buffer = new char[width*height*8];
  service.retrieve_arbimage(1024, 1024, center, dim1, dim2, buffer, 0, false);
  std::cout << "success" << std::endl;

  uint64_t *array = (uint64_t*) buffer;
  for (size_t i = 0; i < 10; ++i) {
    std::cout << array[i] << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setSegmentationName("labels");
  ZDvidReader reader;
  reader.open(target);

//  ZArray *array = reader.readLabels64Lowtis(4003, 5607, 7312, 1024, 1024, 1);

  ZArray *array = reader.readLabels64Lowtis(
        4003, 5607, 7312, 1, 0, 0, 0, 1, 0, 1024, 1024, 0);
  array->printInfo();

  delete array;
  /*
   * {
      "address": "emdata1.int.janelia.org",
      "port": 8500,
      "uuid": "b6bc",
      "name": "MB_Test",
      "comment": "MB seg (branched from febc)",
      "body_label": "bodies",
      "label_block": "labels",
      "roi": "mb_subtracted",
      "user_name": "zhaot"
    }
  */
#endif

#if 0
  //testing labelmap
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "1236", 8700);
  target.setSegmentationName("base20180227_8nm_watershed_fixed");

  ZDvidReader reader;
  reader.open(target);
#endif

#if 0
  ZArray *array = reader.readLabels64Lowtis(
        17152, 22592, 19328, 1024, 1024, 1);
  array->printInfo();
//  array->print();
  std::cout << "Value: " << array->getUint64Value(0) << std::endl;

  uint64_t bodyId =
      reader.readBodyIdAt(17152 + 256, 22592 + 256, 19328 + 100);
  std::cout << "Body ID: " << bodyId << std::endl;

  delete array;
#endif

#if 0
  uint64_t bodyId = 769263962;
  ZObject3dScan body;
  reader.readCoarseBody(bodyId, &body);
  body.save(GET_TEST_DATA_DIR + "/test.sobj");

  ZMesh *mesh = ZMeshFactory::MakeMesh(body);
  mesh->save(GET_TEST_DATA_DIR + "/test.obj");

  std::cout << reader.getDvidTarget().hasCoarseSplit() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleName("grayscalejpeg");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxGrayscaleZoom();

  {
    ZStack *stack = reader.readGrayScaleLowtis(
          17216, 19872, 20704, 1, 0, 0, 0, 1, 0, 1024, 1024, 0, 256, 256);

    stack->save(GET_TEST_DATA_DIR + "/test.tif");
  }

  {
    ZStack *stack = reader.readGrayScaleLowtis(
          17216, 19872, 20704, 1, 0, 0, 0, 1, 0, 1024, 1024, 1, 256, 256);

    stack->save(GET_TEST_DATA_DIR + "/test2.tif");
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleName("grayscalejpeg");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxGrayscaleZoom();

  int x = 17212;
  int y = 19872;
  int z = 20704;

  std::vector<std::vector<double> > ds;



  for (int dy = 0; dy < 1; ++dy) {
    std::vector<double> sds;
    for (int dx = 0; dx < 8; ++dx) {
      ZStack *stack = reader.readGrayScaleLowtis(
            x + dx, y + dy, z, 1024, 1024, 1, 256, 256, true);
      ZStack *stack2 = reader.readGrayScaleLowtis(
            x + dx, y + dy, z, 1024, 1024, 1, 256, 256, false);

      size_t v = stack->getVoxelNumber();
      double d = 0;
      for (size_t i = 0; i < v; ++i) {
        d += std::abs(stack->getIntValue(i) - stack2->getIntValue(i));
      }
      delete stack;
      delete stack2;

      sds.push_back(d);
    }
    ds.push_back(sds);
  }

  for (const auto &sds : ds) {
    for (double d : sds) {
      std::cout << d << " ";
    }
    std::cout << std::endl;
  }
//    stack->save(GET_TEST_DATA_DIR + "/_test2.tif");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleName("grayscalejpeg");

  ZFlyEmArbMvc *mvc = ZFlyEmArbMvc::Make(target);

  ZArbSliceViewParam viewParam;
  viewParam.setSize(1024, 1024);
  viewParam.setCenter(17216, 19872, 20704);
  ZPoint v1 = ZPoint(1, 0, 0);
  ZPoint v2 = ZPoint(0, 1, 0);

  viewParam.setPlane(v1, v2);
  mvc->show();
  mvc->updateViewParam(viewParam);
#endif


#if 0
  ZDvidGraySlice slice;

  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleName("grayscalejpeg");
  ZDvidReader reader;
  reader.open(target);

  reader.updateMaxGrayscaleZoom();

  slice.setDvidTarget(reader.getDvidTarget());
  slice.setSliceAxis(neutube::A_AXIS);

  ZArbSliceViewParam viewParam;
  viewParam.setSize(1024, 1024);
  viewParam.setCenter(17216, 19872, 20704);
  ZPoint v1 = ZPoint(1, 0, 0);
  ZPoint v2 = ZPoint(0, 1, 0);

  viewParam.setPlane(v1, v2);
  slice.update(viewParam);
  slice.saveImage(GET_TEST_DATA_DIR + "/test.tif");

  v1.rotate(0.1, 0.2);
  v2.rotate(0.1, 0.2);
  viewParam.setPlane(v1, v2);

  slice.update(viewParam);

  /*
  ZStackViewParam param;
  param.setSliceAxis(neutube::A_AXIS);
  param.setWidgetRect(QRect(0, 0, 1024, 1024));
  param.setCanvasRect(QRect(17216, 19872, 1024, 1024));
  param.setViewPort(17216, 19872, 17216 + 1023, 19872 + 1023);
  param.setZ(20704);
  */

//  slice.setArbitraryAxis(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
//  slice.update(param);

  slice.savePixmap(GET_TEST_DATA_DIR + "/test2.tif");

  /*
  QPixmap canvas(512, 512);
  QPainter painter(&canvas);
  painter.drawImage(QRect(0, 0, 512, 512), slice.getImage());

  canvas.save((GET_TEST_DATA_DIR + "/test2.tif").c_str());
  */

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setSegmentationName("labels");
  target.setBodyLabelName("bodies");

  QFuture<ZDvidReader*> future = QtConcurrent::run(
        [&](const ZDvidTarget &target) {
    ZDvidReader *reader = new ZDvidReader;
    reader->open(target);
    return reader;
  }, target);

  future.waitForFinished();

  ZDvidReader *reader = future.result();
  std::cout << reader->hasData("labels") << std::endl;
  std::cout << reader->hasData("labels10") << std::endl;
  std::cout << reader->hasCoarseSparseVolume(1) << std::endl;
  std::cout << reader->hasCoarseSparseVolume(2) << std::endl;
  std::cout << reader->hasCoarseSparseVolume(12345678) << std::endl;

#endif

#if 0
  ZContrastProtocol protocol;
  protocol.setScale(5.2);
  protocol.setOffset(-0.5);
  protocol.setNonlinear(ZContrastProtocol::NONLINEAR_SIGMOID);

//  std::cout << protocol.mapFloat(1.0) << std::endl;
  for (int i = 0; i < 256; ++i) {
    std::cout << i << " -> " << int(protocol.mapGrey(i)) << std::endl;
  }
#endif

#if 0
  ZDvidTarget target;
  target.setFromUrl("http://emdata2.int.janelia.org:8700/api/node/0667");
  target.setSegmentationName("segmentation");
  target.print();

  ZDvidWriter writer;
  writer.open(target);
  writer.syncAnnotationToLabel("segmentation_todo");

#endif

#if 0
  host->runNeuTuPaper();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setSegmentationName("labels");
  target.setGrayScaleName("grayscale");

  ZDvidLabelSlice slice;
  slice.setSliceAxis(neutube::X_AXIS);
  slice.setDvidTarget(target);

  ZStackViewParam viewParam;
  viewParam.setCanvasRect(QRect(0, 0, 10000, 20000));
  viewParam.setWidgetRect(QRect(0, 0, 100, 100));
  viewParam.setViewPort(QRect(7286, 5630, 512, 512), 3619);

  viewParam.setSliceAxis(neutube::X_AXIS);

  slice.update(viewParam);
  slice.getPaintBuffer()->save(
        QString::fromStdString(GET_TEST_DATA_DIR+"/test.tif"));
#endif

#if 0
  ZDvidLabelSlice slice;
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "b6bc", 8500);
  target.setSegmentationName("labels");
  target.setGrayScaleName("grayscale");

  slice.setDvidTarget(target);

  ZArbSliceViewParam sliceViewParam;
  sliceViewParam.setSize(512, 512);
  sliceViewParam.setCenter(3079, 6798, 3120);
//  sliceViewParam.setCenter(4003, 5607, 7312);
  ZPoint v1 = ZPoint(0, 0, 1);
  ZPoint v2 = ZPoint(1, 0, 0);
  sliceViewParam.setPlane(v1, v2);

  ZStackViewParam viewParam;
  viewParam.setCanvasRect(QRect(0, 0, 10000, 20000));
  viewParam.setWidgetRect(QRect(0, 0, 100, 100));
  viewParam.setSliceAxis(neutube::A_AXIS);
  viewParam.setArbSliceView(sliceViewParam);
  /*
  viewParam.setCanvasRect(QRect(0, 0, 10000, 20000));
  viewParam.setWidgetRect(QRect(0, 0, 100, 100));
  viewParam.setViewPort(QRect(3079, 5798, 512, 512));
  viewParam.setZ(3120);
  qDebug() <<  viewParam.getViewPort();
  */
  slice.setSliceAxis(neutube::A_AXIS);
  slice.update(viewParam);

//  slice.paintBuffer();
  slice.getPaintBuffer()->save(QString::fromStdString(GET_TEST_DATA_DIR+"/test.tif"));

#endif

#if 0
  ZWidgetMessage msg = ZWidgetMessageFactory("test").
      to(ZWidgetMessage::TARGET_CUSTOM_AREA).
      as(neutube::EMessageType::MSG_WARNING).
      title("Test Title");
  qDebug() << msg.getTitle();
  qDebug() << msg.toHtmlString();
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/swc/dynamic.swc");
  tree.forceVirtualRoot();
//  Swc_Tree_Node *tn = SwcTreeNode::makePointer(ZPoint(0, 0, 0), 1);
//  SwcTreeNode::setParent(tn, tree.root());
  std::vector<zswc::Swc_Tree_Node_Pair> pairList;
  std::vector<Swc_Tree_Node*> nodeList = zswc::Decompose(&tree, &pairList);
  std::for_each(nodeList.begin(), nodeList.end(), [](Swc_Tree_Node *tn) {
    SwcTreeNode::print(tn);
  });

  std::for_each(pairList.begin(), pairList.end(),
                [](const zswc::Swc_Tree_Node_Pair &p) {
    std::cout << SwcTreeNode::id(p.first) << "->" << SwcTreeNode::id(p.second)
              << std::endl;
  });

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1", "1d1d", 8100);

  ZMeshFactory mf;
  mf.setSmooth(3);

  ZDvidReader reader;
  if (reader.open(target)) {
    ZDvidWriter writer;
    ZDvidTarget roiTarget;
    roiTarget.set("emdata2", "5f98", 8900);

    if (writer.open(roiTarget)) {
      auto roiList = reader.readDataInstances("roi");
      for (const auto &roiName : roiList) {
        std::cout << "Processing " << roiName;
        if (!writer.getDvidReader().hasData(roiName)) {
          std::cout << "Generating mesh ..." << std::endl;
          ZObject3dScan obj = reader.readRoi(roiName);
          if (!obj.isEmpty()) {
            writer.createData("roi", roiName);
            writer.writeRoi(obj, roiName);
          } else {
            std::cout << "Failed to load ROI: " << roiName;
          }
        }
      }
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1", "1d1d", 8100);

  ZMeshFactory mf;
  mf.setSmooth(3);

  ZDvidReader reader;
  if (reader.open(target)) {
    ZDvidWriter writer;
    ZDvidTarget roiTarget;
    roiTarget.set("emdata2", "5f98", 8900);

    if (writer.open(roiTarget)) {
      auto roiList = reader.readDataInstances("roi");
      for (const auto &roiName : roiList) {
        std::cout << "Processing " << roiName;
        std::string filePath =
            GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/rois_smooth2/" +
            roiName + ".obj";
        QFileInfo fileInfo(filePath.c_str());
        if (!fileInfo.exists()) {
          std::cout << "Generating mesh ..." << std::endl;
          ZObject3dScan obj = reader.readRoi(roiName);
          if (!obj.isEmpty()) {
            ZMesh *mesh = mf.makeMesh(obj);
            if (mesh != NULL) {
              mesh->save(filePath);
              delete mesh;
            } else {
              std::cout << "Failed to create mesh: " << roiName;
            }
          } else {
            std::cout << "Failed to load ROI: " << roiName;
          }
        }
        writer.uploadRoiMesh(filePath, roiName);
      }
    }
  }
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "fb02", 8900);

  ZJsonObject jsonObj;
  jsonObj.decodeString("{\"Pos\":[23620,30590,20736],\"Kind\":\"Note\","
                       "\"Tags\":[\"user:taggl\"],"
                       "\"Prop\":{\"body ID\":\"1144018233\","
                       "\"comment\":\"Interesting Contralateral Neuron \","
                       "\"custom\":\"1\",\"status\":\"\",\"time\":\"\","
                       "\"type\":\"Other\",\"user\":\"taggl\"},\"Supervoxel\":0}");
  ZDvidWriter *writer = ZGlobal::GetDvidWriter(target);
  writer->writePointAnnotation("bookmark_annotations", jsonObj);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetDvidReader("MB_Test");
  ZDvidSparseStack stack;
  stack.setDvidTarget(reader->getDvidTarget());
  stack.loadBody(123456);
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "9524", 8700);
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");

  ZDvidNode node;
  node.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleSource(node);

  ZDvidSparseStack stack;
  stack.setDvidTarget(target);
  stack.setLabelType(flyem::LABEL_SUPERVOXEL);

  stack.loadBody(987648131);
//  stack.loadBody(1479647609);

  ZSparseStack *ss = stack.getSparseStack();
  ss->save(GET_TEST_DATA_DIR + "/test.zss");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "9524", 8700);
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");

  ZDvidNode node;
  node.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleSource(node);

  ZDvidReader reader;
  reader.open(target);

  ZDvidSparseStack *stack =
      reader.readDvidSparseStack(987648131, flyem::LABEL_SUPERVOXEL);
  ZSparseStack *ss = stack->getSparseStack();
  ss->save(GET_TEST_DATA_DIR + "/test.zss");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "9524", 8700);
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");

  ZDvidNode node;
  node.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleSource(node);

  ZDvidReader reader;
  reader.open(target);

  std::vector<uint64_t> bodyList = {
    5812980779, 5812980781, 5812980782
  };

  for (uint64_t bodyId : bodyList) {
    ZObject3dScan obj;
    reader.readBody(bodyId, flyem::LABEL_SUPERVOXEL, true, &obj);
    if (!obj.isEmpty()) {
      ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
      mesh->save(GET_TEST_DATA_DIR + "/tmp/sp/" + std::to_string(bodyId) + ".obj");
      delete mesh;
    }
  }
#endif

#if 0
  {
    ZDvidTarget target;
    target.set("emdata2.int.janelia.org", "9524", 8700);
    target.setSegmentationName("segmentation");
    target.setGrayScaleName("grayscalejpeg");

    ZDvidNode node;
    node.set("emdata3.int.janelia.org", "a89e", 8600);
    target.setGrayScaleSource(node);

    ZDvidReader reader;
    reader.open(target);

    std::vector<uint64_t> bodyList = {
      5812980925, 5812980927, 5812980928
    };

    for (uint64_t bodyId : bodyList) {
      ZObject3dScan obj;
      reader.readBody(bodyId, flyem::LABEL_SUPERVOXEL, true, &obj);
      if (!obj.isEmpty()) {
        ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
        mesh->save(GET_TEST_DATA_DIR + "/tmp/sp/" + std::to_string(bodyId) + ".obj");
        delete mesh;
      }
    }
  }

  {
    ZDvidTarget target;
    target.set("emdata2.int.janelia.org", "f5bc", 8700);
    target.setSegmentationName("segmentation");
    target.setGrayScaleName("grayscalejpeg");

    ZDvidReader reader;
    reader.open(target);
    ZObject3dScan obj;
    uint64_t bodyId = 1388969101;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "f5bc", 8700);
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");

  ZDvidReader reader;
  reader.open(target);
  ZObject3dScan obj;
  uint64_t bodyId = 1233492629;
  reader.readBody(bodyId, flyem::LABEL_SUPERVOXEL, true, &obj);
  if (!obj.isEmpty()) {
    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save(GET_TEST_DATA_DIR + "/tmp/sp/" + std::to_string(bodyId) + ".obj");
    delete mesh;
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "0667", 8700);
  target.setGrayScaleSource(ZDvidNode("emdata3.int.janelia.org", "a89e", 8600));
  target.setGrayScaleName("grayscalejpeg");
  target.setSegmentationName("segmentation");

  ZDvidReader reader;
  if (reader.open(target.getGrayScaleTarget())) {
    ZPoint center;
    center.set(25940, 24740, 20389);
    ZPoint v1(1, 0, 0);
    ZPoint v2(0, 1, 2);
    v2.normalize();

    int width = 25200 / 10;
    int height = 17400 / 10;

    ZAffineRect rect = ZAffineRectBuilder(width, height).at(center).on(v1, v2);

    int row = 10;
    int col = 10;
    std::vector<ZAffineRect> rectArray = zgeom::Partition(rect, row, col);

    ZStackArray stackArray;

    int index = 0;
    int dy = 0;
    for (int i = 0; i < row; ++i) {
      int dx = 0;
      int height = rectArray[index].getHeight();
      for (int j = 0; j < col; ++j) {
        const ZAffineRect &rect = rectArray[index];
        ZStack *stack = reader.readGrayScaleLowtis(rect, 0, 0, 0);
//            ZFlyEmMisc::MakeColorSegmentation(reader, rect);
        std::cout << "Offset: " << dx << " " << dy << std::endl;
        stack->setOffset(dx, dy, 0);
        stackArray.append(stack);
        dx += rect.getWidth();
        ++index;
      }
      dy += height;
    }

    ZStack *stack = ZStackFactory::CompositeForeground(stackArray);

    stack->save(GET_TEST_DATA_DIR + "/_flyem/art/gray_small.tif");
  }
#endif

#if 0
  ZStack grayscale;
  grayscale.load(GET_TEST_DATA_DIR + "/_flyem/art/gray.tif");

  ZStack segmentation;
  segmentation.load(GET_TEST_DATA_DIR + "/_flyem/art/seg.tif");

  ZStack *cgray = ZStackFactory::MakeZeroStack(GREY, grayscale.getBoundBox(), 3);
  for (int c = 0; c < 3; ++c) {
    cgray->copyValueFrom(grayscale.array8(), grayscale.getVoxelNumber(), c);
  }

  ZStackBlender blender;
  blender.setBlendingMode(ZStackBlender::BLEND_NO_BLACK);
  ZStack *out = blender.blend(*cgray, segmentation, 0.5);

  out->save(GET_TEST_DATA_DIR + "/_flyem/art/blend.tif");

#endif

#if 1
  ZStack grayscale;
  grayscale.load(GET_TEST_DATA_DIR + "/_system/diadem/diadem_e1.tif");

  ZStack segmentation;
  segmentation.load(GET_TEST_DATA_DIR + "/_system/diadem/diadem_e1/bin.tif");

  uint8_t *array = segmentation.array8();
  for (size_t i = 0; i < segmentation.getVoxelNumber(); ++i) {
    if (array[i] > 0) {
      array[i] = 255;
    }
  }

  ZStack *cgray = ZStackFactory::MakeZeroStack(GREY, grayscale.getBoundBox(), 3);
  ZStack *cmask = ZStackFactory::MakeZeroStack(GREY, segmentation.getBoundBox(), 3);

  for (int c = 0; c < 3; ++c) {
    cgray->copyValueFrom(grayscale.array8(), grayscale.getVoxelNumber(), c);
  }
  cmask->copyValueFrom(segmentation.array8(), segmentation.getVoxelNumber(), 0);

  ZStackBlender blender;
  blender.setBlendingMode(ZStackBlender::BLEND_NO_BLACK);
  ZStack *out = blender.blend(*cgray, *cmask, 0.5);

  out->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "0667", 8700);
  target.setGrayScaleSource(ZDvidNode("emdata3.int.janelia.org", "a89e", 8600));
  target.setGrayScaleName("grayscalejpeg");
  target.setSegmentationName("segmentation");

  ZDvidReader reader;
  if (reader.open(target)) {
    ZPoint center;
    center.set(25940, 24740, 20389);
    ZPoint v1(1, 0, 0);
    ZPoint v2(0, 1, 2);
    v2.normalize();

    int width = 25200 / 10;
    int height = 17400 / 10;

    ZAffineRect rect = ZAffineRectBuilder(width, height).at(center).on(v1, v2);

    int row = 10;
    int col = 10;
    std::vector<ZAffineRect> rectArray = zgeom::Partition(rect, row, col);

    ZStackArray stackArray;

    int index = 0;
    int dy = 0;
    for (int i = 0; i < row; ++i) {
      int dx = 0;
      int height = rectArray[index].getHeight();
      for (int j = 0; j < col; ++j) {
        const ZAffineRect &rect = rectArray[index];
        ZStack *stack = ZFlyEmMisc::MakeColorSegmentation(reader, rect);
        std::cout << "Offset: " << dx << " " << dy << std::endl;
        stack->setOffset(dx, dy, 0);
        stackArray.append(stack);
        dx += rect.getWidth();
        ++index;
      }
      dy += height;
    }

    ZStack *stack = ZStackFactory::CompositeForeground(stackArray);

#if 0
    ZObjectColorScheme colorScheme;
    colorScheme.setColorScheme(ZColorScheme::CONV_RANDOM_COLOR);
    ZArray *array = reader.readLabels64Lowtis(center, v1, v2, width, height, 0);
    uint64_t *labelArray = array->getDataPointer<uint64_t>();

    ZStack *stack = ZStackFactory::MakeZeroStack(
          GREY, misc::GetBoundBox(array), 3);
//    ZStack *stack = new ZStack(COLOR, width, height, 1, 1);
    uint8_t *array1 = stack->array8(0);
    uint8_t *array2 = stack->array8(1);
    uint8_t *array3 = stack->array8(2);

    size_t area = width * height;
    for (size_t i = 0; i < area; ++i) {
      QColor color = colorScheme.getColor(labelArray[i]);
      array1[i] = color.red();
      array2[i] = color.green();
      array3[i] = color.blue();
    }
#endif
    stack->save(GET_TEST_DATA_DIR + "/_flyem/art/seg_small.tif");

//    ZDvidReader reader2;
//    reader2.open(target.getGrayScaleTarget());
//    ZStack *stack =
//        reader2.readGrayScaleLowtis(center, v1, v2, width/2, height/2, 0, 0, 0);
//    stack->save(GET_TEST_DATA_DIR + "/test.tif");
  }
#endif

#if 0
  ZMenuConfig config;
  config << "test" << ZActionFactory::ACTION_ABOUT
         << ZActionFactory::ACTION_ACTIVATE_LOCATE;
  config << "test2" << ZActionFactory::ACTION_ACTIVATE_TODO_ITEM
         << ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM;
  config << ZActionFactory::ACTION_ADD_SPLIT_SEED
         << ZActionFactory::ACTION_ADD_SWC_NODE;
  config << "test3" << ZActionFactory::ACTION_ACTIVATE_TODO_ITEM
         << ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM;
  config << "" << ZActionFactory::ACTION_ACTIVATE_TODO_ITEM
         << ZActionFactory::ACTION_ACTIVATE_TOSPLIT_ITEM;
  std::cout << config << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetDvidReader("labelmap_test");
  reader->updateMaxLabelZoom();
  reader->setVerbose(false);
  QElapsedTimer timer;
  timer.start();

  int z = 10256;
  for (size_t i = 0; i < 100; ++i) {
      ZArray *array = reader->readLabels64Lowtis(
            10300, 20351, z + i, 1024, 1024, 0);
      delete array;
  }

  std::cout << "Time elapsed: " << timer.elapsed() << "ms" << std::endl;
#endif

#if 0
  ZGlobalDvidRepo &repo = ZGlobalDvidRepo::GetInstance();
//  repo.init();

  for (auto e : repo) {
    std::cout << e.first << ": " << e.second.getSourceString() << std::endl;
//    e.second.print();
  }

#endif

#if 0
  std::pair<int,int> centerCut = GET_FLYEM_CONFIG.getCenterCut("grayscale");
  std::cout << "Center cut: " << centerCut.first << " " << centerCut.second << std::endl;

  GET_FLYEM_CONFIG.setCenterCut("grayscale", 512, 512);
  centerCut = GET_FLYEM_CONFIG.getCenterCut("grayscale");
  std::cout << "Center cut: " << centerCut.first << " " << centerCut.second << std::endl;

  centerCut = GET_FLYEM_CONFIG.getCenterCut("segmentation");
  std::cout << "Center cut: " << centerCut.first << " " << centerCut.second << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetDvidReader("test");
  reader->updateMaxLabelZoom();
//    reader->setLabelCenterCut(256, 256);
  {
    ZStack *stack = ZFlyEmMisc::MakeColorSegmentation(
          *reader, 10300, 20351, 10256, 1024, 1024, 0, 256, 256);
    stack->save(GET_TEST_DATA_DIR + "/_test.tif");
  }

  {
    ZStack *stack = ZFlyEmMisc::MakeColorSegmentation(
          *reader, 10300, 20351, 10256, 1024, 1024, 0, 0, 0);
    stack->save(GET_TEST_DATA_DIR + "/_test2.tif");
  }

  {
    ZStack *stack = ZFlyEmMisc::MakeColorSegmentation(
          *reader, 10300, 20351, 10256, 1024, 1024, 0, 1024, 1024);
    stack->save(GET_TEST_DATA_DIR + "/_test3.tif");
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "017a", 8900);
  target.setSegmentationName("segmentation");

  if (reader.open(target)) {
    QByteArray payload("[1166130438]");
    ZJsonArray jsonArray = reader.readJsonArray(
          "http://emdata2.int.janelia.org:8700/api/node/0667/segmentation/mapping",
          payload);
    jsonArray.print();

    uint64_t bodyId = reader.readBodyIdAt(15994, 23934, 20696);
    std::cout << bodyId << std::endl;

    uint64_t spId = reader.readSupervoxelIdAt(15994, 23934, 20696);
    std::cout << spId << std::endl;
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "017a", 8900);
  target.setSegmentationName("segmentation");

  if (reader.open(target)) {
    std::cout << "parent id:" << reader.readParentBodyId(1166130438) << std::endl;
  }
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "f", 7000);
  target.setSegmentationName("segmentation");
  reader.open(target);
#endif

#if 0
  ZDvidReader reader;
  reader.open("emdata2.int.janelia.org", "059e", 7000);
  ZObject3dScan obj = reader.readRoi("seven_column_roi");
  std::cout << "Voxel count: " << obj.getVoxelNumber() << std::endl;
  std::cout << "Volume: " << obj.getVoxelNumber() * 0.32 * 0.32 * 0.32 << "um^3" << std::endl;
#endif

#if 0
  ZDvidReader reader;
  reader.open("emdata1.int.janelia.org", "babd", 8500);

  ZObject3dScan obj1 = reader.readRoi("ventral_roi");
  ZObject3dScan obj2 = reader.readRoi("ventral_roi_patch");

  ZObject3dScan obj3;
  obj3.load(GET_TEST_DATA_DIR + "/_flyem/MB/large_outside_block_fixed.sobj");

  obj1.concat(obj2);
  obj1.concat(obj3);
  obj1.canonize();

  obj1.save(GET_TEST_DATA_DIR + "/test.sobj");
  std::cout << "Volume: " << obj1.getVoxelNumber() * 0.032 * 0.032 * 0.032 * 8 * 8 * 8
            << "um^3" << std::endl;



#endif

#if 0
  ZStack stack;
  stack.load(GET_BENCHMARK_DIR + "/em_slice.tif");
  ZStackWatershedContainer container(&stack);
  ZObject3dScan obj;
  obj.addSegment(3201, 4102, 2900, 2905);
  obj.setLabel(1);

  ZObject3dScan obj2;
  obj2.addSegment(3201, 4114, 3010, 3015);
  obj2.setLabel(2);

  container.addSeed(obj);
  container.addSeed(obj2);

  container.setFloodFillingZero(true);
  container.run();

  container.exportMask(GET_BENCHMARK_DIR + "/em_slice_seed.tif");
  //    result->save(GET_TEST_DATA_DIR + "/test.tif");

//  delete result;


#endif

#if 0
  ZStack stack;
  stack.load(GET_DATA_DIR + "/_system/emstack2.tif");
  ZStackWatershedContainer container(&stack);
  container.setFloodFillingZero(true);

  ZStroke2d stroke;
  stroke.setWidth(5);
  stroke.setZ(105);
  stroke.append(90, 125);
  stroke.setLabel(1);
  container.addSeed(stroke);

  ZStroke2d stroke2;
  stroke2.setWidth(5);
  stroke2.setZ(105);
  stroke2.append(103, 57);
  stroke2.setLabel(2);
  container.addSeed(stroke2);

  container.run();
  container.exportMask(GET_DATA_DIR + "/_system/emstack2_seed.tif");
#endif

#if 0
  ZStack signal;
  ZStack label;
  signal.load(GET_DATA_DIR + "/_system/emstack2.tif");
  label.load(GET_DATA_DIR + "/_system/emstack2_seed.tif");

  ZStack *result = ZStackProcessor::SeededWatershed(&signal, &label);
  result->save(GET_TEST_DATA_DIR + "/_test.tif");

  delete result;
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "0397", 8900);
  target.setSegmentationName("segmentation");
  ZDvidWriter writer;
  writer.open(target);

  writer.syncData("segmentation_todo", "segmentation", "replace=true");
#endif

#if 0
  QDir fileInfo(GET_TEST_DATA_DIR.c_str());
  fileInfo.setFile(GET_TEST_DATA_DIR, "test.tif");
  qDebug() << fileInfo.absoluteFilePath();

  fileInfo.setFile("test2.tif");
  qDebug() << fileInfo.absoluteFilePath();

  qDebug() << fileInfo.isFile();
#endif

#if 0
  {
    tic();
    std::vector<ZObject3dStripe> stripeArray;
    for (size_t i = 0; i < 100000; ++i) {
      ZObject3dStripe stripe;
      stripe.addSegment(0, 1, false);
      stripe.addSegment(3, 4, false);
      stripe.addSegment(7, 8, false);
      stripe.addSegment(11, 12, false);

      stripeArray.push_back(std::move(stripe));
    }
    ptoc();
  }

  {
    tic();
    std::vector<ZObject3dStripe> stripeArray;
    for (size_t i = 0; i < 100000; ++i) {
      ZObject3dStripe stripe;
      stripe.addSegment(0, 1, false);
      stripe.addSegment(3, 4, false);
      stripe.addSegment(7, 8, false);
      stripe.addSegment(11, 12, false);

      stripeArray.push_back(stripe);
    }
    ptoc();
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "a6dd", 8700);
  target.setSegmentationName("segmentation");
  ZDvidWriter writer;
  writer.open(target);

  writer.deleteKey(QString("segmentation_meshes"), "0", "z");
#endif

#if 0
  ZDvidTarget target;
  target.setFromUrl("http://emdata1.int.janelia.org:8900/api/node/3d5d");
  target.setSegmentationName("segmentation");
  target.print();

  ZDvidWriter writer;
  writer.open(target);
  writer.syncAnnotationToLabel("segmentation_todo");
#endif

#if 0
  ZPythonProcess proc;
  proc.printPythonPath();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "6134", 8900);
  target.setSegmentationName("segmentation");

  ZDvidReader reader;
  reader.open(target);

  ZObject3dScan obj;
  reader.readBody(853173648, true, &obj);

  std::cout << obj.getBoundBox().toString() << std::endl;
  obj.downsampleMax(15, 15, 15);
  std::cout << obj.getBoundBox().toString() << std::endl;

#endif

#if 0
  qDebug() << std::string("user:mock").substr(5);
  qDebug() << neutube::GetCurrentUserName();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  std::vector<uint64_t> bodyArray = {
    2219, 2515, 2796, 3904, 4376, 7114, 7131, 10319, 19741, 21840, 21894,
    22045, 22301, 26353, 30155, 30465, 35752, 49079, 53216, 88847, 158346,
    248553, 262667, 368415, 390989, 436097, 436607, 446884, 516159, 676018};

  for (uint64_t bodyId : bodyArray) {
    std::ofstream stream(
          GET_TEST_DATA_DIR + "/_paper/neutu_em/experiment/body_speed/" +
          std::to_string(bodyId) + ".txt",
          std::ofstream::out);

    for (int zoom = 0; zoom <= 5; ++zoom) {
      QElapsedTimer timer;
      timer.start();
      ZObject3dScan obj;
      reader.readMultiscaleBody(bodyId, zoom, true, &obj);
      ZSwcTree *tree = ZSwcFactory::CreateSurfaceSwc(obj, 1, 3);
      delete tree;
      qint64 t = timer.elapsed();
      uint64_t v = obj.getVoxelNumber();
      uint64_t bv = obj.getBoundBox().getVolume();
      std::cout << "Body updating time: " << t << std::endl;
      std::cout << "Body size: " << v << std::endl;
      stream << zoom << " " << v << " " << t << " " << bv << std::endl;
    }
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();


  ZIntPoint center(3143, 3569, 4044);

  ZObject3dScan obj;
  reader.readBody(2515, flyem::LABEL_BODY, 0,
                  ZIntCuboid(center - 512, center + 512), true, &obj);
  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "25dc", 8900);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  uint64_t bodyId = 1352434760;

  ZObject3dScan obj;
  reader.readCoarseBody(bodyId, flyem::LABEL_BODY, &obj);

  ZDvidInfo dvidInfo = reader.readDataInfo(target.getSegmentationName());
//  reader.readBody(2515, flyem::LABEL_BODY, 5, ZIntCuboid(), true, &obj);
  obj.setDsIntv(dvidInfo.getBlockSize() - 1);
  std::cout << obj.getDsIntv() << std::endl;

  ZIntPoint center(19376, 22335, 20896);
  int radius = 256;
  ZIntCuboid box(center - radius, center + radius);
  box.scaleDown(dvidInfo.getBlockSize());
  box.expand(-1, -1, -1);
  obj.remove(box);

  ZObject3dScanArray objArray;
  objArray.append(obj);

  ZObject3dScan obj2;
  reader.readBody(
        bodyId, flyem::LABEL_BODY, 0,
        ZIntCuboid(center - radius, center + radius), true, &obj2);
  objArray.append(obj2);

  tic();
  ZMesh *mesh = ZMeshFactory::MakeMesh(objArray);
  std::cout << "Merging time: " << toc() << std::endl;

  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

//  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");

#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  obj.addSegment(0, 1, 0, 1);
  obj.addSegment(1, 0, 0, 1);
  obj.addSegment(1, 1, 0, 1);

  {
//    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    ZMeshFactory mf;
    mf.setOffsetAdjust(true);
    mf.setSmooth(0);
    ZMesh *mesh = mf.makeMesh(obj);
    mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
  }

//  obj.setDsIntv(1);
  {
    ZMesh *mesh = ZMeshFactory::MakeFaceMesh(obj);
    mesh->save(GET_TEST_DATA_DIR + "/_test2.obj");
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  uint64_t bodyId = 2850569;

  if (0) {
    ZObject3dScan obj;
    reader.readCoarseBody(bodyId, flyem::LABEL_BODY, &obj);

    ZDvidInfo dvidInfo = reader.readDataInfo(target.getSegmentationName());
    obj.setDsIntv(dvidInfo.getBlockSize() - 1);
    std::cout << obj.getDsIntv() << std::endl;

    ZMesh *mesh = ZMeshFactory::MakeFaceMesh(obj);
    mesh->generateNormals();

//    ZMesh mesh2 = vtkPolyDataToMesh(meshToVtkPolyData(*mesh));
    ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);

    mesh2.save((GET_TEST_DATA_DIR + "/_test.obj").c_str());
//    mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
    delete mesh;
  }

  if (0) {
    ZObject3dScan obj;
    reader.readCoarseBody(bodyId, flyem::LABEL_BODY, &obj);

    ZDvidInfo dvidInfo = reader.readDataInfo(target.getSegmentationName());
    obj.setDsIntv(dvidInfo.getBlockSize() - 1);
    std::cout << obj.getDsIntv() << std::endl;

    ZMeshFactory mf;
    mf.setOffsetAdjust(true);
    mf.setSmooth(3);
    ZMesh *mesh = mf.MakeMesh(obj);
    mesh->generateNormals();

    ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);

    mesh2.save((GET_TEST_DATA_DIR + "/_test.obj").c_str());

//    mesh->save(GET_TEST_DATA_DIR + "/_test2.obj");
    delete mesh;
  }

  if (0) {
    ZObject3dScan obj;
    reader.readCoarseBody(bodyId, flyem::LABEL_BODY, &obj);

    ZDvidInfo dvidInfo = reader.readDataInfo(target.getSegmentationName());
    obj.setDsIntv(dvidInfo.getBlockSize() - 1);
    std::cout << obj.getDsIntv() << std::endl;

    ZMeshFactory mf;
    mf.setOffsetAdjust(false);
    mf.setSmooth(0);
    ZMesh *mesh = mf.MakeMesh(obj);

    mesh->save(GET_TEST_DATA_DIR + "/_test2.obj");
    delete mesh;
  }



  if (0) {
    ZObject3dScan obj;
    reader.readBody(bodyId, true, &obj);
//    obj.downsampleMax(15, 15, 15);
    reader.readMultiscaleBody(bodyId, 6, true, &obj);
//    obj.setDsIntv(3, 3, 3);
//    reader.readBody(2515, true, &obj);

    ZMesh *mesh = ZMeshFactory::MakeFaceMesh(obj);
//    ZMeshFactory mf;
//    mf.setSmooth(0);
//    mf.setOffsetAdjust(false);
//    ZMesh *mesh = mf.makeMesh(obj);
//    ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
    mesh->save(GET_TEST_DATA_DIR + "/_test2.obj");
    delete mesh;
  }

  if (0) {
    ZObject3dScan obj;
    reader.readBody(bodyId, true, &obj);
//    reader.readMultiscaleBody(bodyId, 2, true, &obj);
//    obj.setDsIntv(3, 3, 3);
//    reader.readBody(2515, true, &obj);
    ZMesh *mesh = ZMeshFactory::MakeFaceMesh(obj);

//    ZMeshFactory mf;
//    mf.setSmooth(0);
//    mf.setOffsetAdjust(false);
//    ZMesh *mesh = mf.MakeMesh(obj);
//          obj.getSlice(obj.getMinZ() + (obj.getMaxZ() - obj.getMinZ())/5), obj.getMaxZ());
    mesh->save(GET_TEST_DATA_DIR + "/_test3.obj");
    delete mesh;
  }


#endif

#if 0

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  ZIntPoint center(3143, 3569, 4044);
  ZIntCuboid box(center - 512, center + 512);

  ZObject3dScan obj;
  reader.readCoarseBody(2515, flyem::LABEL_BODY, box, &obj);

  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  ZObject3dScan obj;
  reader.readCoarseBody(2515, flyem::LABEL_BODY, &obj);

  ZDvidInfo dvidInfo = reader.readDataInfo(target.getSegmentationName());
//  reader.readBody(2515, flyem::LABEL_BODY, 5, ZIntCuboid(), true, &obj);
  obj.setDsIntv(dvidInfo.getBlockSize() - 1);
  std::cout << obj.getDsIntv() << std::endl;

  ZIntPoint center(3143, 3569, 4044);
  ZIntCuboid box(center - 512, center + 512);
  box.scaleDown(dvidInfo.getBlockSize());
  box.expand(-1, -1, -1);
  obj.remove(box);

  ZObject3dScanArray objArray;
  objArray.append(obj);

  ZObject3dScan obj2;
  reader.readBody(
        2515, flyem::LABEL_BODY, 0,
        ZIntCuboid(center - 512, center + 512), true, &obj2);
  objArray.append(obj2);

  tic();
  ZMesh *mesh = ZMeshFactory::MakeMesh(objArray);
  std::cout << "Merging time: " << toc() << std::endl;

  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

//  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");

#endif

#if 0
  ZObject3dScanArray objArray;
  ZObject3dScan obj;
  obj.load(GET_BENCHMARK_DIR + "/29.sobj");
  objArray.append(obj.getSlice(obj.getMinZ(), 391));

  ZObject3dScan obj2 = obj.getSlice(392, obj.getMaxZ());
  obj2.downsampleMax(3, 3, 3);
  objArray.append(obj2);

  tic();
  ZMeshFactory mf;
//  ZMesh *mesh = mf.makeMesh(objArray);
  ZMesh *mesh = mf.makeMesh(*objArray[0]);
  ZMesh *mesh2 = mf.makeMesh(*objArray[1]);
  mesh->append(*mesh2);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

  std::cout << "Merging time: " << toc() << std::endl;

  mesh->generateNormals();
  toc();
//  ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);
//  ptoc();
//  mesh2.save((GET_TEST_DATA_DIR + "/_test.obj").c_str());

  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

#endif

#if 0
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_paper/neuron_type/data_bundle.json");
  ZJsonArray neuronArrayJson(obj.value("neuron"));
  std::cout << neuronArrayJson.size() << std::endl;

  std::ofstream stream(
        GET_TEST_DATA_DIR + "/_paper/neuron_type/data/type_name.txt");
  for (size_t i = 0; i < neuronArrayJson.size(); ++i) {
    ZJsonObject neuronJson(neuronArrayJson.value(i));
    stream << ZJsonParser::stringValue(neuronJson["type"]) << std::endl;
  }
  stream.close();
#endif

#if 0
  ZJsonObject obj;
  std::string dataDir = "/groups/flyem/home/zhaot/Work/neutube_ws/neurolabi/data";
  obj.load(dataDir + "/_paper/neuron_type/data/data_bundle.json");
  ZJsonArray neuronArrayJson(obj.value("neuron"));
  std::cout << neuronArrayJson.size() << std::endl;

  ZObject3dScanArray objArray;
  std::vector<ZIntCuboid> boxArray;
  for (size_t i = 0; i < neuronArrayJson.size(); ++i) {
    ZJsonObject neuronJson(neuronArrayJson.value(i));
    std::string volumeFile =
        dataDir + "/_paper/neuron_type/data/" +
        ZJsonParser::stringValue(neuronJson["volume"]);
    QFileInfo fileInfo(volumeFile.c_str());
    if (!fileInfo.exists()) {
      std::cout << volumeFile << " missing." << std::endl;
    }
    ZObject3dScan obj;
    obj.load(volumeFile);
    objArray.append(obj);
    ZIntCuboid box = obj.getBoundBox();
    box.expand(1, 1, 0);
    boxArray.push_back(box);
  }

  std::vector<std::vector<size_t> > adjmat;
  for (size_t i = 0; i < objArray.size(); ++i) {
    std::cout << "Processing row " << i << std::endl;
    std::vector<size_t> row;
    const ZObject3dScan *obj1 = objArray[i];
    const ZIntCuboid &box1 = boxArray[i];
    for (size_t j = 0; j < i; ++j) {
      const ZIntCuboid &box2 = boxArray[j];
      if (box1.hasOverlap(box2)) {
        const ZObject3dScan *obj2 = objArray[j];
        row.push_back(misc::CountNeighborOnPlane(*obj1, *obj2));
      } else {
        row.push_back(0);
      }
    }
    row.push_back(0);
    adjmat.push_back(row);
  }

  std::ofstream stream(
        dataDir + "/_paper/neuron_type/data/adjmat.txt");

  for (size_t i = 0; i < objArray.size(); ++i) {
    for (size_t j = 0; j < objArray.size(); ++j) {
      if (i >= j) {
        stream << adjmat[i][j] << " ";
      } else {
        stream << adjmat[j][i] << " ";
      }
    }
    stream << std::endl;
  }

#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "a89e", 8600);
  target.setGrayScaleName("grayscale");
  ZDvidReader reader;
  reader.open(target);

  ZDvidInfo dvidInfo = reader.readGrayScaleInfo();
  dvidInfo.print();
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  ZIntPoint center(3143, 3569, 4044);
  ZIntCuboid box(center - 512, center + 512);

  ZDvidBodyHelper helper(&reader);
  helper.setRange(box);

  ZObject3dScan obj;
  helper.readBody(2515, &obj);

  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0

  ZDvidTarget target;
  target.set("emdata1.int.janelia.org", "2b6d", 9000);
  target.setSegmentationName("groundtruth");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  ZIntPoint center(3143, 3569, 4044);
  ZIntCuboid box(center - 512, center + 512);

  ZDvidBodyHelper helper(&reader);
  helper.setRange(box);
  helper.setCoarse(true);

//  ZObject3dScan obj;
  ZObject3dScanArray objArray = helper.readHybridBody(2515);

  ZMeshFactory mf;
  ZMesh *mesh = mf.makeMesh(*objArray[0]);
  ZMesh *mesh2 = mf.makeMesh(*objArray[1]);
  mesh->append(*mesh2);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

//  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZObject3dScanArray objArray;
  ZObject3dScan obj1;
  obj1.addSegment(0, 0, 0, 10);

  ZObject3dScan obj2;
  obj2.addSegment(5, 5, 0, 10);

  objArray.append(obj1);
  objArray.append(obj2);

  ZMeshFactory mf;
  ZMesh *mesh = mf.makeMesh(objArray);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZObject3dScan obj1;
  obj1.addSegment(0, 0, 0, 10);
  ZMesh *mesh1 = ZMeshFactory::MakeMesh(obj1);

  ZObject3dScan obj2;
  obj2.addSegment(5, 5, 0, 10);
  ZMesh *mesh2 = ZMeshFactory::MakeMesh(obj2);

  mesh1->append(*mesh2);

  mesh1->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "5421", 8900);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);
  reader.updateMaxLabelZoom();

  ZObject3dScan obj;
  reader.readBody(1539193374, true, &obj);

  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  std::cout << ZFlyEmBodyManager::encode(3, 1) << std::endl;
  std::cout << ZFlyEmBodyManager::encode(3, 1, false) << std::endl;
  std::cout << ZFlyEmBodyManager::encode(3, 2) << std::endl;
  std::cout << ZFlyEmBodyManager::encode(3, 2, false) << std::endl;
  std::cout << ZFlyEmBodyManager::encodesTar(10100000000003) << std::endl;
  std::cout << ZFlyEmBodyManager::encodesTar(100000000003) << std::endl;
  std::cout << ZFlyEmBodyManager::encodedLevel(10200000000003) << std::endl;
  std::cout << ZFlyEmBodyManager::encodedLevel(200000000003) << std::endl;
  std::cout << ZFlyEmBodyManager::encodeSupervoxel(1665033134) << std::endl;
#endif

#if 0
  std::cout << ZStackObject::GetTypeName(ZStackObject::EType::TYPE_SWC) << std::endl;
#endif


#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "69f1", 8700);
  target.setSegmentationName("segmentation");
  ZDvidWriter writer;
  writer.open(target);

  writer.syncAnnotationToLabel("segmentation_todo", "replace=true");
#endif

#if 0
  ZDvidTarget target;
  target.set("emdata3.int.janelia.org", "5421", 8900);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);

  tic();
  std::cout << reader.readBodyBlockCount(1539193374) << std::endl;
  ptoc();

  tic();
  std::cout << reader.readBodySize(1) << std::endl;
  ptoc();

  tic();
  std::cout << reader.hasBody(1539193374) << std::endl;
  ptoc();
#endif

#if 0
  QRegularExpression regexp("^(supervoxel|sv)[:\\s]*([0-9]+)",
                            QRegularExpression::CaseInsensitiveOption);
  {
  QRegularExpressionMatch match = regexp.match("sv:12345");
  qDebug() << match.hasMatch();
  qDebug() << match.captured(2);
  }
  {
    QRegularExpressionMatch match = regexp.match("supervoxel:12345");
    qDebug() << match.captured(2);
  }
  {
    QRegularExpressionMatch match = regexp.match("sv  12345");
    qDebug() << match.captured(2);
  }
  {
    QRegularExpressionMatch match = regexp.match("sv  ");
    qDebug() << match.hasMatch();
    qDebug() << match.captured(2);
  }
#endif

#if 0
  ZObject3dScan *obj = ZObject3dFactory::MakeBoxObject3dScan(
        ZIntCuboid(ZIntPoint(0, 0, 0), ZIntPoint(10, 10, 10)), NULL);
  obj->setDsIntv(31, 31, 31);
  ZObject3dScanArray objArray;
  objArray.append(obj);

  ZIntCuboid range(ZIntPoint(30, 30, 30), ZIntPoint(100, 100, 100));
  range.scaleDown(32);
  range.expand(-1, -1, -1);
  obj->remove(range);

  obj = ZObject3dFactory::MakeBoxObject3dScan(
          ZIntCuboid(ZIntPoint(30, 30, 30), ZIntPoint(100, 100, 200)), NULL);
  objArray.append(obj);

  ZMeshFactory mf;

  ZMesh *mesh = mf.makeMesh(objArray);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 0, 1);
  ZMeshFactory mf;
  mf.setSmooth(0);
  ZObject3dScanArray objArray;
  objArray.append(obj);
  ZMesh *mesh = mf.makeMesh(objArray);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");

  ZDvidBodyHelper helper(reader);
  helper.setCoarse(true);
  helper.setZoom(0);
  ZIntCuboid range;
  range.setSize(256, 256, 256);
  range.setCenter(ZIntPoint(16710, 31679, 32100));
  helper.setRange(range);
  tic();
  ZObject3dScanArray objArray = helper.readHybridBody(2229212992);
  ZMeshFactory mf;
//  mf.setSmooth(0);
  ZMesh *mesh = mf.makeMesh(objArray);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
  ptoc();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");
//  reader->getDvidTarget().setUuid("7e52");
  std::cout << "Block count: " << reader->readBodyBlockCount(770606927) << std::endl;
  std::cout << "Block count: " << reader->readBodyBlockCount(1882009576) << std::endl;
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibrain_test");
//  ZDvidTarget target;
//  target.setFromUrl("http://emdata3.int.janelia.org:8900/api/node/d59e/segmenation/sparsevol");
//  ZDvidWriter writer;
//  writer.open(target);

  std::vector<std::string> statusList({/*"Putative 0.5",*/
                                       "Prelim Roughly traced",
                                       "Roughly traced",
                                       "Leaves",
                                       "Orphan hotknife",
                                       "Orphan"
                                       /*"Traced",*/
                                       /*"Hard to trace"}*/});
  writer->writeBodyStatusList(statusList);
#endif

#if 0
//  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test_merge");
//  ZJsonArray statusJson = reader->readBodyStatusList();
//  std::cout << statusJson.dumpString(2) << std::endl;

  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("test_merge");
  writer->mergeBody(writer->getDvidTarget().getSegmentationName(),
                    std::vector<uint64_t>({770606927, 1537922823, 1537931903, 5813022814, 1538600496, 1537927379, 1567960688, 1882009576}), true);

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");
  ZObject3dScan obj;
  ZIntPoint center(9988*2, 11001*2, 10361*2);
  reader->readBody(1167969164, flyem::LABEL_BODY, 1,
                   ZIntCuboid(center - 128, center + 128), true, &obj);
  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");

  ZObject3dScan obj;
//  obj.addSegment(300, 300, 300, 305);
  ZIntPoint center(9988*2, 11001*2, 10361*2);

  ZIntCuboid box = ZIntCuboid(center - 128, center + 128);
  reader->readCoarseBody(1167969164, flyem::LABEL_BODY, box, &obj);

//  obj->save(GET_TEST_DATA_DIR + "/_test.sobj");

//  obj.downsampleMax(1, 1, 1);


  tic();
  std::vector<ZArray*> blockArray = reader->readLabelBlock(obj, 0);
  std::cout << "Block count: " << blockArray.size() << std::endl;
  ptoc();


//  blockArray.resize(1);
  ZObject3dScan* body = ZObject3dFactory::MakeObject3dScan(
        blockArray, 1167969164, box, NULL);
  body->canonize();
  body->save(GET_TEST_DATA_DIR + "/_test.sobj");
//  body->print();

  ZStack *stack = ZStackFactory::MakeLabelBinaryStack(blockArray, 1167969164);
  stack->save(GET_TEST_DATA_DIR + "/_test.tif");

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");

  ZArray *array = reader->readLabelBlock(300, 300, 300, 0);

  std::vector<ZArray*> blockArray;
  blockArray.push_back(array);

  ZStack *stack = ZStackFactory::MakeLabelColorStack(blockArray);
  stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");

  ZDvidReader grayReader;
  grayReader.open(reader->getDvidTarget().getGrayScaleTarget());

  ZObject3dScan blockObj;
  blockObj.addSegment(300, 300, 300, 301);
  blockObj.addSegment(300, 301, 302, 303);
  blockObj.downsampleMax(1, 1, 1);
  blockObj.print();
  std::vector<ZStack*> stackArray = grayReader.readGrayScaleBlock(blockObj,  1);

//  std::vector<ZStack*> blockArray;
//  blockArray.push_back(array);

  ZStack *stack = ZStackFactory::Compose(stackArray);
  stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  ZIntCuboid box(1, 2, 3, 2, 3, 4);
  ZArray *array = ZArrayFactory::MakeArray(box, mylib::UINT64_TYPE);
  array->setValue(0, 1ull);
  array->setValue(1, 1ull);
  array->setValue(2, 1ull);

  std::vector<ZArray*> labelArray;
  labelArray.push_back(array);

  ZIntCuboid range(5, 5, 5, 3, 4, 5);

  ZObject3dScan obj;
  ZObject3dFactory::MakeObject3dScan(labelArray, 1, range, &obj);
  obj.print();
#endif

#if 0
  ZIntCuboid box(1, 2, 3, 2, 3, 4);
  ZArray *array = ZArrayFactory::MakeArray(box, mylib::UINT64_TYPE);
  array->setValue(0, 1ull);
  array->setValue(2, 1ull);

  std::vector<ZArray*> labelArray;
  labelArray.push_back(array);

  box.set(ZIntPoint(2, 3, 4), ZIntPoint(3, 4, 4));
  array = ZArrayFactory::MakeArray(box, mylib::UINT64_TYPE);
  array->setValue(0, 1ull);
  array->setValue(2, 1ull);
  labelArray.push_back(array);

  ZStack *stack = ZStackFactory::MakeLabelBinaryStack(labelArray, 1);
  ZStackPrinter printer;
  printer.setDetailLevel(1);
  printer.print(stack);

//  stack->printInfo();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");

  std::cout << reader->hasBody(913831721, flyem::LABEL_SUPERVOXEL) << std::endl;
  std::cout << reader->hasBody(913831721, flyem::LABEL_BODY) << std::endl;
  std::cout << reader->hasBody(701742479, flyem::LABEL_BODY) << std::endl;
#endif

#if 0
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test_merge");
  std::pair<uint64_t, std::vector<uint64_t>> mergeConfig = ZDvid::GetMergeConfig(
        *reader,
        std::vector<uint64_t>({770606927, 1537922823, 1537931903, 5813022814,
                               1538600496, 1537927379, 1567960688, 1882009576}),
        true);

  std::cout << "Merge: " << mergeConfig.first;
  std::cout << " <- ";
  for (uint64_t bodyId : mergeConfig.second) {
    std::cout << bodyId << " ";
  }
  std::cout << std::endl;
#endif

#if 0
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test_merge");
  std::pair<uint64_t, std::vector<uint64_t>> mergeConfig = ZDvid::GetMergeConfig(
        *reader, 770606927,
        std::vector<uint64_t>({1537922823, 1537931903, 5813022814,
                               1538600496, 1537927379, 1567960688, 1882009576}),
        false);

  std::cout << "Merge: " << mergeConfig.first;
  std::cout << " <- ";
  for (uint64_t bodyId : mergeConfig.second) {
    std::cout << bodyId << " ";
  }
  std::cout << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("test");
  ZDvidBodyHelper helper(reader);
  helper.setRange(ZIntCuboid(ZIntPoint(16550, 31554, 31084),
                             ZIntPoint(16806, 31810, 32749)));
  helper.setZoom(1);
  helper.setLowresZoom(6);
  helper.setCoarse(true);

  uint64_t bodyId = 2229212992;
  ZObject3dScanArray objArray = helper.readHybridBody(bodyId);
  ZMeshFactory mf;
  QElapsedTimer timer;
  timer.start();
  ZMesh *mesh = mf.makeMesh(objArray);
  LINFO() << "Mesh generating time:" << timer.elapsed() << "ms";

  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZDvidStackBlockFactory blockFactory;
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");
  blockFactory.setDvidTarget(reader->getDvidTarget());

  ZIntPoint blockIndex(300, 300, 300);
  std::vector<ZStack*> stackArray = blockFactory.make(blockIndex, 3, 0);

  ZStack *stack = ZStackFactory::Compose(stackArray);
  stack->save(GET_TEST_DATA_DIR + "/_test.tif");

#endif

#if 0
  ZDvidStackBlockFactory *blockFactory = new ZDvidStackBlockFactory;
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");
  blockFactory->setDvidTarget(reader->getDvidTarget());

  ZStackBlockSource blockSource;
  blockSource.setBlockFactory(blockFactory);
  blockSource.setBlockSize(blockFactory->getDvidInfo().getBlockSize());
  blockSource.setGridSize(blockFactory->getDvidInfo().getEndBlockIndex() + 1);

  ZStack *stack = blockSource.getStack(150, 150, 150, 1);
  stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");
  ZIntPoint center(18587, 19713, 20696);
  ZSparseStack *spStack = reader->readSparseStackOnDemand(
        915520244, flyem::LABEL_BODY, NULL);
//  ZStack *stack = spStack->getStack();
  QElapsedTimer timer;
  timer.start();
  ZStack *stack = spStack->makeStack(ZIntCuboid(center - 1024, center + 1024), false);
  std::cout << timer.elapsed() << "ms" << std::endl;
  if (stack) {
    stack->save(GET_TEST_DATA_DIR + "/_test.tif");
  } else {
    std::cout << "Null stack" << std::endl;
  }

#endif

#if 0
  ZObject3dScanArray objArray;
  ZObject3dScan obj1;
  obj1.setLabel(1);
  obj1.setColor(ZStroke2d::GetLabelColor(obj1.getLabel()));
  obj1.addSegment(0, 0, 0, 1);

  ZObject3dScan obj2;
  obj2.setLabel(2);
  obj2.setColor(ZStroke2d::GetLabelColor(obj2.getLabel()));
  obj2.addSegment(1, 1, 2, 3);
  objArray.append(obj1);
  objArray.append(obj2);

  objArray.save(GET_TEST_DATA_DIR + "/_test.soba");
#endif

#if 0
  ZObject3dScanArray objArray;
  objArray.load(GET_TEST_DATA_DIR + "/_test.soba");
  for (auto &obj : objArray) {
    std::cout << "Label: " << obj->getLabel() << std::endl;
    obj->printInfo();
  }
#endif

#if 0
  ZObject3dScanArray objArray;
  objArray.load(GET_TEST_DATA_DIR + "/_test.soba");
  std::cout << objArray.size() << " objects." << std::endl;
  for (auto &obj : objArray) {
    obj->printInfo();
  }

  objArray.resize(objArray.size() - 3);

  ZStack *labelStack = objArray.toColorField();

  ZStackWriter writer;
  writer.setCompressHint(ZStackWriter::COMPRESS_NONE);
  writer.write(GET_TEST_DATA_DIR + "/_test.tif", labelStack);
#endif

#if 0
  ZDvidTarget target;
  target.setServer("emdata2.int.janelia.org");
  target.setPort(8700);
  target.setUuid("c62b");
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");
  target.setGrayScaleSource(ZDvidNode("emdata3.int.janelia.org", "a89e", 8600));

  ZDvidReader reader;
  reader.open(target);

  QElapsedTimer timer;
  timer.start();
  ZDvidSparseStack *spStack = reader.readDvidSparseStack(
        1095007427, flyem::LABEL_BODY);
  spStack->fillValue();
  std::cout << "Reading time: " << timer.elapsed() << "ms" << std::endl;
  spStack->getSparseStack()->save(GET_TEST_DATA_DIR + "/_test.zss");

  spStack->getStack()->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  ZDvidTarget target;
  target.setServer("emdata2.int.janelia.org");
  target.setPort(8700);
  target.setUuid("c62b");
  target.setSegmentationName("segmentation");
  target.setGrayScaleName("grayscalejpeg");
  target.setGrayScaleSource(ZDvidNode("emdata3.int.janelia.org", "a89e", 8600));

  ZDvidReader reader;
  reader.open(target);

  ZSparseStack *spStack = reader.readSparseStackOnDemand(
        1095007427, flyem::LABEL_BODY, NULL);
  ZStackWatershedContainer container(spStack);
  container.exportSource(GET_TEST_DATA_DIR + "/_test2.tif");
#endif

#if 0
  ZSparseStack spStack;
  spStack.load(GET_TEST_DATA_DIR + "/_test.zss");
  ZStackWatershedContainer container(&spStack);
  container.exportSource(GET_TEST_DATA_DIR + "/_test.tif");
//  spStack.getStack()->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  neutuse::TaskWriter writer;
  writer.open("http://zhaot-ws1:5000");
//  writer.open("http://127.0.0.1:5000");
  std::cout << writer.ready() << std::endl;

  ZDvidTarget target("emdata1.int.janelia.org", "b6bc", 8500);
  target.setBodyLabelName("bodies");
  neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
        "skeletonize", target, 1, true);

  writer.uploadTask(task);

  std::cout << writer.getStatusCode() << std::endl;
  std::cout << writer.getResponse() << std::endl;
#endif

#if 0
  ZDvidTarget target("emdata1.int.janelia.org", "b6bc", 8500);
  target.setBodyLabelName("bodies");
  neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
        "skeletonize", target, 2, true);


  ZNetBufferReader reader;
  reader.post("http://127.0.0.1:5000/api/v1/tasks",
              task.toJsonObject().dumpString(0).c_str());
#endif

#if 0
  neutuse::TaskWriter writer;
//  writer.open("http://zhaot-ws1:7500");
  writer.open("http://127.0.0.1:5000");
  std::cout << writer.ready() << std::endl;

  neutuse::Task task;
  task.setType("dvid");
  task.setName("skeletonize");
  task.setUser("zhaot");
  ZJsonObject config;
  config.load(GET_TEST_DATA_DIR + "/_flyem/test/skeleton.json");
//  config.setEntry("bodyid", 1);
//  config.setEntry("input", "http:emdata1.int.janelia.org:8500:b6bc:bodies");
//  config.setEntry("force_update", true);

  task.setConfig(config);

  writer.uploadTask(task);

  std::cout << writer.getStatusCode() << std::endl;
  std::cout << writer.getResponse() << std::endl;
#endif

#if 0
  uint64_t bodyId = 1101396820;
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");
  ZObject3dScan obj;
  reader->readSupervoxel(bodyId, true, &obj);
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  if (mesh != nullptr) {
    mesh->save(GET_TEST_DATA_DIR + "/_test.drc", "drc");
  }
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("test");
  writer->writeSupervoxelMesh(*mesh, bodyId);
#endif

#if 0
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");
  ZMesh *mesh = reader->readSupervoxelMesh(1101396820);
  if (mesh != nullptr) {
    mesh->save(GET_TEST_DATA_DIR + "/_test.drc", "drc");
  }
#endif

#if 0
  for (size_t i = 0; i < 10000; ++i) {
    std::cout << "i=" << i << std::endl;
    ZNetBufferReader reader;
    reader.read("http://emdata2.int.janelia.org:2018", false);
    qDebug() << reader.getBuffer();
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("test");
  uint64_t bodyId = 1452234638;

  auto bodyList = reader->readSupervoxelSet(bodyId);
  std::ofstream stream(
        GET_TEST_DATA_DIR + "/my-mesh-job-files/supervoxels-to-process.csv");
  for (uint64_t body : bodyList) {
    stream << body << std::endl;
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("test");
  uint64_t bodyId = 1020280792;
  ZObject3dScan obj;
  reader->readBodyWithPartition(bodyId, &obj);
  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZDvidReader *reader =  ZGlobal::GetInstance().getDvidReader("test");

//  uint64_t bodyId = 582670241;
  uint64_t bodyId = 979627466;
  std::cout << "#voxels" << reader->readBodySize(bodyId, flyem::LABEL_SUPERVOXEL) << std::endl;

  ZObject3dScan obj;
//  reader->readSupervoxel(bodyId, true, &obj);
  reader->readBody(bodyId, flyem::LABEL_BODY,
                   ZIntCuboid(), true, &obj);
  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
#endif

#if 0
  ZMesh zmesh;
  zmesh.load((GET_TEST_DATA_DIR + "/_system/meshes/87839.tif.smooth.obj").c_str());

  ZMeshIO meshIO;
  meshIO.save(zmesh, (GET_TEST_DATA_DIR + "/_test.drc").c_str(), "");
#endif

#if 0
  ZMesh zmesh;
  zmesh.load((GET_TEST_DATA_DIR + "/_system/meshes/87839.tif.smooth.obj").c_str());

  draco::Mesh *dmesh = ZMeshIO::ToDracoMesh(zmesh, NULL);
  draco::Encoder encoder;
  draco::EncoderBuffer buffer;
  encoder.EncodeMeshToBuffer(*dmesh, &buffer);

  FILE *fp = fopen((GET_TEST_DATA_DIR + "/_test.drc").c_str(), "w");
  fwrite(buffer.data(), 1, buffer.size(), fp);
  fclose(fp);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZObject3dScan obj;
  reader->readSupervoxel(5813058594, true, &obj);
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  ZMeshIO meshIo;
  meshIo.save(*mesh, (GET_TEST_DATA_DIR + "/_test.drc").c_str(), "");
#endif

#if 0

  ZMesh mesh;
  ZMeshIO meshIo;
  meshIo.load((GET_TEST_DATA_DIR + "/_test.drc").c_str(), mesh);
  std::cout << mesh.vertices().size() << std::endl;
#endif

#if 0
  ZMesh zmesh;
  zmesh.load((GET_BENCHMARK_DIR + "/test2.obj").c_str());
  draco::Mesh dmesh;

  const std::vector<glm::vec3>& vertices = zmesh.vertices();
  std::cout << vertices.size() << std::endl;

  for (auto &vertex : vertices) {
    std::cout << vertex << std::endl;
  }

  size_t vertexDataLength = vertices.size() * 3;
  float *vertexDataBuffer = new float[vertexDataLength];
  float *vertexDataBufferIter = vertexDataBuffer;
  for (size_t i = 0; i < vertices.size(); ++i) {
    *vertexDataBufferIter++ = vertices[i].x;
    *vertexDataBufferIter++ = vertices[i].y;
    *vertexDataBufferIter++ = vertices[i].z;
  }

  draco::DataBuffer buffer;
  buffer.Update(vertexDataBuffer, sizeof(float) * vertexDataLength);

  draco::GeometryAttribute va;
  va.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32,
          false, sizeof(float) * 3, 0);
  int attId = dmesh.AddAttribute(va, true, vertices.size());
  dmesh.SetAttributeElementType(attId, draco::MESH_VERTEX_ATTRIBUTE);
  dmesh.set_num_points(vertices.size());

  for (size_t i = 0; i < vertices.size(); ++i) {
    float val[3];
    for (size_t j = 0; j < 3; ++j) {
      val[j] = vertices[i][j];
    }
    dmesh.attribute(attId)->SetAttributeValue(draco::AttributeValueIndex(i), val);
  }

  for (size_t i = 0; i < zmesh.numTriangles(); ++i) {
    glm::uvec3 triangle = zmesh.triangleIndices(i);
    draco::Mesh::Face face;
    face[0] = draco::PointIndex(triangle[0]);
    face[1] = draco::PointIndex(triangle[1]);
    face[2] = draco::PointIndex(triangle[2]);
    dmesh.AddFace(face);
  }

  std::cout << (void*) vertexDataBuffer << std::endl;
  std::cout << (void*) buffer.data() << std::endl;


  delete []vertexDataBuffer;

  {
    std::vector<glm::vec3> newVertices;
    const draco::PointAttribute *const att = dmesh.GetNamedAttribute(
          draco::GeometryAttribute::POSITION);
    std::cout << "att size: " << att->size() << std::endl;
    if (att == nullptr || att->size() == 0)
      throw ZIOException("no vertices found in draco file");
    newVertices.resize(dmesh.num_points());
    std::cout << "dmesh #vertices: " << dmesh.num_points() << std::endl;
    for (draco::PointIndex i(0); i < dmesh.num_points(); ++i) {
      if (!att->ConvertValue<float, 3>(
            att->mapped_index(i), &newVertices[i.value()][0])) {
        newVertices.clear();
        throw ZIOException("can not decode draco vertices");
      }
    }

    for (auto &vertex : newVertices) {
      std::cout << vertex << std::endl;
    }

    std::cout << newVertices.size() << std::endl;

    draco::Encoder encoder;
    draco::EncoderBuffer buffer;
    encoder.EncodeMeshToBuffer(dmesh, &buffer);

    FILE *fp = fopen((GET_TEST_DATA_DIR + "/_test.drc").c_str(), "w");
    fwrite(buffer.data(), 1, buffer.size(), fp);
    fclose(fp);
  }
#endif

#if 0
  ZDvidTarget target;
  target.set("waspem-dvid.flatironinstitute.org", "e0c7", 8000);
  target.setMultiscale2dName("sb3911tile");
  ZDvidReader reader;
  reader.open(target);
  ZDvidTileInfo info = reader.readTileInfo("sb3911tile");
  info.print();
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZFlyEmMisc::UploadRoi(
        (GET_TEST_DATA_DIR + "/_flyem/roi/20190111").c_str(),
        (GET_TEST_DATA_DIR + "/_flyem/roi/20180913_tif/roiname.csv").c_str(),
        writer);


#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/_flyem/roi/20180913_tif";

  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  std::string filePath = dataDir + "/synapse_thresh_PAINT_NO3_new.tif.obj";
  std::cout << "Uploading " << filePath << std::endl;
  writer->uploadRoiMesh(filePath, "NO3");
#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/_flyem/roi/20180913_tif";

  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  std::string filePath = dataDir + "/synapse_thresh_PAINT_NO3_new.tif.obj";
  std::cout << "Uploading " << filePath << std::endl;
  writer->uploadRoiMesh(filePath, "NO3");
#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/_flyem/roi/20190325";
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  std::string filePath = dataDir + "/20190325_SLP.labels.tif.obj";
  std::cout << "Uploading " << filePath << std::endl;
  writer->uploadRoiMesh(filePath, "SLP");

  filePath = dataDir + "/20190325_dACA.labels.tif.obj";
  std::cout << "Uploading " << filePath << std::endl;
  writer->uploadRoiMesh(filePath, "dACA");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  std::string dataDir = GET_FLYEM_DATA_DIR + "/roi/20190422";
  flyem::UploadRoi(dataDir.c_str(), (dataDir + "/roiname.csv").c_str(), writer);
#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/_flyem/roi/20180920_tif";
  std::ifstream stream(GET_TEST_DATA_DIR + "/_flyem/roi/20180913_tif/roiname.csv");

  std::string line;
  std::unordered_map<std::string, std::string> nameMap;
  while (std::getline(stream, line)) {
    std::cout << line << std::endl;
    std::vector<std::string> tokens = ZString::Tokenize(line, ',');
    for (auto &str : tokens) {
      std::cout << str << " --- ";
    }
    std::cout << std::endl;
    if (tokens.size() >= 3) {
      ZString name(tokens[2]);
      name.trim();
      nameMap[tokens[1] + ".obj"] = name;
      nameMap[tokens[1] + ".sobj"] = name;
    }
  }

  for (auto &entry : nameMap) {
    std::cout << entry.first << ": " << entry.second << std::endl;
  }

  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");

  //Upload: ZDvidWriter::uploadRoiMesh
  {
    QDir dir(dataDir.c_str());
    QStringList fileList = dir.entryList(QStringList() << "*.obj");
    for (QString &file : fileList) {
      std::string name = nameMap[file.toStdString()];
      std::cout << file.toStdString() << ": " << name << std::endl;
      writer->uploadRoiMesh(dataDir + "/" + file.toStdString(), name);
      //    break;
    }
  }

  //Write ROI data: ZDvidWriter::writeRoi

  {
    QDir dir(dataDir.c_str());
    QStringList fileList = dir.entryList(QStringList() << "*.sobj");
    for (QString &file : fileList) {
      std::string name = nameMap[file.toStdString()];
      std::cout << file.toStdString() << ": " << name << std::endl;
      ZObject3dScan roi;
      roi.load(dataDir + "/" + file.toStdString());
      if (!writer->getDvidReader().hasData(name)) {
        writer->createData("roi", name);
      }
      writer->writeRoi(roi, name);
    }
  }
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/allneuropils/synapse_thresh_allneuropils_nohole.labels.tif.sobj");
  std::string name = "all_neuropils";
  if (!writer->getDvidReader().hasData(name)) {
    writer->createData("roi", name);
  }
  writer->writeRoi(roi, name);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_dACA.labels.tif.sobj");
  std::string name = "dACA";
  if (!writer->getDvidReader().hasData(name)) {
    writer->createData("roi", name);
  } else {
    writer->deleteData("roi", name);
  }
  std::cout << "Writing " << name << std::endl;
  writer->writeRoi(roi, name);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/20190501/20190501_lACA.labels.tif.sobj");
  std::string name = "lACA";
  if (!writer->getDvidReader().hasData(name)) {
    writer->createData("roi", name);
  } else {
    writer->deleteData("roi", name);
  }
  std::cout << "Writing " << name << std::endl;
  writer->writeRoi(roi, name);

  writer->uploadRoiMesh(
        GET_FLYEM_DATA_DIR + "/roi/20190501/20190501_lACA.labels.tif.obj",
        "lACA");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  FlyEmDataWriter::UploadRoi(
        *writer, "lACA",
        GET_FLYEM_DATA_DIR + "/roi/20190516/20190516_lACA.labels.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/20190516/20190516_lACA.labels.tif.obj");

  FlyEmDataWriter::UploadRoi(
        *writer, "SLP",
        GET_FLYEM_DATA_DIR + "/roi/20190516/20190516_SLP.labels.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/20190516/20190516_SLP.labels.tif.obj");

#endif


#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  FlyEmDataWriter::UploadRoi(
        *writer, "BU",
        GET_FLYEM_DATA_DIR + "/roi/20190801BU_LAL/20190801_BU.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/20190801BU_LAL/20190801_BU.tif.obj");

  FlyEmDataWriter::UploadRoi(
        *writer, "LAL",
        GET_FLYEM_DATA_DIR + "/roi/20190801BU_LAL/20190801_LAL.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/20190801BU_LAL/20190801_LAL.tif.obj");
#endif

#if 0
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_dACA.labels.tif.sobj");
  ZJsonArray array =
      ZJsonFactory::MakeJsonArray(roi, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_dACA.labels.tif.json");
#endif

#if 0
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_SLP.labels.tif.sobj");
  ZJsonArray array =
      ZJsonFactory::MakeJsonArray(roi, ZJsonFactory::OBJECT_SPARSE);
  array.dump(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_SLP.labels.tif.json");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZObject3dScan roi;
  roi.load(GET_FLYEM_DATA_DIR + "/roi/20190325/20190325_SLP.labels.tif.sobj");
  std::string name = "SLP";
  if (!writer->getDvidReader().hasData(name)) {
    writer->createData("roi", name);
  } else {
    writer->deleteData("roi", name);
  }
  writer->writeRoi(roi, name);
#endif

#if 0
  neutuse::TaskWriter writer;
  writer.open("http://emdata2.int.janelia.org:2018");
  writer.testConnection();
  std::cout << writer.ready() << std::endl;

#endif

#if 0
  std::cout << GET_APPLICATION_DIR << std::endl;
  QFileInfo configFileInfo = QFileInfo(
        QDir((GET_APPLICATION_DIR).c_str()), "local_flyem_config.json");
  qDebug() << configFileInfo.absoluteFilePath();

  configFileInfo.setFile(QDir((GET_APPLICATION_DIR).c_str()), "flyem_config.json");
  qDebug() << configFileInfo.absoluteFilePath();

#endif

#if 0
//  GET_FLYEM_CONFIG.useDefaultNeuTuServer(false);
  GET_FLYEM_CONFIG.print();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("hemibran-production");
  size_t bodySize = 0;
  size_t blockCount = 0;
  ZIntCuboid box;
  std::tie(bodySize, blockCount, box) = reader->readBodySizeInfo(
        662776660, neutu::EBodyLabelType::BODY);
  std::cout << bodySize << " " << blockCount << std::endl;
  std::cout << box.toString() << std::endl;
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  ZJsonObject jsonObj;
  jsonObj.setEntry("address", "emdata1.int.janelia.org:8900");
  writer->writeJson("branches", "mirror", jsonObj);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("MB_Test");
  std::cout << "Mirror: " << reader->readMirror() << std::endl;

  std::cout << "Mutation ID: " << reader->readBodyMutationId(2) << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();

  if (reader) {
    size_t bodySize = 0;
    size_t blockCount = 0;
    ZIntCuboid box;
    std::tie(bodySize, blockCount, box) = reader->readBodySizeInfo(
          5813050455, neutu::EBodyLabelType::BODY);
    std::cout << blockCount << std::endl;

    ZObject3dScan obj;
    tic();
    reader->readMultiscaleBody(1168314368, 2, true, &obj);
//    reader->readBody(1168314368, flyem::LABEL_BODY, 2, ZIntCuboid(), true, &obj);
    ptoc();
    std::cout << obj.isCanonizedActually() << std::endl;
    std::cout << obj.getDsIntv().toString() << std::endl;
    obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
  } else {
    std::cout << "Cannot retrieve reader. Make sure the database name is correct."
              << std::endl;
  }

#endif

#if 0
  ZObject3dScan obj1;
  ZObject3dScan obj2;
  obj1.load(GET_TEST_DATA_DIR + "/_test.sobj");
  obj2.load(GET_TEST_DATA_DIR + "/_test2.sobj");

  ZObject3dScan diffObj = obj2 - obj1;
  diffObj.print();
  diffObj.save(GET_TEST_DATA_DIR + "/_test3.sobj");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("hemibrain_test");
  ZDvidUrl dvidUrl(reader->getDvidTarget());

  ZDvidUrl::SparsevolConfig config;
  config.bodyId = 1250458360;
  config.format = "blocks";
//  config.range = ZIntCuboid(1, 2, 3, 10, 20, 30);
//  config.supervoxel = true;
  std::cout << dvidUrl.getSparsevolUrl(config) << std::endl;

  QByteArray buffer = reader->readBuffer(dvidUrl.getSparsevolUrl(config));
  ZObject3dScan obj;
  obj.importDvidBlockBuffer(buffer.data(), buffer.size(), true);
  std::cout << obj.isCanonizedActually() << std::endl;
  obj.save(GET_TEST_DATA_DIR + "/_test2.sobj");
#endif

#if 0
  QFile file((GET_BENCHMARK_DIR + "/dvidblock.dat").c_str());
  if (file.exists()) {
    file.open(QFile::ReadOnly);
    QByteArray data = file.readAll();
    file.close();

#if 0
    const char* originalDataPtr = data.data();
    ZMemoryInputStream stream(originalDataPtr, data.size());

    int gx = 0;
    int gy = 0;
    int gz = 0;
    stream >> gx >> gy >> gz;
    std::cout << gx << " " << gy << " " << gz << std::endl;

    uint64_t fg = 0;
    stream >> fg;
    std::cout << fg << std::endl;

    ZObject3dScan obj;
    ZObject3dScan::Appender appender(&obj);

    const char* dataPtr = stream.getCurrentDataPointer();
    while (dataPtr) {
      size_t n = data.size() - (dataPtr - originalDataPtr);
      dataPtr = appender.addBlockSegment(dataPtr, n, gx, gy, gz);
    }

    obj.sort();
#endif

    ZObject3dScan obj;
    obj.importDvidBlockBuffer(data.data(), data.size());
    std::cout << obj.isCanonizedActually() << std::endl;
    obj.save(GET_TEST_DATA_DIR + "/_test.sobj");
  }

#endif

#if 0
  ZObject3dScan obj;

  tic();
  obj.importDvidObject(GET_DATA_DIR + "/_system/1261432158.dvid");
  obj.sort();
  ptoc();

  std::cout << obj.isCanonizedActually() << std::endl;
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  for (uint8_t c = 0; c < 255; ++c) {
    std::cout << std::bitset<8>(c) << std::endl;
    ZObject3dScan obj;
    obj.addStripe(0, 1);
    ZObject3dScan::Appender appender(&obj);
    appender.addCodeSegment(c, 0);
    obj.print();
  }
#endif

#if 0
  uint64_t v = 0x0F000F000FF00A00;
  ZObject3dScan obj;
  obj.addStripe(0, 1);
  ZObject3dScan::Appender appender(&obj);
  appender.addCodeSegment(v, 0, 1, 2);
  obj.print();
#endif

#if 0
  std::vector<uint64_t> codeArray;
  codeArray.push_back(0x0000000000001001);
  codeArray.push_back(0x0000000000000002);
  codeArray.push_back(0x0000000000000003);

  ZObject3dScan obj;
  obj.addStripe(0, 1);
  ZObject3dScan::Appender appender(&obj);
  appender.addCodeSegment(codeArray, 0, 1, 2);
  obj.print();
#endif

#if 0
  ZSwcTree tree;
  tree.loadFromBuffer("#${\"mutation id\": 1}\n1 2 1 2 3 1 -1");
  std::cout << "mid: " <<flyem::GetMutationId(&tree) << std::endl;
  tree.print();
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/swc/meta.swc");
  std::cout << "mid: " <<flyem::GetMutationId(&tree) << std::endl;
  tree.print();
#endif

#if 0
  ZSwcTree tree;
  flyem::SetMutationId(&tree, 123);
  std::cout << "mid: " <<flyem::GetMutationId(&tree) << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();
  ZJsonArray thresholdData = reader->readSynapseLabelsz(
        20000, ZDvid::ELabelIndexType::INDEX_ALL_SYN);
  thresholdData.dump(GET_TEST_DATA_DIR + "/test.json");
#endif

#if 0
  ZFlyEmDataBundle bundle;
  bundle.loadJsonFile(
        GET_TEST_DATA_DIR + "/_paper/neuron_type/data/data_bundle.json");
  std::vector<ZFlyEmNeuron> neuronArray = bundle.getNeuronArray();
  std::cout << "Number of neurons: " << neuronArray.size() << std::endl;

  std::string neuronNameFilePath = GET_DATA_DIR + "/_paper/neuron_type/data/neuron_name.txt";
  std::string neuronIdFilePath = GET_DATA_DIR + "/_paper/neuron_type/data/neuron_id.txt";
  std::string neuronTypeFilePath = GET_DATA_DIR + "/neuron_type.txt";


  std::ofstream nameStream(neuronNameFilePath.c_str());
  std::ofstream idStream(neuronIdFilePath.c_str());
  std::ofstream typeStream(neuronTypeFilePath.c_str());

  for (std::vector<ZFlyEmNeuron>::const_iterator iter = neuronArray.begin();
       iter != neuronArray.end(); ++iter) {
    const ZFlyEmNeuron &neuron = *iter;
    nameStream << neuron.getName() << std::endl;
    idStream << neuron.getId() << std::endl;
    typeStream << neuron.getType() << std::endl;
  }
#endif

#if 0
  ZDoubleVector offset;
  offset.importTextFile("/Users/zhaot/Work/neutu/neurolabi/data/_paper/neuron_type/data/offset2/3856.offset.txt");
#endif

#if 0
  std::string dataDir = GET_TEST_DATA_DIR + "/_paper/neuron_type/data";
  std::string volumeDir = dataDir + "/volume/old";

  QFileInfoList fileList;
  QDir dir(volumeDir.c_str());
  QStringList filters;
  filters << "*.sobj";
  fileList = dir.entryInfoList(filters);

  QDir offsetDir = QDir((dataDir + "/offset2").c_str());
  QDir outputDir = QDir((dataDir + "/volume/aligned").c_str());

  foreach (QFileInfo fileInfo, fileList) {

    ZString objPath = fileInfo.absoluteFilePath().toStdString();

    std::cout << objPath << std::endl;
    ZObject3dScan obj;
    obj.load(objPath.c_str());
    uint64_t bodyId = objPath.lastUint64();

    QString offsetFile = offsetDir.absoluteFilePath(
          (std::to_string(bodyId) + ".offset.txt").c_str());
    if (QFileInfo(offsetFile).exists()) {
      ZDoubleVector offset;
      offset.importTextFile(offsetFile.toStdString());
      std::cout << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
      obj.translate(offset[0] / 2 - offset[0], offset[1] / 2 - offset[1], 0);
      obj.save(outputDir.absoluteFilePath((std::to_string(bodyId) + ".sobj").c_str()).toStdString());
    } else {
      std::cout << "No offset found. Abort." << std::endl;
      return;
    }
  }
#endif

#if 0
  ZObject3dScan obj1;
  ZObject3dScan obj2;
  obj1.load(GET_TEST_DATA_DIR + "/_paper/neuron_type/data/volume/446263.sobj");
  obj1.setColor(255, 0, 0);
  {
    ZDoubleVector offset;
    offset.importTextFile(
          GET_TEST_DATA_DIR + "/_paper/neuron_type/data/offset/Mi1a_446263.tif.offset.txt");
    std::cout << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
    obj1.translate(-offset[0] / 2, -offset[1] / 2, 0);
  }


  obj2.load(GET_TEST_DATA_DIR + "/_paper/neuron_type/data/volume/1196.sobj");
  obj2.setColor(0, 255, 0);
  {
    ZDoubleVector offset;
    offset.importTextFile(
          GET_TEST_DATA_DIR + "/_paper/neuron_type/data/offset/Pm2-1_1196.tif.offset.txt");
    std::cout << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
    obj2.translate(-offset[0] / 2, -offset[1] / 2, 0);
  }

  ZObject3dScanArray objArray;
  objArray.append(obj1);
  objArray.append(obj2);

  std::cout << objArray.getBoundBox().toString() << std::endl;

  objArray.save(GET_TEST_DATA_DIR + "/_test.soba");
//  ZStack *stack = ZStackFactory::MakeColorStack(objArray);
//  stack->save(GET_TEST_DATA_DIR + "/test.v3draw");
#endif

#if 0
  ZIntPointArray ptArray;

  {
    FILE *fp = fopen((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/fix/coord.txt").c_str(), "r");
    ZString line;
    while (line.readLine(fp)) {
      std::vector<int> pt = line.toIntegerArray();
      if (pt.size() == 3) {
        ptArray.append(pt[0], pt[1], pt[2]);
      }
    }
    fclose(fp);
  }

  for (const ZIntPoint &pt : ptArray) {
    std::cout << pt.toString() << std::endl;
  }

  std::vector<std::string> uuidList;

  {
    FILE *fp = fopen((GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/fix/uuid.txt").c_str(), "r");
    ZString line;
    while (line.readLine(fp)) {
      line.trim();
      if (!line.empty()) {
        uuidList.push_back(line);
      }
    }
    fclose(fp);
  }

  for (const std::string &uuid : uuidList) {
    std::cout << uuid << std::endl;
    ZDvidTarget target;
    target.set("emdata1", uuid, 8400);
    target.setSegmentationName("segmentation");
    ZDvidWriter writer;
    writer.open(target);
    for (const ZIntPoint &pt : ptArray) {
      std::cout << pt.toString() << std::endl;
      ZFlyEmMisc::UpdateBodyStatus(pt, "anchor", &writer);
    }
  }
//    std::cout << writer.getDvidReader().readBodyIdAt(ptArray[0]) << std::endl;
#endif

#if 0
  ZDvidWriter *writer =
      ZGlobal::GetInstance().getDvidWriter("hemibran-production");

  ZFlyEmMisc::UpdateSupervoxelMesh(*writer, 5813082914);
#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 2);
  obj.addSegment(0, 1, 2, 3);
  obj.addSegment(1, 1, 2, 3);
  obj.addSegment(2, 1, 1, 3);

  ZObject3dScan::ConstStripeIterator iter(&obj);
  while (iter.hasNext()) {
    iter.next().print();
  }

#endif

#if 0
  ZObject3dScan obj;
  obj.addSegment(0, 0, 1, 2);
  obj.addSegment(0, 1, 2, 3);
  obj.addSegment(1, 1, 2, 3);
  obj.addSegment(2, 1, 1, 3);
  obj.addSegment(2, 2, 1, 3);
  obj.addSegment(2, 3, 1, 3);
  obj.addSegment(3, 1, 1, 3);

  ZObject3dScan::ConstSliceIterator iter(&obj);
  while (iter.hasNext()) {
    iter.next().print();
    std::cout << iter.current().isCanonized() << std::endl;
  }
#endif

#if 0
  ZRandomGenerator rng;

  for (int t = 0; t < 100000; ++t) {
    int v = rng.rndint(2, 1000);

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(rng.rndint(-1, 1), 0, 0);
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D1));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(0, 0, rng.rndint(-1, 1));
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D1));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(0, rng.rndint(-1, 1), 0);
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D1));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(rng.rndint(-1, 1), rng.rndint(-1, 1), 0);
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D2));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(rng.rndint(-1, 1), 0, rng.rndint(-1, 1));
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D2));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(0, rng.rndint(-1, 1), rng.rndint(-1, 1));
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D2));
      }
    }

    {
      ZIntPoint pt;
      ZObject3dScan obj1;
      ZObject3dScan obj2;
      for (int c = 0; c< v; ++c) {
        //      std::cout << rng.rndint(3) - 2 << std::endl;
        pt += ZIntPoint(rng.rndint(-1, 1), rng.rndint(-1, 1), rng.rndint(-1, 1));
        if (rng.rndint(0, 1) == 0) {
          obj1.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        } else {
          obj2.addSegment(pt.getZ(), pt.getY(), pt.getX(), pt.getX());
        }
      }
      std::cout << "Size: " << obj1.getVoxelNumber() << " " << obj2.getVoxelNumber() << std::endl;
      if (!obj1.isEmpty() && !obj2.isEmpty()) {
        ASSERT_TRUE(obj1.isAdjacentTo(obj2, neutube::EStackNeighborhood::D3));
      }

      {
        std::vector<ZObject3dScan> objArray = obj1.getConnectedComponent(
              ZObject3dScan::ACTION_NONE);
        std::cout << objArray.size() << " components" << std::endl;
        if (objArray.size() > 1) {
          for (size_t i = 0; i < objArray.size() - 1; ++i) {
            for (size_t j = i + 1; j < objArray.size(); ++j) {
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D1));
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D2));
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D3));
            }
          }
        }
      }

      {
        std::vector<ZObject3dScan> objArray = obj2.getConnectedComponent(
              ZObject3dScan::ACTION_NONE);
        std::cout << objArray.size() << " components" << std::endl;
        if (objArray.size() > 1) {
          for (size_t i = 0; i < objArray.size() - 1; ++i) {
            for (size_t j = i + 1; j < objArray.size(); ++j) {
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D1));
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D2));
              ASSERT_FALSE(objArray[i].isAdjacentTo(
                             objArray[j], neutube::EStackNeighborhood::D3));
            }
          }
        }
      }
    }
  }

#endif


#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("MB_Test");
  std::cout << "Writer: " << writer;

  ZFlyEmMisc::UpdateBodyStatus(ZIntPoint(3957, 5658, 7309), "test2", writer);
#endif

#if 0
  ZObject3dScan obj1;
  obj1.addSegment(0, 1, 0, 1);
  obj1.addSegment(0, 3, 0, 1);
  obj1.addSegment(0, 5, 0, 1);
  obj1.addSegment(0, 6, 0, 1);
  obj1.addSegment(0, 9, 0, 1);
  obj1.addSegment(0, 10, 0, 1);
  obj1.addSegment(0, 11, 0, 1);

  ZObject3dScan obj2;
  obj2.addSegment(0, 2, 3, 4);
  obj2.addSegment(0, 3, 3, 4);
  obj2.addSegment(0, 7, 3, 4);
  obj2.addSegment(0, 8, 3, 4);
  obj2.addSegment(0, 9, 3, 4);
  obj2.addSegment(0, 10, 3, 4);
  obj2.addSegment(0, 12, 1, 4);
  obj2.addSegment(0, 13, 3, 4);
  obj1.isAdjacentTo(obj2);
//  ASSERT_TRUE(obj2.isAdjacentTo(obj1));
#endif

#if 0
  uint64_t bodyId = 5813087431;
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibran-production");

  ZObject3dScan obj;
  writer->getDvidReader().readSupervoxel(bodyId, true, &obj);
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  if (mesh != nullptr) {
    mesh->save(GET_TEST_DATA_DIR + "/_test.drc", "drc");
  }

  writer->writeSupervoxelMesh(*mesh, bodyId);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZJsonObject obj;
  obj.decodeString(reader->readKeyValue("neutu_config", "body_status_v2").
                   toStdString().c_str());
  ZFlyEmBodyAnnotationMerger merger;
  merger.loadJsonObject(obj);
  merger.print();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  QString dataName = "segmentation_annotations";
  QStringList keyList = reader->readKeys(dataName);
  std::cout << keyList.size() << " annotations" << std::endl;
  int batchSize = 5000;

  std::ofstream stream(GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/fix/bodylist.txt");

  int currentIndex = 0;
  while (currentIndex < keyList.size()) {
    QList<QByteArray> values =
          reader->readKeyValues(dataName, keyList.mid(currentIndex, batchSize));
    currentIndex += batchSize;
    std::cout << values.size() << " values" << std::endl;

    std::set<std::string> statusSet = {
      "anchor", "roughly traced", "prelim roughly traced"};

    for (const auto &data : values) {
      ZJsonObject obj;
      obj.decodeString(QString(data).toStdString().c_str());
//      obj.print();
      ZString status = ZJsonParser::stringValue(obj["status"]);
      status.toLower();
      if (statusSet.count(status) > 0) {
        int64_t bodyId = ZJsonParser::integerValue(obj["body ID"]);
        std::cout << bodyId << " " << status << std::endl;
        stream << bodyId << std::endl;
      }
    }
  }
//  QList<QByteArray> values =
//      reader->readKeyValues(dataName, keyList.mid(10, 10));
//  for (const auto &data : values) {
//    ZJsonObject obj;
//    obj.decodeString(QString(data).toStdString().c_str());
//    obj.print();
//  }
#endif

#if 0
  QElapsedTimer timer;
  timer.start();
  for (int i = 0; i < 1000; ++i) {
    ZNetBufferReader reader;
    reader.hasHead("http://emdata1.int.janelia.org:8900");
  }
  std::cout << timer.elapsed() << "ms" << std::endl;
#endif

#if 0
  QElapsedTimer timer;
  timer.start();
  for (int i = 0; i < 1000; ++i) {
    ZSharedPointer<libdvid::DVIDConnection> conn = ZDvid::MakeDvidConnection(
          "emdata1.int.janelia.org:8900");
    try {
      int statusCode;
      ZDvid::MakeRequest(
            *conn, "", "HEAD", libdvid::BinaryDataPtr(),
            libdvid::DEFAULT, statusCode);
      //    std::cout << conn->make_head_request("/api/help") << std::endl;
    } catch (exception &e) {
      std::cout << e.what() << std::endl;
    }
  }
  std::cout << timer.elapsed() << "ms" << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->getDvidTarget().print();

  QElapsedTimer timer;
  timer.start();
  for (int i = 0; i < 1000; ++i) {
    std::cout << "Has key: " << reader->hasKey("rois", "bLo") << std::endl;
  }
  std::cout << timer.elapsed() << "ms" << std::endl;

#endif

#if 0
  {
  ZNetBufferReader reader;

  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/token.json");
  std::string token = ZJsonParser::stringValue(obj["token"]);
  reader.setHeader("Authorization", QString("Bearer ") + token.c_str());
  reader.read("https://emdata1.int.janelia.org:11000/api/dbmeta/datasets", true);
  std::cout << reader.getBuffer().size() << std::endl;
  ZJsonObject dataObj;
  dataObj.decodeString(reader.getBuffer().toStdString().c_str());
  dataObj.print();
  }
#endif

#if 0
  NeuPrintReader reader("https://emdata1.int.janelia.org:11000");

  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/token.json");
  std::string token = ZJsonParser::stringValue(obj["token"]);
  std::cout << token << std::endl;
  reader.authorize(token.c_str());

//  reader.readDatasets();

  QList<uint64_t> bodyList = reader.queryNeuron("AL", "MB (left)");
  for (uint64_t bodyId : bodyList) {
    std::cout << bodyId << std::endl;
  }
#endif

#if 0
  NeuPrintReader reader("https://emdata1.int.janelia.org:11000");

  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/token.json");
  std::string token = ZJsonParser::stringValue(obj["token"]);
  std::cout << token << std::endl;
  reader.authorize(token.c_str());

  QList<uint64_t> bodyList = reader.findSimilarNeuron(915520244);
  for (uint64_t bodyId : bodyList) {
    std::cout << "  " << bodyId << std::endl;
  }
#endif

#if 0
  qDebug() << qgetenv("NEUPRINT");
  qDebug() << qgetenv("HOME");
#endif


#if 0
  ZSharedPointer<libdvid::DVIDConnection> conn = ZDvid::MakeDvidConnection(
        "https://emdata1.int.janelia.org:11000");

  try {
    int statusCode;
    libdvid::BinaryDataPtr data = ZDvid::MakeRequest(
          *conn, "/", "HEAD", libdvid::BinaryDataPtr(),
          libdvid::DEFAULT, statusCode);
    std::cout << data->length() << std::endl;
    //    std::cout << conn->make_head_request("/api/help") << std::endl;
  } catch (exception &e) {
    std::cout << e.what() << std::endl;
  }
#endif

#if 0
  {
    ZJsonParser parser;
    std::cout << parser.getValue<int64_t>(NULL) << std::endl;
  }

  {
    ZJsonObject obj;
    obj.setEntry("test", "hello");

    ZJsonObjectParser parser;
    std::cout << parser.getValue(obj, "test", "") << std::endl;
  }
#endif

#if 0
  ZJsonObject obj;
  obj.setEntry(ZFlyEmBodyStatus::KEY_NAME, "test");
  obj.setEntry(ZFlyEmBodyStatus::KEY_EXPERT, true);
  obj.setEntry(ZFlyEmBodyStatus::KEY_PRIORITY, 1);
  ZFlyEmBodyStatus bodyStatus("");
  bodyStatus.loadJsonObject(obj);
  bodyStatus.toJsonObject().print();
#endif

#if 0
  ZJsonObject obj;

  ZJsonArray statusArrayObj;
  {
    ZFlyEmBodyStatus status("Finalized");
    status.setPriority(0);
    status.setFinal(true);
    status.setProtectionLevel(9);
    statusArrayObj.append(status.toJsonObject());
  }

  {
    ZFlyEmBodyStatus status("Traced");
    status.setPriority(10);
    status.setFinal(false);
    status.setProtectionLevel(9);
    statusArrayObj.append(status.toJsonObject());
  }

  obj.setEntry("status", statusArrayObj);

  obj.print();
  obj.dump(GET_TEST_DATA_DIR + "/test.json");
#endif

#if 0
  ZFlyEmBodyAnnotationMerger annotMerger;
  annotMerger.loadJsonObject(
        GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/body_staus.json");
  annotMerger.print();
#endif


#if 0
  ZDvidTarget target = ZGlobal::GetInstance().getDvidReader("MB_Test")->getDvidTarget();

  QElapsedTimer timer;
  timer.start();

  tic();

  for (int i = 0; i < 1000; ++i) {
//    ZSharedPointer<libdvid::DVIDNodeService> newService =
//        ZDvid::MakeDvidNodeService(service.get());
    ZSharedPointer<libdvid::DVIDNodeService> service = ZDvid::MakeDvidNodeService(target);
    service->get_keys("neutu_config");
  }
  ptoc();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->getDvidTarget().print();

  ZDvidUrl url(reader->getDvidTarget());

  ZNetBufferReader bufferReader;
  std::cout << bufferReader.isReadable(
                 url.getInfoUrl("segmentation_skeletons").c_str()) << std::endl;
#endif

#if 0
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("hemibran-production");
  tic();
  if (reader) {
    reader->readKeyValue("roi_data", "be890b70bcd206be62e81377250e9dbe");
  }
  ptoc();
#endif

#if 0
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("hemibran-production");

  ZFlyEmBodyMergeProject project;
  project.setDvidTarget(reader->getDvidTarget());
  project.getAnnotationMerger().print();

  QMap<uint64_t, ZFlyEmBodyAnnotation> annotMap;

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("anchors");
    annotMap[1] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Anchors");
    annotMap[2] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Finalized");
    annotMap[3] = annot;
  }

  {
    ZFlyEmBodyAnnotation annot;
    annot.setStatus("Roughly traced");
    annotMap[4] = annot;
  }

  std::vector<std::vector<uint64_t>> bodySet =
      project.getAnnotationMerger().getConflictBody(annotMap);
  for (const auto& bodyArray : bodySet) {
    std::cout << "Conflicted:";
    for (uint64_t body : bodyArray) {
      std::cout << " " << body;
    }
    std::cout << std::endl;
  }

  QString msg = project.composeStatusConflictMessage(annotMap);
  qDebug() << msg;

#endif

#if 0
  ZFlyEmRoiManager roiManager;
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("MB_Test");
  roiManager.setDvidTarget(reader->getDvidTarget());

  roiManager.loadRoiList();
//  roiManager.updateMesh("alpha1_below_roi");
  roiManager.updateMesh("mb_subtracted");
  roiManager.print();
#endif

#if 0
  ZFlyEmRoiManager roiManager;
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("hemibran-production");
  roiManager.setDvidTarget(reader->getDvidTarget());

  roiManager.loadRoiList();

  roiManager.updateMesh("vACA");
  roiManager.updateMesh("gL");

  roiManager.print();

  std::cout << roiManager.getMesh("vACA") << std::endl;
  std::cout << roiManager.getMesh("gL") << std::endl;


#endif

#if 0
  ZOptionListWidget *widget = new ZOptionListWidget(NULL);

  widget->setName("test");
  widget->setOptionList(QStringList({"1", "2", "3", "4", "5"}));
  widget->show();
#endif

#if 0

  NeuPrintQueryDialog *dlg = new NeuPrintQueryDialog(host);
  dlg->setRoiList({"1", "2", "3"});
  dlg->exec();

  QStringList inputList = dlg->getInputRoi();
  for (const QString &roi : inputList) {
    qDebug() << roi;
  }

#endif

#if 0
  NeuPrintReader *reader = ZGlobal::GetInstance().getNeuPrintReader();
  reader->updateCurrentDataset("137d");
  if (reader->isReady()) {
    reader->queryTopNeuron(100000);
//    reader->queryAllNamedNeuron().print();
//    reader->queryNeuronByStatus("Roughly traced").print();
  }

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("GT cube chris");
  reader->getDvidTarget().setSegmentationName("segmentation");

  reader->getDvidTarget().print();

  try {
    reader->readLabels64Lowtis(0, 0, 57, 446, 443, 0, 256, 256, false);
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }
#endif

#if 0
  CypherQuery query = CypherQueryBuilder().
      match("(n:label)").where("exists(n.name)").ret("n.name");
  qDebug() << query.getQueryString();
#endif

#if 0
  ZStack *stack = ZStackFactory::LoadFromFile(
        GET_BENCHMARK_DIR + "/rn003/cross_30_0.tif");

  std::cout << stack->sourcePath() << std::endl;

  ZNeuronTracer tracer;
  tracer.setDiagnosis(true);
  tracer.setIntensityField(stack);

  tracer.test();

#endif

#if 0
  ZLineSegment seg(ZPoint(1, 2, 0), ZPoint(3, 4, 5));
  bool visible;
  ZLineSegment seg2 = ZStackObjectPainter::GetFocusSegment(seg, visible, 1);
  if (visible) {
    seg2.print();
  } else {
    std::cout << "Not visible." << std::endl;
  }
#endif

#if 0
  Z3DGraph graph;
  graph.load(GET_TEST_DATA_DIR + "/_test.g3d");
  graph.print();
#endif

#if 0
  Z3DGraph graph;
  {
    Z3DGraphNode node;
    node.set(0, 0, 0, 5);
    node.setColor(QColor(255, 0, 0));
    graph.addNode(node);
  }

  {
    Z3DGraphNode node;
    node.set(100, 100, 10, 5);
    node.setColor(QColor(0, 255, 0));
    graph.addNode(node);
  }

  {
    Z3DGraphNode node;
    node.set(200, 100, 20, 5);
    node.setColor(QColor(255, 255, 0));
    graph.addNode(node);
  }

  {
    Z3DGraphNode node;
    node.set(200, 300, 30, 5);
    node.setColor(QColor(0, 255, 255));
    graph.addNode(node);
  }

  graph.addEdge(0, 1, 10, GRAPH_LINE);
  graph.addEdge(1, 2, 20, GRAPH_LINE);
  graph.addEdge(2, 3, 30, GRAPH_LINE);
  graph.addEdge(0, 3, 40, GRAPH_LINE);

  graph.print();
  graph.save(GET_TEST_DATA_DIR + "/_test.g3d");
#endif

#if 0
  ZCuboid box;
  box.setFirstCorner(0, 0, 0);
  box.setLastCorner(100, 200, 30);
  Z3DGraph *graphObj = Z3DGraphFactory::MakeBox(box, 10.0);
  graphObj->print();

  graphObj->save(GET_TEST_DATA_DIR + "/_test.g3d");

//  std::cout << graphObj->toJsonObject().dumpString() << std::endl;
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/_system/diadem/diadem_e1.Edit.swc");
  tree.setColor(Qt::cyan);
  Z3DGraph graph = Z3DGraphFactory::MakeSwcGraph(tree, 2.0);
  graph.print();
  graph.save(GET_TEST_DATA_DIR + "/_test.g3d");
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/_misc/1.swc");
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/_misc/1.tif");
  ZSwcTree::DepthFirstIterator iter(&tree);
  for (Swc_Tree_Node *tn = iter.begin(); tn != NULL; tn = iter.next()) {
    tn->feature = SwcTreeNode::averageIntensity(tn, stack.c_stack()) / 255.0;
  }
  Z3DGraph graph = Z3DGraphFactory::MakeSwcFeatureGraph(tree);
  graph.save(GET_TEST_DATA_DIR + "/_test.g3d");

#endif

#if 0
  tic();
  KLog log;
  log << ZLog::Category(std::string(__FILE__) + ".testing")
      << ZLog::Diagnostic("testing klog functions") << ZLog::Duration(200);
  log << ZLog::Time() << ZLog::End;

  log << ZLog::Category(std::string(__FILE__) + ".testing")
      << ZLog::Diagnostic("testing klog functions") << ZLog::Duration(300)
      << ZLog::Time() << ZLog::End;

  std::cout << "post log" << std::endl;
//  KLog log;
//  log.logCategory(std::string(__FUNCTION__) + ".testing");
#endif

#if 0
  if (neuopentracing::Tracer::Global()) {
    std::unique_ptr<neuopentracing::Span> span =
        neuopentracing::Tracer::Global()->StartSpan("client");
    span->SetTag("client", "NeuTu");
    span->SetTag("category", "neutu.test");
    span->SetTag("Test", "Test");
  }
#endif

#if 0
//  std::string kafkaBrokers = "localhost:2181";
  std::string kafkaBrokers = "kafka.int.janelia.org:9092";
  if (const char* kafkaBrokersEnv = std::getenv("NEU3_KAFKA_BROKERS")) {

    // The list of brokers should be separated by commans, per this example:
    // https://www.npmjs.com/package/node-rdkafka

    kafkaBrokers = kafkaBrokersEnv;
  }

  auto config = neuopentracing::Config(kafkaBrokers);
  auto tracer = neuopentracing::Tracer::make("neutu", config);
  neuopentracing::Tracer::InitGlobal(tracer);

  tic();
  for (int i = 0; i < 1000; ++i) {
    std::unique_ptr<neuopentracing::Span> span =
        neuopentracing::Tracer::Global()->StartSpan("client");
    span->SetTag("client", "NeuTu");
    span->SetTag("category", "neutu.test");
    span->SetTag("Test", "This is a relatively long message: FlyEM (Electron Microscopic Reconstructing of the Drosophila Nervous System) aims to develop a fully detailed, cellular- and synaptic-resolution map of the central nervous system of Drosophila melanogaster.");
//    span->Finish();
  }
  ptoc();

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();

//  std::vector<uint64_t> bodyList({895441451});
  std::vector<uint64_t> bodyList = ZFlyEmMisc::LoadBodyList(
        GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/test/body_list.txt");

  for (uint64_t body : bodyList) {
    ZObject3dScan obj;
//    reader->readBody(body, true, &obj);
    reader->readBody(
          body, neutu::EBodyLabelType::BODY, 1, ZIntCuboid(), true, &obj);

    size_t byteCount = obj.getByteCount();
    size_t voxelCount = obj.getVoxelNumber();
    std::cout << body << ": #voxel=" << voxelCount << "; "
              << obj.getBoundBox().toString() << "; "
              << "#Byte=" << byteCount << std::endl;
    size_t v = obj.getBoundBox().getVolume();
    ZIntCuboid box = obj.getBoundBox();
    std::cout << box.getWidth() << "x" << box.getHeight() << "x" << box.getDepth() << std::endl;
    std::cout << "Compression ratio: " << double(v) / byteCount << std::endl;
    std::cout << "CRV: " << double(voxelCount) / byteCount * 12 << std::endl;
    std::cout << std::endl;
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();

  int blockCount = reader->readBodyBlockCount(799586652, 1);
  std::cout << blockCount << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();

  std::vector<uint64_t> bodyList = ZFlyEmMisc::LoadBodyList(
        GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/test/allbody.txt");

  std::vector<qint64> ccaTime;
  std::vector<size_t> bodySize;
  std::vector<size_t> ccc;

  QElapsedTimer timer;
  timer.start();
  for (uint64_t body : bodyList) {
    ZObject3dScan obj;
    reader->readBody(
          body, neutu::EBodyLabelType::BODY, 1, ZIntCuboid(), true, &obj);
    timer.restart();
    ccc.push_back(obj.getConnectedComponent(ZObject3dScan::ACTION_CANONIZE).size());
    ccaTime.push_back(timer.elapsed());
    bodySize.push_back(obj.getSegmentNumber());
//    std::cout << "CCA time: " << timer.elapsed() << " ms" << std::endl;
//    std::cout << std::endl;
  }
  std::cout << std::endl;

  std::cout << "Body Size:" << std::endl;
  for (auto s : bodySize) {
    std::cout << s << std::endl;
  }
  std::cout << std::endl;

  std::cout << "#CC: " << std::endl;
  for (auto c : ccc) {
    std::cout << c << std::endl;
  }
  std::cout << std::endl;

  std::cout << "CCA time: " << std::endl;
  for (auto t : ccaTime) {
    std::cout << t << std::endl;
  }

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->updateMaxLabelZoom();

  QElapsedTimer timer;
  timer.start();

  std::vector<uint64_t> bodyList({5813101160});
  std::vector<size_t> bodySize;
  for (uint64_t body : bodyList) {
    ZObject3dScan obj;
    reader->readSupervoxel(body, true, &obj);
    bodySize.push_back(obj.getSegmentNumber());

    timer.restart();
    obj.getConnectedComponent(ZObject3dScan::ACTION_CANONIZE);
    std::cout << "CCA time: " << timer.elapsed() << " ms" << std::endl;
  }
  std::cout << std::endl;

  std::cout << "Body Size:" << std::endl;
  for (auto s : bodySize) {
    std::cout << s << std::endl;
  }
  std::cout << std::endl;
#endif

#if 0
  ZDvidReader reader;
  ZDvidTarget target;
  target.set("emdata2.int.janelia.org", "40c2", 7000);
  reader.open(target);

  ZDvidRoi roi;
  reader.readRoi("ROI_chiasm_body2", &roi);
  std::cout << "Volume: " << roi.getVolume() << std::endl;
#endif

#if 0
  std::cout << ZStackObject::GetTypeName(ZStackObject::EType::SWC) << std::endl;
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetDvidWriter("MB_Test");

  writer->createData("keyvalue", "test3");

  std::cout << writer->isStatusOk() << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().GetDvidReader("hemibrain_test");

  ZJsonObject obj = reader->readBodyStatusV2();

  obj.print();

  ZFlyEmBodyAnnotationMerger merger;
  merger.loadJsonObject(obj);

  merger.print();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("cleave_split_test");
  ZObject3dScan obj;
  reader->readSupervoxel(1104125131, true, &obj);
  std::cout << "Segment number: " << obj.getSegmentNumber() << std::endl;
#endif

#if 0
  std::cout << (std::ostringstream() << "test " << 1).str() << std::endl;
#endif

#if 0
  KINFO << "Test: to kafka only";
  LKINFO << "Test: to both local and kafka";
  ZINFO << "Test: auto logging";

  KWARN << "Test: to kafka only";
  LKWARN << "Test: to both local and kafka";
  ZWARN << "Test: auto logging";

  KERROR << "Test: to kafka only";
  LKERROR << "Test: to both local and kafka";
  ZERROR << "Test: auto logging";

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: no logging", neutu::EMessageType::INFORMATION,
          ZWidgetMessage::TARGET_NULL));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to local only",
          neutu::EMessageType::INFORMATION,
          ZWidgetMessage::TARGET_LOG_FILE));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to kafka only",
          neutu::EMessageType::INFORMATION,
          ZWidgetMessage::TARGET_KAFKA));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to both local and kafka",
          neutu::EMessageType::INFORMATION,
          ZWidgetMessage::TARGET_KAFKA |
          ZWidgetMessage::TARGET_LOG_FILE));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to local only",
          neutu::EMessageType::WARNING,
          ZWidgetMessage::TARGET_LOG_FILE));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to kafka only",
          neutu::EMessageType::WARNING,
          ZWidgetMessage::TARGET_KAFKA));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to both local and kafka",
          neutu::EMessageType::WARNING,
          ZWidgetMessage::TARGET_KAFKA |
          ZWidgetMessage::TARGET_LOG_FILE));


  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to local only",
          neutu::EMessageType::ERROR,
          ZWidgetMessage::TARGET_LOG_FILE));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to kafka only",
          neutu::EMessageType::ERROR,
          ZWidgetMessage::TARGET_KAFKA));

  neutu::LogMessage(
        ZWidgetMessage(
          "Test ZWidgetMessage: to both local and kafka",
          neutu::EMessageType::ERROR,
          ZWidgetMessage::TARGET_KAFKA |
          ZWidgetMessage::TARGET_LOG_FILE));

  KINFO << "Test:" << " to kafka only";
  LKINFO << "Test:" <<  " to both local and kafka" << " " << 0;

  KWARN << "Test:" << " to kafka only";
  LKWARN << "Test:" <<  " to both local and kafka" << " " << 0;

  KERROR << "Test:" << " to kafka only";
  LKERROR << "Test:" <<  " to both local and kafka" << " " << 0;

  KLOG << ZLog::Info() << ZLog::Info() << ZLog::Description("test");
  KLOG << ZLog::Info() << ZLog::Warn() << ZLog::Description("test")
       << ZLog::Description("warn");

#endif

#if 0
  qDebug() << neuopentracing::ToString(neuopentracing::Value(1));
  qDebug() << neuopentracing::ToString(neuopentracing::Value(true));
  qDebug() << neuopentracing::ToString(neuopentracing::Value("test"));

#endif

#if 0
  ZObject3dScan *obj = flyem::LoadRoiFromJson(
        GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/rois/0.part.json");
  obj->save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0

  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  reader->readLabels64Lowtis(0, 0, 20696, 268 * 128, 310 * 128, 6, true, 256, 256);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibrain_test");
  FlyEmDataConfig config = FlyEmDataReader::ReadDataConfig(*reader);
  config.print();

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibrain_test");

  ZFlyEmProofDoc doc;
  doc.setDvidTarget(reader->getDvidTarget());
  doc.test();

//  doc.setSupervoxelMode(true);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibrain_test");
  ZDvidLabelSlice *labelSlice = new ZDvidLabelSlice;
  ZDvidTarget target = reader->getDvidTarget();
  target.setSupervoxelView(true);
  labelSlice->setDvidTarget(target);
  std::cout << labelSlice->isSupervoxel() << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  reader->readJsonObject(
        "http://emdata4.int.janelia.org:8900/api/node/b98b4829e305479ca7ac4b17194c425b/neutu_config/key/data_status");

  reader->readJsonObjectFromKey("neutu_config", "contrast");
  reader->readJsonObject(
        "http://emdata4.int.janelia.org:8900/api/node/b98b4829e305479ca7ac4b17194c425b/neutu_config/key/contrast");


#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("MB_Test");

  reader->getDvidTarget().print();
  std::cout << "Has synapse: " << reader->getDvidTarget().hasSynapse() << std::endl;
  std::cout << "Has synapse labelsz: " << reader->getDvidTarget().hasSynapseLabelsz() << std::endl;
  std::cout << reader->getDvidTarget().getSynapseLabelszName() << std::endl;
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibran-production");
  std::string roiName = "PVLP";
  ZMesh *mesh =
      FlyEmDataReader::ReadRoiMesh(writer->getDvidReader(), roiName);
  std::string filePath = GET_TEST_DATA_DIR + "/test.drc";
  mesh->save(filePath.c_str());
  writer->uploadRoiMesh(filePath, roiName);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibran-production");
  std::string roiName = "AL-DC3";
  ZObject3dScan obj = writer->getDvidReader().readRoi(roiName);
  ZMeshFactory mf;
  mf.setSmooth(3);
  ZMesh *mesh = mf.makeMesh(obj);

  std::string filePath = GET_TEST_DATA_DIR + "/test.drc";
  mesh->save(filePath.c_str());
  writer->uploadRoiMesh(filePath, roiName);
#endif

#if 0
  ZFlyEmBodyAnnotation annot;
  ZJsonObject obj;
  obj.load(GET_FLYEM_DATA_DIR + "/test/json/body_annot.json");
  annot.loadJsonObject(obj);
  annot.print();
  std::cout << annot.toJsonObject().dumpString(2) << std::endl;

  ZFlyEmBodyAnnotation annot2;
//  annot2.loadJsonObject(annot.toJsonObject());
//  std::cout << "compare: " << (annot == annot2) << std::endl;

  FlyEmBodyAnnotationDialog *dlg = new FlyEmBodyAnnotationDialog(host);
  dlg->loadBodyAnnotation(annot);
  dlg->exec();

  annot2 = dlg->getBodyAnnotation();
  annot2.print();
  std::cout << annot2.toJsonObject().dumpString(2) << std::endl;
  std::cout << "compare: " << (annot == annot2) << std::endl;

#endif

#if 0

  ZFlyEmBodyAnnotation annot;
  ZJsonObject obj;
  obj.load(GET_FLYEM_DATA_DIR + "/test/json/body_annot.json");
  annot.loadJsonObject(obj);
  annot.print();

  ZFlyEmBodyAnnotation annot2;
  ZJsonObject obj2;
  obj2.load(GET_FLYEM_DATA_DIR + "/test/json/body_annot2.json");
  annot2.loadJsonObject(obj2);
  annot2.print();

  annot.mergeAnnotation(annot2, &ZFlyEmBodyAnnotation::GetStatusRank);
  annot.print();

#endif

#if 0
  std::cout << ZNetworkUtils::HasHead("http://emdata2.int.janelia.org:2018");
#endif

#if 0
  try {
    http::Request request("http://emdata2.int.janelia.org:2018");

//    http::Request request("http://127.0.0.1:1600/api/node/4280/grayscale/info");

    http::Response response = request.send("GET");
    std::string str(response.body.begin(), response.body.end());
    std::cout << "Code: " << response.code << std::endl;
    std::cout << response.code << " " << str << std::endl;
  } catch (std::exception &e) {
    std::cout << e.what() << std::endl;
  }

#endif


#if 0
//  try
//  {
      // you can pass http::InternetProtocol::V6 to Request to make an IPv6 request


      // send a get request
      tic();
      for (size_t i = 0; i < 5000; ++i) {
        http::Request request("http://emdata1.int.janelia.org:8500/api/node/babdf6dbc23e44a69953a66e2260ff0a/bodies3/info");
        http::Response response = request.send("GET");
//        std::string str(response.body.begin(), response.body.end());
//        std::cout << i << ": " << str << std::endl; // print the result
      }
      ptoc();

      {
      http::Request request("http://emdata1.int.janelia.org:8500/api/node/babdf6dbc23e44a69953a66e2260ff0a/bodies3/info");
      tic();
      for (size_t i = 0; i < 5000; ++i) {

        http::Response response = request.send("GET");
//        std::string str(response.body.begin(), response.body.end());
//        std::cout << i << ": " << str << std::endl; // print the result
      }
      ptoc();
      }
      // send a post request
//      response = request.send("POST", "foo=1&bar=baz", {
//          "Content-Type: application/x-www-form-urlencoded"
//      });
//      std::cout << response.body.data() << std::endl; // print the result

      // pass parameters as a map
//      std::map<std::string, std::string> parameters = {{"foo", "1"}, {"bar", "baz"}};
//      response = request.send("POST", parameters, {
//          "Content-Type: application/x-www-form-urlencoded"
//      });
//      std::cout << std::string(response.body.begin(), response.body.end()) << std::endl; // print the result
//  }
//  catch (const std::exception& e)
//  {
//      std::cerr << "Request failed, error: " << e.what() << std::endl;
//  }

//  ZDvidReader *reader = ZGlobal().GetInstance().getDvidReader("MB_Test");
//  tic();
//  reader->setVerbose(false);
//  for (size_t i = 0; i < 5000; ++i) {
//    reader->readDataInfo("bodies3");
//  }
//  ptoc();

//  tic();
//  ZNetBufferReader nreader;
//  for (size_t i = 0; i < 5000; ++i) {
//    nreader.read("http://emdata1.int.janelia.org:8500/api/node/babdf6dbc23e44a69953a66e2260ff0a/bodies3/info", false);
//  }
//  ptoc();
#endif

#if 0
  NeutubeConfig::getInstance().print();

  ZFlyEmConfig::getInstance().print();

  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  neutuse::Task task = neutuse::TaskFactory::MakeDvidSkeletonizeTask(
        reader->getDvidTarget(), 1, true);
  std:cout << task.toJsonObject().dumpString(2) << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  auto masterList = reader->readMasterList();
  std::for_each(masterList.begin(), masterList.end(), [](const std::string &node) {
    std::cout << node << std::endl;
  });

  std::cout << reader->readMasterNode() << std::endl;
#endif

#if 0
  ZDvidNeuronTracer tracer;
  ZDvidTarget target;
  target.set("localhost", "4d3e", 8000);
  target.setGrayScaleName("grayscale");
  tracer.setDvidTarget(target);
  tracer.trace(505, 1075, 61, 3.0);

  ZSwcTree *tree = tracer.getResult();
  tree->save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
//  ZDvidTarget target = ZGlobalDvidRepo::GetInstance().getDvidTarget("MB_Synapse_Shift");
  ZDvidTarget target = ZGlobalDvidRepo::GetInstance().getDvidTarget("hemibran-production");
  std::string master = ZDvidReader::ReadMasterNode(target);
  std::cout << "Master node: " << master << std::endl;

  auto masterList = ZDvidReader::ReadMasterList(target);
  std::for_each(masterList.begin(), masterList.end(), [](const std::string &node) {
    std::cout << node << std::endl;
  });
#endif

#if 0
  std::vector<std::string> fileArray = {
    GET_FLYEM_DATA_DIR + "/roi/20190521FB/15_0.20_15_2.part.json",
    GET_FLYEM_DATA_DIR + "/roi/20190521FB/15_0.20_15_3.part.json",
    GET_FLYEM_DATA_DIR + "/roi/20190521FB/15_0.20_15_4.part.json",
    GET_FLYEM_DATA_DIR + "/roi/20190521FB/15_0.20_15_6.part.json"
  };

  for (const std::string inputPath : fileArray) {
    ZObject3dScan obj;
    //  std::string inputPath =
    //      GET_FLYEM_DATA_DIR + "/roi/20190521FB/15_0.20_15_2.part.json";
    flyem::LoadRoiFromJson(inputPath, &obj);

    obj.save(inputPath + ".sobj");

    ZStack *stack = ZStackFactory::MakeZeroStack(1088, 1280, 1344);
    obj.upSample(3, 3, 3);
    obj.drawStack(stack, 255);
    stack->save(inputPath + ".tif");
    delete stack;
  }
#endif

#if 0
  ZFileList fileList;
  fileList.load(
        GET_FLYEM_DATA_DIR + "/roi/2019AugDiffused/Diffused_50_0.2_30", "json",
        ZFileList::SORT_BY_LAST_NUMBER);

  std::vector<std::string> fileArray(fileList.size());
  for (int i = 0; i < fileList.size(); ++i) {
    fileArray[i] = fileList.getFilePath(i);
  }

  for (const std::string inputPath : fileArray) {
    if (ZString(inputPath).endsWith("29.part.json")) {
      ZObject3dScan obj;
      flyem::LoadRoiFromJson(inputPath, &obj);

      obj.save(inputPath + ".sobj");

      ZStack *stack = ZStackFactory::MakeZeroStack(1088, 1280, 1344);
      obj.upSample(3, 3, 3);
      obj.drawStack(stack, 255);
      stack->save(inputPath + ".tif");
      delete stack;
    }
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  std::vector<std::string> roiList = { "(L)NO1", "(L)NO2", "(L)NO3" };

  ZObject3dScan roi;
  FlyEmDataReader::ReadRoi(*reader, roiList, &roi);
  roi.save(GET_TEST_DATA_DIR + "/_flyem/roi/NO_Left.sobj");

  ZMesh *mesh = ZMeshFactory::MakeMesh(roi);
  mesh->save(GET_TEST_DATA_DIR + "/_flyem/roi/NO_Left.obj");
//  ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  std::vector<std::string> roiList = { "NO1", "NO2", "NO3" };

  ZObject3dScan roi;
  FlyEmDataReader::ReadRoi(*reader, roiList, &roi);
  roi.save(GET_TEST_DATA_DIR + "/_flyem/roi/NO_Right.sobj");

  ZMesh *mesh = ZMeshFactory::MakeMesh(roi);
  mesh->save(GET_TEST_DATA_DIR + "/_flyem/roi/NO_Right.obj");
//  ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  FlyEmDataWriter::UploadRoi(
        *writer, "NO(Left)",
        GET_FLYEM_DATA_DIR + "/roi/NO_Left.sobj",
        GET_FLYEM_DATA_DIR + "/roi/NO_Left.obj");

  FlyEmDataWriter::UploadRoi(
        *writer, "NO(Right)",
        GET_FLYEM_DATA_DIR + "/roi/NO_Right.sobj",
        GET_FLYEM_DATA_DIR + "/roi/NO_Right.obj");

#endif

#if 0
  ZObject3dScan obj;
  obj.load(GET_TEST_DATA_DIR + "/_test.sobj");
  ZMesh *mesh = ZMeshFactory::MakeMesh(obj);
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
  mesh->generateNormals();
  ZMesh mesh2 = ZMeshUtils::Decimate(*mesh);
  mesh2.save(GET_TEST_DATA_DIR + "/_test2.obj");

  ZMesh mesh3 = ZMeshUtils::Smooth(mesh2);
  mesh3.save(GET_TEST_DATA_DIR + "/_test3.obj");
#endif


#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  FlyEmDataWriter::UploadRoi(
        *writer, "FB07",
        GET_FLYEM_DATA_DIR + "/roi/2019May_FB/FB07.labels.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/2019May_FB/FB07.labels.tif.obj");

  FlyEmDataWriter::UploadRoi(
        *writer, "FB08v",
        GET_FLYEM_DATA_DIR + "/roi/2019May_FB/FB08v.labels.tif.sobj",
        GET_FLYEM_DATA_DIR + "/roi/2019May_FB/FB08v.labels.tif.obj");

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  ZMesh *mesh = ZFlyEmMeshFactory::MakeRoiMesh(*reader, "(L)NO3");
  mesh->save(GET_TEST_DATA_DIR + "/_flyem/roi/LNO3.drc");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");

  std::string filePath = GET_TEST_DATA_DIR + "/_flyem/roi/LNO3.drc";
  std::cout << "Uploading " << filePath << std::endl;
  writer->uploadRoiMesh(filePath, "(L)NO3");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemibran-production");
  writer->deleteKey("rois", "(L)NO2D");
  writer->deleteKey("rois", "(L)NO2V");
#endif

#if 0
  neutu::LogMessageF("test", neutu::EMessageType::INFORMATION);
#endif

#if 0
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("cleave_split_test");

  ZNeuroglancerPath gpath;

  ZNeuroglancerNavigation nav;
  nav.setVoxelSize(8, 8, 8);
  nav.setCoordinates(15000, 15000, 10000);
  gpath.setNavigation(nav);

  gpath.addLayer(
        ZNeuroglancerLayerSpecFactory::MakeGrayscaleLayer(
          reader->getDvidTarget()));
  gpath.addLayer(
        ZNeuroglancerLayerSpecFactory::MakeSegmentationLayer(
          reader->getDvidTarget()));

  std::shared_ptr<ZNeuroglancerAnnotationLayerSpec> annotLayer =
      ZNeuroglancerLayerSpecFactory::MakePointAnnotationLayer(
        "segmentation");
  annotLayer->setVoxelSize(8, 8, 8);
  gpath.addLayer(
        std::dynamic_pointer_cast<ZNeuroglancerLayerSpec>(annotLayer), true);

  std::cout << "Neuroglancer path: " << gpath.getPath() << std::endl;

  QUrl url(gpath.getPath().c_str());
  qDebug() << url.toEncoded();
#endif

#if 0
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("cleave_split_test");
  QString path = ZNeuroglancerPathFactory::MakePath(
        reader->getDvidTarget(), ZIntPoint(8, 8, 8), ZPoint(15000, 15000, 10000));
  qDebug() << GET_FLYEM_CONFIG.getNeuroglancerServer() + path.toStdString();
#endif

#if 0
  ZDvidReader *reader =
      ZGlobal::GetInstance().getDvidReader("cleave_split_test");
  std::cout << reader->hasData("segmentation") << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.setFromSourceString("http:localhost:1900:uuid::grayscale");
  target.toJsonObject().print();
#endif

#if 0
  //Generate data for hbtest/tif
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");

  ZDvidTarget target = reader->getDvidTarget().getGrayScaleTarget();
  target.setGrayScaleName("grayscale");
  ZDvidReader greader;
  greader.open(target);

  ZIntPoint s(2048, 2048, 64);
  for (int i = 0; i < 32; ++i) {
    ZIntPoint start(13452, 20071, 20696 + s.getZ() * i);
    ZIntCuboid box = ZIntCuboid(start, start + s - 1);
    ZStack *stack = greader.readGrayScale(box);
    std::cout << "=====> box: " << box.toString() << std::endl;

    ZString output = "/Volumes/FlyEM-Data1/data/hbtest2/tif/grayscale/";
    output.appendNumber(i + 1, 4);
    output += ".tif";
    stack->save(output);
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZDvidWriter writer;
  ZDvidTarget target;
  target.set("127.0.0.1", "4280", 1600);
  target.setSegmentationName("segmentation");
  writer.open(target);

  ZIntPoint s(512, 512, 64);
  ZIntPoint sourceStart(13452, 20071, 20696);

  for (int k = 0; k < 32; ++k) {
    for (int j = 0; j < 4; ++j) {
      for (int i = 0; i < 4; ++i) {
        ZIntPoint dest = s * ZIntPoint(i, j, k);
        ZIntPoint start(sourceStart + dest);
        ZIntCuboid box = ZIntCuboid(start, start + s - 1);
        ZArray *array = reader->readLabels64(box);
        array->setStartCoordinate(dest.getX(), dest.getY(), dest.getZ());
        std::cout << start.toString() << " ==> " << dest.toString() << std::endl;
        writer.writeLabel(*array);
        delete array;
      }
    }
  }
#endif

#if 0
//  QUrl url("http://localhost:8080/test!");
  QUrl url("http://localhost:8080/#!%7B%22layers%22:%5B%7B%22source%22:%22dvid://http://emdata3.int.janelia.org:8600/a89e/grayscale%22%2C%22type%22:%22image%22%2C%22name%22:%22grayscale%22%7D%2C%7B%22source%22:%22dvid://http://emdata2.int.janelia.org:7900/4151/segmentation%22%2C%22type%22:%22segmentation%22%2C%22segments%22:%5B%222%22%5D%2C%22name%22:%22segmentation%22%7D%2C%7B%22tool%22:%22annotatePoint%22%2C%22type%22:%22annotation%22%2C%22voxelSize%22:%5B8%2C8%2C8%5D%2C%22linkedSegmentationLayer%22:%22segmentation%22%2C%22name%22:%22annotation%22%7D%5D%2C%22navigation%22:%7B%22pose%22:%7B%22position%22:%7B%22voxelSize%22:%5B8%2C8%2C8%5D%2C%22voxelCoordinates%22:%5B17032%2C20889%2C25479%5D%7D%2C%22orientation%22:%5B0.04880080372095108%2C-0.527595043182373%2C0.011728000827133656%2C0.8480120301246643%5D%7D%2C%22zoomFactor%22:8%7D%2C%22perspectiveOrientation%22:%5B0.04880080372095108%2C-0.527595043182373%2C0.011728000827133656%2C0.8480120301246643%5D%2C%22perspectiveZoom%22:64%2C%22showSlices%22:false%2C%22selectedLayer%22:%7B%22layer%22:%22annotation%22%2C%22visible%22:true%7D%2C%22layout%22:%224panel%22%7D");
  qDebug() << url.fragment(QUrl::FullyDecoded);

  std::cout << neutu::Length("test") << std::endl;
#endif

#if 0
  QUrl url("http://emdata3.int.janelia.org:8600/a89e/grayscale");
  qDebug() << url.path().split("/", QString::SkipEmptyParts);
#endif

#if 0
  ZDvidEnv env = ZNeuroglancerPathParser::MakeDvidEnvFromUrl(
        "http://localhost:8080/#!%7B%22layers%22:%5B%7B%22source%22:%22"
        "dvid://http://emdata3.int.janelia.org:8600/a89e/grayscale%22%2C%22"
        "type%22:%22image%22%2C%22name%22:%22grayscale%22%7D%2C%7B%22source%22:%22"
        "dvid://http://emdata2.int.janelia.org:7900/4151/segmentation%22%2C%22"
        "type%22:%22segmentation%22%2C%22segments%22:%5B%222%22%5D%2C%22"
        "name%22:%22segmentation%22%7D%2C%7B%22tool%22:%22annotatePoint%22%2C%22"
        "type%22:%22annotation%22%2C%22voxelSize%22:%5B8%2C8%2C8%5D%2C%22"
        "linkedSegmentationLayer%22:%22segmentation%22%2C%22name%22:%22"
        "annotation%22%7D%5D%2C%22navigation%22:%7B%22pose%22:%7B%22"
        "position%22:%7B%22voxelSize%22:%5B8%2C8%2C8%5D%2C%22"
        "voxelCoordinates%22:%5B17032%2C20889%2C25479%5D%7D%2C%22"
        "orientation%22:%5B0.04880080372095108%2C-0.527595043182373%2C"
        "0.011728000827133656%2C0.8480120301246643%5D%7D%2C%22"
        "zoomFactor%22:8%7D%2C%22perspectiveOrientation%22:%5B"
        "0.04880080372095108%2C-0.527595043182373%2C0.011728000827133656%2C"
        "0.8480120301246643%5D%2C%22perspectiveZoom%22:64%2C%22showSlices%22:"
        "false%2C%22selectedLayer%22:%7B%22layer%22:%22annotation%22%2C%22"
        "visible%22:true%7D%2C%22layout%22:%224panel%22%7D");
  std::cout << env.toJsonObject().dumpString() << std::endl;
#endif


#if 0
  ZStack *stack = new ZStack;
  stack->load(GET_BENCHMARK_DIR + "/fork2/fork2.tif");

  ZNeuronTracer tracer;
  tracer.setIntensityField(stack);
  tracer.setTraceLevel(0);

  ZSwcTree tree;
  tree.load(GET_BENCHMARK_DIR + "/fork2/partial.swc");
  tracer.trace(64, 64, 61, &tree);

  tree.resortId();
  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZJsonObject obj = reader->getDvidTarget().toJsonObject();
  obj.dump(GET_TEST_DATA_DIR + "/_test/dvid_setting.json");
#endif

#if 0
  qDebug() << QSysInfo::prettyProductName()
           << QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
#endif

#if 0
  std::set<int> s({1, 2, 3, 4, 5});
  neutu::setremoveif(s, [](int x)->bool {
    return (x%2 == 0);
  });

  std::for_each(s.begin(), s.end(), [](int x) {
    std::cout << x << " ";
  });
  std::cout << std::endl;
#endif


#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("light");
  ZJsonObject obj;
  obj.setEntry("tracing", true);
  writer->writeJson("neutu_config", "meta", obj);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZObject3dScan obj;
  reader->readSupervoxel(1877429256, true, &obj);
  obj.save(GET_TEST_DATA_DIR + "/test.sobj");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemibran-production");
  ZDvidUrl dvidUrl(reader->getDvidTarget());
  uint64_t svId = 1877429256;
  std::string url = dvidUrl.getSupervoxelMeshUrl(svId);
  std::cout<< url << std::endl;
#endif

#if 0
  QString str = "123->456";
  qDebug() << str.split("->", QString::SkipEmptyParts);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibrain-test");
  QStringList roiList = writer->getDvidReader().readKeys("rois");
  qDebug() << roiList.size();

  QStringList preservedList;
  preservedList << "AL-DC3" << "FB07" << "FB08v";
//  for (const QString &preserved : preservedList) {
//    qDebug() << roiList.indexOf(preserved);
//  }

  for (const QString &roi : roiList) {
    if (preservedList.indexOf(roi) >= 0) {
      qDebug() << "Preserved: " << roi;
    } else {
      writer->deleteKey("rois", roi);
    }
  }
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemibrain-test");

  QDir dir((GET_FLYEM_DATA_DIR + "/roi/element").c_str());

  qDebug() << dir;

  QStringList fileList = dir.entryList(QStringList() << "*.obj");
  qDebug() << fileList;
  for (const QString &meshFilename : fileList) {
    qDebug() << dir.filePath(meshFilename);
    QString roiName = meshFilename.left(
          meshFilename.length() - QString(".tif.obj").length());
    qDebug() << roiName;
    QString meshFilePath = dir.filePath(meshFilename);
    QString roiFilePath = dir.filePath(roiName + ".tif.sobj");
    if (!QFileInfo(meshFilePath).exists()) {
      qDebug() << meshFilePath << " does not exist. Abort!";
      break;
    }
    if (!QFileInfo(roiFilePath).exists()) {
      qDebug() << roiFilePath << " does not exist. Abort!";
      break;
    }
    FlyEmDataWriter::UploadRoi(
          *writer, roiName.toStdString(),
          roiFilePath.toStdString(),
          meshFilePath.toStdString());
  }
#endif

#if 0
  Rgb_Color color;
  color.r = 255;
  color.g = 0;
  color.b = 0;

  double h = 0.0;
  double s = 0.0;
  double v = 0.0;

  Rgb_Color_To_Hsv(&color, &h, &s, &v);
  std::cout << h << " " << s << " " << v << std::endl;

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("cleave_test");
  std::vector<uint64_t> bodyArray({1011728599, 1817893929, 1946108371});
  auto result = reader->readBodySize(
        bodyArray, neutu::EBodyLabelType::SUPERVOXEL);
  for (size_t i = 0; i < result.size(); ++i) {
    std::cout << reader->readBodySize(
                   bodyArray[i], neutu::EBodyLabelType::SUPERVOXEL) << " == ";
    std::cout << result[i] << std::endl;
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("cleave_test");
  std::vector<uint64_t> bodyArray(
    {703282980, 767123688, 1011728599, 1011996877, 1162081064 });
  auto result = reader->readBodySize(
        bodyArray, neutu::EBodyLabelType::BODY);
  for (size_t i = 0; i < result.size(); ++i) {
    std::cout << reader->readBodySize(
                   bodyArray[i], neutu::EBodyLabelType::BODY) << " == ";
    std::cout << result[i] << std::endl;
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");

  flyem::ZFileParser parser;

  uint64_t bodyId = 5813026514;
  ZObject3dScan obj;
  reader->readCoarseBody(bodyId, &obj);
  auto sizeArray = obj.getConnectedObjectSize();
  if (sizeArray.size() > 1) {
    int bigPartCount = 0;
    for (auto s : sizeArray) {
      if (s > 10000) {
        ++bigPartCount;
      }
    }
    if (bigPartCount > 1) {
      for (auto s : sizeArray) {
        std::cout << s << " ";
      }
      std::cout << std::endl;
    }
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");

  std::ofstream outStream("/Users/zhaot/Work/neutu/neurolabi/data/badbody.txt");
  flyem::ZFileParser parser;
  map<uint64_t, string> bodyMap =
      parser.loadBodyList("/Users/zhaot/Work/neutu/neurolabi/data/test.txt");
  for (auto p : bodyMap) {
    uint64_t bodyId = p.first;
    ZObject3dScan obj;
    reader->readCoarseBody(bodyId, &obj);
    auto sizeArray = obj.getConnectedObjectSize();
    if (sizeArray.size() > 1) {
      int bigPartCount = 0;
      for (auto s : sizeArray) {
        if (s > 5000) {
          ++bigPartCount;
        }
      }
      if (bigPartCount > 1) {
        outStream << bodyId << ": ";
        for (auto s : sizeArray) {
          outStream << s << " ";
        }
        outStream << std::endl;
      }
    }
  }
#endif

#if 0
  std::cout << neutu::FileExists(GET_BENCHMARK_DIR + "/em_slice.tif") << std::endl;
  std::cout << neutu::FileExists(GET_BENCHMARK_DIR + "/em_slice_nonexisting.tif") << std::endl;
  std::cout << neutu::FileExists(GET_BENCHMARK_DIR) << std::endl;

  std::vector<double> array({1, 2});
  std::swap(array[0], array[1]);
  std::cout << array[0] << " " << array[1] << std::endl;

  std::ifstream s(GET_BENCHMARK_DIR);
  std::cout << s.good() << std::endl;

  {
    ZFileList fileList(3);
    std::cout << fileList.size() << std::endl;
  }

  {
    ZFileList fileList;
    fileList.load(
          GET_BENCHMARK_DIR, neutu::FileExtension("test.tif"),
          ZFileList::SORT_ALPHABETICALLY);
    std::cout << fileList.size() << std::endl;
    for (int i = 0; i < fileList.size(); ++i) {
      std::cout << fileList.getFilePath(i) << " "
                << neutu::FileSize(fileList.getFilePath(i))
                << std::endl;
    }
  }

  std::cout << neutu::FileExtension("test.tif") << std::endl;
  std::cout << neutu::Join({"test1", "test2"}) << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  reader->setVerbose(false);
  std::ofstream stream(GET_TEST_DATA_DIR + "/bodylist.txt");

  neutuse::TaskWriter neutuseWriter;
  neutuseWriter.open("http://emdata2.int.janelia.org:2018");

  tic();
  QStringList keyList =
      reader->readKeys(reader->getDvidTarget().getBodyAnnotationName().c_str());
  int index = 0;
  double totalLength = 0.0;
  for (const QString &key : keyList) {
    ++index;
    uint64_t bodyId = ZString(key.toStdString()).firstUint64();
    if (bodyId > 0) {
      ZFlyEmBodyAnnotation annot =
          FlyEmDataReader::ReadBodyAnnotation(*reader, bodyId);
      if (annot.getStatus() == "Roughly traced" ||
          annot.getStatus() == "Traced" ||
          annot.getStatus() == "Leaves") {
        std::cout << index << "/" << keyList.size() << ": "
                  << bodyId << " " << annot.getStatus() << std::endl;
        ZSwcTree *tree = reader->readSwc(bodyId);
        double length = 0.0;
        if (tree) {
          length = tree->length();
          totalLength += length;
        }
        if (length == 0.0) {
          size_t bodySize = reader->readBodySize(bodyId);
          if (bodySize > 30000000 && bodyId != 106979579) {
            stream << bodyId << std::endl;
            std::cout << index << "/" << keyList.size()
                      << " Missing skeleton: " << bodyId << " " << bodySize << std::endl;
            neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
                  "skeletonize", reader->getDvidTarget(), bodyId, false);
            task.setPriority(8);

            neutuseWriter.uploadTask(task);
          }
        }
      }

    }
  }
  ptoc();
  std::cout << keyList.size() << " bodies" << std::endl;
  std::cout << totalLength << std::endl;
  stream.close();
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("local_test");
  FlyEmDataWriter::TransferKeyValue(
        *reader, "neutu_config", "user_status",
        *writer, "", "user_status2");
//  writer->uploadRoiMesh(GET_TEST_DATA_DIR + "/test.obj", "test");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi_roi");
  reader->setVerbose(false);
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("local_test");

  ZJsonArray objArray;
  objArray.load(GET_TEST_DATA_DIR + "/_flyem/FIB/hemibrain/roi.json");
  ZJsonObjectParser parser;
  std::vector<std::string> errorMsgList;
  for (size_t i = 0; i < objArray.size(); ++i) {
    ZJsonObject transfer(objArray.value(i));
    std::string source = parser.getValue(transfer, "source", "");
    std::string target = parser.getValue(transfer, "target", "");
    std::cout << source << " --> " << target << std::endl;
    FlyEmDataWriter::TransferRoi(
          *reader, source, *writer, target, [&](const std::string &msg) {
      errorMsgList.push_back(msg);
    });
  }

  for (const std::string &msg : errorMsgList) {
    std::cout << msg << std::endl;
  }

//  FlyEmDataWriter::TransferRoiData(*reader, "(L)AB", *writer, "");
//  FlyEmDataWriter::TransferRoiRef(*reader, "(L)AB", *writer, "");
//  FlyEmDataWriter::TransferRoi(*reader, "(L)AL", *writer, "");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemi");
  FlyEmDataWriter::TransferRoiData(*reader, "IPS", *writer, "IPS(R)");
  FlyEmDataWriter::TransferRoiRef(*reader, "IPS", *writer, "IPS(R)");
  FlyEmDataWriter::TransferRoi(*reader, "IPS", *writer, "IPS(R)");
#endif

#if 0
  ZStackReader reader;
  ZStack *stack = reader.Read(
        "dvid://127.0.0.1:1600/c315/"
        "grayscale?x0=728&y0=706&z0=980&width=512&height=512&depth=1");
  ZStackWriter writer;
  writer.write(GET_TEST_DATA_DIR + "/_test.tif", stack);
#endif

#if 0
  ZStack *stack = ZStackReader::Read(GET_TEST_DATA_DIR + "/_test/stack_block.json");
  ZStackWriter writer;
  writer.write(GET_TEST_DATA_DIR + "/_test.tif", stack);
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  std::cout << FlyEmDataReader::IsSkeletonSynced(*reader, 1198132041) << std::endl;
#endif

#if 0
  ZString s = "<json>{\"test\": 1}</json>";
  if (s.startsWith("<json>") && s.endsWith("</json>")) {
    std::cout << s.substr(6, s.length() - 13).c_str() << std::endl;
  }
#endif

#if 0
  ZStack stack;
  stack.load(GET_TEST_DATA_DIR + "/_system/tracing/30_18_10.tif");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  ZDvidRoi roi;
  std::string roiName = "FB-column3";
  reader->readRoi(roiName, &roi);
  ZMesh *mesh = ZMeshFactory::MakeMesh(*roi.getRoiRef());
  std::string meshPath = GET_TEST_DATA_DIR + "/_test.drc";
  mesh->save(meshPath);

  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemi");
  writer->uploadRoiMesh(meshPath, roiName);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("local_test");
  ZObject3dScan roi;
  roi.addSegment(2, 4, 3, 10);
  FlyEmDataWriter::WriteRoiData(*writer, "test3", roi);
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("local_test");
  std::vector<std::string> sourceList({"test", "test2", "test3"});
  FlyEmDataWriter::TransferRoiData(
        writer->getDvidReader(), sourceList, *writer, "test4");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemi");
  FlyEmDataWriter::TransferRoi(
        writer->getDvidReader(), "LAL(R)", *writer, "LAL(-GA)(R)");
  FlyEmDataWriter::TransferRoi(
        writer->getDvidReader(), "CRE(R)", *writer, "CRE(-ROB,-RUB)(R)");
  FlyEmDataWriter::TransferRoi(
        writer->getDvidReader(), "SAD", *writer, "SAD(-AMMC)");
#endif

#if 0
  ZDvidWriter *writer = ZGlobal::GetInstance().getDvidWriter("hemi");
  {
    std::vector<std::string> roiList({"LAL(-GA)(R)", "GA(R)"});
    FlyEmDataWriter::TransferRoiData(
          writer->getDvidReader(), roiList, *writer, "LAL(R)");
  }

  {
    std::vector<std::string> roiList({"CRE(-ROB,-RUB)(R)", "ROB(R)", "RUB(R)"});
    FlyEmDataWriter::TransferRoiData(
          writer->getDvidReader(), roiList, *writer, "CRE(R)");
  }

  {
    std::vector<std::string> roiList({"SAD(-AMMC)", "AMMC"});
    FlyEmDataWriter::TransferRoiData(
          writer->getDvidReader(), roiList, *writer, "SAD");
  }
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  ZDvidRoi roi;
  std::string roiName = "SAD";
  reader->readRoi(roiName, &roi);
  ZMesh *mesh = ZMeshFactory::MakeMesh(*roi.getRoiRef());
  std::string meshPath = GET_TEST_DATA_DIR + "/_test.drc";
  mesh->save(meshPath);

  ZDvidWriter *writer = ZGlobal::GetInstance().GetDvidWriter("hemi");
  writer->uploadRoiMesh(meshPath, roiName);

#endif

#if 1
  int x;
  std::cout << "neulib.core test: " << neulib::ToString(&x) << std::endl;
#endif

#if 0
  neutu::RangePartitionProcess(1, 10, 1, [](int x0, int x1) {
    std::cout << "[" << x0 << ", " << x1 << "]" << std::endl;
  });

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("local_test");
  reader->updateMaxLabelZoom();
  ZObject3dScan obj;
  dvid::SparsevolConfig config;
  config.bodyId = 947573616;
  config.labelType = neutu::EBodyLabelType::BODY;
  config.format = "blocks";
  config.zoom = 0;
  reader->readBodyWithPartition(config, true, 5, &obj);
  std::cout << obj.isCanonizedActually() << std::endl;

  ZObject3dScan obj2;
  reader->readBody(config.bodyId, config.labelType, config.zoom, ZIntCuboid(), true, &obj2);
  std::cout << obj.equalsLiterally(obj2) << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  reader->updateMaxLabelZoom();

  dvid::SparsevolConfig config;
  config.bodyId = 425790257;
//  config.format = "blocks";
//  config.range = ZIntCuboid(0, 0, 14720, -1, -1, 14800);
//  config.zoom = 2;
  config.labelType = neutu::EBodyLabelType::BODY;

  size_t nvoxels = 0;
  size_t nblocks = 0;
  ZIntCuboid box;
  std::tie(nvoxels, nblocks, box) =
      reader->readBodySizeInfo(config.bodyId, config.labelType);
  std::cout << "#nblocks: " << nblocks << std::endl;
  std::cout << "#nvoxels: " << nvoxels << std::endl;

//  config.range.setFirstZ(box.getFirstZ());
//  config.range.setLastZ(box.getFirstZ() + box.getDepth() / 2);

//  ZDvidUrl dvidUrl(reader->getDvidTarget());
//  QByteArray buffer = reader->readBuffer(dvidUrl.getSparsevolUrl(config));
//  std::cout << buffer.size() << std::endl;

  ZObject3dScan obj;
//  obj.importDvidBlockBuffer(buffer.data(), buffer.size(), true);
//  obj.save(GET_TEST_DATA_DIR + "/_test.sobj");

  reader->readMultiscaleBody(config.bodyId, 2, true, &obj);
  std::cout << "#Voxels: " << obj.getVoxelNumber() << std::endl;

  obj.save(GET_TEST_DATA_DIR + "/_test2.sobj");
#endif

#if 0
  ZDvidTarget target = dvid::MakeTargetFromUrlSpec(
        "http://127.0.0.1:1600?uuid=4280&segmentation=segmentation&admintoken=test");
  ZDvidWriter writer;
  writer.open(target);
  writer.setAdmin(true);
  writer.deleteSkeleton(1505851998);
#endif

#if 0
  ZSwcTree tree;
  tree.load(GET_TEST_DATA_DIR + "/_test.swc");

  Swc_Tree_Node_Label_Workspace workspace;
  Default_Swc_Tree_Node_Label_Workspace(&workspace);

  ZStack *stack = ZStackFactory::MakeZeroStack(GREY16, 512, 512, 60, 1);

  Swc_Tree_Label_Stack(tree.data(), stack->c_stack(), &workspace);

  stack->save(GET_TEST_DATA_DIR + "/_test.tif");
#endif

#if 0
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  double r = 2.0;
  Set_Local_Neuroseg(
        locseg,  r + r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);
  Geo3d_Scalar_Field *field = Local_Neuroseg_Field_Sp(locseg, NULL, NULL);
  Print_Geo3d_Scalar_Field(field);
#endif

#if 0
  ZStack *stack = ZStackFactory::MakeZeroStack(512, 512, 512);
  double x = 10.0;
  double y = 20.0;
  double z = 30.0;
  Local_Neuroseg *locseg = New_Local_Neuroseg();
  double r = 5.0;
//  Set_Local_Neuroseg(
//        locseg,  r, 0.1, NEUROSEG_DEFAULT_H, 1.0, 1.0, 0.0, 2.0, 3.0,
//        x, y, z);

  Set_Local_Neuroseg(
        locseg,  r, 0.0, NEUROSEG_DEFAULT_H, 0.0, 0.0, 0.0, 0.0, 1.0,
        x, y, z);

  ZLocalNeuroseg s(locseg);

  tic();
  for (size_t i = 0; i < 10000; ++i) {
    ZPointArray points = s.sample(1.0, 1.0);
    for (const ZPoint &pt : points) {
      stack->getIntValue(neutu::iround(pt.x()), neutu::iround(pt.y()),
                         neutu::iround(pt.z()));
    }
  }
  ptoc();
//  points.print();

//  ZSwcTree tree;
//  tree.forceVirtualRoot();
//  for (ZPoint pt : points) {
//    SwcTreeNode::MakePointer(pt.x(), pt.y(), pt.z(), 0.1, tree.root());
//  }

//  tree.save(GET_TEST_DATA_DIR + "/test.swc");
#endif

#if 0
  ZDvidTarget target = ZDvidTargetFactory::MakeFromSpec(
        "http://127.0.0.1:1600?uuid=c315&segmentation=segmentation&"
        "grayscale=grayscale");

  ZFlyEmArbMvc *mvc = ZFlyEmArbMvc::Make(ZDvidEnv(target));
  mvc->getCompleteDocument()->downloadBookmark();
//  mvc->getCompleteDocument()->downloadBookmark();

  /*
  ZFlyEmBookmark *bookmark = new ZFlyEmBookmark;
  bookmark->setCenter(1232, 1117, 1023);
  bookmark->setZScale(4);
  mvc->getCompleteDocument()->addObject(bookmark);
  */

  ZArbSliceViewParam viewParam;
  viewParam.setSize(1024, 1024);
  viewParam.setCenter(1415, 1105, 1027);
  ZPoint v1 = ZPoint(1, 1, 0);
  ZPoint v2 = ZPoint(-1, 1, 0);

  viewParam.setPlane(v1, v2);
  mvc->show();
  mvc->updateViewParam(viewParam);
#endif

#if 0
  ZAffineRect rect;
  rect.set(ZPoint(1, 2, 3), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 10, 20);
  std::cout << rect << std::endl;
#endif

#if 0
  ZAffineRect rect;
  rect.set(ZPoint(1, 2, 3), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 10, 20);

  ZCuboid box(1, 2, 3, 11, 22, 33);
  std::cout << zgeom::Intersects(rect, box) << std::endl;

  std::cout << neutu::ToString(rect) << std::endl;

  rect.set(ZPoint(1, 2, 3), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 10, 20);
#endif

#if 0
  ZDvidNode node;
  node.setHost("https://neuroglancer.janelia.org");
  std::cout << node.getSourceString() << std::endl;
#endif

#if 0
  ZDvidTarget target;
  target.set("https://hemibrain-dvid.janelia.org", "a89e", -1);
  target.setGrayScaleName("grayscalejpeg");
  ZDvidReader reader;
  reader.open(target);
  reader.readGrayScaleLowtis(2610, 2354, 3197, 512, 512, 1);

//  stack->save(GET_TEST_DATA_DIR + "/test.tif");
#endif

#if 0
  ZDvidTarget target;
  target.set("https://hemibrain-dvid2.janelia.org", "abdd", -1);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);

  reader.readLabels64Lowtis(
          17152, 22592, 19328, 1024, 1024, 1);

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("local_test");
  reader->updateMaxLabelZoom();

  ZDvidInfo dvidInfo =
      reader->readDataInfo(reader->getDvidTarget().getSegmentationName());

  std::set<uint64_t> bodySet = reader->readBodyId(
        dvidInfo.getDataRange(), 2);
  std::cout << bodySet.size() << std::endl;
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("vnc");
  int z = 40896;
  for (int dz = 0; dz < 1000; ++dz) {
    std::cout << z + dz << std::endl;
    ZStack *stack =
        reader->readGrayScaleLowtis(0, 0, z + dz, 42944, 54837, 5, 256, 256, false);
    delete stack;
  }
#endif

#if 0
//  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi_cloud");
//  reader->getDvidTarget().setUuid("abdd");

  ZDvidTarget target;
  target.set("https://hemibrain-dvid2.janelia.org", "abdd", -1);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);

  auto response = reader.readKeyValues("neutu_merge_opr", "progress_a", "progress_z");
  std::cout << response.size() << std::endl;

  for (const auto &entry : response) {
    ZJsonObject obj;
    obj.decode(entry.constData());
    obj.print();
  }

#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi");
  ZObject3dScan obj = reader->readRoi("AL-DC3(R)");
  ZStack *stack = obj.toStack(ZIntCuboid(0, 0, 0, 1087, 1279, 1343), 255);
  stack->save(GET_TEST_DATA_DIR + "/AL-DC3(R).tif");
#endif

#if 0
  ZStackObject *obj = new ZFlyEmBookmark;
  ZStackObject *obj2 = obj->clone();

  ZFlyEmBookmark bookmark;
  ZFlyEmBookmark* obj3 = bookmark.clone();
#endif

#if 0
  ZMesh mesh;
  ZMeshIO meshIO;

  meshIO.load(
        (GET_TEST_DATA_DIR + "/_benchmark/mesh/1282274661.ngmesh").c_str(), mesh);

  mesh.save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("hemi_cloud");
  ZMesh *mesh = reader->readMesh("segmentation_meshes", "5813023304.ngmesh");
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("local_test");
  ZMesh *mesh = reader->readMesh("segmentation_meshes", "5901310807.merge");
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 0
  ZDvidTarget target;
  target.set("https://hemibrain-dvid2.janelia.org", "793a", -1);
  target.setSegmentationName("segmentation");
  ZDvidReader reader;
  reader.open(target);

  ZMesh *mesh = reader.readMesh("segmentation_meshes", "1003672833.merge");
  mesh->save(GET_TEST_DATA_DIR + "/_test.obj");
#endif

#if 1
  ZDvidReader *reader = ZGlobal::GetInstance().getDvidReader("local_test");
  std::vector<uint64_t> bodyArray =
      reader->readConsistentMergedMeshKeys("5901278576.merge");

  std::cout << neutu::ToString(bodyArray, ", ") << std::endl;

#endif

  std::cout << "Done." << std::endl;
}

