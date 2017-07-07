#include "zsurfreconcommand.h"
#include <iostream>
#include <QCryptographicHash>
#include <QFile>
#include <QString>
#include <QByteArray>
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zjsonvalue.h"
#if defined(_ENABLE_SURFRECON_)
#include "sandbox/zsurfreconmodule.h"
#endif
ZSurfreconCommand::ZSurfreconCommand()
{

}
int ZSurfreconCommand::run(const std::vector<std::string> &/*input*/, const std::string &/*output*/,
                           const ZJsonObject &config)
{

#if defined(_ENABLE_SURFRECON_)
  if(!config.hasKey("npoints"))
    return 1;
  int npoints=config.value("npoints").toInteger();
  VoxelSet in;
  for(int i=0;i<npoints;++i)
  {
     double x,y,z;
     std::cin>>x>>y>>z;
     in.push_back(VoxelType(x,y,z));
  }
  std::cout<<"number of input points:"<<npoints<<std::endl;
  Surf surface;
  VoxelSet out;
  surface.surfrecon(in,out,26,1);
  std::cout<<"number of output points:"<<out.size()<<std::endl;
  std::srand(std::time(0));
  QString id=QString::number(std::time(0))+"."+QString::number(std::rand());
  std::ofstream fout(std::string("./result/")+id.toStdString());
  for(size_t i=0; i<out.size(); i++)
  {
    fout <<out[i].x <<" "<<out[i].y <<" "<<out[i].z<<std::endl;
  }
  fout.close();
  QFile f(QString("./result/")+id);
  f.open(QFile::ReadOnly);
  QByteArray ba=QCryptographicHash::hash(f.readAll(),QCryptographicHash::Md5);
  f.close();
  std::cout<<(QString("result id:")+id).toStdString()<<std::endl;
  std::cout<<"md5:"<<ba.toHex().constData()<<std::endl;
  return 0;
#else
  return 1;
#endif
}
