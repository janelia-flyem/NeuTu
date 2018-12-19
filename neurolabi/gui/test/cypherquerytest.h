#ifndef CYPHERQUERYTEST_H
#define CYPHERQUERYTEST_H

#include "ztestheader.h"
#include "service/cypherquery.h"

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
}

#endif

#endif // CYPHERQUERYTEST_H
