#ifndef ZDVIDURLTEST_H
#define ZDVIDURLTEST_H

#include "ztestheader.h"

#include "dvid/zdvidurl.h"

#ifdef _USE_GTEST_

TEST(ZDvidUrl, Basic)
{
  ZDvidTarget target("emdata.janelia.org", "bf1");
  target.setSegmentationName("labels");
  target.setBodyLabelName("bodies");
  target.setGrayScaleName("grayscale");
  target.setAdminToken("testtoken");

  //const std::vector<ZDvidTarget> &dvidRepo =
  //    NeutubeConfig::getInstance().getFlyEmConfig().getDvidRepo();
  ZDvidUrl dvidUrl(target);
  std::cout << dvidUrl.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

//  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons",
            dvidUrl.getSkeletonUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons/key/1_swc",
            dvidUrl.getSkeletonUrl(1));

  ASSERT_EQ("", dvidUrl.getSkeletonUrl(""));

  dvidUrl.setAdmin(true);
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

//  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons",
            dvidUrl.getSkeletonUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/skeletons/key/1_swc?admintoken=testtoken",
            dvidUrl.getSkeletonUrl(1));
  dvidUrl.setAdmin(false);

//  std::cout << dvidUrl.getMergeOperationUrl(
//                 ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION))
//            << std::endl;
  /*
  ASSERT_EQ(std::string("http://emdata.janelia.org/api/node/bf1/") +
            ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION),
            dvidUrl.getMergeOperationUrl(
              ZDvidData::GetName(ZDvidData::ROLE_MERGE_OPERATION)));
*/
//  std::cout << dvidUrl.getNodeUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1", dvidUrl.getNodeUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getDataUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getDataUrl(ZDvidData::ERole::GRAYSCALE));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getDataUrl(ZDvidData::ERole::GRAYSCALE,
                               ZDvidData::ERole::SPARSEVOL, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_grayscale",
            dvidUrl.getDataUrl(ZDvidData::ERole::GRAYSCALE,
                               ZDvidData::ERole::SPARSEVOL, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/info",
            dvidUrl.getInfoUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/help",
            dvidUrl.getHelpUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/server/info",
            dvidUrl.getServerInfoUrl());
//  std::cout << dvidUrl.getServerInfoUrl() << std::endl;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.obj",
            dvidUrl.getMeshUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1_2.obj",
            dvidUrl.getMeshUrl(1, 2));
//  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1_info",
//            dvidUrl.getMeshInfoUrl(1, 0));
//  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes_2/key/1_info",
//            dvidUrl.getMeshInfoUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.obj",
            dvidUrl.getMeshUrl(1, ZDvidUrl::EMeshType::DEFAULT));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.ngmesh",
            dvidUrl.getMeshUrl(1, ZDvidUrl::EMeshType::NG));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.merge",
            dvidUrl.getMeshUrl(1, ZDvidUrl::EMeshType::MERGED));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.ngmesh",
            dvidUrl.getNgMeshUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/meshes/key/1.merge",
            dvidUrl.getMergedMeshUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api", dvidUrl.getApiUrl());
//  std::cout << dvidUrl.getApiUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/repo/bf1", dvidUrl.getRepoUrl());
//  std::cout << dvidUrl.getRepoUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/repo/bf1/instance",
            dvidUrl.getInstanceUrl());

//  target.setBodyLabelName("bodies2");
//  std::cout << dvidUrl.getSkeletonUrl("") << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons",
            dvidUrl.getSkeletonUrl("bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons/key/1_swc",
            dvidUrl.getSkeletonUrl(1, "bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/thumbnails/key/1_mraw",
            dvidUrl.getThumbnailUrl(1));

  target.setGrayScaleName("grayscale2");
  target.setBodyLabelName("bodies2");
  target.setSegmentationName("labels2");
  ZDvidUrl dvidUrl2(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons",
            dvidUrl2.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_skeletons/key/1_swc",
            dvidUrl2.getSkeletonUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_meshes/key/1.obj",
            dvidUrl2.getMeshUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_meshes/key/1_1.obj",
            dvidUrl2.getMeshUrl(1, 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol",
            dvidUrl.getSparsevolUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol/1",
            dvidUrl.getSparsevolUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol/1",
            dvidUrl.getSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol",
            dvidUrl.getSparsevolUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl.getSparsevolUrl(1, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl2.getSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol/1",
            dvidUrl2.getMultiscaleSparsevolUrl(1, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_1/sparsevol/1",
            dvidUrl2.getMultiscaleSparsevolUrl(1, 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels2/mapping",
            dvidUrl2.getLabelMappingUrl());



  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse",
            dvidUrl.getCoarseSparsevolUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse",
            dvidUrl.getCoarseSparsevolUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse/1",
            dvidUrl.getCoarseSparsevolUrl(1, "bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2/sparsevol-coarse/1",
            dvidUrl2.getCoarseSparsevolUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale",
            dvidUrl.getGrayscaleUrl());
//  std::cout << dvidUrl.getGrayscaleUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/raw/0_1/512_1024/1_2_3",
            dvidUrl.getGrayscaleUrl(512, 1024, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/raw/0_1/512_1024/1_2_3/png",
            dvidUrl.getGrayscaleUrl(512, 1024, 1, 2, 3, "png"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2",
            dvidUrl2.getGrayscaleUrl());
//  std::cout << dvidUrl.getGrayscaleUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/raw/0_1/512_1024/1_2_3",
            dvidUrl2.getGrayscaleUrl(512, 1024, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/raw/0_1/512_1024/1_2_3/png",
            dvidUrl2.getGrayscaleUrl(512, 1024, 1, 2, 3, "png"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/blocks/1_2_3/1",
            dvidUrl.getGrayScaleBlockUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale/blocks/1_2_3/2",
            dvidUrl.getGrayScaleBlockUrl(1, 2, 3, 2));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/blocks/1_2_3/1",
            dvidUrl2.getGrayScaleBlockUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/grayscale2/blocks/1_2_3/2",
            dvidUrl2.getGrayScaleBlockUrl(1, 2, 3, 2));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl.getLabels64Url("labels", 100, 200, 300, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl.getLabels64Url(100, 200, 300, 1, 2, 3));
  ASSERT_TRUE(dvidUrl.getLabels64Url(100, 200, 300, 1, 2, 3, 1).empty());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl2.getLabels64Url("labels", 100, 200, 300, 1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels2/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl2.getLabels64Url(100, 200, 300, 1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/key/1",
            dvidUrl.getKeyUrl("test", "1"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/keys",
            dvidUrl.getAllKeyUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/keyrange/1/3",
            dvidUrl.getKeyRangeUrl("test", "1", "3"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/keyrangevalues/1/3?tar=true",
            dvidUrl.getKeyValuesUrl("test", "1", "3"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations",
            dvidUrl.getBodyAnnotationUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_annotations",
            dvidUrl.getBodyAnnotationUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations/key/1",
            dvidUrl.getBodyAnnotationUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_annotations/key/1",
            dvidUrl2.getBodyAnnotationUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo",
            dvidUrl.getBodyInfoUrl("bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_bodyinfo",
            dvidUrl.getBodyInfoUrl("bodies2"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo/key/1",
            dvidUrl.getBodyInfoUrl(1, "bodies"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodyinfo/key/1",
            dvidUrl.getBodyInfoUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2_bodyinfo/key/1",
            dvidUrl2.getBodyInfoUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bound_box",
            dvidUrl.getBoundBoxUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bound_box/key/1",
            dvidUrl.getBoundBoxUrl(1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies",
            dvidUrl.getBodyLabelUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies2",
            dvidUrl.getBodyLabelUrl("bodies2"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sizerange/100",
            dvidUrl.getBodyListUrl(100));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/sizerange/100/200",
            dvidUrl.getBodyListUrl(100, 200));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bodies/allsynapse",
            dvidUrl.getSynapseListUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/annotations/key/test",
            dvidUrl.getSynapseAnnotationUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/merge",
            dvidUrl.getMergeUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1",
            dvidUrl.getSplitUrl("test", 1));

  /*
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getMergeOperationUrl("test"));
            */
//  std::cout << dvidUrl.getMergeOperationUrl("test") << std::endl;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/neutu_merge_opr/key/labels",
            dvidUrl.getMergeOperationUrl(""));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/neutu_merge_opr/key/labels_zhaot",
            dvidUrl.getMergeOperationUrl("zhaot"));
//  std::cout << dvidUrl.getMergeOperationUrl() << std::endl;


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1",
            dvidUrl.getSplitUrl("test", 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split/1?splitlabel=2",
            dvidUrl.getSplitUrl("test", 1, 2));


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/split-coarse/1",
            dvidUrl.getCoarseSplitUrl("test", 1));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bookmarks/key/1_2_3",
            dvidUrl.getBookmarkKeyUrl(1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/bookmark_annotations/elements/100_200_300/1_2_3",
            dvidUrl.getBookmarkUrl(1, 2, 3, 100, 200, 300));


//  std::string getMergeOperationUrl(const std::string &dataName) const;

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles",
            dvidUrl.getTileUrl("tiles"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/1",
            dvidUrl.getTileUrl("tiles", 1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/tiles/tile/xy/0/1_2_3",
            dvidUrl.getTileUrl("tiles", 0, 1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/label/1_2_3",
            dvidUrl.getLocalBodyIdUrl(1, 2, 3));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labels/labels",
            dvidUrl.getLocalBodyIdArrayUrl());

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/new_roi/roi",
            dvidUrl.getRoiUrl("new_roi"));


  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test",
            dvidUrl.getAnnotationUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getAnnotationSyncUrl("test"));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getAnnotationSyncUrl("test", ""));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync?replace=true",
            dvidUrl.getAnnotationSyncUrl("test", "replace=true"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/sync",
            dvidUrl.getLabelszSyncUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/tag/zhaot",
            dvidUrl.getAnnotationUrl("test", "zhaot"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/label/123",
            dvidUrl.getAnnotationUrl("test", 123));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/elements/100_200_300/1_2_3",
            dvidUrl.getAnnotationUrl("test", 1, 2, 3, 100, 200, 300));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/elements",
            dvidUrl.getAnnotationElementsUrl("test"));

  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/test/element/1_2_3",
            dvidUrl.getAnnotationDeleteUrl("test", 1, 2, 3));

  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, 2, 3));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, 2, 3, 100, 200, 300));
  ASSERT_EQ("",
            dvidUrl.getSynapseMoveUrl(ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, true));
  ASSERT_EQ("",
            dvidUrl.getSynapseUrl(1, false));
  ASSERT_EQ("",
            dvidUrl.getSynapseLabelszUrl(1));
  ASSERT_EQ("",
            dvidUrl.getSynapseLabelszUrl(
              0, dvid::ELabelIndexType::ALL_SYN));

  target.setSynapseName("synapse");
  dvidUrl.setDvidTarget(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/element/1_2_3",
            dvidUrl.getSynapseUrl(1, 2, 3));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/elements/100_200_300/1_2_3",
            dvidUrl.getSynapseUrl(1, 2, 3, 100, 200, 300));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/move/1_2_3/4_5_6",
            dvidUrl.getSynapseMoveUrl(ZIntPoint(1, 2, 3), ZIntPoint(4, 5, 6)));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/label/1?relationships=true",
            dvidUrl.getSynapseUrl(1, true));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse/label/1?relationships=false",
            dvidUrl.getSynapseUrl(1, false));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse_labelsz/top/1",
            dvidUrl.getSynapseLabelszUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/synapse_labelsz/top/1/AllSyn",
            dvidUrl.getSynapseLabelszUrl(1, dvid::ELabelIndexType::ALL_SYN));

  target.setSegmentationName("labelstest");
  target.setMaxLabelZoom(5);
  ZDvidUrl dvidUrl3(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_2/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 0));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_5/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 5));
  ASSERT_EQ("http://emdata.janelia.org/api/node/bf1/labelstest_1/raw/0_1_2/100_200_300/1_2_3",
            dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 1));
  ASSERT_EQ("", dvidUrl3.getLabels64Url(100, 200, 300, 1, 2, 3, 6));


  ZDvidUrl dvidUrl4(target, "1234", true);
  std::cout << dvidUrl4.getHelpUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/help", dvidUrl.getHelpUrl());

//  std::cout << dvidUrl.getSkeletonUrl() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/1234/bodies2_skeletons",
            dvidUrl4.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/1234/branches/key/master",
            dvidUrl4.getOldMasterUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/repo/1234/branch-versions/master",
            dvidUrl4.getMasterUrl());

  dvidUrl4.setDvidTarget(target, "3456");
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2_skeletons",
            dvidUrl4.getSkeletonUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/branches/key/master",
            dvidUrl4.getOldMasterUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/repo/3456/branch-versions/master",
            dvidUrl4.getMasterUrl());
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/default_instances/key/data",
            dvidUrl4.getDefaultDataInstancesUrl());
  ASSERT_TRUE(dvidUrl4.getSparsevolSizeUrl(1, neutu::EBodyLabelType::BODY).empty());

//  target.useLabelArray(true);
  target.setSegmentationType(ZDvidData::EType::LABELARRAY);
  dvidUrl4.setDvidTarget(target, "3456");
//  std::cout << target.getBodyLabelName() << std::endl;
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol-size/1",
            dvidUrl4.getSparsevolSizeUrl(1, neutu::EBodyLabelType::BODY));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol-size/1?"
            "supervoxels=true",
            dvidUrl4.getSparsevolSizeUrl(1, neutu::EBodyLabelType::SUPERVOXEL));

  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1",
            dvidUrl4.getSparsevolUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=2&maxx=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutu::EAxis::X));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?miny=2&maxy=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutu::EAxis::Y));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minz=2&maxz=2&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, neutu::EAxis::Z));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=2&maxx=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutu::EAxis::X));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?miny=2&maxy=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutu::EAxis::Y));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minz=2&maxz=3&exact=true",
            dvidUrl4.getSparsevolUrl(1, 2, 3, neutu::EAxis::Z));
  ZIntCuboid box(10, 20, 30, 40, 50, 60);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?minx=10&maxx=40&miny=20&maxy=50&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, box));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?scale=2",
            dvidUrl4.getMultiscaleSparsevolUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?scale=2",
            dvidUrl4.getMultiscaleSparsevolUrl(1, 2));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&minx=10&maxx=40&miny=20&maxy=50&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, 2, box));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true",
            dvidUrl4.getSupervoxelUrl(1));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true&minz=2&maxz=3&exact=true",
            dvidUrl4.getSupervoxelUrl(1, 2, 3, neutu::EAxis::Z));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1?supervoxels=true&scale=2",
            dvidUrl4.getMultiscaleSupervoxelUrl(1, 2));

  box.setMinX(0);
  box.setMaxX(-1);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&miny=20&maxy=50&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, 2, box));

  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/test/label/123",
            dvidUrl4.getAnnotationUrl("test", 123));

  box.setMinY(0);
  box.setMaxY(-1);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&minz=30&maxz=60",
            dvidUrl4.getSparsevolUrl(1, 2, box));

  box.setMinZ(0);
  box.setMaxZ(-1);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2",
            dvidUrl4.getSparsevolUrl(1, 2, box));

  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=0",
            dvidUrl4.getSparsevolUrl(1, 0, box));

  dvid::SparsevolConfig config;
  config.bodyId = 1;
  config.labelType = neutu::EBodyLabelType::BODY;
  config.format = "blocks";
  config.zoom = 0;
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=0&format=blocks",
            dvidUrl4.getSparsevolUrl(config));

  config.range.setMinX(1);
  config.range.setMaxX(10);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=0&format=blocks&minx=1&maxx=10",
            dvidUrl4.getSparsevolUrl(config));

  config.zoom = 2;
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&format=blocks&minx=1&maxx=10",
            dvidUrl4.getSparsevolUrl(config));

  config.range.setMinY(2);
  config.range.setMaxY(20);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/bodies2/sparsevol/1"
            "?scale=2&format=blocks&minx=1&maxx=10&miny=2&maxy=20",
            dvidUrl4.getSparsevolUrl(config));

  ASSERT_EQ(12345, int(ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/12345")));
  ASSERT_EQ(int(0), int(ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/")));
  ASSERT_EQ(uint64_t(123451234512345L), ZDvidUrl::GetBodyId(
              "http://localhost:8000/api/node/uuid/segname/sparsevol/123451234512345"));

  ASSERT_EQ("head__6c8409b833d57d9c62856b6cab608aa5",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/task_split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5"));
  ASSERT_EQ("",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/split/key/"
              "head__6c8409b833d57d9c62856b6cab608aa5"));
  ASSERT_EQ("",
            ZDvidUrl::ExtractSplitTaskKey(
              "http://localhost:8000/api/node/4d3e/split/key/"));

  target.setSegmentationType(ZDvidData::EType::LABELMAP);
  std::cout << target.getSegmentationName() << std::endl;
  dvidUrl4.setDvidTarget(target, "3456");
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/test/label/123",
            dvidUrl4.getAnnotationUrl("test", 123));
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/labelstest/size/123",
            dvidUrl4.getBodySizeUrl(123));

  target.setBodyLabelName("segmentation");
  target.setUuid("3456");
  dvidUrl4.setDvidTarget(target);
  ASSERT_EQ("http://emdata.janelia.org/api/node/3456/segmentation_sv_meshes/supervoxel/123",
            dvidUrl4.getSupervoxelMeshUrl(123));

}

TEST(ZDvidUrl, Mesh)
{
  ASSERT_EQ("1.obj", ZDvidUrl::GetMeshKey(1, ZDvidUrl::EMeshType::DEFAULT));
  ASSERT_EQ("1.ngmesh", ZDvidUrl::GetMeshKey(1, ZDvidUrl::EMeshType::NG));
  ASSERT_EQ("1.merge", ZDvidUrl::GetMeshKey(1, ZDvidUrl::EMeshType::MERGED));
  ASSERT_EQ("1.drc", ZDvidUrl::GetMeshKey(1, ZDvidUrl::EMeshType::DRACO));

  ASSERT_EQ("1_2.obj", ZDvidUrl::GetMeshKey(1, 2, ZDvidUrl::EMeshType::DEFAULT));
  ASSERT_EQ("1_2.ngmesh", ZDvidUrl::GetMeshKey(1, 2, ZDvidUrl::EMeshType::NG));
  ASSERT_EQ("", ZDvidUrl::GetMeshKey(1, 2, ZDvidUrl::EMeshType::MERGED));
  ASSERT_EQ("1_2.drc", ZDvidUrl::GetMeshKey(1, 2, ZDvidUrl::EMeshType::DRACO));
}


#endif

#endif // ZDVIDURLTEST_H
