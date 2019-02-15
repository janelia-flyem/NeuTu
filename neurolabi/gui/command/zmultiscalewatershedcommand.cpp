#include <fstream>
#include <strstream>
#include "zmultiscalewatershedcommand.h"
#include"flyem/zstackwatershedcontainer.h"
#include "mvc/zstackdoc.h"
#include "zjsonparser.h"
#include "zjsonobject.h"
#include "zjsonvalue.h"
#include "zswctree.h"
#if defined(_ENABLE_SURFRECON_)
#include "imgproc/zsurfrecon.h"
#endif

ZMultiscaleWatershedCommand::ZMultiscaleWatershedCommand()
{

}

int ZMultiscaleWatershedCommand::run(const std::vector<std::string> &/*input*/,
    const std::string &/*output*/,
    const ZJsonObject &/*config*/)
{
/*
    #if defined(_ENABLE_SURFRECON_)
  if(!config.hasKey("stack_file")
     || !config.hasKey("skeleton_file")
     || !config.hasKey("surf_file")
     || !config.hasKey("scale")
     || !config.hasKey("result_file")
     )
  {
    return 1;
  }

  ZStack* src=new ZStack();
  ZSwcTree* tree=new ZSwcTree();

  if(!src->load(ZJsonParser::stringValue(config["stack_file"]))
     || !tree->load(ZJsonParser::stringValue(config["skeleton_file"]))){
     delete src;
     delete tree;
     return 1;
  }
  std::vector<int> size=ZJsonParser::integerArray(config["size"]);
  if(size.size()!=3){
    return 1;
  }
  int scale=ZJsonParser::integerValue(config["scale"]);

//load surface points
  std::ifstream fin(ZJsonParser::stringValue(config["surf_file"]));
  if(!fin){
    return 1;
  }
  VoxelSet in;
  std::string line;
  while(getline(fin,line))
  {
    if(strstr(line.c_str(),"#")){
    }
    else{
       std::istringstream iss(line);
       if(iss.fail() || iss.eof())
       continue;
       double n,t,x,y,z,r,p;
       iss>>n>>t>>x>>y>>z>>r>>p;
       in.push_back(VoxelType(x/scale,y/scale,z/scale));
    }
  }
  fin.close();
//surfrecon and prune skeleton
  VoxelSet out;
  ZSurfRecon surf;
  ZSurfRecon::SurfRecon(in,out);
  ZSurfRecon::PruneSkeleton(out,tree);
  ZSurfRecon::LabelStack(out,src);
  ZSurfRecon::GaussianBlur(out,src,5);

//water shed
  ZStackWatershedContainer container(src);
  container.setScale(scale);
  container.addSeed(*tree);
  container.run();
  ZStack* result=container.getResultStack();
  result->save(ZJsonParser::stringValue(config["result_file"]));

  delete src;
  delete tree;
  return 0;

#else
  return 1;
#endif*/
  return 1;
}




