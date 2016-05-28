include($${PWD}/json.pri)
include($${PWD}/mylib/mylib.pri)

HEADERS +=$${PWD}/zstackprocessor.h \
   $${PWD}/zstack.hxx \
   $${PWD}/zlocalneuroseg.h \
   $${PWD}/zellipse.h \
   $${PWD}/zswctree.h \
   $${PWD}/zobject3d.h \
   $${PWD}/zcircle.h \
   $${PWD}/zpoint.h \
   $${PWD}/zdirectionaltemplate.h \
   $${PWD}/zlocalrect.h \
   $${PWD}/zsinglechannelstack.h \
   $${PWD}/zbenchtimer.h \
   $${PWD}/zspgrowparser.h \
   $${PWD}/zvoxel.h \
   $${PWD}/zvoxelarray.h \
   $${PWD}/zsuperpixelmap.h \
   $${PWD}/zsegmentmap.h \
   $${PWD}/zsuperpixelmaparray.h \
   $${PWD}/zsegmentmaparray.h \
   $${PWD}/zswcforest.h \
   $${PWD}/zqtheader.h \
   $${PWD}/zstackaccessor.h \
   $${PWD}/zswctreematcher.h \
   $${PWD}/zswcbranch.h \
   $${PWD}/zmatrix.h \
   $${PWD}/zinttree.h \
   $${PWD}/zstring.h \
   $${PWD}/zfilelist.h \
   $${PWD}/zrandomgenerator.h \
   $${PWD}/zfilelock.h \
   $${PWD}/zargumentprocessor.h \
#   $${PWD}/flyem/zsynapseannotation.h \
#   $${PWD}/flyem/zsynapseannotationarray.h \
#   $${PWD}/flyem/zsynapseannotationmetadata.h \
#   $${PWD}/flyem/zsynapseannotationanalyzer.h \
   $${PWD}/flyem/zfileparser.h \
#   $${PWD}/flyem/zneuronnetwork.h \
   $${PWD}/zvaa3dmarker.h \
   $${PWD}/zvaa3dapo.h \
   $${PWD}/zswcnode.h \
   $${PWD}/zball.h \
   $${PWD}/ztreenode.h \
   $${PWD}/ztree.h \
   $${PWD}/zswctreenode.h \
   $${PWD}/zsquarematrix.h \
   $${PWD}/zsymmetricmatrix.h \
   $${PWD}/zsvggenerator.h \
   $${PWD}/zdendrogram.h \
   $${PWD}/zcuboid.h \
   $${PWD}/ztest.h \
   $${PWD}/zswcfeatureanalyzer.h \
   $${PWD}/zswcsizefeatureanalyzer.h \
   $${PWD}/zobject3darray.h \
   $${PWD}/zdoublevector.h \
   $${PWD}/zkmeansclustering.h \
   $${PWD}/zswcshollfeatureanalyzer.h \
   $${PWD}/zswcspatialfeatureanalyzer.h \
   $${PWD}/swctreenode.h \
   $${PWD}/zswcnetwork.h \
   $${PWD}/zcloudnetwork.h \
   $${PWD}/zweightedpoint.h \
   $${PWD}/zweightedpointarray.h \
   $${PWD}/zpointnetwork.h \
   $${PWD}/zswcpath.h \
   $${PWD}/zswctrunkanalyzer.h \
   $${PWD}/zswcdisttrunkanalyzer.h \
   $${PWD}/zswcbranchingtrunkanalyzer.h \
   $${PWD}/zswctrunksizefeatureanalyzer.h \
   $${PWD}/zpointarray.h \
#   $${PWD}/flyem/zsynapselocationmatcher.h \
   $${PWD}/c_stack.h \
   $${PWD}/zfiletype.h \
#   $${PWD}/flyem/zsynapselocationmetric.h \
   $${PWD}/zstackfile.h \
   $${PWD}/zxmldoc.h \
   $${PWD}/zintmap.h \
#   $${PWD}/flyem/zsegmentationanalyzer.h \
#   $${PWD}/flyem/zsegmentationbundle.h \
   $${PWD}/zstackblender.h \
   $${PWD}/zgraph.h \
   $${PWD}/zarray.h \
   $${PWD}/zintpairmap.h \
   $${PWD}/zswcsizetrunkanalyzer.h \
   $${PWD}/zswcweighttrunkanalyzer.h \
   $${PWD}/zdebug.h \
   $${PWD}/tubemodel.h \
   $${PWD}/zhdf5reader.h \
   $${PWD}/zhdf5_header.h \
   $${PWD}/zopencv_header.h \
   $${PWD}/neutubeconfig.h \
   $${PWD}/zhdf5writer.h \
#   $${PWD}/flyem/zbcfset.h \
   $${PWD}/zstackskeletonizer.h \
   $${PWD}/zswclayerfeatureanalyzer.h \
#   $${PWD}/flyem/zflyemneuron.h \
   $${PWD}/zswctypetrunkanalyzer.h \
   $${PWD}/zobject3dscan.h \
   $${PWD}/zswclayershollfeatureanalyzer.h \
   $${PWD}/zswclayertrunkanalyzer.h \
   $${PWD}/zlogmessagereporter.h \
   $${PWD}/zstackgraph.h\
   $${PWD}/zgraphcompressor.h \
   $${PWD}/zprogressreporter.h \
   $${PWD}/zstackdoccommand.h \
   $${PWD}/zcursorstore.h \
   $${PWD}/zmessagereporter.h \
   $${PWD}/zreportable.h \
   $${PWD}/zstackstatistics.h \
   $${PWD}/zswcconnector.h \
   $${PWD}/zprogressable.h \
   $${PWD}/zuncopyable.h \
   $${PWD}/biocytin/zstackprojector.h \
   $${PWD}/biocytin/swcprocessor.h \
   $${PWD}/zresolution.h \
   $${PWD}/zswcpositionadjuster.h \
   $${PWD}/zswcrangeanalyzer.h \
   $${PWD}/zellipsoid.h \
   $${PWD}/zswcnodeselector.h \
   $${PWD}/zswcnodezrangeselector.h \
   $${PWD}/zswcnodecompositeselector.h \
   $${PWD}/zswcnodeellipsoidrangeselector.h \
#   $${PWD}/flyem/zstitchgrid.h \
   $${PWD}/zintcuboidarray.h \
#   $${PWD}/flyem/zflyemqualityanalyzer.h \
#   $${PWD}/flyem/zintcuboidcomposition.h \
   $${PWD}/zswcglobalfeatureanalyzer.h \
   $${PWD}/zswclocationanalyzer.h \
   $${PWD}/biocytin/zbiocytinfilenameparser.h \
   $${PWD}/zerror.h \
   $${PWD}/zhistogram.h \
   $${PWD}/misc/miscutility.h \
#   $${PWD}/flyem/zflyemneuronrange.h \
#   $${PWD}/flyem/zflyemneuronaxis.h \
   $${PWD}/zswcgenerator.h \
   $${PWD}/zpaintbundle.h \
   $${PWD}/zswcnodebufferfeatureanalyzer.h \
#   $${PWD}/flyem/zflyemneuronrangecompare.h \
   $${PWD}/swc/zswcresampler.h \
   $${PWD}/swc/zswcmetric.h \
   $${PWD}/swc/zswcterminalsurfacemetric.h \
   $${PWD}/zapclustering.h \
   $${PWD}/swc/zswcterminalanglemetric.h \
   $${PWD}/swc/zswcdeepanglemetric.h \
   $${PWD}/zswcsubtreefeatureanalyzer.h \
   $${PWD}/swc/zswcsubtreeanalyzer.h \
   $${PWD}/zswctreenodearray.h \
#   $${PWD}/flyem/zflyembodyanalyzer.h \
   $${PWD}/swc/zswcnodedistselector.h \
   $${PWD}/zstackbinarizer.h \
   $${PWD}/zvectorgenerator.h \
   $${PWD}/zstackfactory.h \
   $${PWD}/zeigensolver.h \
   $${PWD}/zmapgenerator.h \
#   $${PWD}/flyem/zflyemneuronfeatureset.h \
#   $${PWD}/flyem/zflyemneuronfilter.h \
#   $${PWD}/flyem/zflyemneuronfilterfactory.h \
#   $${PWD}/flyem/zflyemneuronfeature.h \
#   $${PWD}/flyem/zflyemneuronfeaturefactory.h \
#   $${PWD}/flyem/zflyemneuronarray.h \
   $${PWD}/swc/zswcpruner.h \
   $${PWD}/zneurontracer.h \
#   $${PWD}/flyem/zflyemneuronlayermatcher.h \
   $${PWD}/zdynamicprogrammer.h \
#   $${PWD}/flyem/zhotspot.h \
   $${PWD}/ztextlinearray.h \
#   $${PWD}/flyem/zhotspotarray.h \
#   $${PWD}/flyem/zhotspotfactory.h \
   $${PWD}/ztextlinecompositer.h \
   $${PWD}/zobject3dscanarray.h \
   $${PWD}/zstringarray.h \
#   $${PWD}/flyem/zflyemcoordinateconverter.h \
#   $${PWD}/flyem/zflyem.h \
#   $${PWD}/flyem/zflyemdatainfo.h \
#   $${PWD}/dvid/zdvidinfo.h \
   $${PWD}/zlinesegment.h \
   $${PWD}/zlinesegmentarray.h \
#   $${PWD}/dvid/zdvidtarget.h \
#   $${PWD}/dvid/zdvidfilter.h \
   $${PWD}/zintcuboidface.h \
#   $${PWD}/zflyembodyannotation.h \
   $${PWD}/zintpoint.h \
   $${PWD}/zintpointarray.h \
   $${PWD}/zintarray.h \
   $${PWD}/zintset.h \
#   $${PWD}/flyem/zflyemsubstackroi.h \
   $${PWD}/zstackwatershed.h \
   $${PWD}/zstackarray.h \
#   $${PWD}/flyem/zflyemconfig.h \
   $${PWD}/tr1_header.h \
   $${PWD}/zsharedpointer.h \
   $${PWD}/geometry/zgeo3dtransform.h \
   $${PWD}/geometry/zgeometry.h \
   $${PWD}/geometry/zgeo3dscalarfield.h \
   $${PWD}/zvoxelgraphics.h \
   $${PWD}/zintcuboid.h \
#   $${PWD}/bigdata/zdvidblockgrid.h \
#   $${PWD}/bigdata/zstackblockgrid.h \
   $${PWD}/bigdata/zblockgrid.h \
   $${PWD}/bigdata/zblockgridfactory.h \
   $${PWD}/zsparsestack.h \
   $${PWD}/zstackobject.h \
   $${PWD}/zobject3dfactory.h \
   $${PWD}/zclosedcurve.h \
#   $${PWD}/zflyemutilities.h \
#   $${PWD}/dvid/zdvidurl.h \
   $${PWD}/zstackobjectsourcefactory.h \
#   $${PWD}/dvid/zdviddata.h \
#   $${PWD}/flyem/zflyemneuronbodyinfo.h \
   $${PWD}/zstackobjectsource.h \
   $${PWD}/zstackobjectrole.h \
   $${PWD}/zarrayfactory.h \
   $${PWD}/zstackobjectselector.h \
   $${PWD}/zinthistogram.h \
#   $${PWD}/flyem/zflyemneuroninfo.h \
   $${PWD}/zqtheader_undef.h \
   $${PWD}/zscalablestack.h \
#   $${PWD}/flyem/zflyemneurondensity.h \
#   $${PWD}/flyem/zflyemneurondensitymatcher.h \
   $${PWD}/zdag.h \
   $${PWD}/zselector.h \
   $${PWD}/zobject3dstripe.h \
   $${PWD}/jneurontracer.h \
   $${PWD}/neutube_def.h \
#   $${PWD}/flyem/zflyembodyannotationbundle.h \
   $${PWD}/zswcfactory.h \
   $${PWD}/swc/zswcradiusestimator.h \
   $${PWD}/zswctreenodeselector.h \
   $${PWD}/swc/zswcsignalfitter.h \
   $${PWD}/zneurontracerconfig.h

SOURCES +=$${PWD}/zstackprocessor.cpp \
   $${PWD}/zstack.cxx \
   $${PWD}/zlocalneuroseg.cpp \
   $${PWD}/zellipse.cpp \
   $${PWD}/zswctree.cpp \
   $${PWD}/zobject3d.cpp \
   $${PWD}/zcircle.cpp \
   $${PWD}/zpoint.cpp \
   $${PWD}/zdirectionaltemplate.cpp \
   $${PWD}/zlocalrect.cpp \
   $${PWD}/zsinglechannelstack.cpp \
   $${PWD}/zspgrowparser.cpp \
   $${PWD}/zvoxel.cpp \
   $${PWD}/zvoxelarray.cpp \
   $${PWD}/zsuperpixelmap.cpp \
   $${PWD}/zsegmentmap.cpp \
   $${PWD}/zsuperpixelmaparray.cpp \
   $${PWD}/zsegmentmaparray.cpp \
   $${PWD}/zswcforest.cpp \
   $${PWD}/zstackaccessor.cpp \
   $${PWD}/zswctreematcher.cpp \
   $${PWD}/zswcbranch.cpp \
   $${PWD}/zmatrix.cpp \
   $${PWD}/zinttree.cpp \
   $${PWD}/zstring.cpp \
   $${PWD}/zfilelist.cpp \
   $${PWD}/zrandomgenerator.cpp \
   $${PWD}/zfilelock.cpp \
   $${PWD}/zargumentprocessor.cpp \
#   $${PWD}/flyem/zsynapseannotation.cpp \
#   $${PWD}/flyem/zsynapseannotationarray.cpp \
#   $${PWD}/flyem/zsynapseannotationmetadata.cpp \
#   $${PWD}/flyem/zsynapseannotationanalyzer.cpp \
   $${PWD}/flyem/zfileparser.cpp \
#   $${PWD}/flyem/zneuronnetwork.cpp \
   $${PWD}/zvaa3dmarker.cpp \
   $${PWD}/zvaa3dapo.cpp \
   $${PWD}/zswcnode.cpp \
   $${PWD}/zlogmessagereporter.cpp \
   $${PWD}/zball.cpp \
   $${PWD}/zswctreenode.cpp \
   $${PWD}/zsquarematrix.cpp \
   $${PWD}/zsymmetricmatrix.cpp \
   $${PWD}/zsvggenerator.cpp \
   $${PWD}/zdendrogram.cpp \
   $${PWD}/zcuboid.cpp \
   $${PWD}/zswcfeatureanalyzer.cpp \
   $${PWD}/zswcsizefeatureanalyzer.cpp \
   $${PWD}/zobject3darray.cpp \
   $${PWD}/zdoublevector.cpp \
   $${PWD}/zneurontracer.cpp \
   $${PWD}/zkmeansclustering.cpp \
   $${PWD}/zswcshollfeatureanalyzer.cpp \
   $${PWD}/zswcspatialfeatureanalyzer.cpp \
   $${PWD}/swctreenode.cpp \
   $${PWD}/zswcnetwork.cpp \
   $${PWD}/zcloudnetwork.cpp \
   $${PWD}/zweightedpoint.cpp \
   $${PWD}/zweightedpointarray.cpp \
   $${PWD}/zpointnetwork.cpp \
   $${PWD}/zswcpath.cpp \
   $${PWD}/zswctrunkanalyzer.cpp \
   $${PWD}/zswcdisttrunkanalyzer.cpp \
   $${PWD}/zswcbranchingtrunkanalyzer.cpp \
   $${PWD}/zswctrunksizefeatureanalyzer.cpp \
   $${PWD}/zpointarray.cpp \
#   $${PWD}/flyem/zsynapselocationmatcher.cpp \
   $${PWD}/c_stack.cpp \
   $${PWD}/zfiletype.cpp \
#   $${PWD}/flyem/zsynapselocationmetric.cpp \
   $${PWD}/zstackfile.cpp \
   $${PWD}/zxmldoc.cpp \
   $${PWD}/zintmap.cpp \
#   $${PWD}/flyem/zsegmentationanalyzer.cpp \
#   $${PWD}/flyem/zsegmentationbundle.cpp \
   $${PWD}/zstackblender.cpp \
   $${PWD}/zgraph.cpp \
   $${PWD}/zarray.cpp \
   $${PWD}/zintpairmap.cpp \
   $${PWD}/zswcsizetrunkanalyzer.cpp \
   $${PWD}/zswcweighttrunkanalyzer.cpp \
   $${PWD}/zdebug.cpp \
   $${PWD}/tubemodel.cpp \
   $${PWD}/zhdf5reader.cpp \
   $${PWD}/neutubeconfig.cpp \
   $${PWD}/zhdf5writer.cpp \
#   $${PWD}/flyem/zbcfset.cpp \
   $${PWD}/zstackskeletonizer.cpp \
   $${PWD}/zswclayerfeatureanalyzer.cpp \
#   $${PWD}/flyem/zflyemneuron.cpp \
   $${PWD}/zswctypetrunkanalyzer.cpp \
   $${PWD}/zobject3dscan.cpp \
   $${PWD}/zswclayershollfeatureanalyzer.cpp \
   $${PWD}/zswclayertrunkanalyzer.cpp \
   $${PWD}/zstackgraph.cpp \
   $${PWD}/zgraphcompressor.cpp \
   $${PWD}/zprogressreporter.cpp \
   $${PWD}/zmessagereporter.cpp \
   $${PWD}/zreportable.cpp \
   $${PWD}/zstackstatistics.cpp \
   $${PWD}/zswcconnector.cpp \
   $${PWD}/zprogressable.cpp \
   $${PWD}/biocytin/zstackprojector.cpp \
   $${PWD}/biocytin/swcprocessor.cpp \
   $${PWD}/zresolution.cpp \
   $${PWD}/zswcpositionadjuster.cpp \
   $${PWD}/zswcrangeanalyzer.cpp \
   $${PWD}/zellipsoid.cpp \
   $${PWD}/zswcnodeselector.cpp \
   $${PWD}/zswcnodezrangeselector.cpp \
   $${PWD}/zswcnodecompositeselector.cpp \
   $${PWD}/zswcnodeellipsoidrangeselector.cpp \
#   $${PWD}/flyem/zstitchgrid.cpp \
   $${PWD}/zintcuboidarray.cpp \
#   $${PWD}/flyem/zflyemqualityanalyzer.cpp \
#   $${PWD}/flyem/zintcuboidcomposition.cpp \
   $${PWD}/zswcglobalfeatureanalyzer.cpp \
   $${PWD}/zswclocationanalyzer.cpp \
   $${PWD}/biocytin/zbiocytinfilenameparser.cpp \
   $${PWD}/zhistogram.cpp \
   $${PWD}/misc/miscutility.cpp \
#   $${PWD}/flyem/zflyemneuronrange.cpp \
#   $${PWD}/flyem/zflyemneuronaxis.cpp \
   $${PWD}/zswcgenerator.cpp \
   $${PWD}/zswcnodebufferfeatureanalyzer.cpp \
#   $${PWD}/flyem/zflyemneuronrangecompare.cpp \
   $${PWD}/swc/zswcresampler.cpp \
   $${PWD}/swc/zswcmetric.cpp \
   $${PWD}/swc/zswcterminalsurfacemetric.cpp \
   $${PWD}/zapclustering.cpp \
   $${PWD}/swc/zswcterminalanglemetric.cpp \
   $${PWD}/swc/zswcdeepanglemetric.cpp \
   $${PWD}/zswcsubtreefeatureanalyzer.cpp \
   $${PWD}/swc/zswcsubtreeanalyzer.cpp \
   $${PWD}/zswctreenodearray.cpp \
#   $${PWD}/flyem/zflyembodyanalyzer.cpp \
   $${PWD}/swc/zswcnodedistselector.cpp \
   $${PWD}/zstackbinarizer.cpp \
   $${PWD}/zvectorgenerator.cpp \
   $${PWD}/zstackfactory.cpp \
   $${PWD}/zeigensolver.cpp \
   $${PWD}/zmapgenerator.cpp \
#   $${PWD}/flyem/zflyemneuronfeatureset.cpp \
#   $${PWD}/flyem/zflyemneuronfilter.cpp \
#   $${PWD}/flyem/zflyemneuronfilterfactory.cpp \
#   $${PWD}/flyem/zflyemneuronfeature.cpp \
#   $${PWD}/flyem/zflyemneuronfeaturefactory.cpp \
#   $${PWD}/flyem/zflyemneuronarray.cpp \
   $${PWD}/swc/zswcpruner.cpp \
#   $${PWD}/flyem/zflyemneuronlayermatcher.cpp \
   $${PWD}/zdynamicprogrammer.cpp \
#   $${PWD}/flyem/zhotspot.cpp \
   $${PWD}/ztextlinearray.cpp \
#   $${PWD}/flyem/zhotspotarray.cpp \
#   $${PWD}/flyem/zhotspotfactory.cpp \
   $${PWD}/ztextlinecompositer.cpp \
   $${PWD}/zobject3dscanarray.cpp \
   $${PWD}/zstringarray.cpp \
#   $${PWD}/flyem/zflyemcoordinateconverter.cpp \
#   $${PWD}/flyem/zflyemdatainfo.cpp \
#   $${PWD}/dvid/zdvidinfo.cpp \
   $${PWD}/zlinesegment.cpp \
   $${PWD}/zlinesegmentarray.cpp \
#   $${PWD}/dvid/zdvidtarget.cpp \
#   $${PWD}/dvid/zdvidfilter.cpp \
   $${PWD}/zintcuboidface.cpp \
#   $${PWD}/zflyembodyannotation.cpp \
   $${PWD}/zintpoint.cpp \
   $${PWD}/zintpointarray.cpp \
   $${PWD}/zintarray.cpp \
   $${PWD}/zintset.cpp \
#   $${PWD}/flyem/zflyemsubstackroi.cpp \
   $${PWD}/zstackwatershed.cpp \
   $${PWD}/zstackarray.cpp \
#   $${PWD}/flyem/zflyemconfig.cpp \
   $${PWD}/geometry/zgeo3dtransform.cpp \
   $${PWD}/geometry/zgeometry.cpp \
   $${PWD}/geometry/zgeo3dscalarfield.cpp \
   $${PWD}/zvoxelgraphics.cpp \
   $${PWD}/zintcuboid.cpp \
#   $${PWD}/bigdata/zdvidblockgrid.cpp \
   $${PWD}/bigdata/zstackblockgrid.cpp \
   $${PWD}/bigdata/zblockgrid.cpp \
   $${PWD}/bigdata/zblockgridfactory.cpp \
   $${PWD}/zsparsestack.cpp \
   $${PWD}/zstackobject.cpp \
   $${PWD}/zobject3dfactory.cpp \
   $${PWD}/zclosedcurve.cpp \
#   $${PWD}/zflyemutilities.cpp \
#   $${PWD}/dvid/zdvidurl.cpp \
   $${PWD}/zstackobjectsourcefactory.cpp \
#   $${PWD}/dvid/zdviddata.cpp \
#   $${PWD}/flyem/zflyemneuronbodyinfo.cpp \
   $${PWD}/zstackobjectsource.cpp \
   $${PWD}/zstackobjectrole.cpp \
   $${PWD}/zarrayfactory.cpp \
   $${PWD}/zstackobjectselector.cpp \
   $${PWD}/zinthistogram.cpp \
#   $${PWD}/flyem/zflyemneuroninfo.cpp \
   $${PWD}/zscalablestack.cpp \
#   $${PWD}/flyem/zflyemneurondensity.cpp \
#   $${PWD}/flyem/zflyemneurondensitymatcher.cpp \
   $${PWD}/zdag.cpp \
   $${PWD}/zobject3dstripe.cpp \
   $${PWD}/jneurontracer.cpp \
#   $${PWD}/flyem/zflyembodyannotationbundle.cpp \
   $${PWD}/zswcfactory.cpp \
   $${PWD}/swc/zswcradiusestimator.cpp \
   $${PWD}/zswctreenodeselector.cpp \
   $${PWD}/swc/zswcsignalfitter.cpp \
   $${PWD}/zneurontracerconfig.cpp

