#ifndef ZSURFRECONMODULE_H
#define ZSURFRECONMODULE_H
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "zsandboxmodule.h"
#include <QWidget>
#undef ASCII
#undef BOOL
#undef TRUE
#undef FALSE

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
  void surfRecon(const VoxelSet& in,VoxelSet& out);
  void updateStack(bool* surf_map,const VoxelSet& surf);
  void pruneSkeleton(bool* surf_map);
  void getPostData(const VoxelSet& points,QByteArray& data);
  std::pair<QString,QString> getResultUrlAndMd5(const QByteArray& reply);
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
#endif // ZSURFRECONMODULE_H
