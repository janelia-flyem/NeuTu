#ifndef ZSTACKDOCOBJECTUPDATE_H
#define ZSTACKDOCOBJECTUPDATE_H

#include <functional>

class ZStackObject;

class ZStackDocObjectUpdate {
public:
  enum class EAction {
    NONE, CALLBACK, ADD_NONUNIQUE, ADD_UNIQUE,
    UPDATE, SELECT, DESELECT,
    ADD_BUFFER, RECYCLE, EXPEL, KILL
  };

  ZStackDocObjectUpdate(ZStackObject *m_obj, EAction action);
  ~ZStackDocObjectUpdate();

  EAction getAction() const {
    return m_action;
  }

  ZStackObject* getObject() const {
    return m_obj;
  }

  void setAction(EAction action) {
    m_action = action;
  }

  bool isMergable() const;

  /*!
   * \brief Apply the update
   *
   * If \a f doesn't have a valid target, the internal callback function will
   * be applied instead.
   */
  void apply(std::function<void(ZStackObject*, EAction action)> f = nullptr);
  void print() const;

  void setCallback(std::function<void(ZStackObject*obj)> f);

  /*!
   * \brief Set clean up function to avoid memory leak.
   *
   * This is mainly used for explicit control of resource release, especially
   * under the action CALLBACK, which needs the user to free up captured resources
   * more explicitly if the callback is not called before the object destruction.
   *
   * IMPORTANT NOTE: it is NOT for helping the callback clean up resources after
   * the callback calling.
   */
  void setCleanup(std::function<void()> f);

  /*!
   * \brief Reset the update
   *
   * This function will be called when a callback function is applied in
   * \a apply() or the object is cleaned up.
   */
  void reset();

private:
  void cleanUp();

//  static QMap<ZStackObject*, ZStackDocObjectUpdate::EAction>
//  MakeActionMap(QList<ZStackDocObjectUpdate*> updateList);

private:
  ZStackObject *m_obj = nullptr;
  EAction m_action;
  std::function<void(ZStackObject*obj)> m_callback = nullptr;
  std::function<void()> m_dispose = nullptr;
};


#endif // ZSTACKDOCOBJECTUPDATE_H
