#ifndef ZPAINTBUNDLE_H
#define ZPAINTBUNDLE_H

#include <QList>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>

#include "common/neutudefs.h"
#include "swctreenode.h"
#include "zstackball.h"

#include "geometry/zpoint.h"
#include "zswctree.h"
#include "geometry/zintpoint.h"
#include "zstackobjectsourcefactory.h"

namespace impl {

// note: dereference of this iterator return a pointer to [const] ZStackObject
//       instead of a reference to the pointer, so you can not use it to change
//       the pointer itself.
template<class TPaintBundle, class TStackDrawablePtr>
class drawable_iter
    : public boost::iterator_facade<
    drawable_iter<TPaintBundle, TStackDrawablePtr>
    , TStackDrawablePtr
    , boost::forward_traversal_tag
    , TStackDrawablePtr
    >
{
  struct enabler {};
public:
  enum Position {
    Begin, End
  };

  drawable_iter()
  {}

  explicit drawable_iter(TPaintBundle *bundle, Position pos)
    : m_bundle(bundle), m_drawableIdx(0)
  {
    if (m_bundle) {
      if (pos == Begin) {
        m_listIdx = 0;
//        m_swcNodeIter = m_bundle->m_swcNodes->begin();
        while (m_listIdx < m_bundle->m_objLists.size() &&
               m_drawableIdx >= m_bundle->m_objLists[m_listIdx]->size()) {
          ++m_listIdx;
        }
        if (m_listIdx == m_bundle->m_objLists.size()) {
//          setSwcNodeAdaptor();
        }
      } else if (pos == End) {
        m_listIdx = m_bundle->m_objLists.size();
//        m_swcNodeIter = m_bundle->m_swcNodes->end();
      }
    }
  }

  template <class OtherValue, class OtherTStackDrawablePtr>
  drawable_iter(drawable_iter<OtherValue, OtherTStackDrawablePtr> const& other, typename boost::enable_if<
                boost::is_convertible<OtherValue*,TPaintBundle*>
                , enabler
                >::type = enabler()
      )
    : m_bundle(other.m_bundle), m_listIdx(other.m_listIdx), m_drawableIdx(other.m_drawableIdx)
//    , m_swcNodeIter(other.m_swcNodeIter)
  {}

private:
  friend class boost::iterator_core_access;
  template <class, class> friend class drawable_iter;

  template <class OtherValue, class OtherTStackDrawablePtr>
  bool equal(drawable_iter<OtherValue, OtherTStackDrawablePtr> const& other) const
  {
    return this->m_bundle == other.m_bundle &&
        this->m_listIdx == other.m_listIdx &&
        this->m_drawableIdx == other.m_drawableIdx;
//    &&
//        this->m_swcNodeIter == other.m_swcNodeIter;
  }

  void increment()
  {
    if (!m_bundle) {
      return;
    }
    if (m_listIdx < m_bundle->m_objLists.size()) {
      if (++m_drawableIdx >= m_bundle->m_objLists[m_listIdx]->size()) {
        ++m_listIdx;
        m_drawableIdx = 0;
        while (m_listIdx < m_bundle->m_objLists.size() &&
               m_drawableIdx >= m_bundle->m_objLists[m_listIdx]->size()) {
          ++m_listIdx;
        }
        if (m_listIdx == m_bundle->m_objLists.size()) { // move out of list, return first item of node set
//          setSwcNodeAdaptor();
        }
      }
    }/* else if (m_swcNodeIter != m_bundle->m_swcNodes->end()) { // inside node set, move to next
      ++m_swcNodeIter;
      setSwcNodeAdaptor();
    }*/
  }

  TStackDrawablePtr dereference() const
  {
    return m_listIdx < m_bundle->m_objLists.size() ?
          m_bundle->m_objLists[m_listIdx]->at(m_drawableIdx) :
          dynamic_cast<TStackDrawablePtr>(&m_nodeAdaptor);
  }

#if 0
  void setSwcNodeAdaptor()
  {
    if (m_swcNodeIter != m_bundle->m_swcNodes->end()) { // update ZCircle
      /*
      if ((iround(SwcTreeNode::z(*m_swcNodeIter)) ==
           m_bundle->m_sliceIndex + m_bundle->getStackOffset().getZ()) ||
          (m_bundle->m_sliceIndex == -1)) {
        m_nodeAdaptor.setColor(255, 255, 0);
      } else {
        m_nodeAdaptor.setColor(164, 164, 0);
      }
      */
      m_nodeAdaptor.set(SwcTreeNode::x(*m_swcNodeIter), SwcTreeNode::y(*m_swcNodeIter),
                        SwcTreeNode::z(*m_swcNodeIter), SwcTreeNode::radius(*m_swcNodeIter));

      m_nodeAdaptor.setColor(255, 255, 0);
      m_nodeAdaptor.setSelected(true);
      /*
      m_nodeAdaptor.setVisualEffect(ZStackBall::VE_BOUND_BOX |
                                    ZStackBall::VE_DASH_PATTERN |
                                    ZStackBall::VE_NO_FILL);
                                    */
      m_nodeAdaptor.useCosmeticPen(
            ZSwcTree::getHostState(*m_swcNodeIter, ZSwcTree::NODE_STATE_COSMETIC));
      m_nodeAdaptor.setSource(ZStackObjectSourceFactory::MakeNodeAdaptorSource());
//      m_nodeAdaptor.setSource(ZStackObject::getNodeAdapterId());
    }
  }
#endif
  TPaintBundle* m_bundle;
  int m_listIdx;
  int m_drawableIdx;
//  std::set<Swc_Tree_Node*>::const_iterator m_swcNodeIter;

  ZStackBall m_nodeAdaptor;
};

} // namespace impl

class ZPaintBundle
{
public:
  typedef impl::drawable_iter<ZPaintBundle, ZStackObject*> iterator;
  typedef impl::drawable_iter<ZPaintBundle const, const ZStackObject*> const_iterator;

  ZPaintBundle(neutu::EAxis sliceAxis = neutu::EAxis::Z);

  inline const_iterator begin() const { return const_iterator(this, const_iterator::Begin); }
  inline const_iterator end() const { return const_iterator(this, const_iterator::End); }

  inline iterator begin() { return iterator(this, iterator::Begin); }
  inline iterator end() { return iterator(this, iterator::End); }

  inline void clearAllDrawableLists() {
    m_objLists.clear();
    m_otherDrawables.clear();
    m_objLists.push_back(&m_otherDrawables);
  }

  inline void addDrawable(ZStackObject* obj) { if (obj) m_otherDrawables.push_back(obj); }
  inline void removeDrawable(ZStackObject* obj) { m_otherDrawables.removeAll(obj); }
  inline void addDrawableList(const QList<ZStackObject*>* lst) { if (lst) m_objLists.push_back(lst); }
  inline void removeDrawableList(const QList<ZStackObject*>* lst) { m_objLists.removeAll(lst); }

//  inline void setSwcNodeList(const std::set<Swc_Tree_Node*>* lst) { if (lst) m_swcNodes = lst; }
//  inline void unsetSwcNodeList() { m_swcNodes = &m_emptyNodeSet; }

  inline void setSliceIndex(int idx) { m_sliceIndex = idx; }
  inline int sliceIndex() const { return m_sliceIndex; }

  void setSliceAxis(neutu::EAxis axis) { m_sliceAxis = axis; }

  inline int getZ() const {
    return m_sliceIndex + m_stackOffset.getZ();//.getSliceCoord(m_sliceAxis);
  }

  inline void setDisplayStyle(ZStackObject::EDisplayStyle style) { m_style = style; }
  inline ZStackObject::EDisplayStyle displayStyle() const { return m_style; }

  inline void setStackOffset(int x, int y, int z) {
    m_stackOffset.set(x, y, z);
  }
  inline void setStackOffset(const ZIntPoint &offset) {
    m_stackOffset = offset;
  }

  inline const ZIntPoint& getStackOffset() const {
    return m_stackOffset;
  }


private:
  template<typename T1, typename T2> friend class impl::drawable_iter;

  QList<const QList<ZStackObject*>*> m_objLists;
//  const std::set<Swc_Tree_Node*>* m_swcNodes;
  int m_sliceIndex;
  ZStackObject::EDisplayStyle m_style;

  QList<ZStackObject*> m_otherDrawables; // collect single input
//  std::set<Swc_Tree_Node*> m_emptyNodeSet; // make sure m_swcNodes always point to something

  ZIntPoint m_stackOffset;
  neutu::EAxis m_sliceAxis;
};

#endif // ZPAINTBUNDLE_H
