#ifndef ZDOCPLAYER_H
#define ZDOCPLAYER_H

#include <QList>
#include <QString>
#include <vector>
#include "tz_utilities.h"
#include "tz_cdefs.h"
#include "zstackobject.h"
#include "c_stack.h"
#include "zuncopyable.h"
#include "z3dgraph.h"
#include "zsharedpointer.h"

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
    UNUSED_PARAMETER(&stackArray);
    UNUSED_PARAMETER(offset);
    UNUSED_PARAMETER(xIntv);
    UNUSED_PARAMETER(yIntv);
    UNUSED_PARAMETER(zIntv);
  }

  virtual ZStack* toStack() const { return NULL; }
  virtual int getLabel() const { return 0; }
  virtual QString getTypeName() const { return "Unknown"; }
  virtual ZSwcTree* getSwcDecoration() const { return NULL; }
  virtual Z3DGraph get3DGraph() const { return Z3DGraph(); }

  virtual ZJsonObject toJsonObject() const;

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
class ZDocPlayerList : public QList<ZDocPlayer*>, ZUncopyable
{
public:
  virtual ~ZDocPlayerList();

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

  void print() const;
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
  QString getTypeName() const;
  ZJsonObject toJsonObject() const;

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
  ZSwcTree* getSwcDecoration() const;
  Z3DGraph get3DGraph() const;
  ZJsonObject toJsonObject() const;
  QString getTypeName() const { return "Object3d"; }

  const ZObject3d *getCompleteData() const;

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

//  int getLabel() const;
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
#if defined (_FLYEM_)
/**************************************************/
class ZDvidLabelSlicePlayer : public ZDocPlayer
{
public:
  ZDvidLabelSlicePlayer(ZStackObject* data = NULL);

public:
  QString getTypeName() const { return "DvidLabelSlice"; }
  bool updateData(const ZStackViewParam &viewParam) const;
  ZDvidLabelSlice *getCompleteData() const;
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
#endif
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
