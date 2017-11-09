#include "zstackdiffcommand.h"
#include "zstackdoc.h"

ZStackDiffCommand::ZStackDiffCommand()
{

}


int ZStackDiffCommand::run(const std::vector<std::string> &input, const std::string& output,
                           const ZJsonObject &/*config*/)
{

  if(input.size()!=2 ||output=="")
  {
    return 1;
  }
  ZStack *p=new ZStack,*q=new ZStack;
  if(!p->load(input[0]) || !q->load(input[1]))
  {
    delete p;
    delete q;
    return 1;
  }
  if((p->kind()!=GREY) ||(p->kind()!=q->kind()) || (p->getVoxelNumber() !=q->getVoxelNumber()) )
  {
    delete p;
    delete q;
    return 1;
  }
  ZStack* o=new ZStack(p->kind(),p->width(),p->height(),p->depth(),p->channelNumber());

  for(size_t i=0;i<p->getVoxelNumber();++i)
  {
    o->array8()[i]=p->array8()[i]==q->array8()[i]?0:255;
  }
  o->save(output);
  delete o;
  delete p;
  delete q;
  return 0;
}
