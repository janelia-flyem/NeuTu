#include "zflyemdataloader.h"

#include <QtConcurrentRun>
#include <QProgressDialog>

#include "mainwindow.h"
#include "dvid/zdvidfilter.h"
#include "flyem/zflyemdatabundle.h"
#include "flyem/zflyemdataframe.h"
#include "dvid/zdvidtarget.h"

ZFlyEmDataLoader::ZFlyEmDataLoader(QObject *parent) :
  QObject(parent)
{
  connectSignalSlot();
}

void ZFlyEmDataLoader::connectSignalSlot()
{
  connect(this, SIGNAL(dataReady(ZFlyEmDataBundle*,QString)),
          this, SLOT(createFrame(ZFlyEmDataBundle*,QString)));

  connect(this, SIGNAL(progressStarted()),
          getMainWindow(), SLOT(startProgress()));
  connect(this, SIGNAL(progressEnded()),
          getMainWindow(), SLOT(endProgress()));
  connect(this, SIGNAL(progressAdvanced(double)),
          getMainWindow(), SLOT(advanceProgress(double)));
  connect(this, SIGNAL(loadFailed(QString)),
          this, SLOT(processFailure(QString)));
}

void ZFlyEmDataLoader::loadDataBundle(const ZDvidFilter &filter)
{
  QtConcurrent::run(this, &ZFlyEmDataLoader::loadDataBundleFunc, filter);
}

void ZFlyEmDataLoader::processFailure(const QString &source)
{
  if (getMainWindow() != NULL) {
    getMainWindow()->reportFileOpenProblem(source);
  }
}

void ZFlyEmDataLoader::loadDataBundleFunc(const ZDvidFilter &filter)
{
  emit progressStarted();

  ZFlyEmDataBundle *dataBundle = new ZFlyEmDataBundle;

  emit progressAdvanced(0.5);

  if (dataBundle->loadDvid(filter)) {
    emit dataReady(dataBundle, filter.getDvidTarget().getSourceString().c_str());
  } else {
    emit loadFailed(filter.getDvidTarget().getSourceString().c_str());
  }

  emit progressEnded();
}

MainWindow* ZFlyEmDataLoader::getMainWindow()
{
  return qobject_cast<MainWindow*>(parent());
}

void ZFlyEmDataLoader::createFrame(ZFlyEmDataBundle *data, const QString &source)
{
  if (getMainWindow() != NULL) {
    ZFlyEmDataFrame *frame = new ZFlyEmDataFrame;
    ZDvidTarget target;
    target.setFromSourceString(source.toStdString());
    frame->setDvidTarget(target);
    frame->addData(data);
    getMainWindow()->addFlyEmDataFrame(frame);
  } else {
    delete data;
  }
}
