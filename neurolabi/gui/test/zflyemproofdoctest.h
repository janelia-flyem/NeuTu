#ifndef ZFLYEMPROOFDOCTEST_H
#define ZFLYEMPROOFDOCTEST_H

#include "ztestheader.h"
#include "flyem/zflyemproofdoc.h"
#include "neutubeconfig.h"
#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidsparsevolslice.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmProofDoc, DVID)
{
  ZFlyEmProofDoc doc;
  ZDvidReader reader;
  ZDvidTarget target("emdata2.int.janelia.org", "b6bc", 8500);
  if (reader.open(target)) {
    std::cout << "Connected to " << reader.getDvidTarget().getAddressWithPort()
              << std::endl;
    doc.setDvidTarget(target);
    ASSERT_TRUE(doc.getDvidReader().good());
    ASSERT_TRUE(doc.getDvidWriter().good());

    ASSERT_EQ(NULL, doc.getDvidLabelSlice(neutube::EAxis::X));
    ASSERT_EQ(NULL, doc.getDvidLabelSlice(neutube::EAxis::Y));
    ASSERT_TRUE(doc.getDvidLabelSlice(neutube::EAxis::Z) != NULL);

    ZDvidLabelSlice *slice = doc.getDvidLabelSlice(neutube::EAxis::Z);
    ZStackViewParam param;
    param.setViewPort(3565, 5292, 4260, 5853);
    param.setZ(7313);
    slice->update(param);

    ASSERT_TRUE(slice->hit(3812, 5465, 7313));

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

    doc.removeDvidSparsevol(neutube::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(0, sparsevolList2.size());

    doc.updateDvidLabelObject(neutube::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(1, sparsevolList2.size());
    sparsevol = sparsevolList2[0];
    ASSERT_EQ(sparsevol->getLabel(), bodyId);

    doc.updateDvidLabelObject(neutube::EAxis::Z);
    sparsevolList2 = doc.getDvidSparsevolSliceList();
    ASSERT_EQ(1, sparsevolList2.size());
    ASSERT_EQ(sparsevol->getLabel(), bodyId);
  }
}

TEST(ZFlyEmProofDoc, ColorMap)
{
  ZFlyEmProofDoc doc;
  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_NORMAL));
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_NORMAL));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_NAME));
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_NAME));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER));
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_SEQUENCER));

  doc.activateBodyColorMap(ZFlyEmBodyColorOption::GetColorMapName(
                             ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL));
  ASSERT_TRUE(doc.isActive(ZFlyEmBodyColorOption::BODY_COLOR_PROTOCOL));
}

#endif

#endif // ZFLYEMPROOFDOCTEST_H
