#ifndef ZSTACKDOCSELECTOR_H
#define ZSTACKDOCSELECTOR_H

#include <map>
#include <memory>

//#include "common/zsharedpointer.h"
#include "zstackobject.h"

class ZStackDoc;

class ZStackDocSelector
{
public:
  ZStackDocSelector();
  explicit ZStackDocSelector(const std::shared_ptr<ZStackDoc> &doc);

  void setDocument(const std::shared_ptr<ZStackDoc> &doc);

  enum ESelectOption {
    SELECT_IGNORE, SELECT_NORMAL, SELECT_RECURSIVE
  };

  void deselectAll();

  void setSelectOption(ZStackObject::EType type, ESelectOption option);

  /*!
   * \brief Delect all objects in a document
   */
  static void Deselect(
      ZStackDoc *doc,
      const std::map<ZStackObject::EType, ESelectOption> &optionMap);

  static void DeselectAll(ZStackDoc *doc);

private:
  std::shared_ptr<ZStackDoc> m_doc;
  std::map<ZStackObject::EType, ESelectOption> m_option;
};

#endif // ZSTACKDOCSELECTOR_H
