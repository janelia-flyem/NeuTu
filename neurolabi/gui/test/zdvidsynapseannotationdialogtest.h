#ifndef ZDVIDSYNAPSEANNOTATIONDIALOGTEST_H
#define ZDVIDSYNAPSEANNOTATIONDIALOGTEST_H

#include "ztestheader.h"
#include "dialogs/zflyemsynapseannotationdialog.h"
#include "dvid/zdvidsynapse.h"
#include "zjsonobjectparser.h"

#ifdef _USE_GTEST_

TEST(ZFlyEmSynapseAnnotationDialog, Basic)
{
  ZFlyEmSynapseAnnotationDialog dlg;
  ASSERT_FALSE(dlg.hasConfidence());

  dlg.setConfidence("0.1");
  ASSERT_TRUE(dlg.hasConfidence());

  ZJsonObjectParser parser;

  ZJsonObject obj = dlg.getPropJson();
  ASSERT_EQ("0.1", parser.getValue(obj, "conf", ""));

  dlg.setConfidence("0.2");
  obj = dlg.getPropJson();
  ASSERT_EQ("0.2", parser.getValue(obj, "conf", ""));

  ZDvidSynapse synapse;
  synapse.setConfidence("0.6");
  dlg.set(synapse);
  obj = dlg.getPropJson();
  ASSERT_EQ("0.6", parser.getValue(obj, "conf", ""));

  synapse.setConfidence("0.1");
  dlg.set(synapse);
  obj = dlg.getPropJson();
  ASSERT_EQ("0.1", parser.getValue(obj, "conf", ""));

  dlg.setConfidence("");
  obj = dlg.getPropJson();
  ASSERT_EQ("", parser.getValue(obj, "conf", ""));
  ASSERT_FALSE(dlg.hasConfidence());

}

#endif


#endif // ZDVIDSYNAPSEANNOTATIONDIALOGTEST_H
