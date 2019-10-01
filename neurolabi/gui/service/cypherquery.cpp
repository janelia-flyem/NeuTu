#include "cypherquery.h"

const char *CypherQuery::KW_MATCH = "MATCH";
const char *CypherQuery::KW_WHERE = "WHERE";
const char *CypherQuery::KW_RETURN = "RETURN";
const char *CypherQuery::KW_WITH = "WITH";
const char *CypherQuery::KW_AS = "AS";
const char *CypherQuery::KW_ORDER_BY = "ORDER BY";
const char *CypherQuery::KW_DESC = "DESC";
const char *CypherQuery::KW_LIMIT = "LIMIT";

CypherQuery::CypherQuery()
{
}

void CypherQuery::appendMatch(const QString &pattern, const QString &where)
{
  m_query.append(QueryPair(KW_MATCH, pattern));
  appendWhere(where);
}

void CypherQuery::appendWhere(const QString &where)
{
  appendQuery(KW_WHERE, where);
}

void CypherQuery::appendWith(const QString &with)
{
  appendQuery(KW_WITH, with);
}

void CypherQuery::appendWith(const QString &preAs, const QString &postAs)
{
  if (!preAs.isEmpty() && !postAs.isEmpty()) {
    appendWith(preAs + " " + KW_AS + " " + postAs);
  }
}

void CypherQuery::appendOrderDesc(const QString &pattern)
{
  if (!pattern.isEmpty()) {
    m_postProc +=  " " + QString(KW_ORDER_BY) + " " + pattern + " " + KW_DESC;
  }
}

void CypherQuery::appendLimit(int n)
{
  if (n > 0) {
    m_postProc += " " + QString(KW_LIMIT) + " " + QString::number(n);
  }
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

void CypherQuery::appendQuery(const QString &keyword, const QString &pattern)
{
  if (!pattern.isEmpty()) {
    m_query.append(QueryPair(keyword, pattern));
  }
}

QString CypherQuery::getQueryString() const
{
  QString query;

  if (!m_query.isEmpty()) {
    for (const auto& p : m_query) {
      AppendQuery(query, p.first, p.second);
    }
    AppendQuery(query, KW_RETURN, m_return);
    if (!m_postProc.isEmpty()) {
      query += " " + m_postProc;
    }
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
  m_query.appendMatch(pattern, where);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::matchNode(const QString &label)
{
  return match("(n:" + label + ")");
}

CypherQueryBuilder& CypherQueryBuilder::where(const QString &pattern)
{
  m_query.appendWhere(pattern);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::with(const QString &pattern)
{
  m_query.appendWith(pattern);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::with(
    const QString &preAs, const QString &postAs)
{
  m_query.appendWith(preAs, postAs);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::orderDesc(const QString &pattern)
{
  m_query.appendOrderDesc(pattern);

  return *this;
}

CypherQueryBuilder& CypherQueryBuilder::limit(int n)
{
  m_query.appendLimit(n);

  return *this;
}


CypherQueryBuilder& CypherQueryBuilder::ret(const QString &pattern)
{
  m_query.setReturn(pattern);

  return *this;
}
