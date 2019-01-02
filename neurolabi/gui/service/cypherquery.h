#ifndef CYPHERQUERY_H
#define CYPHERQUERY_H

#include <QString>
#include <QPair>
#include <QList>

class CypherQuery
{
public:
  CypherQuery();

  QString getQueryString() const;

  void appendMatch(const QString &pattern, const QString &where = "");
  void appendWhere(const QString &where);
  void appendWith(const QString &with);
  void appendWith(const QString &preAs, const QString &postAs);
  void appendOrderDesc(const QString &pattern);
  void appendLimit(int n);
  void setReturn(const QString &pattern);

public:
  static const char *KW_MATCH;
  static const char *KW_WITH;
  static const char *KW_AS;
  static const char *KW_WHERE;
  static const char *KW_ORDER_BY;
  static const char *KW_DESC;
  static const char *KW_LIMIT;
  static const char *KW_RETURN;

private:
  static void AppendQuery(
      QString &query, const QString &keyword, const QString &pattern);
  void appendQuery(const QString &keyword, const QString &pattern);

  using QueryPair = QPair<QString, QString>;

private:
  QList<QueryPair> m_query;
  QString m_return;
  QString m_postProc;
};

struct CypherQueryBuilder {
  operator CypherQuery() const;
  CypherQueryBuilder& match(const QString &pattern, const QString &where = "");
  CypherQueryBuilder& matchNode(const QString &label);
  CypherQueryBuilder& where(const QString &pattern);
  CypherQueryBuilder& with(const QString &pattern);
  CypherQueryBuilder& with(const QString &preAs, const QString &postAs);
  CypherQueryBuilder& orderDesc(const QString &pattern);
  CypherQueryBuilder& limit(int n);
  CypherQueryBuilder& ret(const QString &pattern);

private:
  CypherQuery m_query;
};

#endif // CYPHERQUERY_H
