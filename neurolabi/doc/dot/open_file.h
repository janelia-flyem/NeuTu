connect ZStackDoc, locsegChainSelected, ZStackFrame, setLocsegChainInfo;
connect ZStackDoc, SIGNAL(stackLoaded()), ZStackFrame, SLOT(stackLoaded());
connect ZStackFrame, SIGNAL(stackLoaded()), ZStackFrame, SLOT(setupDisplay());
connect ZStackDoc, SIGNAL(stackModified()), ZStackView, SLOT(updateChannelControl());
connect ZStackDoc, SIGNAL(stackModified()), ZStackView, SLOT(updateThresholdSlider());
connect ZStackDoc, SIGNAL(stackModified()), ZStackView, SLOT(updateSlider();
connect ZStackDoc, SIGNAL(stackModified()), ZStackPresenter, SLOT(updateStackBc();
connect ZStackDoc, SIGNAL(stackModified()), ZStackView, SLOT(updateView();

connect ZStackDoc, SIGNAL(objectModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(chainModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(swcModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(punctaModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(obj3dModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(strokeModified()), ZStackView, SLOT(paintObject();
connect ZStackDoc, cleanChanged(), ZStackFrame, SLOT(changeWindowTitle();
connect ZStackDoc, SIGNAL(holdSegChanged()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(chainSelectionChanged()), ZStackView, SLOT((paintObject);
connect ZStackDoc, SIGNAL(swcTreeNodeSelectionChanged()), ZStackView, SLOT(paintObject();
connect ZStackDoc, SIGNAL(punctaSelectionChanged()), ZStackView, SLOT(paintObject();
  connect ZStackDoc, SIGNAL(chainVisibleStateChanged),
                ZStackView, SLOT(paintObject();
  connect ZStackDoc, SIGNAL(swcVisibleStateChanged(),
                ZStackView, SLOT(paintObject();
  connect ZStackDoc, SIGNAL(punctumVisibleStateChanged(),
                ZStackView, SLOT(paintObject();
  connect ZStackView, SIGNAL(currentSliceChanged()),
                ZStackPresenter, SLOT(processSliceChangeEvent();
cmp ZStackFrame::ZStackPresenter;
cmp ZStackFrame::ZStackDoc;
cmp ZStackFrame::ZStackView;

function ZStackFrame::@ready

