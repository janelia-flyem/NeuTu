#ifndef ZSTACKDOCACCESSOR_H
#define ZSTACKDOCACCESSOR_H

#include <QList>

#include "zstackobject.h"
#include "mvc/zstackdocdatabuffer.h"

class ZStackDoc;
class ZStackWatershedContainer;
class ZObject3dScan;
class ZObject3dScanArray;

/*!
 * \brief The class for accessing data in ZStackDoc in a thread-safe way.
 */
class ZStackDocAccessor
{
public:
  ZStackDocAccessor();

public:
  static void SetObjectVisible(
      ZStackDoc *doc, ZStackObject::EType type, const std::string &source,
      bool on);
  static void SetObjectVisible(
      ZStackDoc *doc, ZStackObject::EType type,
      std::function<bool(const ZStackObject*)> pred, bool on);

  static void SetObjectSelection(
      ZStackDoc *doc, ZStackObject::EType type,
      std::function<bool(const ZStackObject*)> pred, bool on);

  static void RemoveObject(
      ZStackDoc *doc, ZStackObject *obj, bool deleteObject);
  static void RemoveObject(
      ZStackDoc *doc, ZStackObject::EType type, bool deleteObject);
  static void RemoveObject(
      ZStackDoc *doc, ZStackObject::EType type, const std::string &source,
      bool deleteObject);
  static void RemoveObject(
      ZStackDoc *doc, ZStackObjectRole::TRole role, bool deleteObject);
  static void RemoveAllSwcTree(ZStackDoc *doc, bool deleteObject);
  static void AddObject(ZStackDoc *doc, ZStackObject *obj);
  static void AddObjectUnique(ZStackDoc *doc, ZStackObject *obj);
  static void AddObject(ZStackDoc *doc, const QList<ZStackObject*> &objList);

  static void ParseWatershedContainer(
      ZStackDoc *doc, ZStackWatershedContainer *container);

  static void RemoveSplitSeed(ZStackDoc *doc, uint64_t label);
  static void RemoveSplitSeed(ZStackDoc *doc);
  static void RemoveSplitSeed(ZStackDoc *doc, std::string objectClass);

  /*!
   * \brief Remove seeds except those with the main label
   */
  static void RemoveSideSplitSeed(ZStackDoc *doc);

  static void UpdateSplitResult(
      ZStackDoc *doc, const QList<ZObject3dScan *> &result);

  /*!
   * \brief Consume split result
   *
   * \a result will be consumed by the function and should NOT be used after
   * the call.
   */
  static void ConsumeSplitResult(
      ZStackDoc *doc, ZObject3dScanArray *result);

private:
  static ZStackDocObjectUpdate::EAction GetRemoveAction(bool deleteObject);
};

#endif // ZSTACKDOCACCESSOR_H
