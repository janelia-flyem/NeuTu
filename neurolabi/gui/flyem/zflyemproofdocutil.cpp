#include "zflyemproofdocutil.h"

#include "zstack.hxx"
#include "zsparsestack.h"
#include "zstackwriter.h"
#include "zstackobjectsourcefactory.h"
#include "zstackwatershedcontainer.h"

#include "dvid/zdvidlabelslice.h"
#include "dvid/zdvidreader.h"
#include "dvid/zdvidsparsestack.h"
#include "dvid/zdvidgrayslice.h"
#include "dvid/zdvidgraysliceensemble.h"

#include "zflyemproofdoc.h"
#include "zflyembodymanager.h"


ZFlyEmProofDocUtil::ZFlyEmProofDocUtil()
{
}

ZDvidLabelSlice* ZFlyEmProofDocUtil::GetActiveLabelSlice(
    ZFlyEmProofDoc *doc)
{
  if (doc) {
    return doc->getActiveLabelSlice();
  }

  return nullptr;
}

/*
ZDvidLabelSlice* ZFlyEmProofDocUtil::GetActiveLabelSlice(ZFlyEmProofDoc *doc)
{
  return GetActiveLabelSlice(doc, neutu::EAxis::Z);
}
*/

ZDvidGraySlice* ZFlyEmProofDocUtil::GetActiveGraySlice(ZFlyEmProofDoc *doc)
{
  return doc->getDvidGraySlice();
}

std::set<uint64_t> ZFlyEmProofDocUtil::GetSelectedBodyId(
    ZFlyEmProofDoc *doc, neutu::ELabelSource type)
{
  std::set<uint64_t> bodySet;

  if (doc) {
    ZDvidLabelSlice *slice = GetActiveLabelSlice(doc);
    if (slice != NULL) {
      bodySet = slice->getSelected(type);
      if (slice->isSupervoxel()) {
        std::set<uint64_t> newBodySet;
        std::transform(
              bodySet.begin(), bodySet.end(),
              std::inserter(newBodySet, newBodySet.begin()),
              ZFlyEmBodyManager::EncodeSupervoxel);
        bodySet = newBodySet;
      }
    }
  }

  return bodySet;
}

void ZFlyEmProofDocUtil::ExportSelectedBodyStack(
    ZFlyEmProofDoc *doc, bool isSparse, bool isFullRange,
    const ZIntCuboid &box, const QString &filePath)
{
  if (doc && !filePath.isEmpty()) {
    ZDvidLabelSlice *slice = doc->getActiveLabelSlice();
    if (slice != NULL) {
      std::set<uint64_t> idSet =
          slice->getSelected(neutu::ELabelSource::ORIGINAL);

      ZDvidReader &reader = doc->getDvidReader();
      ZDvidSparseStack *sparseStack = NULL;
      neutu::EBodyLabelType labelType = doc->isSupervoxelMode() ?
            neutu::EBodyLabelType::SUPERVOXEL : neutu::EBodyLabelType::BODY;
      if (reader.isReady() && !idSet.empty()) {
        std::set<uint64_t>::const_iterator iter = idSet.begin();
        sparseStack = reader.readDvidSparseStack(*iter, labelType);

        ++iter;
        for (; iter != idSet.end(); ++iter) {
          ZDvidSparseStack *sparseStack2 =
              reader.readDvidSparseStack(*iter, labelType);
          sparseStack->getSparseStack()->merge(*(sparseStack2->getSparseStack()));

          delete sparseStack2;
        }
      }

      bool saved = false;
      ZStackWriter stackWriter;
      //        stackWriter.setCompressHint(ZStackWriter::COMPRESS_NONE);
      if (isFullRange) {
        if (isSparse) {
          sparseStack->getSparseStack()->save(filePath.toStdString());
          saved = true;
        } else {
          ZStack *stack = sparseStack->makeIsoDsStack(neutu::ONEGIGA, true);
          stackWriter.write(filePath.toStdString(), stack);
          delete stack;
        }
        //          sparseStack->getStack()->save(fileName.toStdString());
      } else {
        if (isSparse) {
          sparseStack->getSparseStack()->save(filePath.toStdString());
          saved = true;
        } else {
          ZStack *stack = sparseStack->makeStack(box, true);
          //          stack->save(fileName.toStdString());
          stackWriter.write(filePath.toStdString(), stack);
          delete stack;
          saved = true;
        }
      }

      if (saved) {
        doc->notify(filePath + " saved.");
      }
      delete sparseStack;
    }
  }
}

void ZFlyEmProofDocUtil::ExportSelectedBody(
    ZFlyEmProofDoc *doc, const QString &filePath)
{
  if (doc && !filePath.isEmpty()) {
    ZDvidLabelSlice *slice = doc->getActiveLabelSlice();
    if (slice != NULL) {
      neutu::EBodyLabelType labelType = doc->isSupervoxelMode() ?
                  neutu::EBodyLabelType::SUPERVOXEL : neutu::EBodyLabelType::BODY;

      std::set<uint64_t> idSet =
          slice->getSelected(neutu::ELabelSource::ORIGINAL);
      ZObject3dScan obj;

      ZDvidReader &reader = doc->getDvidReader();
      if (reader.isReady()) {
        for (std::set<uint64_t>::const_iterator iter = idSet.begin();
             iter != idSet.end(); ++iter) {
          ZObject3dScan subobj;
          reader.readBody(*iter, labelType, ZIntCuboid(), false, &subobj);
          obj.concat(subobj);
        }
      }
      obj.canonize();
      obj.save(filePath.toStdString());
    }
  }
}

void ZFlyEmProofDocUtil::ExportSelecteBodyLevel(
    ZFlyEmProofDoc *doc, const ZIntCuboid &range,
    const QString &filePath)
{
  if (doc && !filePath.isEmpty()) {
    ZDvidLabelSlice *slice = doc->getActiveLabelSlice();
    if (slice != NULL) {
      neutu::EBodyLabelType labelType = doc->isSupervoxelMode() ?
                  neutu::EBodyLabelType::SUPERVOXEL : neutu::EBodyLabelType::BODY;

      std::set<uint64_t> idSet =
          slice->getSelected(neutu::ELabelSource::ORIGINAL);

      ZObject3dScanArray objArray;
      objArray.resize(idSet.size());

      ZDvidReader &reader = doc->getDvidReader();
      if (reader.isReady()) {
        int index = 0;
        for (std::set<uint64_t>::const_iterator iter = idSet.begin();
             iter != idSet.end(); ++iter) {
//            ZObject3dScan *obj = objArray[index];
          objArray[index] = reader.readBody(*iter, labelType, range, false, NULL);
          index++;
        }
      }

      ZStack *stack =  objArray.toLabelField();
      if (stack != NULL) {
        stack->save(filePath.toStdString());
        delete stack;
      }
    }
  }
}

QList<ZFlyEmBookmark*> ZFlyEmProofDocUtil::GetUserBookmarkList(
    ZFlyEmProofDoc *doc)
{
  QList<ZFlyEmBookmark*> objList;
  QList<ZFlyEmBookmark*> bookmarkList =
      doc->getObjectList<ZFlyEmBookmark>();
  for (ZFlyEmBookmark *bookmark : bookmarkList) {
    if (bookmark->isCustom()) {
      objList.append(bookmark);
    }
  }

  return objList;
}

bool ZFlyEmProofDocUtil::HasSupervoxel(ZFlyEmProofDoc *doc)
{
  if (doc) {
    return doc->getDvidTarget().hasSupervoxel();
  }

  return false;
}

bool ZFlyEmProofDocUtil::HasWrittableSynapse(ZFlyEmProofDoc *doc)
{
  if (HasSynapse(doc)) {
    return !doc->getDvidTarget().readOnly() &&
        doc->getDvidTarget().isSynapseEditable();
  }

  return false;
}

bool ZFlyEmProofDocUtil::HasSynapse(ZFlyEmProofDoc *doc)
{
  if (doc) {
    return doc->getDvidTarget().hasSynapse();
  }

  return false;
}

