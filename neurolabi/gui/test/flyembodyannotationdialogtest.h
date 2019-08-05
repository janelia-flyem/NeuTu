#ifndef FLYEMBODYANNOTATIONDIALOGTEST_H
#define FLYEMBODYANNOTATIONDIALOGTEST_H

#include <cstdlib>

#include "ztestheader.h"
#include "flyem/dialogs/flyembodyannotationdialog.h"
#include "flyem/zflyembodyannotation.h"

#ifdef _USE_GTEST_

TEST(FlyEmBodyAnnotationDialog, Basic)
{
  FlyEmBodyAnnotationDialog dlg(true, nullptr);
//  dlg.updateStatusBox();

  ZFlyEmBodyAnnotation annotation;
  annotation.loadJsonString(
        "{"
           "\"body ID\": 5901214961,"
           "\"instance\": \"test3\","
           "\"naming user\": \"zhaot\","
           "\"status\": \"Traced\","
           "\"user\": \"zhaot\""
        "}");
  dlg.loadBodyAnnotation(annotation);
  ASSERT_EQ(annotation, dlg.getBodyAnnotation());
  ASSERT_TRUE(annotation.hasSameUserStatus(dlg.getBodyAnnotation()));

  annotation.loadJsonString(
        "{"
           "\"body ID\": 5901214961,"
           "\"instance\": \"test3\","
           "\"status\": \"Traced\","
           "\"user\": \"" + std::string(std::getenv("USER")) +
        "\"}");
  dlg.loadBodyAnnotation(annotation);
  ASSERT_EQ(annotation, dlg.getBodyAnnotation());
  ASSERT_TRUE(annotation.hasSameUserStatus(dlg.getBodyAnnotation()));

  annotation.loadJsonString(
        "{"
           "\"body ID\": 5901214961,"
           "\"instance\": \"test3\","
           "\"status\": \"Traced\","
           "\"user\": \"test\""
        "}");
  dlg.loadBodyAnnotation(annotation);
  ASSERT_EQ(annotation, dlg.getBodyAnnotation());
  ASSERT_FALSE(annotation.hasSameUserStatus(dlg.getBodyAnnotation()));

  dlg.updateStatusBox();
  annotation.loadJsonString(
        "{"
           "\"body ID\": 5901214961,"
           "\"instance\": \"test3\","
           "\"naming user\": \"zhaot\","
           "\"status\": \"Traced\","
           "\"user\": \"zhaot\""
        "}");
  dlg.loadBodyAnnotation(annotation);
  ASSERT_EQ(annotation, dlg.getBodyAnnotation());
  ASSERT_TRUE(annotation.hasSameUserStatus(dlg.getBodyAnnotation()));
}

#endif

#endif // FLYEMBODYANNOTATIONDIALOGTEST_H
