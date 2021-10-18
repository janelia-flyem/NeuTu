#ifndef ZFLYEMMISCTEST_H
#define ZFLYEMMISCTEST_H


#ifdef _USE_GTEST_

#include "gtest/gtest.h"
#include "flyem/zflyemmisc.h"

TEST(flyemmisc, HasConnecion)
{
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", "->", {123, 5}, {234, 345}));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123->[345][789]", "->", {12, 5}, {234, 35}));
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", "<-", {123}, {345, 789}));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", "<-", {12}, {345, 789}));

  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 789, neutu::EBiDirection::FORWARD));
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 345, neutu::EBiDirection::FORWARD));

  ASSERT_TRUE(
        flyem::HasConnecion(
          "123->[345][789]", {123, 5}, {234, 345}, neutu::EBiDirection::FORWARD));

  ASSERT_FALSE(
        flyem::HasConnecion(
          "123->[345][789]", 123, 689, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "12->[345][789]", 123, 345, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 123, 789, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123->[345][789]", {12, 5}, {234, 35}, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123->[345][789]", {345, 789}, {123}, neutu::EBiDirection::FORWARD));


  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", 789, 123, neutu::EBiDirection::BACKWARD));
  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", 345, 123, neutu::EBiDirection::BACKWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 123, 789, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", 345, 345, neutu::EBiDirection::FORWARD));

  ASSERT_TRUE(
        flyem::HasConnecion(
          "123<-[345][789]", {345, 789}, {123}, neutu::EBiDirection::BACKWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", {123}, {345, 789}, neutu::EBiDirection::FORWARD));
  ASSERT_FALSE(
        flyem::HasConnecion(
          "123<-[345][789]", {123, 345}, {345}, neutu::EBiDirection::FORWARD));
}

#endif

#endif // ZFLYEMMISCTEST_H
