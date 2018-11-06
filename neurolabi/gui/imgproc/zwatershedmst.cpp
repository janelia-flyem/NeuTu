#include "zwatershedmst.h"

ZStack* ZWatershedMST::run()
{
  init();

  while(!m_queue_of_edges.empty())
  {
    ZEdge edge = m_queue_of_edges.top();
    uint a = edge.m_a, b = edge.m_b;
    ZActiveSet* aa = m_map_of_activeset[a];
    ZActiveSet* ab = m_map_of_activeset[b];

    if(aa != ab)
    {
      if(!ab->m_label)//b does not contain seeds
      {
        aa->merge(ab);
        updateActiveSet(ab, aa);
        ab->m_vertices.clear();
        delete ab;
      }
      else if(!aa->m_label)//b contains seeds
      {
        ab->merge(aa);
        updateActiveSet(aa, ab);
        aa->m_vertices.clear();
        delete aa;
      }
    }
    m_queue_of_edges.pop();
  }

  ZStack* rv = new ZStack(GREY,m_stack->width(),m_stack->height(),m_stack->depth(),m_stack->channelNumber());
  uint8_t* prv = rv->array8();
  for(uint i = 0 ; i < m_map_of_activeset.size(); ++i)
  {
    prv[i] = m_map_of_activeset[i]->m_label;
  }
  rv->setOffset(m_stack->getOffset());
  return rv;
}


void ZWatershedMST::init()
{
  size_t size = m_stack->getVoxelNumber();
  m_map_of_activeset.resize(size);

  int width = m_stack->width();
  int height = m_stack->height();
  int depth = m_stack->depth();
  size_t area = width * height;

  uint8_t* pdata = m_stack->array8();
  uint8_t* pseed = m_seed->array8();

  for(uint i = 0 ; i < size ; ++i)
  {
    ZActiveSet* set = new ZActiveSet();
    set->add(i);
    if(pseed[i])
    {
      set->setLabel(pseed[i]);
    }
    m_map_of_activeset[i] = set;
  }

  for(int z = 0; z < depth-1; ++z)
  {
    for(int y = 0; y < height-1; ++y)
    {
      for(int x = 0; x < width-1; ++x)
      {
        uint index = z*area + y*width + x;
        if(pdata[index])
        {
          if(pdata[index+1])m_queue_of_edges.push(ZEdge(index,index+1,weight(pdata[index],pdata[index+1])));
          if(pdata[index+width])m_queue_of_edges.push(ZEdge(index,index+width,weight(pdata[index],pdata[index+width])));
          if(pdata[index+area])m_queue_of_edges.push(ZEdge(index,index+area,weight(pdata[index],pdata[index+area])));
        }
      }
    }
  }
}


void ZWatershedMST::updateActiveSet(ZActiveSet *source, ZActiveSet *target)
{
  for(auto v:source->m_vertices)
  {
    m_map_of_activeset[v] = target;
  }
}
