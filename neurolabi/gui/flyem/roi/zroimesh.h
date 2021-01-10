#ifndef ZROIMESH_H
#define ZROIMESH_H

#include <string>
#include <memory>

#include <QColor>

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
  ZRoiMesh(const std::string &name);
  ZRoiMesh(const std::string &name, const QColor &color);
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

  QColor getColor() const;
  void setColor(const QColor &color);

  bool isLoaded() const;
  bool isVisible() const;
  void setVisible(bool on);

  EStatus getStatus() const;

private:
  void updateMeshProperty();

private:
  std::string m_name;
  QColor m_color;
  bool m_isVisible = false;
  std::shared_ptr<ZMesh> m_mesh;
};

#endif // ZROIMESH_H
