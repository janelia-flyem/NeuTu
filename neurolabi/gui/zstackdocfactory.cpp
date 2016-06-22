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

ZSharedPointer<ZStackDoc> ZStackDocFactory::Make(NeuTube::Document::ETag tag)
{
  ZStackDoc *doc = NULL;
  switch (tag) {
  case NeuTube::Document::BIOCYTIN_PROJECTION:
    doc = new ZBiocytinProjectionDoc;
    break;
#if defined(_FLYEM_)
  case NeuTube::Document::FLYEM_PROOFREAD:
    doc = new ZFlyEmProofDoc;
    break;
  case NeuTube::Document::FLYEM_STACK:
    doc = new ZFlyEmStackDoc;
    break;
  case NeuTube::Document::FLYEM_MERGE:
    doc = new ZFlyEmBodyMergeDoc;
    break;
#endif
  default:
    doc = new ZStackDoc;
    break;
  }

  doc->setTag(tag);
  if (tag == NeuTube::Document::BIOCYTIN_STACK) {
    doc->setResolution(1, 1, 8, 'p');
    doc->setStackBackground(NeuTube::IMAGE_BACKGROUND_BRIGHT);
  }

  return ZSharedPointer<ZStackDoc>(doc);
}
