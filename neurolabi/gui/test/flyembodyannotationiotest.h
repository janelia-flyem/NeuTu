#ifndef FLYEMBODYANNOTATIONIOTEST_H
#define FLYEMBODYANNOTATIONIOTEST_H

#include "ztestheader.h"

#include "zjsonobject.h"

#include "flyem/zflyembodyannotation.h"
#include "flyem/flyembodyannotationlocalio.h"

#ifdef _USE_GTEST_

TEST(FlyEmBodyAnnotationIO, Basic)
{
  FlyEmBodyAnnotationLocalIO bio;

  ZJsonObject obj;

  obj.setEntry("status", "Traced");

  bio.writeBodyAnnotation(1, obj);

  {
    ZJsonObject fetched =  bio.readBodyAnnotation(1);
    ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(fetched));

    fetched.setEntry("status", "Anchor");
    fetched =  bio.readBodyAnnotation(1);
    ASSERT_EQ("Traced", ZFlyEmBodyAnnotation::GetStatus(fetched));

    bio.deleteBodyAnnotation(1);
    ASSERT_FALSE(bio.hasBodyAnnotation(1));

    fetched =  bio.readBodyAnnotation(2);
    ASSERT_TRUE(fetched.isEmpty());
  }
}

#endif

#endif // FLYEMBODYANNOTATIONIOTEST_H
