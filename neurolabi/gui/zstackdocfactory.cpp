#include "zstackdocfactory.h"
#include "zstackdoc.h"
#include "flyem/zflyemproofdoc.h"
#include "biocytin/zbiocytinprojectiondoc.h"
#include "flyem/zflyembody3ddoc.h"
#include "flyem/zflyembodymergedoc.h"
#include "flyem/zflyemstackdoc.h"


ZStackDocFactory::ZStackDocFactory()
{
}

ZSharedPointer<ZStackDoc> ZStackDocFactory::Make(neutu::Document::ETag tag)
{
  ZStackDoc *doc = NULL;
  switch (tag) {
  case neutu::Document::ETag::BIOCYTIN_PROJECTION:
    doc = new ZBiocytinProjectionDoc;
    break;
  case neutu::Document::ETag::FLYEM_PROOFREAD:
    doc = new ZFlyEmProofDoc;
    break;
  case neutu::Document::ETag::FLYEM_STACK:
    doc = new ZFlyEmStackDoc;
    break;
  case neutu::Document::ETag::FLYEM_MERGE:
    doc = new ZFlyEmBodyMergeDoc;
    break;
  default:
    doc = new ZStackDoc;
    break;
  }

  doc->setTag(tag);
  if (tag == neutu::Document::ETag::BIOCYTIN_STACK) {
    doc->setResolution(1, 1, 8, 'p');
    doc->setStackBackground(neutu::EImageBackground::BRIGHT);
  }

  return ZSharedPointer<ZStackDoc>(doc);
}
