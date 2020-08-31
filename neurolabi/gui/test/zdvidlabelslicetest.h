#ifndef ZDVIDLABELSLICETEST_H
#define ZDVIDLABELSLICETEST_H

#include "ztestheader.h"

#ifdef _USE_GTEST_

#include "common/utilities.h"
#include "dvid/zdvidlabelslice.h"
#include "flyem/zflyembodymerger.h"
#include "flyem/zflyembodyidcolorscheme.h"

TEST(ZDvidLabelSlice, Selection)
{
  ZDvidLabelSlice slice;

  slice.addSelection(1, neutu::ELabelSource::ORIGINAL);
  ASSERT_TRUE(slice.isBodySelected(1, neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.isBodySelected(1, neutu::ELabelSource::MAPPED));

  ZFlyEmBodyMerger *merger = new ZFlyEmBodyMerger;

  slice.setBodyMerger(merger);
  merger->pushMap(1, 2);
  ASSERT_TRUE(slice.isBodySelected(1, neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.isBodySelected(2, neutu::ELabelSource::MAPPED));
  ASSERT_FALSE(slice.isBodySelected(1, neutu::ELabelSource::MAPPED));

  slice.deselectAll();
  ASSERT_FALSE(slice.isBodySelected(1, neutu::ELabelSource::ORIGINAL));
  ASSERT_FALSE(slice.isBodySelected(2, neutu::ELabelSource::MAPPED));

  std::vector<uint64_t> bodies({1, 2, 3, 4});
  slice.addSelection(bodies.begin(), bodies.end(), neutu::ELabelSource::ORIGINAL);

  for (uint64_t bodyId : bodies) {
    ASSERT_TRUE(slice.isBodySelected(bodyId, neutu::ELabelSource::ORIGINAL));
    ASSERT_TRUE(slice.isBodySelected(merger->getFinalLabel(bodyId),
                                     neutu::ELabelSource::MAPPED));
  }

  merger->pushMap(2, 3);
  ASSERT_EQ(4, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_EQ(2, slice.getSelected(neutu::ELabelSource::MAPPED).size());

  merger->pushMap(100, 200);
  merger->pushMap(150, 200);
  slice.addSelection(
        std::vector<uint64_t>({200, 300}), neutu::ELabelSource::MAPPED);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_EQ(4, slice.getSelected(neutu::ELabelSource::MAPPED).size());

  slice.addSelection(100, neutu::ELabelSource::MAPPED);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_EQ(4, slice.getSelected(neutu::ELabelSource::MAPPED).size());

  slice.addSelection(std::vector<uint64_t>({100}), neutu::ELabelSource::MAPPED);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_EQ(4, slice.getSelected(neutu::ELabelSource::MAPPED).size());

  slice.xorSelection(1, neutu::ELabelSource::ORIGINAL);
  ASSERT_EQ(7, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_FALSE(slice.isBodySelected(1, neutu::ELabelSource::ORIGINAL));

  slice.xorSelection(1, neutu::ELabelSource::ORIGINAL);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_TRUE(slice.isBodySelected(1, neutu::ELabelSource::ORIGINAL));

  slice.xorSelection(200, neutu::ELabelSource::MAPPED);
  ASSERT_EQ(5, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_FALSE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));

  slice.xorSelection(200, neutu::ELabelSource::MAPPED);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_TRUE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));

  slice.xorSelection(std::vector<uint64_t>({200}), neutu::ELabelSource::MAPPED);
  ASSERT_EQ(5, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_FALSE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));

  slice.xorSelectionGroup(
        std::vector<uint64_t>({200, 200}), neutu::ELabelSource::MAPPED);
  ASSERT_EQ(8, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_TRUE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));

  std::cout << neutu::ToString(slice.getSelected(neutu::ELabelSource::ORIGINAL),
                               ", ") << std::endl;

  slice.xorSelection(
        std::vector<uint64_t>({1, 100, 500}), neutu::ELabelSource::ORIGINAL);
  ASSERT_EQ(3, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_FALSE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.isBodySelected(500, neutu::ELabelSource::ORIGINAL));

//  std::cout << neutu::ToString(
//                 slice.getSelected(neutu::ELabelSource::ORIGINAL), ", ")
//            << std::endl;

  slice.xorSelectionGroup(
        std::vector<uint64_t>({1, 100, 500}), neutu::ELabelSource::ORIGINAL);

//  std::cout << neutu::ToString(
//                 slice.getSelected(neutu::ELabelSource::ORIGINAL), ", ")
//            << std::endl;

  ASSERT_EQ(5, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_TRUE(slice.isBodySelected(100, neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.isBodySelected(500, neutu::ELabelSource::ORIGINAL));

  slice.setSelection(
        std::vector<uint64_t>({200, 300}), neutu::ELabelSource::MAPPED);
  ASSERT_EQ(4, slice.getSelected(neutu::ELabelSource::ORIGINAL).size());
  ASSERT_EQ(2, slice.getSelected(neutu::ELabelSource::MAPPED).size());

//  merger->print();
  slice.startSelection();
  slice.setSelection(std::vector<uint64_t>({100, 300}),
                     neutu::ELabelSource::MAPPED);
  slice.endSelection();

  ASSERT_TRUE(slice.getSelector().getSelectedSet().empty());
  ASSERT_EQ(3, slice.getSelector().getDeselectedSet().size());

  /*
  std::cout << neutu::ToString(
                 slice.getSelector().getSelectedSet(), ", ")
            << std::endl;

  std::cout << neutu::ToString(
                 slice.getSelector().getDeselectedSet(), ", ")
            << std::endl;
*/

  delete merger;
}

TEST(ZDvidLabelSlice, ColorScheme)
{
  ZDvidLabelSlice slice;

  ASSERT_NE(QColor(0, 0, 0, 0),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 0, 0),
            slice.getLabelColor(uint64_t(0), neutu::ELabelSource::ORIGINAL));

  ASSERT_TRUE(slice.setLabelColor(100, QColor(0, 0, 100, 0), 0));
  ASSERT_TRUE(slice.setLabelColor(200, "#FFFFFFFF", 0));
  ASSERT_TRUE(slice.setLabelColor(1, QColor(0, 0, 100, 0), 0));
  ASSERT_EQ(QColor(0, 0, 100, 0),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 100, 0),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.setLabelColor(1, "", 0));
  ASSERT_NE(QColor(0, 0, 1, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL).name().toStdString());

  slice.resetLabelColor(0);

  auto defaultScheme = new ZFlyEmBodyIdColorScheme;
  defaultScheme->setColor(1, 1);
  defaultScheme->setColor(2, 2);
  defaultScheme->setColor(3, 3);

  slice.setCustomColorMap(
        std::shared_ptr<ZFlyEmBodyColorScheme>(defaultScheme));
  ASSERT_EQ(QColor(0, 0, 1, 0),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 2, 0),
            slice.getLabelColor(uint64_t(2), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 3, 0),
            slice.getLabelColor(uint64_t(3), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 0, 0),
            slice.getLabelColor(uint64_t(4), neutu::ELabelSource::ORIGINAL));


  ASSERT_TRUE(slice.setLabelColor(100, QColor(0, 0, 100, 0), 0));
  ASSERT_TRUE(slice.setLabelColor(200, "#FFFFFFFF", 0));
  ASSERT_TRUE(slice.setLabelColor(1, QColor(0, 0, 100, 0), 0));
  ASSERT_EQ(QColor(0, 0, 100, 0),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL));
  ASSERT_EQ(QColor(0, 0, 100, 0),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL));
  ASSERT_TRUE(slice.setLabelColor(1, "", 0));
  ASSERT_EQ(QColor(0, 0, 1, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL).name().toStdString());

//  slice.addSelection(1, neutu::ELabelSource::ORIGINAL);
  ZFlyEmBodyMerger *merger = new ZFlyEmBodyMerger;
  slice.setBodyMerger(merger);
  merger->pushMap(1, 2);
  ASSERT_EQ(QColor(0, 0, 2, 0),
            slice.getLabelColor(uint64_t(1), neutu::ELabelSource::ORIGINAL));

  ASSERT_EQ(QColor(0, 0, 100, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL).name().toStdString());
  slice.setLabelColor(100, "", 0);
  ASSERT_NE(QColor(0, 0, 100, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL).name().toStdString());

  slice.setLabelColor(100, QColor(100, 0, 0), 1);
  ASSERT_NE(QColor(100, 0, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL).name().toStdString());
  slice.removeCustomColorMap();
  ASSERT_EQ(QColor(100, 0, 0).name().toStdString(),
            slice.getLabelColor(uint64_t(100), neutu::ELabelSource::ORIGINAL).name().toStdString());

}

#endif

#endif // ZDVIDLABELSLICETEST_H
