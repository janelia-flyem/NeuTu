#ifndef ZFLYEMPROOFDOCTEST_H
#define ZFLYEMPROOFDOCTEST_H

#include "ztestheader.h"

#include "neutubeconfig.h"
#include "zjsonobject.h"

#include "flyem/zflyemproofdoc.h"
#include "flyem/zflyemorthodoc.h"
#include "flyem/zflyemarbdoc.h"

#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidsparsevolslice.h"
#include "dvid/zdvidsparsestack.h"

#ifdef _USE_GTEST_

#if 1
TEST(ZFlyEmProofDoc, DVID)
{
  ZFlyEmProofDoc doc;
  ZDvidReader reader;
  ZDvidTarget target("127.0.0.1", "4280", 1600);
  target.setGrayScaleName("grayscale");
  target.setSegmentationName("segmentation");
  target.enableSupervisor(false);
  if (reader.open(target)) {
    std::cout << "Connected to " << reader.getDvidTarget().getAddressWithPort()
              << std::endl;
    ZDvidEnv env;
    env.set(target);
    doc.setDvid(env);
    ASSERT_TRUE(doc.getDvidReader().good());
    ASSERT_TRUE(doc.getDvidWriter().good());

    ASSERT_TRUE(doc.getDvidLabelSlice(neutu::EAxis::X, false));
    ASSERT_TRUE(doc.getDvidLabelSlice(neutu::EAxis::Y, false));
    ASSERT_TRUE(doc.getDvidLabelSlice(neutu::EAxis::Z, false) != NULL);

    ZDvidLabelSlice *slice = doc.getDvidLabelSlice(neutu::EAxis::Z, false);
    ZStackViewParam param;
    param.setCutCenter(ZIntPoint(1000, 700, 1023));
    param.setSize(1000, 600, neutu::data3d::ESpace::MODEL);
//    param.setWidgetRect(QRect(0, 0, 800, 600));
//    param.setCanvasRect(QRect(479, 448, 1510, 1023));
//    param.setViewPort(479, 448, 1510, 1023);
//    param.setZ(1023);
    slice->update(param);

    ASSERT_TRUE(slice->hit(1017, 797, 1023));

    slice->selectHit();
    ASSERT_EQ(1, (int) slice->getSelectedOriginal().size());
    uint64_t bodyId = *(slice->getSelectedOriginal().begin());
    std::cout << "Selected body ID: " << *(slice->getSelectedOriginal().begin())
              << std::endl;

    std::vector<ZDvidSparsevolSlice*> sparsevolList =
        doc.makeSelectedDvidSparsevol(slice);
    ASSERT_EQ(1, (int) sparsevolList.size());
    ZDvidSparsevolSlice *sparsevol = sparsevolList.front();
    ASSERT_EQ(sparsevol->getLabel(), bodyId);

    doc.addObject(sparsevol);
    QList<ZDvidSparsevolSlice*> sparsevolList2 =
        doc.getDvidSparsevolSliceList();
    ASSERT_EQ(1, sparsevolList2.size());

    doc.removeDvidSparsevol(neutu::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(0, sparsevolList2.size());

    doc.updateDvidLabelObject(neutu::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(1, sparsevolList2.size());
    sparsevol = sparsevolList2[0];
    ASSERT_EQ(sparsevol->getLabel(), bodyId);

    doc.updateDvidLabelObject(neutu::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(1, sparsevolList2.size());
//    ASSERT_EQ(sparsevol->getLabel(), bodyId);
  }
}

TEST(ZFlyEmProofDoc, grayscale)
{
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_test/dvid_setting2.json");
  ZDvidEnv env;
  env.loadJsonObject(obj);

  ZFlyEmProofDoc doc;
  doc.setDvid(env);

  ASSERT_TRUE(doc.getDvidTarget().hasGrayScaleData());
  ASSERT_EQ(2, int(doc.getDvidEnv().getTargetList(
                     ZDvidEnv::ERole::GRAYSCALE).size()));

  ASSERT_EQ("http:127.0.0.1:1600:4280", doc.getDvidTarget().getSourceString());

  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getCurrentGrayscaleReader(neutu::EAxis::Z)->getDvidTarget().
            getGrayscaleSourceString());

  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getCurrentGrayscaleReader()->getDvidTarget().
            getGrayscaleSourceString());

  ZDvidInfo info = doc.getMainGrayscaleInfo();
  ASSERT_EQ("nm", info.getVoxelResolution().getUnitName());
  ASSERT_EQ(8, info.getVoxelResolution().getVoxelSize(
              neutu::EAxis::X, ZResolution::EUnit::UNIT_NANOMETER));
  ASSERT_EQ(32, info.getBlockSize().getX());

  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getDvidTarget().getGrayscaleSourceString());
  ASSERT_EQ("grayscale",
            doc.getDvidTarget().getGrayScaleName());

//  target.print();

  doc.toggleGrayscale(neutu::EAxis::Z);

  ASSERT_EQ("http:127.0.0.1:1600:970f::grayscale",
            doc.getDvidTarget().getGrayscaleSourceString());
  ASSERT_EQ("grayscale",
            doc.getDvidTarget().getGrayScaleName());

  ASSERT_EQ("http:127.0.0.1:1600:970f::grayscale",
            doc.getCurrentGrayscaleReader(neutu::EAxis::Z)->getDvidTarget().
            getGrayscaleSourceString());

  ASSERT_EQ("http:127.0.0.1:1600:970f::grayscale",
            doc.getCurrentGrayscaleReader()->getDvidTarget().
            getGrayscaleSourceString());

  doc.toggleGrayscale(neutu::EAxis::Z);
  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getCurrentGrayscaleReader(neutu::EAxis::Z)->getDvidTarget().
            getGrayscaleSourceString());

  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getCurrentGrayscaleReader()->getDvidTarget().
            getGrayscaleSourceString());

}

TEST(ZFlyEmProofDoc, SparseStack)
{
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_test/dvid_setting3.json");
  ZDvidEnv env;
  env.loadJsonObject(obj);

  ZFlyEmProofDoc doc;
  doc.setDvid(env);


  {
    ZDvidReader reader;
    reader.open(doc.getDvidTarget());

    ZDvidSparseStack *body = reader.readDvidSparseStack(
          944729222, neutu::EBodyLabelType::BODY);
    ASSERT_EQ("http:127.0.0.1:1600:c315::grayscale",
              body->getDvidTarget().getGrayscaleSourceString());
  }

   doc.toggleGrayscale(neutu::EAxis::Z);
   {
     ZDvidReader reader;
     reader.open(doc.getDvidTarget());

     ZDvidSparseStack *body = reader.readDvidSparseStack(
           944729222, neutu::EBodyLabelType::BODY);
     ASSERT_EQ("http:127.0.0.1:1600:970f::grayscale",
               body->getDvidTarget().getGrayscaleSourceString());
   }

   doc.toggleGrayscale(neutu::EAxis::Z);
   {
     ZDvidReader reader;
     reader.open(doc.getDvidTarget());
     ZDvidSparseStack *body = reader.readDvidSparseStack(
           944729222, neutu::EBodyLabelType::BODY);
     ASSERT_EQ("http:127.0.0.1:1600:c315::grayscale",
               body->getDvidTarget().getGrayscaleSourceString());
   }
}

TEST(ZFlyEmOrthoDoc, Basic)
{
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_test/dvid_setting2.json");
  ZDvidEnv env;
  env.loadJsonObject(obj);

  ZFlyEmOrthoDoc doc;
  doc.setDvid(env);

  ASSERT_TRUE(doc.getDvidTarget().hasGrayScaleData());
  ASSERT_EQ(2, int(doc.getDvidEnv().getTargetList(
                     ZDvidEnv::ERole::GRAYSCALE).size()));

  ASSERT_EQ("http:127.0.0.1:1600:4280", doc.getDvidTarget().getSourceString());
//  ASSERT_EQ(nullptr, doc.getCurrentGrayscaleReader(neutu::EAxis::Z));
}

TEST(ZFlyEmArbDoc, Grayscale)
{
  ZJsonObject obj;
  obj.load(GET_TEST_DATA_DIR + "/_test/dvid_setting4.json");
  ZDvidEnv env;
  env.loadJsonObject(obj);

  ZFlyEmArbDoc doc;
  doc.setDvid(env);

  ASSERT_TRUE(doc.getDvidTarget().hasGrayScaleData());
  ASSERT_EQ(2, int(doc.getDvidEnv().getTargetList(
                     ZDvidEnv::ERole::GRAYSCALE).size()));

  ASSERT_EQ("http:127.0.0.1:1600:4280:segmentation", doc.getDvidTarget().getSourceString());
//  ASSERT_EQ(nullptr, doc.getCurrentGrayscaleReader(neutu::EAxis::Z));
  ASSERT_EQ("http:127.0.0.1:1600:4280::grayscale",
            doc.getCurrentGrayscaleReader(neutu::EAxis::ARB)->getDvidTarget().
            getGrayscaleSourceString());
}

#endif

TEST(ZFlyEmProofDoc, ColorMap)
{
  ZFlyEmProofDoc doc;
  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_NORMAL), false);
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_NORMAL));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_NAME), false);
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_NAME));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER), false);
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL), false);
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL));
}

#endif

#endif // ZFLYEMPROOFDOCTEST_H
