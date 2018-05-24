class QMainWindow;
class ZStackMvc;
class ZProofreadWindow:QMainWindow;
class ZFlyEmProofMvc:ZStackMvc;
class ZFlyEmProofDoc:ZStackDoc;
cmp ZProofreadWindow::ZFlyEmProofMvc;
cmp ZProofreadWindow::ZFlyEmMessageWidget;  
cmp ZProofreadWindow::QStackedWidget;
cmp QStackedWidget::FlyEmProofControlForm;
cmp QStackedWidget::FlyEmSplitControlForm;
cmp ZFlyEmProofMvc::ZFlyEmBodySplitProject;
cmp ZFlyEmProofMvc::ZFlyEmBodyMergeProject;
cmp ZFlyEmProofMvc::ZDvidDialog;
cmp ZStackMvc::ZStackPresenter;
cmp ZStackMvc::ZStackView;
cmp ZStackMvc::ZStackDoc;
cmp ZFlyEmProofMvc::ZFlyEmProofDoc;
cmp ZFlyEmProofDoc::ZFlyEmBodyMerger;
cmp ZFlyEmProofDoc::ZDvidTarget;
cmp ZFlyEmProofDoc::ZDvidTileEnsemble;
cmp ZFlyEmProofDoc::ZDvidLabelSlice;
cmp ZFlyEmProofDoc::ZDvidSparseStack;

connect ZStackPresenter::bodySplitTriggered, ZFlyEmProofMvc::notifySplitTriggered;
connect ZFlyEmBodySplitProject::locating2DViewTriggered, ZStackView::setView;
connect ZStackDoc::messageGenerated, ZFlyEmProofMvc::messageGenerated;
connect ZFlyEmProofMvc::splitBodyLoaded, ZFlyEmProofMvc::presentBodySplit;
connect FlyEmProofControlForm::segmentVisibleChanged, ZFlyEmProofMvc::setSegmentationVisible;
connect FlyEmProofControlForm::mergingSelected, ZFlyEmProofMvc::mergeSelected;
connect FlyEmProofControlForm::edgeModeToggled, ZFlyEmProofMvc::toggleEdgeMode;
connect FlyEmProofControlForm::dvidSetTriggered, ZFlyEmProofMvc::setDvidTarget;
connect ZFlyEmProofMvc::launchingSplit, FlyEmProofControlForm::splitTriggered;
connect FlyEmSplitControlForm::quickViewTriggered, ZFlyEmProofMvc::showBodyQuickView;
connect FlyEmSplitControlForm::exitingSplit, ZFlyEmProofMvc::exitSplit;



