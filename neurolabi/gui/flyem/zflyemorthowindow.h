#ifndef ZFLYEMORTHOWINDOW_H
#define ZFLYEMORTHOWINDOW_H

#include <QMainWindow>

class ZDvidTarget;
class ZFlyEmOrthoWidget;
class ZIntPoint;
class ZFlyEmOrthoDoc;
class ZFlyEmProofDoc;
class ZWidgetMessage;
class QProgressDialog;
class ZProgressSignal;
class ZDvidEnv;

class ZFlyEmOrthoWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit ZFlyEmOrthoWindow(const ZDvidEnv &env, QWidget *parent = 0);
  explicit ZFlyEmOrthoWindow(
      const ZDvidEnv &env, int width, int height, int depth,
      QWidget *parent = 0);

  ZFlyEmOrthoDoc *getDocument() const;
  void copyBookmarkFrom(ZFlyEmProofDoc *doc);

signals:
  void bookmarkEdited(int x, int y, int z);
  void synapseEdited(int x, int y, int z);
  void synapseVerified(int x, int y, int z, bool verified);
  void zoomingTo(int x, int y, int z);
  void bodyMergeEdited();
  void todoEdited(int x, int y, int z);

public slots:
  void updateData(const ZIntPoint &center);
  void downloadBookmark(int x, int y, int z);
  void downloadSynapse(int x, int y, int z);
  void downloadTodo(int x, int y, int z);
  void syncMergeWithDvid();
  void processMessage(const ZWidgetMessage &message);
  void syncBodyColorMap(ZFlyEmProofDoc *doc);
//  void updateSequencerBodyMap(ZFlyEmBodyColorOption::EColorOption type);
//  void notifyBodyMergeEdited();

private:
  void initWidget();

private:
  ZFlyEmOrthoWidget *m_orthoWidget;
  QProgressDialog *m_progressDlg;
  ZProgressSignal *m_progressSignal;
};

#endif // ZFLYEMORTHOWINDOW_H
