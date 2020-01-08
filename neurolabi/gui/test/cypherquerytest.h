#ifndef CYPHERQUERYTEST_H
#define CYPHERQUERYTEST_H

#include "ztestheader.h"
#include "service/cypherquery.h"

#include <QDebug>

#ifdef _USE_GTEST_

TEST(CypherQuery, basic)
{
  CypherQuery query;
  ASSERT_TRUE(query.getQueryString().isEmpty());

  query.appendMatch("(n:label)");
  ASSERT_EQ("MATCH (n:label)", query.getQueryString());

  query.setReturn("n.name");
  ASSERT_EQ("MATCH (n:label) RETURN n.name", query.getQueryString());
}

TEST(CypherQueryBuilder, basic)
{
  CypherQuery query = CypherQueryBuilder().matchNode("label");
  ASSERT_EQ("MATCH (n:label)", query.getQueryString());

  ASSERT_EQ("name",
            CypherQueryBuilder::OrEqualClause("name", QList<QString>()));

  QList<QString> valueList;
  valueList.append("test1");
  ASSERT_EQ("name = test1",
            CypherQueryBuilder::OrEqualClause("name", valueList));

  valueList.append("test2");
  ASSERT_EQ("name = test1 OR name = test2",
            CypherQueryBuilder::OrEqualClause("name", valueList));

  QList<QString> lowerValueList = valueList;
  std::transform(lowerValueList.begin(), lowerValueList.end(), lowerValueList.begin(),
                 [](const QString &str) { return "LOWER(\"" + str + "\")"; });

  query = CypherQueryBuilder().matchNode("label").
      match("(n:label").
      where(CypherQueryBuilder::OrEqualClause("LOWER(n.status)", lowerValueList));
//  qDebug() << query.getQueryString();
  ASSERT_EQ("MATCH (n:label) MATCH (n:label WHERE "
            "LOWER(n.status) = LOWER(\"test1\") OR "
            "LOWER(n.status) = LOWER(\"test2\")", query.getQueryString());
}

#endif

#endif // CYPHERQUERYTEST_H
