#ifndef ZFLYEMDATALOADER_H
#define ZFLYEMDATALOADER_H

#include <QObject>
#include <QString>

class ZFlyEmDataBundle;
class ZDvidFilter;
class MainWindow;

class ZFlyEmDataLoader : public QObject
{
  Q_OBJECT
public:
  explicit ZFlyEmDataLoader(QObject *parent = 0);

  MainWindow* getMainWindow();
  void loadDataBundle(const ZDvidFilter &filter);

signals:
  void progressStarted();
  void progressEnded();
  void progressAdvanced(double dp);

  void dataReady(ZFlyEmDataBundle *data, const QString &source);
  void loadFailed(const QString &source);

public slots:
  void createFrame(ZFlyEmDataBundle *data, const QString &source);
  void processFailure(const QString &source);

private:
  void loadDataBundleFunc(const ZDvidFilter &filter);
  void connectSignalSlot();
};

#endif // ZFLYEMDATALOADER_H
