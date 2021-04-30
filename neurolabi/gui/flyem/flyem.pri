include(dialogs/dialogs.pri)
include(widgets/widgets.pri)
include(neuroglancer/neuroglancer.pri)
include(roi/roi.pri)
include(tests/tests.pri)

FORMS += \
    $$PWD/auth/flyemauthtokendialog.ui \
    $$PWD/flyemdataframeoptiondialog.ui \
    $$PWD/flyemorthocontrolform.ui \
    $$PWD/flyemproofcontrolform.ui \
    $$PWD/flyemsplitcontrolform.ui \
    $$PWD/zflyembookmarkannotationdialog.ui \
    $$PWD/zflyembookmarkfilter.ui \
    $$PWD/zflyemhackathonconfigdlg.ui

HEADERS += \
    $$PWD/auth/flyemauthserverclient.h \
    $$PWD/auth/flyemauthtokendialog.h \
    $$PWD/auth/flyemauthtokenhandler.h \
    $$PWD/auth/flyemauthtokenstorage.h \
    $$PWD/flyemdataframeoptiondialog.h \
    $$PWD/flyemdef.h \
    $$PWD/flyemorthocontrolform.h \
    $$PWD/flyempointannotationensemble.hpp \
    $$PWD/flyemproofcontrolform.h \
    $$PWD/flyemsplitcontrolform.h \
    $$PWD/flyemsynapseblockgrid.h \
    $$PWD/flyemsynapsechunk.h \
    $$PWD/flyemsynapsedvidsource.h \
    $$PWD/flyemsynapseensemble.h \
    $$PWD/flyemsynapsesource.h \
    $$PWD/flyemtodoblockgrid.h \
    $$PWD/flyemtodochunk.h \
    $$PWD/flyemtododvidsource.h \
    $$PWD/flyemtodoensemble.h \
    $$PWD/flyemtodomocksource.h \
    $$PWD/flyemtodosource.h \
    $$PWD/zarbslicescrollstrategy.h \
    $$PWD/zbcfset.h \
    $$PWD/zdvidgrayslicehighrestask.h \
    $$PWD/zdvidlabelslicehighrestask.h \
    $$PWD/zdvidtileupdatetaskmanager.h \
    $$PWD/zflyemarbdoc.h \
    $$PWD/zflyemarbmvc.h \
    $$PWD/zflyemarbpresenter.h \
    $$PWD/zflyembody3ddoc.h \
    $$PWD/zflyembody3ddoccommand.h \
    $$PWD/zflyembody3ddockeyprocessor.h \
    $$PWD/zflyembody3ddocmenufactory.h \
    $$PWD/zflyembodyannotationprotocol.h \
    $$PWD/zflyembodycoloroption.h \
    $$PWD/zflyembodycolorscheme.h \
    $$PWD/zflyembodyconfig.h \
    $$PWD/zflyembodyenv.h \
    $$PWD/zflyembodyevent.h \
    $$PWD/zflyembodyidcolorscheme.h \
    $$PWD/zflyembodyideditor.h \
    $$PWD/zflyembodylistdelegate.h \
    $$PWD/zflyembodylistmodel.h \
    $$PWD/zflyembodylistview.h \
    $$PWD/zflyembodymanager.h \
    $$PWD/zflyembodymergedoc.h \
    $$PWD/zflyembodymergeframe.h \
    $$PWD/zflyembodymergeproject.h \
    $$PWD/zflyembodymerger.h \
    $$PWD/zflyembodysplitproject.h \
    $$PWD/zflyembodysplitter.h \
    $$PWD/zflyembodystateaccessor.h \
    $$PWD/zflyembodywindowfactory.h \
    $$PWD/zflyembookmark.h \
    $$PWD/zflyembookmarkannotationdialog.h \
    $$PWD/zflyembookmarkarray.h \
    $$PWD/zflyembookmarkfilter.h \
    $$PWD/zflyembookmarklistmodel.h \
    $$PWD/zflyembookmarkpresenter.h \
    $$PWD/zflyembookmarkptrarray.h \
    $$PWD/zflyemcompositebodycolorscheme.h \
    $$PWD/zflyemdatabundle.h \
    $$PWD/zflyemdataframe.h \
    $$PWD/zflyemdoc3dbodystateaccessor.h \
    $$PWD/zflyemexternalneurondoc.h \
    $$PWD/zflyemgeneralbodycolorscheme.h \
    $$PWD/zflyemhackathonconfigdlg.h \
    $$PWD/zflyemkeyoperationconfig.h \
    $$PWD/zflyemmb6analyzer.h \
    $$PWD/zflyemmeshfactory.h \
    $$PWD/zflyemmessagewidget.h \
    $$PWD/zflyemmisc.h \
    $$PWD/zflyemnamebodycolorscheme.h \
    $$PWD/zflyemneuronexporter.h \
    $$PWD/zflyemneuronfeatureanalyzer.h \
    $$PWD/zflyemneuronfiltertaskmanager.h \
    $$PWD/zflyemneuronimagefactory.h \
    $$PWD/zflyemneuronlistmodel.h \
    $$PWD/zflyemneuronmatchtaskmanager.h \
    $$PWD/zflyemneuronpresenter.h \
    $$PWD/zflyemorthodoc.h \
    $$PWD/zflyemorthomvc.h \
    $$PWD/zflyemorthoviewhelper.h \
    $$PWD/zflyemorthowidget.h \
    $$PWD/zflyemorthowindow.h \
    $$PWD/zflyemproofdoc.h \
    $$PWD/zflyemproofdoccommand.h \
    $$PWD/zflyemproofdocmenufactory.h \
    $$PWD/zflyemproofmvc.h \
    $$PWD/zflyemproofmvccontroller.h \
    $$PWD/zflyemproofpresenter.h \
    $$PWD/zflyemproofutil.h \
    $$PWD/zflyemqualityanalyzertask.h \
    $$PWD/zflyemqualityanalyzertaskmanager.h \
    $$PWD/zflyemrandombodycolorscheme.h \
    $$PWD/zflyemroimanager.h \
    $$PWD/zflyemroiobjsmodel.h \
    $$PWD/zflyemroiproject.h \
    $$PWD/zflyemroutinechecktask.h \
    $$PWD/zflyemsequencercolorscheme.h \
    $$PWD/zflyemservice.h \
    $$PWD/zflyemstackdoc.h \
    $$PWD/zflyemstackframe.h \
    $$PWD/zflyemsupervisor.h \
    $$PWD/zflyemsynapsedatafetcher.h \
    $$PWD/zflyemsynapsedataupdater.h \
    $$PWD/zflyemtaskhelper.h \
    $$PWD/zflyemtododelegate.h \
    $$PWD/zflyemtodoitem.h \
    $$PWD/zflyemtodolist.h \
    $$PWD/zflyemtodolistfilter.h \
    $$PWD/zflyemtodolistmodel.h \
    $$PWD/zflyemtodopresenter.h \
    $$PWD/zglobaldvidrepo.h \
    $$PWD/zintcuboidcomposition.h \
    $$PWD/zinteractionengine.h \
    $$PWD/zkeyeventbodymapper.h \
    $$PWD/zmainwindowcontroller.h \
    $$PWD/zneutuservice.h \
    $$PWD/zpaintlabelwidget.h \
    $$PWD/zproofreadwindow.h \
    $$PWD/zserviceconsumer.h \
    $$PWD/zskeletonizeservice.h \
    $$PWD/zstackwatershedcontainer.h \
    $$PWD/zswctreebatchmatcher.h \
    $$PWD/logging.h \
    $$PWD/flyemmvcdialogmanager.h \
    $$PWD/flyemdataconfig.h \
    $$PWD/zflyemproofdochelper.h \
    $$PWD/flyembodystatusprotocol.h \
    $$PWD/flyemdatareader.h \
    $$PWD/flyemdatawriter.h \
    $$PWD/zflyembody3ddochelper.h \
    $$PWD/zflyemproofdocutil.h \
    $$PWD/zdvidlabelslicehighrestaskfactory.h \
    $$PWD/flyembodyselectionmanager.h \
    $$PWD/zflyemproofdockeyprocessor.h \
    $$PWD/zflyemproofdoctracinghelper.h

SOURCES += \
    $$PWD/auth/flyemauthserverclient.cpp \
    $$PWD/auth/flyemauthtokendialog.cpp \
    $$PWD/auth/flyemauthtokenhandler.cpp \
    $$PWD/auth/flyemauthtokenstorage.cpp \
    $$PWD/flyemdataframeoptiondialog.cpp \
    $$PWD/flyemorthocontrolform.cpp \
    $$PWD/flyemproofcontrolform.cpp \
    $$PWD/flyemsplitcontrolform.cpp \
    $$PWD/flyemsynapseblockgrid.cpp \
    $$PWD/flyemsynapsechunk.cpp \
    $$PWD/flyemsynapsedvidsource.cpp \
    $$PWD/flyemsynapseensemble.cpp \
    $$PWD/flyemsynapsesource.cpp \
    $$PWD/flyemtodoblockgrid.cpp \
    $$PWD/flyemtodochunk.cpp \
    $$PWD/flyemtododvidsource.cpp \
    $$PWD/flyemtodoensemble.cpp \
    $$PWD/flyemtodomocksource.cpp \
    $$PWD/flyemtodosource.cpp \
    $$PWD/zarbslicescrollstrategy.cpp \
    $$PWD/zbcfset.cpp \
    $$PWD/zdvidgrayslicehighrestask.cpp \
    $$PWD/zdvidlabelslicehighrestask.cpp \
    $$PWD/zdvidtileupdatetaskmanager.cpp \
    $$PWD/zflyemarbdoc.cpp \
    $$PWD/zflyemarbmvc.cpp \
    $$PWD/zflyemarbpresenter.cpp \
    $$PWD/zflyembody3ddoc.cpp \
    $$PWD/zflyembody3ddoccommand.cpp \
    $$PWD/zflyembody3ddockeyprocessor.cpp \
    $$PWD/zflyembody3ddocmenufactory.cpp \
    $$PWD/zflyembodyannotationprotocol.cpp \
    $$PWD/zflyembodycoloroption.cpp \
    $$PWD/zflyembodycolorscheme.cpp \
    $$PWD/zflyembodyconfig.cpp \
    $$PWD/zflyembodyenv.cpp \
    $$PWD/zflyembodyevent.cpp \
    $$PWD/zflyembodyidcolorscheme.cpp \
    $$PWD/zflyembodyideditor.cpp \
    $$PWD/zflyembodylistdelegate.cpp \
    $$PWD/zflyembodylistmodel.cpp \
    $$PWD/zflyembodylistview.cpp \
    $$PWD/zflyembodymanager.cpp \
    $$PWD/zflyembodymergedoc.cpp \
    $$PWD/zflyembodymergeframe.cpp \
    $$PWD/zflyembodymergeproject.cpp \
    $$PWD/zflyembodymerger.cpp \
    $$PWD/zflyembodysplitproject.cpp \
    $$PWD/zflyembodysplitter.cpp \
    $$PWD/zflyembodystateaccessor.cpp \
    $$PWD/zflyembodywindowfactory.cpp \
    $$PWD/zflyembookmark.cpp \
    $$PWD/zflyembookmarkannotationdialog.cpp \
    $$PWD/zflyembookmarkarray.cpp \
    $$PWD/zflyembookmarkfilter.cpp \
    $$PWD/zflyembookmarklistmodel.cpp \
    $$PWD/zflyembookmarkpresenter.cpp \
    $$PWD/zflyembookmarkptrarray.cpp \
    $$PWD/zflyemcompositebodycolorscheme.cpp \
    $$PWD/zflyemdatabundle.cpp \
    $$PWD/zflyemdataframe.cpp \
    $$PWD/zflyemdoc3dbodystateaccessor.cpp \
    $$PWD/zflyemexternalneurondoc.cpp \
    $$PWD/zflyemgeneralbodycolorscheme.cpp \
    $$PWD/zflyemhackathonconfigdlg.cpp \
    $$PWD/zflyemkeyoperationconfig.cpp \
    $$PWD/zflyemmb6analyzer.cpp \
    $$PWD/zflyemmeshfactory.cpp \
    $$PWD/zflyemmessagewidget.cpp \
    $$PWD/zflyemmisc.cpp \
    $$PWD/zflyemnamebodycolorscheme.cpp \
    $$PWD/zflyemneuronexporter.cpp \
    $$PWD/zflyemneuronfeatureanalyzer.cpp \
    $$PWD/zflyemneuronfiltertaskmanager.cpp \
    $$PWD/zflyemneuronimagefactory.cpp \
    $$PWD/zflyemneuronlistmodel.cpp \
    $$PWD/zflyemneuronmatchtaskmanager.cpp \
    $$PWD/zflyemneuronpresenter.cpp \
    $$PWD/zflyemorthodoc.cpp \
    $$PWD/zflyemorthomvc.cpp \
    $$PWD/zflyemorthoviewhelper.cpp \
    $$PWD/zflyemorthowidget.cpp \
    $$PWD/zflyemorthowindow.cpp \
    $$PWD/zflyemproofdoc.cpp \
    $$PWD/zflyemproofdoccommand.cpp \
    $$PWD/zflyemproofdocmenufactory.cpp \
    $$PWD/zflyemproofmvc.cpp \
    $$PWD/zflyemproofmvccontroller.cpp \
    $$PWD/zflyemproofpresenter.cpp \
    $$PWD/zflyemproofutil.cpp \
    $$PWD/zflyemqualityanalyzertask.cpp \
    $$PWD/zflyemqualityanalyzertaskmanager.cpp \
    $$PWD/zflyemrandombodycolorscheme.cpp \
    $$PWD/zflyemroimanager.cpp \
    $$PWD/zflyemroiobjsmodel.cpp \
    $$PWD/zflyemroiproject.cpp \
    $$PWD/zflyemroutinechecktask.cpp \
    $$PWD/zflyemsequencercolorscheme.cpp \
    $$PWD/zflyemservice.cpp \
    $$PWD/zflyemstackdoc.cpp \
    $$PWD/zflyemstackframe.cpp \
    $$PWD/zflyemsupervisor.cpp \
    $$PWD/zflyemsynapsedatafetcher.cpp \
    $$PWD/zflyemsynapsedataupdater.cpp \
    $$PWD/zflyemtaskhelper.cpp \
    $$PWD/zflyemtododelegate.cpp \
    $$PWD/zflyemtodoitem.cpp \
    $$PWD/zflyemtodolist.cpp \
    $$PWD/zflyemtodolistfilter.cpp \
    $$PWD/zflyemtodolistmodel.cpp \
    $$PWD/zflyemtodopresenter.cpp \
    $$PWD/zglobaldvidrepo.cpp \
    $$PWD/zintcuboidcomposition.cpp \
    $$PWD/zinteractionengine.cpp \
    $$PWD/zkeyeventbodymapper.cpp \
    $$PWD/zmainwindowcontroller.cpp \
    $$PWD/zneutuservice.cpp \
    $$PWD/zpaintlabelwidget.cpp \
    $$PWD/zproofreadwindow.cpp \
    $$PWD/zserviceconsumer.cpp \
    $$PWD/zskeletonizeservice.cpp \
    $$PWD/zstackwatershedcontainer.cpp \
    $$PWD/zswctreebatchmatcher.cpp \
    $$PWD/flyemmvcdialogmanager.cpp \
    $$PWD/flyemdataconfig.cpp \
    $$PWD/zflyemproofdochelper.cpp \
    $$PWD/flyembodystatusprotocol.cpp \
    $$PWD/logging.cpp \
    $$PWD/flyemdatareader.cpp \
    $$PWD/flyemdatawriter.cpp \
    $$PWD/zflyembody3ddochelper.cpp \
    $$PWD/zflyemproofdocutil.cpp \
    $$PWD/zdvidlabelslicehighrestaskfactory.cpp \
    $$PWD/flyembodyselectionmanager.cpp \
    $$PWD/zflyemproofdockeyprocessor.cpp \
    $$PWD/zflyemproofdoctracinghelper.cpp
