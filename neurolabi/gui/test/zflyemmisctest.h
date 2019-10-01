#ifndef ZFLYEMMISCTEST_H
#define ZFLYEMMISCTEST_H

#include "ztestheader.h"
#include "flyem/zflyemmisc.h"
#include "zpunctum.h"

#ifdef _USE_GTEST_

TEST(flyemmisc, HasConnecion)
{
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 789, neutu::EBiDirection::FORWARD));
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 345, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 689, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "12->[345][789]", 123, 345, neutu::EBiDirection::FORWARD));

  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 123, 789, neutu::EBiDirection::FORWARD));


  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", 789, 123, neutu::EBiDirection::BACKWARD));
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", 345, 123, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 123, 789, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 345, 345, neutu::EBiDirection::FORWARD));
}

#endif

#endif // ZFLYEMMISCTEST_H
