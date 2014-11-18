#ifndef ZSTACKDOCREADER_H
#define ZSTACKDOCREADER_H

#include <QString>
#include "zstackfile.h"

#include "zstackobjectgroup.h"
#include "zdocplayer.h"

class ZStack;
class ZSparseStack;
class ZSwcNetwork;

////////////////////////////////////////////////////
/// \brief The ZStackDocReader class
///
class ZStackDocReader {
public:
  ZStackDocReader();
  ~ZStackDocReader();

  bool readFile(const QString &filePath);
  void clear();
  void loadSwc(const QString &filePath);
  void loadLocsegChain(const QString &filePath);
  void loadStack(const QString &filePath);
  void loadSwcNetwork(const QString &filePath);
  void loadPuncta(const QString &filePath);

  inline ZStack* getStack() const { return m_stack; }
  inline ZSparseStack* getSparseStack() const { return m_sparseStack; }
  inline const ZStackFile& getStackSource() const { return m_stackSource; }

  inline const ZStackObjectGroup& getObjectGroup() const {
    return m_objectGroup;
  }

  inline const ZDocPlayerList& getPlayerList() const {
    return m_playerList;
  }

  bool hasData() const;
  inline const QString& getFileName() const {
    return m_filePath;
  }

  //void addPlayer(ZStackObject *obj, NeuTube::EDocumentableType type,
  //               ZDocPlayer::TRole role);
  void addPlayer(ZStackObject *obj);
  void addObject(ZStackObject *obj, bool uniqueSource = true);

public:
  void setStack(ZStack *stack);
  void setStackSource(const ZStackFile &stackFile);
  void setSparseStack(ZSparseStack *spStack);
private:
  QString m_filePath;

  //Main stack
  ZStack *m_stack;
  ZSparseStack *m_sparseStack;
  ZStackFile m_stackSource;

  ZStackObjectGroup m_objectGroup;
  ZDocPlayerList m_playerList;

  //Special object
  ZSwcNetwork *m_swcNetwork;
};

#endif // ZSTACKDOCREADER_H
