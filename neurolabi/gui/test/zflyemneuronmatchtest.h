#ifndef ZFLYEMNEURONMATCHTEST_H
#define ZFLYEMNEURONMATCHTEST_H

#include "ztestheader.h"
#include "neutubeconfig.h"
#include "flyem/zflyemneuronlayermatcher.h"
#include "flyem/zflyemdatabundle.h"
#include "flyem/zswctreebatchmatcher.h"

#ifdef _USE_GTEST_
TEST(ZFlyEmNeuronMatch, Layer) {
#if 0
  ZFlyEmDataBundle dataBundle;
  dataBundle.loadJsonFile(GET_TEST_DATA_DIR +
                          "/flyem/TEM/data_release/bundle1/data_bundle.json");
  ZFlyEmNeuron *neuron1 = dataBundle.getNeuron(209);
  ZFlyEmNeuron *neuron2 = dataBundle.getNeuron(214);

  ZFlyEmNeuronLayerMatcher matcher;
  matcher.setLayerScale(100.0);
  tic();
  matcher.match(neuron1, neuron2);
  ptoc();
  //matcher.print();

  neuron2 = dataBundle.getNeuron(285743);
  tic();
  matcher.match(neuron1, neuron2);
  ptoc();
  //matcher.print();

  neuron2 = dataBundle.getNeuron(205);
  tic();
  matcher.match(neuron1, neuron2);
  ptoc();
  //matcher.print();

  for (size_t i = 0; i < dataBundle.getNeuronArray().size(); ++i) {
    neuron2 = &(dataBundle.getNeuronArray()[i]);
    neuron2->getBody();
  }

  tic();
  for (size_t i = 0; i < dataBundle.getNeuronArray().size(); ++i) {
    neuron2 = &(dataBundle.getNeuronArray()[i]);
    //neuron2->print();
    matcher.match(neuron1, neuron2);
    //matcher.print();
  }
  ptoc();
#endif
}

#endif

#endif // ZFLYEMNEURONMATCHTEST_H
