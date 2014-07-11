#ifndef ZDOCPLAYER_H
#define ZDOCPLAYER_H

#include <QList>
#include <QString>
#include <vector>
#include "tz_utilities.h"
#include "tz_cdefs.h"
#include "zdocumentable.h"
#include "c_stack.h"
#include "zuncopyable.h"
#include "z3dgraph.h"

class ZStack;
class ZStroke2d;
class ZSparseObject;
class ZObject3d;
class ZSwcTree;

class ZDocPlayer
{
public:
  typedef uint32_t TRole;
  const static TRole ROLE_NONE;
  const static TRole ROLE_SEED;
  const static TRole ROLE_DISPLAY;
  const static TRole ROLE_TMP_RESULT; //Temporary result
  const static TRole ROLE_3DPAINT;
  const static TRole ROLE_MANAGED_OBJECT;
  const static TRole ROLE_3DSWC_DECORATOR;
  const static TRole ROLE_3DGRAPH_DECORATOR;
  const static TRole ROLE_TMP_BOOKMARK;

public:
  ZDocPlayer();
  ZDocPlayer(ZDocumentable* data, TRole role);
  virtual ~ZDocPlayer();

  /*!
   * \return true iff the player contains \a data. It always returns false if
   *         \a data is NULL.
   */
  bool hasData(ZDocumentable *data) const;

  /*!
   * \brief Check if a player has specific roles.
   *
   * It returns true iff:
   *   1) The player has all roles specified by non-empty \a role
   *   2) Both the player's role and \a role are ROLE_NONE
   */
  bool hasRole(TRole role) const;

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

  inline ZDocumentable* getData() const {
    return m_data;
  }
  inline TRole getRole() const {
    return m_role;
  }

protected:
  ZDocumentable *m_data; //not owned by the player
  TRole m_role;
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
  ZDocPlayer::TRole removePlayer(ZDocumentable *data);

  /*!
   * \brief Remove players with certain roles.
   *
   * Remove all players that has roles specified by \a role. The removed objects
   * will be freed from memory.
   *
   * \return The roles of the removed data.
   */
  ZDocPlayer::TRole removePlayer(ZDocPlayer::TRole role);

  /*!
   * \brief Get all players with specific roles
   */
  QList<ZDocPlayer*> getPlayerList(ZDocPlayer::TRole role);

  QList<const ZDocPlayer*> getPlayerList(ZDocPlayer::TRole role) const;

  /*!
   * \brief Check if any player with specific roles exists
   */
  bool hasPlayer(ZDocPlayer::TRole role) const;

  void print() const;
};

/***************************************************/
class ZStroke2dPlayer : public ZDocPlayer
{
public:
  ZStroke2dPlayer();
  ZStroke2dPlayer(ZDocumentable* data, TRole role);

public:
  void labelStack(ZStack*stack) const;
  ZStack* toStack() const;
  int getLabel() const;
  QString getTypeName() const;

  ZStroke2d* getCompleteData() const;
};

/***************************************************/
class ZObject3dPlayer : public ZDocPlayer
{
public:
  ZObject3dPlayer();
  ZObject3dPlayer(ZDocumentable* data, TRole role);

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

  const ZObject3d *getCompleteData() const;

};

/***************************************************/
class ZSparseObjectPlayer : public ZDocPlayer
{
public:
  ZSparseObjectPlayer();
  ZSparseObjectPlayer(ZDocumentable* data, TRole role);

public:
  void labelStack(ZStack*stack) const;
  ZStack* toStack() const;

  ZSparseObject *getCompleteData() const;
};

/***************************************************/
class ZPlanePlayer : public ZDocPlayer
{
public:

};

#endif // ZDOCPLAYER_H
