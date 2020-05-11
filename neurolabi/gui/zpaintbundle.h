#ifndef ZPAINTBUNDLE_H
#define ZPAINTBUNDLE_H

#include <memory>
#include <QList>
#include <QMutex>

//#include <boost/iterator/iterator_facade.hpp>
//#include <boost/type_traits/is_convertible.hpp>
//#include <boost/utility/enable_if.hpp>

#include "common/neutudefs.h"
#include "swctreenode.h"
#include "zstackball.h"

#include "geometry/zpoint.h"
#include "zswctree.h"
#include "geometry/zintpoint.h"
#include "zstackobjectsourcefactory.h"
#include "data3d/displayconfig.h"
#include "data3d/zsliceviewtransform.h"
//#include "zstackviewparam.h"

#if 0
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
        while (m_listIdx < 1 &&
               m_drawableIdx >= m_bundle->m_objList.size()) {
          ++m_listIdx;
        }
//        if (m_listIdx == m_bundle->m_objLists.size()) {
//          setSwcNodeAdaptor();
//        }
      } else if (pos == End) {
//        m_listIdx = m_bundle->m_objLists.size();
        m_listIdx = 1;
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
  }

  void increment()
  {
    if (!m_bundle) {
      return;
    }
    if (m_listIdx < 1) {
      if (++m_drawableIdx >= m_bundle->m_objList.size()) {
        ++m_listIdx;
        m_drawableIdx = 0;
        while (m_listIdx < 1 &&
               m_drawableIdx >= m_bundle->m_objList.size()) {
          ++m_listIdx;
        }
//        if (m_listIdx == 1) { // move out of list, return first item of node set
//          setSwcNodeAdaptor();
//        }
      }
    }
  }

  TStackDrawablePtr dereference() const
  {
    return m_listIdx < 1 ?
          m_bundle->m_objList.at(m_drawableIdx) :
          dynamic_cast<TStackDrawablePtr>(&m_nodeAdaptor);
  }

  TPaintBundle* m_bundle;
  int m_listIdx;
  int m_drawableIdx;
//  std::set<Swc_Tree_Node*>::const_iterator m_swcNodeIter;

  ZStackBall m_nodeAdaptor;
};

} // namespace impl
#endif


class ZPaintBundle
{
public:
//  typedef impl::drawable_iter<ZPaintBundle, ZStackObject*> iterator;
//  typedef impl::drawable_iter<ZPaintBundle const, const ZStackObject*> const_iterator;

  ZPaintBundle();
  ~ZPaintBundle();

//  inline const_iterator begin() const { return const_iterator(this, const_iterator::Begin); }
//  inline const_iterator end() const { return const_iterator(this, const_iterator::End); }

//  inline iterator begin() { return iterator(this, iterator::Begin); }
//  inline iterator end() { return iterator(this, iterator::End); }

  void clearAllDrawableLists();

  void addDrawableList(const QList<ZStackObject*>& lst);

  void addDynamicObject(ZStackObject *obj);
  void clearDynamicObjectList();

  void setSliceViewTransform(const ZSliceViewTransform &t);

//  inline int sliceIndex() const { return m_viewParam.getSliceIndex(); }
//  void setViewParam(const ZStackViewParam &param);

  neutu::EAxis getSliceAxis() const;

  /*
  inline int getZ() const {
    return m_viewParam.getZ();
  }
  */

  void setDisplayStyle(ZStackObject::EDisplayStyle style);
  ZStackObject::EDisplayStyle getDisplayStyle() const;

  void setDisplaySliceMode(neutu::data3d::EDisplaySliceMode mode);
  neutu::data3d::EDisplaySliceMode getDisplaySliceMode() const;

  inline void setStackOffset(int x, int y, int z) {
    m_stackOffset.set(x, y, z);
  }
  inline void setStackOffset(const ZIntPoint &offset) {
    m_stackOffset = offset;
  }

  inline const ZIntPoint& getStackOffset() const {
    return m_stackOffset;
  }

  QList<ZStackObject*> getVisibleObjectList(ZStackObject::ETarget target) const;
  QList<std::shared_ptr<ZStackObject> > getVisibleDynamicObjectList() const;
//  void alignToCutPlane(const QList<std::shared_ptr<ZStackObject>> &objList) const;
  bool hasDynamicObject() const;

private:
//  template<typename T1, typename T2> friend class impl::drawable_iter;

  QList<ZStackObject*> m_objList;

  QList<std::shared_ptr<ZStackObject>> m_dynamicObjectList; //Dynamically changing object; owned by the painter itself
  mutable QMutex m_dynamicObjectListMutex;

  neutu::data3d::DisplayConfig m_displayConfig;
//  ZStackObject::EDisplayStyle m_style;
//  ZSliceViewTransform m_sliceViewTransform;
//  ZStackViewParam m_viewParam;
  ZIntPoint m_stackOffset;
//  ZStackObject::ETarget m_target = ZStackObject::ETarget::WIDGET;
};

#endif // ZPAINTBUNDLE_H
