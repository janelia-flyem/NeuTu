#ifndef ZMESHIO_H
#define ZMESHIO_H

#include "z3dgl.h"
#include <QStringList>
#include <vector>
#include "common/neutudefs.h"
#include "assimp/scene.h"

class QByteArray;
class ZMesh;

namespace Assimp {
class Importer;
}

namespace draco {
class Mesh;
}

class ZMeshIO
{
public:
  static ZMeshIO& instance();

  ZMeshIO();

  bool canReadFile(const QString& filename);

  bool canWriteFile(const QString& filename);

  const QString& getQtReadNameFilter() const
  { return m_readFilter; }

  void getQtWriteNameFilter(QStringList& filters, QList<std::string>& formats);

  void load(const QString& filename, ZMesh& mesh) const;
  void save(const ZMesh& mesh, const QString& filename, std::string format) const;

  QByteArray writeToMemory(const ZMesh& mesh, std::string format) const;

  void loadFromMemory(
      const QByteArray &buffer, ZMesh &mesh, const std::string &format) const;
  ZMesh* loadFromMemory(
      const QByteArray &buffer, const std::string &format) const;

  static draco::Mesh *ToDracoMesh(const ZMesh &zmesh, draco::Mesh *dmesh);

private:
  void readAllenAtlasMesh(
      const QString& filename, std::vector<glm::vec3>& normals,
      std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices) const;
  void readDracoMesh(const QString& filename, ZMesh& mesh) const;
  void writeDracoMesh(const QString& filename, const ZMesh &mesh) const;
  ATTRIBUTE_NO_SANITIZE_ADDRESS
  void readDracoMeshFromMemory(const char *data, size_t size, ZMesh &mesh) const;
  void loadMesh(const aiScene *scene, ZMesh &mesh) const;
  void initImporter(Assimp::Importer &importer) const;

private:
  QStringList m_readExts;
  QStringList m_writeExts;
  QString m_readFilter;
  QStringList m_writeFilters;
  QList<std::string> m_writeFormats;
};

#endif // ZMESHIO_H
