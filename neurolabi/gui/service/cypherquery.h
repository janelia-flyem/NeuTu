#ifndef CYPHERQUERY_H
#define CYPHERQUERY_H

#include <QString>
#include <QPair>

class CypherQuery
{
public:
  CypherQuery();

  QString getQueryString() const;

  void setMatch(const QString &pattern, const QString &where = "");
  void setWhere(const QString &where);
  void setReturn(const QString &pattern);

private:
  static void AppendQuery(
      QString &query, const QString &keyword, const QString &pattern);

public:
  static const char *KW_MATCH;
  static const char *KW_WHERE;
  static const char *KW_RETURN;

private:
  QPair<QString, QString> m_match;
  QString m_return;
};

struct CypherQueryBuilder {
  operator CypherQuery() const;
  CypherQueryBuilder& match(const QString &pattern, const QString &where = "");
  CypherQueryBuilder& where(const QString &pattern);
  CypherQueryBuilder& ret(const QString &pattern);

private:
  CypherQuery m_query;
};

#endif // CYPHERQUERY_H
