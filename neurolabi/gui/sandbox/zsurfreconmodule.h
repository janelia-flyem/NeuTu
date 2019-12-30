#ifndef ZSURFRECONMODULE_H
#define ZSURFRECONMODULE_H
#if defined(_ENABLE_SURFRECON_)
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "zsandboxmodule.h"
#include <QWidget>
#undef ASCII
#undef BOOL
#undef _TRUE_
#undef _FALSE_
//#include <surfrecon.h>

#include "imgproc/surfrecon.h"

class ZStack;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class ZStackDoc;
class ZSurfReconWindow:public QWidget
{
  Q_OBJECT
public:
  ZSurfReconWindow(QWidget *parent = 0);
private slots:
  void onOk();
  void onServiceDone();
  void onNetworkError(QNetworkReply::NetworkError);
private:
  void getPoints(VoxelSet& points);
  void getPostData(const VoxelSet& points,QByteArray& data);
  void getResultUrlAndMd5(const QByteArray& reply,QString& result_url,QString& md5);
  void getResult(QString url);
private:
  QPushButton*  ok;
  QCheckBox*    use_service;
  QLineEdit*    service_url;
  QHBoxLayout*  lay;
  ZStackDoc * doc;
  ZStack* src;
  QString result_md5;
  QNetworkAccessManager* man;
  QString result_url;
};

class ZSurfReconModule : public ZSandboxModule
{
  Q_OBJECT
public:
  explicit ZSurfReconModule(QObject *parent = 0);
  ~ZSurfReconModule(){delete window;}
  void surfrecon(ZStack* stack,const VoxelSet& points_in);
signals:

public slots:

private slots:
  void execute();

private:
  void init();
private:
  ZSurfReconWindow* window;

};
#endif
#endif // ZSURFRECONMODULE_H
