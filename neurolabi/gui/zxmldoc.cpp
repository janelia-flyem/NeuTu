#include "zxmldoc.h"

#include <iostream>
#include <iomanip>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QDebug>

using namespace std;

ZXmlDoc::ZXmlDoc() : m_doc(NULL)
{
}

ZXmlDoc::~ZXmlDoc()
{
#if defined(HAVE_LIBXML2)
  if (m_doc != NULL) {
    xmlFreeDoc(m_doc);
    xmlCleanupParser();
  }
#endif
}

void ZXmlDoc::parseFile(const std::string &filePath)
{
#if defined(HAVE_LIBXML2)
  if (m_doc != NULL) {
    xmlFreeDoc(m_doc);
  }

  m_doc = xmlParseFile(filePath.c_str());
#else
  m_doc = ZSharedPointer<QDomDocument>(new QDomDocument());
  QFile file(filePath.c_str());
  if (file.open(QIODevice::ReadOnly)) {
    m_doc->setContent(&file);
  }
  file.close();
#endif
}

ZXmlNode ZXmlDoc::getRootElement()
{
#if defined(HAVE_LIBXML2)
  return ZXmlNode(Xml_Doc_Root_Element(m_doc), m_doc);
#else
  return ZXmlNode(m_doc->documentElement(), m_doc);
#endif
}

void ZXmlDoc::printInfo()
{
  getRootElement().printInfo();
}

ZXmlNode::ZXmlNode()
#if defined(HAVE_LIBXML2)
  : ZXmlNode(NULL, NULL)
#endif
{
}

ZXmlNode::ZXmlNode(xmlNodePtr node, xmlDocPtr doc)
  : m_node(node), m_doc(doc)
{
}

ZXmlNode::ZXmlNode(const QDomNode &node, xmlDocPtr doc) :
  ZXmlNode(xmlNodePtr(new QDomNode(node)), doc)
{
}

string ZXmlNode::name() const
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return "";
  }

  char* nameStr = Xml_Node_Name(m_node);

  string name(nameStr);

  free(nameStr);

  return name;
#else
  return m_node->nodeName().toStdString();
#endif
}

QString ZXmlNode::getText() const
{
  QString text;

  if (!empty()) {
    QDomText textNode = m_node->firstChild().toText();
    if (!textNode.isNull()) {
      text = textNode.data();
    }
  }

  return text;
}

string ZXmlNode::stringValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return "";
  }

  char *value = Xml_Node_String_Value(m_doc, m_node);
  string strValue;
  if (value != NULL) {
    strValue = value;
  }
  free(value);

  return strValue;
#else
  return getText().toStdString();
#endif
}

double ZXmlNode::doubleValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return NaN;
  }

  return Xml_Node_Double_Value(m_doc, m_node);
#else
  return getText().toDouble();
#endif
}

int ZXmlNode::intValue()
{
#if defined(HAVE_LIBXML2)
  if (empty()) {
    return 0;
  }

  return Xml_Node_Int_Value(m_doc, m_node);
#else
  return getText().toInt();
#endif
}

bool ZXmlNode::empty() const
{
#if defined(HAVE_LIBXML2)
  return (m_node == NULL);
#else
  if (m_node) {
    return m_node->isNull();
  }
  return true;
#endif
}

ZXmlNode ZXmlNode::firstChild() const
{
  if (empty()) {
    return ZXmlNode();
  }

#if defined(HAVE_LIBXML2)
  ZXmlNode child;
  child.m_node = m_node->xmlChildrenNode;
  child.m_doc = m_doc;
#else
  ZXmlNode child(m_node->firstChild(), m_doc);
#endif
  return child;
}

ZXmlNode ZXmlNode::nextSibling()
{
  if (empty()) {
    return ZXmlNode();
  }

#if defined(HAVE_LIBXML2)
  ZXmlNode sibling;
  sibling.m_node = m_node->next;
  sibling.m_doc = m_doc;
#else
  ZXmlNode sibling(m_node->nextSibling(), m_doc);
#endif

  return sibling;
}

#if 0
ZXmlNode ZXmlNode::next()
{
#if defined(HAVE_LIBXML2)
  ZXmlNode nextNode;
  if (!empty()) {
    nextNode.m_node = m_node->next;
    nextNode.m_doc = m_doc;
  }
#else
  ZXmlNode nextNode(m_node->nextSibling(), m_doc);
#endif
  return nextNode;
}

int ZXmlNode::type() const
{
#if defined(HAVE_LIBXML2)
  if (m_node != NULL) {
    return m_node->type;
  }
#endif

  return 0;
}
#endif

bool ZXmlNode::isElement() const
{
  if (empty()) {
    return false;
  }

#if defined(HAVE_LIBXML2)
  return type() == XML_ELEMENT_NODE;
#else
  return m_node->isElement();
#endif
}

void ZXmlNode::printElementNames(int indent) const
{
  if (!empty()) {
    if (m_node->isElement()) {
      std::cout << setfill(' ') << setw(indent) << "";
      std::cout << name() << std::endl;
      ZXmlNode child = firstChild();
#ifdef _DEBUG_2
      std::cout << child.name() << std::endl;
#endif
      while (!child.empty()) {
        child.printElementNames(indent + 2);
        child = child.nextSibling();
      }
    }
  }
}

void ZXmlNode::printInfo(int indent) const
{
  if (!empty()) {
    if (m_node->isElement()) {
      std::cout << setfill(' ') << setw(indent) << "";
      std::cout << name() << "(" << m_node->childNodes().size() << "): ";
      QDomText textNode = m_node->firstChild().toText();
      if (!textNode.isNull()) {
        std::cout << qPrintable(textNode.data());
      }
      std::cout << std::endl;
      ZXmlNode child = firstChild();
#ifdef _DEBUG_2
      std::cout << child.name() << std::endl;
#endif
      while (!child.empty()) {
        child.printInfo(indent + 2);
        child = child.nextSibling();
      }
    }
  }
}

ZXmlNode ZXmlNode::queryNode(const std::string &nodeName) const
{
  ZXmlNode node;
  if (!empty()) {
    if (isElement()) {
#ifdef _DEBUG_2
      std::cout << name() << std::endl;
#endif
      if (name() == nodeName) {
        node = *this;
      } else {
        ZXmlNode child = firstChild();
        while (!child.empty()) {
          node = child.queryNode(nodeName);
          if (!node.empty()) {
            break;
          }
          child = child.nextSibling();
        }
      }
    }
  }

  return node;
}

string ZXmlNode::getAttribute(const char *attribute) const
{
  string attributeValue;
  if (!empty()) {
#if defined(HAVE_LIBXML2)
    if (m_node != NULL) {
      xmlChar *prop = xmlGetProp(m_node, CONST_XML_STRING(attribute));
      char *tmpStr = Xml_String_To_String(prop);
      attributeValue = tmpStr;
      free(tmpStr);
      free(prop);
    }
#else
    QDomNamedNodeMap nodeMap = m_node->attributes();
    QDomAttr node = nodeMap.namedItem(attribute).toAttr();
    if (!node.isNull()) {
      return node.nodeValue().toStdString();
    }
#endif
  }
  return attributeValue;
}
