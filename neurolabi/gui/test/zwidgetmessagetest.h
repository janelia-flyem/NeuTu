#ifndef ZWIDGETMESSAGETEST_H
#define ZWIDGETMESSAGETEST_H

#include "ztestheader.h"
#include "zwidgetmessage.h"

#ifdef _USE_GTEST_

TEST(ZWidgetMessage, target)
{
  {
    ZWidgetMessage msg(ZWidgetMessage::TARGET_TEXT);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT));
    ASSERT_FALSE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));

    msg.addTarget(ZWidgetMessage::TARGET_TEXT_APPENDING);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT));
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));

    msg.removeTarget(ZWidgetMessage::TARGET_TEXT_APPENDING);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT));
    ASSERT_FALSE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));

    msg.addTarget(ZWidgetMessage::TARGET_CUSTOM_AREA);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_CUSTOM_AREA));

    msg.removeTarget(ZWidgetMessage::TARGET_CUSTOM_AREA);
    ASSERT_FALSE(msg.hasTarget(ZWidgetMessage::TARGET_CUSTOM_AREA));
  }

  {
    ZWidgetMessage msg(ZWidgetMessage::TARGET_TEXT_APPENDING);
    ASSERT_TRUE(msg.hasTargetIn(
                  ZWidgetMessage::TARGET_TEXT | ZWidgetMessage::TARGET_TEXT_APPENDING));
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));
  }

  {
    ZWidgetMessage msg(ZWidgetMessage::TARGET_TEXT_APPENDING |
                       ZWidgetMessage::TARGET_TEXT);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT));
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));
    ASSERT_FALSE(msg.hasTargetOtherThan(
                   ZWidgetMessage::TARGET_TEXT_APPENDING|ZWidgetMessage::TARGET_TEXT));
    ASSERT_TRUE(msg.hasTargetOtherThan(ZWidgetMessage::TARGET_TEXT));
  }

  {
    ZWidgetMessage msg(ZWidgetMessage::TARGET_TEXT |
                       ZWidgetMessage::TARGET_DIALOG);
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT));
    ASSERT_FALSE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));
    ASSERT_TRUE(msg.hasTargetOtherThan(ZWidgetMessage::TARGET_TEXT_APPENDING));
    ASSERT_FALSE(msg.hasTargetOtherThan(ZWidgetMessage::TARGET_TEXT |
                                        ZWidgetMessage::TARGET_DIALOG));
  }

  {
    ZWidgetMessage msg("", neutu::EMessageType::DEBUG);
    ASSERT_EQ(neutu::EMessageType::DEBUG, msg.getType());
  }

  {
    ZWidgetMessage msg("test", "", neutu::EMessageType::DEBUG);
    ASSERT_EQ(neutu::EMessageType::DEBUG, msg.getType());
    ASSERT_EQ("test", msg.getTitle());
  }

  {
    ZWidgetMessage msg("test", "", neutu::EMessageType::DEBUG,
                       ZWidgetMessage::TARGET_DIALOG);
    ASSERT_EQ(neutu::EMessageType::DEBUG, msg.getType());
    ASSERT_EQ("test", msg.getTitle());
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::TARGET_DIALOG));
    ASSERT_FALSE(msg.hasTarget(ZWidgetMessage::TARGET_TEXT_APPENDING));
  }
}

TEST(ZWidgetMessageFactory, Basic)
{
  {
    ZWidgetMessage msg = ZWidgetMessageFactory("test").
        title("test_title").
        to(ZWidgetMessage::ETarget::TARGET_CUSTOM_AREA);
    ASSERT_EQ("test_title", msg.getTitle());
    ASSERT_EQ("test", msg.toPlainString()) << msg.toPlainString().toStdString();
    ASSERT_TRUE(msg.hasTarget(ZWidgetMessage::ETarget::TARGET_CUSTOM_AREA));

    ZWidgetMessage msg2 =
        ZWidgetMessageFactory(msg).without(ZWidgetMessage::ETarget::TARGET_CUSTOM_AREA);
    ASSERT_FALSE(msg2.hasTarget(ZWidgetMessage::ETarget::TARGET_CUSTOM_AREA));
  }
}

#endif

#endif // ZWIDGETMESSAGETEST_H
