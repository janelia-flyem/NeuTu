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

ZSharedPointer<ZStackDoc> ZStackDocFactory::Make(neutube::Document::ETag tag)
{
  ZStackDoc *doc = NULL;
  switch (tag) {
  case neutube::Document::BIOCYTIN_PROJECTION:
    doc = new ZBiocytinProjectionDoc;
    break;
  case neutube::Document::FLYEM_PROOFREAD:
    doc = new ZFlyEmProofDoc;
    break;
  case neutube::Document::FLYEM_STACK:
    doc = new ZFlyEmStackDoc;
    break;
  case neutube::Document::FLYEM_MERGE:
    doc = new ZFlyEmBodyMergeDoc;
    break;
  default:
    doc = new ZStackDoc;
    break;
  }

  doc->setTag(tag);
  if (tag == neutube::Document::BIOCYTIN_STACK) {
    doc->setResolution(1, 1, 8, 'p');
    doc->setStackBackground(neutube::IMAGE_BACKGROUND_BRIGHT);
  }

  return ZSharedPointer<ZStackDoc>(doc);
}
