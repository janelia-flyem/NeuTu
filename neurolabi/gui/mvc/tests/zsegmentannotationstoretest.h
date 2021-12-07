#ifndef ZSEGMENTANNOTATIONSTORETEST_H
#define ZSEGMENTANNOTATIONSTORETEST_H

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

#include "mvc/annotation/zsegmentannotationstore.h"
#include "zjsonobjectparser.h"

class ZMockSegmentAnnotationStore : public ZSegmentAnnotationStore {
public:
  ZMockSegmentAnnotationStore() {}

  void invalidateCache(uint64_t bodyId) {
    m_annotationCache.remove(bodyId);
  }

  ZJsonObject getAnnotation(
      uint64_t sid, neutu::ECacheOption option = neutu::ECacheOption::CACHE_FIRST) {
    if (option == neutu::ECacheOption::SOURCE_ONLY) {
      invalidateCache(sid);
    }

    bool updatingCache = (option == neutu::ECacheOption::SOURCE_FIRST) ||
        !m_annotationCache.contains(sid);
    if (updatingCache) {
      m_annotationCache[sid] = m_annotationStore.value(sid);
    }

    if (m_annotationCache.contains(sid)) {
      return m_annotationCache.value(sid).clone();
    }

    return ZJsonObject();
  }

  void saveAnnotation(uint64_t sid, const ZJsonObject &obj) {
    ZJsonObject oldAnnotation = getAnnotation(sid);
    ZJsonObject newAnnotation = obj.clone();
    std::vector<std::string> keysToRemove;
    newAnnotation.forEachValue([&](const std::string &key, ZJsonValue value) {
      if (!oldAnnotation.hasKey(key)) {
        if (value.isString()) {
          if (value.toString().empty()) {
            keysToRemove.push_back(key);
          }
        }
      }
    });
    for (const auto &key : keysToRemove) {
      newAnnotation.removeKey(key.c_str());
    }
    m_annotationStore[sid] = newAnnotation;
    m_annotationCache[sid] = newAnnotation;
  }

  void removeAnnotation(uint64_t sid) {
    m_annotationStore.remove(sid);
    invalidateCache(sid);
  }

private:
  QMap<uint64_t, ZJsonObject> m_annotationStore;
};

TEST(ZSegmentAnnotationStore, Basic)
{
  ZMockSegmentAnnotationStore store;
  ASSERT_TRUE(store.getAnnotation(1).isNull());
  ASSERT_TRUE(store.getAnnotation(1, neutu::ECacheOption::SOURCE_ONLY).isNull());

  ZJsonObject obj;
  obj.setEntry("status", "test");
  store.saveAnnotation(1, obj);
  ASSERT_FALSE(store.getAnnotation(1).isNull());
  ASSERT_FALSE(store.getAnnotation(1, neutu::ECacheOption::SOURCE_ONLY).isNull());

  obj.setEntry("status", "test2");
  store.saveAnnotation(2, obj);
  ASSERT_FALSE(store.getAnnotation(2).isNull());
  ASSERT_FALSE(store.getAnnotation(2, neutu::ECacheOption::SOURCE_ONLY).isNull());

  ASSERT_EQ("test", ZJsonObjectParser::GetValue(
              store.getAnnotation(1), "status", ""));
  ASSERT_EQ("test2", ZJsonObjectParser::GetValue(
              store.getAnnotation(2), "status", ""));

  auto annotations = store.getAnnotations({1, 2, 3});
  ASSERT_EQ(3, int(annotations.size()));
  ASSERT_EQ("test", ZJsonObjectParser::GetValue(
              annotations[0], "status", ""));
  ASSERT_EQ("test2", ZJsonObjectParser::GetValue(
              annotations[1], "status", ""));
  ASSERT_TRUE(annotations[2].isNull());
}

#endif

#endif // ZSEGMENTANNOTATIONSTORETEST_H
