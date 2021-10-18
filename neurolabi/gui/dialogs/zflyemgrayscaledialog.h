#ifndef ZFLYEMGRAYSCALEDIALOG_H
#define ZFLYEMGRAYSCALEDIALOG_H

#include <QDialog>

class ZProofreadWindow;
class ZFlyEmProofMvc;
class ZIntPoint;
class ZIntCuboid;
class QRect;
class ZStackViewParam;

namespace Ui {
class ZFlyEmGrayscaleDialog;
}

class ZFlyEmGrayscaleDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ZFlyEmGrayscaleDialog(QWidget *parent = nullptr);
  ~ZFlyEmGrayscaleDialog();

  ZProofreadWindow* getMainWindow() const;
  ZFlyEmProofMvc* getFlyEmProofMvc() const;

  ZIntPoint getCenter() const;
  ZIntPoint getSize() const;
  ZIntCuboid getBoundBox() const;
  ZIntPoint getFirstCorner() const;
  ZIntPoint getLastCorner() const;

  int getCenterX() const;
  int getCenterY() const;
  int getCenterZ() const;
  int getWidth() const;
  int getHeight() const;
  int getDepth() const;
  int getDsIntv() const;

  void setCenter(int x, int y, int z);
  void setCenter(const ZIntPoint &pt);
  void setWidth(int width);
  void setHeight(int height);
  void setDepth(int depth);

  bool isFullRange() const;
  bool isSparse() const;
  /*!
   * \brief Get the actual range.
   *
   * It returns an empty box for full range.
   */
  ZIntCuboid getRange() const;

  void makeGrayscaleExportAppearance();
  void makeBodyExportAppearance();
  void makeBodyFieldExportAppearance();

private slots:
  void useCurrentOffset();
  void useViewCenter();
  void useViewPort();
  void updateWidget();

private:
  void connectSignalSlot();
  ZStackViewParam getViewParam() const;

private:
  Ui::ZFlyEmGrayscaleDialog *ui;

};

#endif // ZFLYEMGRAYSCALEDIALOG_H
