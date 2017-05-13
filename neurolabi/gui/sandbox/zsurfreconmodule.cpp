#include "zsurfreconmodule.h"
#include <functional>
#include <unordered_map>
#include <QAction>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QJsonParseError>
#include <QHBoxLayout>
#include <QCryptographicHash>
#include "zsandbox.h"
#include "mainwindow.h"
#include "zswcforest.h"



int gaussianBlur(ZStack* src,int x,int y,int z,int r,double sigma)
{
  const static double tpi=std::pow(std::sqrt(2*3.1415926),3);
  double sigma_2=sigma*sigma;
  double sigma_3=sigma_2*sigma;

  int width=src->width(),height=src->height(),depth=src->depth(),area=width*height;
  int x_min=std::max(0,x-r),x_max=std::min(width,x+r);
  int y_min=std::max(0,y-r),y_max=std::min(height,y+r);
  int z_min=std::max(0,z-r),z_max=std::min(depth,z+r);

  int size=(x_max-x_min)*(y_max-y_min)*(z_max-z_min);
  double* weights=new double[size];
  memset(weights,0,sizeof(double)*size);
  double total=0.0,weight=0.0;
  int index=0;

  for(int zz=z_min;zz<z_max;++zz)
  {
    for(int yy=y_min;yy<y_max;++yy)
    {
      for(int xx=x_min;xx<x_max;++xx)
      {
        double p=-1*((xx-x)*(xx-x)+(yy-y)*(yy-y)+(zz-z)*(zz-z));
        weight=1.0/(tpi*sigma_3)*std::exp(p/(2*sigma_2));
        weights[index++]=weight;
        total+=weight;
      }
    }
  }


  double value=0.0;
  index=0;
  for(int zz=z_min;zz<z_max;++zz)
  {
    for(int yy=y_min;yy<y_max;++yy)
    {
      for(int xx=x_min;xx<x_max;++xx)
      {
         value+=weights[index++]/total*src->array8()[xx+yy*width+zz*area];
      }
    }
  }

  delete[] weights;
  return int(value);
}



bool crossSurface(bool* surf_map,ZStack* src,Swc_Tree_Node* p,Swc_Tree_Node* q)
{
  int ofx=src->getOffset().m_x,ofy=src->getOffset().m_y,ofz=src->getOffset().m_z;

  double px=p->node.x-ofx,py=p->node.y-ofy,pz=p->node.z-ofz;
  double qx=q->node.x-ofx,qy=q->node.y-ofy,qz=q->node.z-ofz;

  double x, y,z, xincre, yincre,zincre;
  int k = std::max(abs(px -qx),abs(py - qy));
  k=  std::max(double(k),abs(pz-qz));

  xincre = (qx - px)/(double)k;
  yincre = (qy - py)/(double)k;
  zincre = (qz - pz)/(double)k;

  x = px;
  y = py;
  z = pz;

  int width=src->width(),height=src->height(),area=width*height;
  size_t max_off=src->getVoxelNumber();
  for (int i = 1; i < k; i++)
  {
      for(int zz=std::floor(z);zz<=std::ceil(z);++zz)
      {
        for(int yy=std::floor(y);yy<=std::ceil(y);++yy)
        {
          for(int xx=std::floor(x);xx<=std::ceil(x);++xx)
          {
            size_t offset=xx+yy*width+zz*area;
            if(offset<max_off && surf_map[offset])
            {
              return true;
            }
          }
        }
      }
      x += xincre;
      y += yincre;
      z += zincre;
  }

  return false;
}


ZSurfReconWindow::ZSurfReconWindow(QWidget *parent):QWidget(parent)
{
  man=new QNetworkAccessManager(this);
  lay=new QHBoxLayout(this);
  ok=new QPushButton("ok");
  use_service=new QCheckBox("use service");
  service_url=new QLineEdit();
  lay->addWidget(use_service);
  lay->addWidget(new QLabel("url:"));
  lay->addWidget(service_url);
  lay->addWidget(ok);
  this->setLayout(lay);
  this->move(300,200);
  connect(ok,SIGNAL(clicked()),this,SLOT(onOk()));
}


void ZSurfReconWindow::onNetworkError(QNetworkReply::NetworkError)
{
  QMessageBox::information(0,"service error","netwrok error, please check your service url");
}


void ZSurfReconWindow::onServiceDone()
{
  QNetworkReply* reply=(QNetworkReply*)sender();
  if(reply->objectName()=="post")//here comes the result url and md5
  {
    std::pair<QString,QString> x=getResultUrlAndMd5(reply->readAll());
//    QString url=std::get<0>(x);
//    QString md5=std::get<1>(x);
    url = x.first;
    md5 = x.second;
    if(url=="" || md5=="")
    {
      QMessageBox::information(0,"service error","no result or md5 createad");
      return;
    }
    result_md5=md5;
    result_url=url;
    getResult(url);
  }
  if(reply->objectName()=="get")//here comes the result
  {
    QByteArray result=reply->readAll();
    QByteArray ba=QCryptographicHash::hash(result,QCryptographicHash::Md5);
    if(ba.toHex()!=this->result_md5)
    {
      getResult(result_url);
      return;
    }
    std::cout<<"result md5 correct"<<std::endl;
    VoxelSet out;
    QString result_str=result;
    if(result_str!="")
    {
      QStringList lines=result_str.split('\n');
      for(int i=0;i<lines.size();++i)
      {
        QStringList nums=lines[i].split(' ');
        if(nums.size()==3)
        {
          out.push_back(VoxelType(nums[0].toDouble(),
                                nums[1].toDouble(),
                                nums[2].toDouble()));
        }
      }
      bool* surf_map=new bool[src->width()*src->height()*src->depth()];
      memset(surf_map,0,sizeof(bool)*src->width()*src->height()*src->depth());
      updateStack(surf_map,out);
      pruneSkeleton(surf_map);
      delete surf_map;
    }
  }
  reply->deleteLater();
}


void ZSurfReconWindow::getResult(QString sub_url)
{

  QString url=service_url->text();
  if(! url.startsWith("http"))
  {
    url=QString("http://")+url;
  }
  if(url.endsWith('/'))
  {
    url.remove(url.length(),1);
  }
  url+=sub_url;
  std::cout<<url.toStdString()<<std::endl;
  QNetworkReply* reply=man->get(QNetworkRequest(QUrl(url)));
  reply->setObjectName("get");
  connect(reply, SIGNAL(finished()), this, SLOT(onServiceDone()));
  connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
           this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
}


void ZSurfReconWindow::onOk()
{
  doc =ZSandbox::GetCurrentDoc();
  if(!doc)return;
  src=doc->getStack();
  if(!src)return;

  VoxelSet points_in;
  getPoints(points_in);

  if(use_service->isChecked())
  {
    QString url=service_url->text();
    if(! url.startsWith("http"))
    {
      url=QString("http://")+url;
    }
    if(! url.endsWith('/'))
    {
      url+="/";
    }
    url+="surfrecon";

    std::cout<<url.toStdString()<<std::endl;
    QByteArray data;
    getPostData(points_in,data);
    std::cout<<data.toStdString()<<std::endl;
    QNetworkReply* reply=man->post(QNetworkRequest(QUrl(url)),data);
    reply->setObjectName("post");
    connect(reply, SIGNAL(finished()), this, SLOT(onServiceDone()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
             this, SLOT(onNetworkError(QNetworkReply::NetworkError)));
  }
  else//compute at local
  {
    VoxelSet points_out;
    surfRecon(points_in,points_out);
    bool* surf_map=new bool[src->width()*src->height()*src->depth()];
    memset(surf_map,0,sizeof(bool)*src->width()*src->height()*src->depth());
    updateStack(surf_map,points_out);
    pruneSkeleton(surf_map);
    delete surf_map;
  }

  this->hide();
}


std::pair<QString,QString> ZSurfReconWindow::getResultUrlAndMd5(const QByteArray& reply)
{
  QJsonParseError json_error;
  QJsonDocument doucment = QJsonDocument::fromJson(reply, &json_error);

  QString result_url="";
  QString md5="";
  if(json_error.error == QJsonParseError::NoError)
  {
    if(doucment.isObject())
    {
      QJsonObject obj = doucment.object();
      if(obj.contains("md5"))
        md5=obj.take("md5").toString();
      if(obj.contains("result url"))
        result_url=obj.take("result url").toString();
    }
  }
  return std::pair(result_url,md5);
}


void ZSurfReconWindow::getPostData(const VoxelSet& points,QByteArray& data)
{
  QString p="";
  for(auto& x:points)
  {
    p+="(";
    p+=QString::number(x.x)+",";
    p+=QString::number(x.y)+",";
    p+=QString::number(x.z);
    p+=")";
  }
  data.append(QString("points=")+p);
}


void ZSurfReconWindow::pruneSkeleton(bool* surf_map)
{
  std::vector<Swc_Tree_Node*> cnn;
  QList<ZSwcTree*> trees=doc->getSwcList();
  std::for_each(trees.begin(),trees.end(),[&](ZSwcTree* tree)
  {
    if(tree->toString().find("skeleton")!=std::string::npos)
    {
        Swc_Tree_Node* root=tree->forceVirtualRoot();
        ZSwcTree::DepthFirstIterator it(tree);
        it.next();//skip virtual root
        while(it.hasNext())
        {
          Swc_Tree_Node* n=it.next();
          for(Swc_Tree_Node* p=n->first_child;p;p=p->next_sibling)
          {
            if(crossSurface(surf_map,src,n,p))
            {
              cnn.push_back(p);
            }
          }
        }
        for(auto x:cnn)
        {
          SwcTreeNode::setParent(x,root);
        }
        //std::cout<<"remove connection:"<<cnn.size()<<std::endl;
    }
  });

  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {

    if((*it)->toString().find("skeleton")==std::string::npos)
    {
      doc->removeObject(*it);
    }
  }
}


void ZSurfReconWindow::updateStack(bool* surf_map,const VoxelSet& surf)
{
  int width=src->width(),height=src->height(),area=width*height;
  uint8_t* p=src->array8();
  size_t max_off=src->getVoxelNumber();
  for(uint i=0;i<surf.size();++i)
  {
    const VoxelType& v=surf[i];
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area;
          if(offset<max_off)
          {
            p[offset]=0;
            if(surf_map)
              surf_map[offset]=true;
          }
        }
      }
    }
    int index=0;
    std::vector<int> gv;
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area;
          if(offset<max_off)
            gv.push_back(gaussianBlur(src,x,y,z,5,1.2));
        }
      }
    }
    for(int x=std::floor(v.x);x<=std::ceil(v.x);++x)
    {
      for(int y=std::floor(v.y);y<=std::ceil(v.y);++y)
      {
        for(int z=std::floor(v.z);z<=std::ceil(v.z);++z)
        {
          size_t offset=x+y*width+z*area;
          if(offset<max_off)
            p[offset]=gv[index++];
        }
      }
    }
  }

}


void ZSurfReconWindow::surfRecon(const VoxelSet& in,VoxelSet& out)
{
  Surf surface;
  surface.surfrecon(in,out,26,1);
}


void ZSurfReconWindow::getPoints(VoxelSet& points)
{
  int ofx=src->getOffset().m_x,ofy=src->getOffset().m_y,ofz=src->getOffset().m_z;

  QList<ZSwcTree*> trees=doc->getSwcList();

  for(QList<ZSwcTree*>::iterator it=trees.begin();it!=trees.end();++it)
  {
    if((*it)->toString().find("skeleton")==std::string::npos)
    {
      ZSwcTree::DepthFirstIterator iter(*it);
      while(iter.hasNext())
      {
        Swc_Tree_Node* tn=iter.next();
        if(!SwcTreeNode::isVirtual(tn))
        {
          points.push_back(VoxelType(SwcTreeNode::x(tn)-ofx,
                                     SwcTreeNode::y(tn)-ofy,
                                     SwcTreeNode::z(tn)-ofz));
        }
      }
    }
  }
}


ZSurfReconModule::ZSurfReconModule(QObject *parent) :
  ZSandboxModule(parent)
{
  init();
}


void ZSurfReconModule::init()
{
  window=new ZSurfReconWindow();
  m_action = new QAction("SurfRecon", this);
  connect(m_action, SIGNAL(triggered()), this, SLOT(execute()));
}


void ZSurfReconModule::execute()
{
  this->window->show();
}
