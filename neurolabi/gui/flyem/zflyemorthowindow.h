#ifndef ZFLYEMORTHOWINDOW_H
#define ZFLYEMORTHOWINDOW_H

#include <QMainWindow>

class ZDvidTarget;
class ZFlyEmOrthoWidget;
class ZIntPoint;
class ZFlyEmOrthoDoc;
class ZFlyEmProofDoc;
class ZWidgetMessage;

class ZFlyEmOrthoWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWindow(const ZDvidTarget &target, QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;
  void copyBookmarkFrom(ZFlyEmProofDoc *doc);

signals:
  void bookmarkEdited(int x, int y, int z);
  void synapseEdited(int x, int y, int z);
  void zoomingTo(int x, int y, int z);
  void bodyMergeEdited();

public slots:
  void updateData(const ZIntPoint &center);
  void downloadBookmark(int x, int y, int z);
  void downloadSynapse(int x, int y, int z);
  void syncMergeWithDvid();
  void processMessage(const ZWidgetMessage &message);

private:
  ZFlyEmOrthoWidget *m_orthoWidget;
};

#endif // ZFLYEMORTHOWINDOW_H
