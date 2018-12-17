#include "cypherquery.h"

const char *CypherQuery::KW_MATCH = "MATCH";
const char *CypherQuery::KW_WHERE = "WHERE";
const char *CypherQuery::KW_RETURN = "RETURN";

CypherQuery::CypherQuery()
{
}

void CypherQuery::setMatch(const QString &pattern, const QString &where)
{
  m_match.first = pattern;
  m_match.second = where;
}

void CypherQuery::setWhere(const QString &where)
{
  m_match.second = where;
}

void CypherQuery::setReturn(const QString &pattern)
{
  m_return = pattern;
}

void CypherQuery::AppendQuery(
    QString &query, const QString &keyword, const QString &pattern)
{
  if (!pattern.isEmpty()) {
    if (!query.isEmpty()) {
      query += " ";
    }
    query += keyword + " " + pattern;
  }
}

QString CypherQuery::getQueryString() const
{
  QString query;

  if (!m_match.first.isEmpty()) {
    AppendQuery(query, KW_MATCH, m_match.first);
    AppendQuery(query, KW_WHERE, m_match.second);
    AppendQuery(query, KW_RETURN, m_return);
  }

  return query;
}

CypherQueryBuilder::operator CypherQuery() const
{
  return m_query;
}

CypherQueryBuilder& CypherQueryBuilder::match(
    const QString &pattern, const QString &where)
{
  m_query.setMatch(pattern, where);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::where(const QString &pattern)
{
  m_query.setWhere(pattern);

  return *this;
}


CypherQueryBuilder& CypherQueryBuilder::ret(const QString &pattern)
{
  m_query.setReturn(pattern);

  return *this;
}
