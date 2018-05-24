#ifndef ZDVIDRESULTSERVICETEST_H
#define ZDVIDRESULTSERVICETEST_H

#include "ztestheader.h"
#include "dvid/zdvidresultservice.h"
#include "neutubeconfig.h"
#include "dvid/zdvidurl.h"

#ifdef _USE_GTEST_
TEST(ZDvidResultService, Basic)
{
  /*
  ASSERT_EQ("head__6c8409b833d57d9c62856b6cab608aa5",
            ZDvidResultService::GetEndPointKey(
              "http://localhost:8000/api/node/4d3e/task_split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5").toStdString());

  ASSERT_EQ("head__6c8409b833d57d9c62856b6cab608aa5",
            ZDvidResultService::GetSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/task_split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5").toStdString());
              */

}

#endif

#endif // ZDVIDRESULTSERVICETEST_H
