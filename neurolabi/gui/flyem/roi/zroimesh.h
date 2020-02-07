#ifndef ZROIMESH_H
#define ZROIMESH_H

#include <string>
#include <memory>

class ZMesh;

/*!
 * \brief The class for modeling ROI mesh
 *
 * A ROI mesh is a mesh that has the following properties:
 *  > a ROI name
 *  > a unique map from the ROI name to the mesh source name
 *  > a state to distinguish the mesh has been loaded
 */
class ZRoiMesh
{
public:
  ZRoiMesh();
  ~ZRoiMesh();

  enum class EStatus {
    READY, EMPTY, PENDING
  };

  void setMesh(ZMesh *mesh);
  void setMesh(const std::shared_ptr<ZMesh> &mesh);
  void setMesh(const std::string &name, const std::shared_ptr<ZMesh> &mesh);
  void setMesh(const std::string &name, ZMesh *mesh);
  ZMesh* getMesh() const;
  ZMesh* makeMesh() const;

  std::string getName() const;
  std::string getSourceName() const;
  void setName(const std::string &name);

  bool isLoaded() const;
  bool isVisible() const;

  EStatus getStatus() const;

private:
  void updateMeshSource();

private:
  std::string m_name;
  std::shared_ptr<ZMesh> m_mesh;
};

#endif // ZROIMESH_H
