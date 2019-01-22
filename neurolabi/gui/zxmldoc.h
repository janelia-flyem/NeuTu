#ifndef ZXMLDOC_H
#define ZXMLDOC_H

#include <string>
#include "core/zsharedpointer.h"

class ZXmlNode;
class QDomDocument;
class QDomElement;
class QDomNode;
class QString;

typedef ZSharedPointer<QDomNode> xmlNodePtr;
typedef ZSharedPointer<QDomDocument> xmlDocPtr;


//#define XML_NODE_DEFINED 1

//#include "tz_xml_utils.h"

//#undef HAVE_LIBXML2

class ZXmlDoc
{
public:
  ZXmlDoc();
  ~ZXmlDoc();

  void parseFile(const std::string &filePath);
  ZXmlNode getRootElement();

  void printInfo();

private:
  xmlDocPtr m_doc;
};

class ZXmlNode
{
public:
  ZXmlNode();
  ZXmlNode(xmlNodePtr node, xmlDocPtr doc);
  ZXmlNode(const QDomNode &node, xmlDocPtr doc);

  std::string stringValue();
  double doubleValue();
  int intValue();
  std::string name() const;

  bool empty() const;
  ZXmlNode firstChild() const;
  ZXmlNode nextSibling();
//  ZXmlNode next();

  std::string getAttribute(const char *attribute) const;

  int type() const;
  bool isElement() const;
  ZXmlNode queryNode(const std::string &nodeName) const;

  void printElementNames(int indent = 0) const;
  void printInfo(int indent = 0) const;

private:
  QString getText() const;

private:
  xmlNodePtr m_node;
  xmlDocPtr m_doc;
};

#endif // ZXMLDOC_H
