#include <fstream>
#include <strstream>
#include "zmultiscalewatershedcommand.h"
#include "imgproc/zstackmultiscalewatershed.h"
#include "imgproc/zstackwatershed.h"
#include "zstackdoc.h"
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
    const ZJsonObject &config)
{
    #if defined(_ENABLE_SURFRECON_)
//check params
  if(!config.hasKey("stack_file")
     || !config.hasKey("skeleton_file")
     || !config.hasKey("surf_file")
     || !config.hasKey("scale")
     || !config.hasKey("size")
     || !config.hasKey("result_file")
     || !config.hasKey("seed_file")
     || !config.hasKey("area_need_update_file")
     )
  {
    return 1;
  }

//load stack
  ZStack* src=new ZStack();
  if(!src->load(ZJsonParser::stringValue(config["stack_file"])))
  {
     return 1;
  }


  std::vector<int> size=ZJsonParser::integerArray(config["size"]);
  if(size.size()!=3)
  {
    return 1;
  }
  int width=size[0],height=size[1],depth=size[2],scale=ZJsonParser::integerValue(config["scale"]);

//load skeleton
  ZSwcTree* tree=new ZSwcTree();
  if(!tree->load(ZJsonParser::stringValue(config["skeleton_file"])))
  {
    return 1;
  }
  tree->rescale(1.0/scale,1.0/scale,1.0/scale);

//load surface points
  std::ifstream fin(ZJsonParser::stringValue(config["surf_file"]));
  if(!fin)
  {
    return 1;
  }
  VoxelSet in;
  std::string line;
  while(getline(fin,line))
  {
    if(strstr(line.c_str(),"#"))
    {
    }
    else
    {
       std::istringstream iss(line);
       if(iss.fail() || iss.eof())
       continue;
       double n,t,x,y,z,r,p;
       iss>>n>>t>>x>>y>>z>>r>>p;
       in.push_back(VoxelType(x/scale,y/scale,z/scale));
    }
  }
//surfrecon and prune skeleton
  VoxelSet out;
  ZSurfRecon::SurfRecon(in,out);
  ZSurfRecon::PruneSkeleton(out,tree);
  ZSurfRecon::LabelStack(out,src);
  ZSurfRecon::GaussianBlur(out,src,5);

//water shed
  ZStackMultiScaleWatershed water;
  ZStack* seed=new ZStack(GREY,src->width(),src->height(),src->depth(),1);
  water.setScale(1);
  QList<ZSwcTree*>trees;
  trees.push_back(tree);
  water.fillSeed(seed,trees,src->getOffset());
  ZStack* result=ZStackWatershed().run(src,seed);
  result->save(ZJsonParser::stringValue(config["result_file"]));

  water.setScale(scale);
  ZStack* new_seeds=new ZStack(GREY,width,height,depth,1);
  new_seeds->setOffset(
        std::min(width-1,src->getOffset().m_x*scale),
        std::min(height-1,src->getOffset().m_y*scale),
        std::min(depth-1,src->getOffset().m_z*scale));
  ZStack* edge_map=water.getEdgeMap(*result);
  water.generateSeeds(new_seeds,width,height,depth,edge_map,result);
  new_seeds->save(ZJsonParser::stringValue(config["seed_file"]));
  ZStack* area_need_update=water.labelAreaNeedUpdate(edge_map,new_seeds);
  area_need_update->save(ZJsonParser::stringValue(config["area_need_update_file"]));

  fin.close();
  delete src;
  delete tree;
  delete result;
  delete edge_map;
  delete area_need_update;
  delete new_seeds;
  return 0;

#else
  return 1;
#endif
}




