#ifndef ZDOCPLAYER_H
#define ZDOCPLAYER_H

#include <QList>
#include <QString>
#include <QMutex>

#include <vector>

#include "zstackobject.h"
#include "c_stack.h"
#include "zuncopyable.h"
#include "z3dgraph.h"
#include "common/zsharedpointer.h"

class ZStack;
class ZStroke2d;
class ZSparseObject;
class ZObject3d;
class ZSwcTree;
class ZJsonObject;
class ZObject3dScan;
class ZStackViewParam;
class ZDvidSparsevolSlice;
class ZDvidLabelSlice;
class ZDvidGraySlice;
class ZDvidGraySliceEnsemble;
class ZArbSliceViewParam;
class ZDvidTileEnsemble;
class ZTask;
class ZStackDoc;

/*!
 * \brief The basic class of manage roles to a stack object
 */
class ZDocPlayer
{
public:
  ZDocPlayer(ZStackObject* data = NULL);
  virtual ~ZDocPlayer();

  /*!
   * \return true iff the player contains \a data. It always returns false if
   *         \a data is NULL.
   */
  bool hasData(ZStackObject *data) const;

  /*!
   * \brief Check if a player has specific roles.
   *
   * It returns true iff:
   *   1) The player has all roles specified by non-empty \a role
   *   2) Both the player's role and \a role are ROLE_NONE
   */
  bool hasRole(ZStackObjectRole::TRole role) const;

  bool isEmpty() const;

  virtual void labelStack(ZStack*) const {}
  virtual void labelStack(ZStack *, int) const {}
  virtual void labelStack(Stack*, int*, int) const {}
  virtual void labelStack(Stack*, int*, int, int, int, int) const {}
  virtual void paintStack(ZStack*) const {}
  virtual void paintStack(
      const std::vector<Stack*> &stackArray,
      const int *offset, int xIntv, int yIntv, int zIntv) const {
    UNUSED_PARAMETER(stackArray)
    UNUSED_PARAMETER(offset)
    UNUSED_PARAMETER(xIntv)
    UNUSED_PARAMETER(yIntv)
    UNUSED_PARAMETER(zIntv)
  }

  virtual ZStack* toStack() const { return NULL; }
  virtual int getLabel() const { return 0; }
  virtual void setLabel(int /*label*/) {}
  virtual QString getTypeName() const { return "Unknown"; }
  virtual ZSwcTree* getSwcDecoration() const { return NULL; }
  virtual Z3DGraph get3DGraph() const { return Z3DGraph(); }

  virtual ZJsonObject toJsonObject() const;
  virtual ZJsonObject toSeedJson() const;

  virtual ZIntCuboid getBoundBox() const;

  virtual ZTask* getFutureTask() const;
  virtual ZTask* getFutureTask(ZStackDoc *doc) const;

  inline ZStackObject* getData() const {
    return m_data;
  }

  /*
  inline TRole getRole() const {
    return m_role;
  }
  */

  const ZStackObjectRole::TRole& getRole() const {
    if (m_data == NULL) {
      return ZStackObjectRole::ROLE_NONE;
    }

    return m_data->getRole().getRole();
  }

  void enableUpdate(bool on) { m_enableUpdate = on; }

  virtual bool updateData(const ZStackViewParam &/*param*/) const {
    return true;
  }

protected:
  ZStackObject *m_data; //not owned by the player
  bool m_enableUpdate;
  //TRole m_role;
};

/********************************************************/
class ZDocPlayerList : public ZUncopyable
{
public:
  virtual ~ZDocPlayerList();

  inline QList<ZDocPlayer*>& getPlayerList() {
    return m_playerList;
  }

  inline const QList<ZDocPlayer*>& getPlayerList() const {
    return m_playerList;
  }

  void add(ZDocPlayer *data);

  int size() const {
    return m_playerList.size();
  }

  /*!
   * \brief Remove players containing certain data
   *
   * Remove all players that contain \a data in the list. The removed objects
   * will be freed from memory.
   *
   * \return The roles of \a data.
   */
  ZStackObjectRole::TRole removePlayer(ZStackObject *data);

  QList<ZDocPlayer*> takePlayer(ZStackObject *data);


  /*!
   * \brief Remove players with certain roles.
   *
   * Remove all players that has roles specified by \a role. The removed objects
   * will be freed from memory.
   *
   * \return The roles of the removed data.
   */
  ZStackObjectRole::TRole removePlayer(ZStackObjectRole::TRole role);

  ZStackObjectRole::TRole removeAll();

  /*!
   * \brief Get all players with specific roles
   */
  QList<ZDocPlayer*> getPlayerList(ZStackObjectRole::TRole role);

  QList<const ZDocPlayer*> getPlayerList(ZStackObjectRole::TRole role) const;

  /*!
   * \brief Check if any player with specific roles exists
   */
  bool hasPlayer(ZStackObjectRole::TRole role) const;

  /*!
   * \brief Check if the player list contains certain data pointer
   *
   * \param data A pointer to check. It can be a pointer that has already been
   *        freed.
   * \return true iff the list contains \a data
   */
  bool contains(const ZStackObject *data);
  bool containsUnsync(const ZStackObject *data);

  void clear();
  void clearUnsync();

  void print() const;

  void moveTo(ZDocPlayerList &playerList);

  QMutex* getMutex() const {
    return &m_mutex;
  }

public:
  void addUnsync(ZDocPlayer *data);
  ZStackObjectRole::TRole removePlayerUnsync(ZStackObject *data);

  QList<ZDocPlayer*> takePlayerUnsync(ZStackObject *data);


  ZStackObjectRole::TRole removePlayerUnsync(ZStackObjectRole::TRole role);

  ZStackObjectRole::TRole removeAllUnsync();

  QList<ZDocPlayer*> getPlayerListUnsync(ZStackObjectRole::TRole role);

  QList<const ZDocPlayer*> getPlayerListUnsync(ZStackObjectRole::TRole role) const;

  bool hasPlayerUnsync(ZStackObjectRole::TRole role) const;

  void printUnsync() const;

private:
  QList<ZDocPlayer*> m_playerList;
  mutable QMutex m_mutex;
};

/***************************************************/
class ZStroke2dPlayer : public ZDocPlayer
{
public:
  ZStroke2dPlayer(ZStackObject* data = NULL);

public:
  void labelStack(ZStack*stack) const;
  ZStack* toStack() const;
  int getLabel() const;
  void setLabel(int label);
  QString getTypeName() const;
  ZJsonObject toJsonObject() const;
  ZJsonObject toSeedJson() const;

  ZSwcTree* getSwcDecoration() const;
  Z3DGraph get3DGraph() const;

  ZStroke2d* getCompleteData() const;
};

/***************************************************/
class ZObject3dPlayer : public ZDocPlayer
{
public:
  ZObject3dPlayer(ZStackObject* data = NULL);

public:
  void labelStack(ZStack *stack) const;
  void labelStack(ZStack *stack, int value) const;
  void labelStack(Stack *stack, int *offset, int value) const;
  void labelStack(Stack *stack, int *offset, int value,
                  int xIntv, int yIntv, int zIntv) const;
  ZStack* toStack() const;
  void paintStack(ZStack *stack) const;
  void paintStack(
        const std::vector<Stack*> &stackArray,
        const int *offset, int xIntv, int yIntv, int zIntv) const;

  int getLabel() const;
  void setLabel(int label);
  ZSwcTree* getSwcDecoration() const;
  Z3DGraph get3DGraph() const;
  ZJsonObject toJsonObject() const;
  QString getTypeName() const { return "Object3d"; }

  const ZObject3d *getCompleteData() const;
  ZObject3d *getCompleteData();

  ZJsonObject toSeedJson() const;
};

/***************************************************/
class ZObject3dScanPlayer : public ZDocPlayer
{
public:
  ZObject3dScanPlayer(ZStackObject* data = NULL);

public:
//  void labelStack(ZStack *stack) const;
//  void labelStack(ZStack *stack, int value) const;
//  void labelStack(Stack *stack, int *offset, int value) const;
//  void labelStack(Stack *stack, int *offset, int value,
//                  int xIntv, int yIntv, int zIntv) const;
//  ZStack* toStack() const;
//  void paintStack(ZStack *stack) const;
//  void paintStack(
//        const std::vector<Stack*> &stackArray,
//        const int *offset, int xIntv, int yIntv, int zIntv) const;

  int getLabel() const;
//  ZSwcTree* getSwcDecoration() const;
//  Z3DGraph get3DGraph() const;
//  ZJsonObject toJsonObject() const;
  QString getTypeName() const { return "Object3dScan"; }

  const ZObject3dScan *getCompleteData() const;

};


/***************************************************/
class ZSparseObjectPlayer : public ZDocPlayer
{
public:
  ZSparseObjectPlayer(ZStackObject* data = NULL);

public:
  void labelStack(ZStack*stack) const;
  ZStack* toStack() const;

  QString getTypeName() const { return "SparseObject"; }

  ZSparseObject *getCompleteData() const;
};

/***************************************************/
class ZStackBallPlayer : public ZDocPlayer
{
public:
  ZStackBallPlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "StackBall"; }

  Z3DGraph get3DGraph() const;
  ZStackBall *getCompleteData() const;
};

/***************************************************/
class ZPlanePlayer : public ZDocPlayer
{
public:
  QString getTypeName() const { return "Plane"; }
};

/**************************************************/
class ZDvidGraySlicePlayer : public ZDocPlayer
{
public:
  ZDvidGraySlicePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidGraySlice"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidGraySlice *getCompleteData() const;
  ZTask* getFutureTask(ZStackDoc *doc) const;
};

/**************************************************/
class ZDvidGraySliceEnsemblePlayer : public ZDocPlayer
{
public:
  ZDvidGraySliceEnsemblePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidGraySliceEnsemble"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidGraySliceEnsemble *getCompleteData() const;
  ZTask* getFutureTask(ZStackDoc *doc) const;
};

/**************************************************/
class ZDvidLabelSlicePlayer : public ZDocPlayer
{
public:
  ZDvidLabelSlicePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidLabelSlice"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidLabelSlice *getCompleteData() const;
  ZTask* getFutureTask(ZStackDoc *doc) const;
};

/**************************************************/
class ZDvidTileEnsemblePlayer : public ZDocPlayer
{
public:
  ZDvidTileEnsemblePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidTileEnsemble"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidTileEnsemble *getCompleteData() const;
};

/**************************************************/
class ZDvidSparsevolSlicePlayer : public ZDocPlayer
{
public:
  ZDvidSparsevolSlicePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidSparsevolSlice"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidSparsevolSlice *getCompleteData() const;
};

/************************************************/
class ZCuboidRoiPlayer : public ZDocPlayer
{
public:
  ZCuboidRoiPlayer(ZStackObject *data = NULL);

public:
  QString getTypeName() const { return "CuboidRoi"; }
  Z3DGraph get3DGraph() const;
};

#endif // ZDOCPLAYER_H
