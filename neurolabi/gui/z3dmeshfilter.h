#ifndef Z3DMESHFILTER_H
#define Z3DMESHFILTER_H

#include <map>
#include <vector>

#include "z3dgeometryfilter.h"
#include "widgets/zoptionparameter.h"
#include "widgets/zwidgetsgroup.h"
#include "widgets/znumericparameter.h"
#include "z3dmeshrenderer.h"
#include "zeventlistenerparameter.h"
#include "zstringutils.h"

class Z3DMeshFilter : public Z3DGeometryFilter
{
Q_OBJECT
public:
  explicit Z3DMeshFilter(Z3DGlobalParameters& globalParas, QObject* parent = nullptr);

  void setMeshColor(const glm::vec4& col)
  { m_singleColorForAllMesh.set(col); }

  /*
  bool isFixed() const
  { return m_meshList[0]->numVertices() == 96957; }
  */

  virtual void process(Z3DEye eye) override;

  void setData(const std::vector<ZMesh*>& meshList);
  void setData(const QList<ZMesh*>& meshList);

  virtual bool isReady(Z3DEye eye) const override;

  std::shared_ptr<ZWidgetsGroup> widgetsGroup();

  virtual void renderOpaque(Z3DEye eye) override;

  virtual void renderTransparent(Z3DEye eye) override;

  void updateMeshVisibleState();

  ZBBox<glm::dvec3> meshBound(const ZMesh *p);

  void setColorMode(const std::string &mode);

  void enablePreservingSourceColors(bool on);
  bool preservingSourceColorsEnabled() const;

  void showSourceColors(bool show);
  bool showingSourceColors() const;

  bool hitObject(int x, int y);

  /*!
   * \brief hitObject
   * \param ptArray
   * \return
   */
  std::vector<bool> hitObject(const std::vector<std::pair<int, int> > &ptArray);

  ZMesh* hitMesh(int x, int y);

  // Meshes not mentioned in meshIdToColorIndex will get indexedColors[0].
  void setColorIndexing(const std::vector<glm::vec4> &indexedColors,
                        const std::map<uint64_t, std::size_t> &meshIdToColorIndex);

  // The function (which can be a lambda) returns the color index to used for the mesh ID.
  void setColorIndexing(const std::vector<glm::vec4> &indexedColors,
                        std::function<std::size_t(uint64_t)> meshIdToColorIndexFunc);

  void emitDumpParaGarbage();

public slots:
  void dumpParamGarbage();

signals:
  void meshSelected(ZMesh*, bool append);
  void clearingParamGarbage();
  void selectionRectStarted(int x, int y);
  void selectionRectChanged(int x, int y);
  void selectionRectEnded();

protected:
  void prepareColor();

  void adjustWidgets();

  void selectMesh(QMouseEvent* e, int w, int h);

  void onApplyTransform();

  virtual void renderPicking(Z3DEye eye) override;

  void prepareData();

  virtual void registerPickingObjects() override;

  virtual void deregisterPickingObjects() override;

  //virtual void updateAxisAlignedBoundBoxImpl() override;
  virtual void updateNotTransformedBoundBoxImpl() override;

  virtual void addSelectionLines() override;

private:
  // get visible data from m_origMeshList put into m_meshList
  void getVisibleData();
  void removeOldColorParameter(
      const std::set<QString, QStringNaturalCompare> &allSources);
  void addNewColorParameter(
      const std::set<QString, QStringNaturalCompare> &allSources);
  void addSourceColor(const QString &source);
  void printColorMapper();
  void updateSourceColorMapper();
  bool sourceChanged(
      const std::set<QString, QStringNaturalCompare> &allSources) const;
  void removeSourceColorWidget();
  void addSourceColorWidget();
  void updateWidgetGroup();

private slots:
  void processColorModeChange();

private:
  Z3DMeshRenderer m_meshRenderer;

  ZStringIntOptionParameter m_colorMode;
  ZVec4Parameter m_singleColorForAllMesh;
  std::map<QString, std::shared_ptr<ZVec4Parameter>, QStringNaturalCompare>
  m_sourceColorMapper;
  std::vector<std::shared_ptr<ZVec4Parameter>> m_paramGarbage;
  bool m_preserveSourceColors;
  bool m_showSourceColors;

  // mesh list used for rendering, it is a subset of m_origMeshList. Some mesh are
  // hidden because they are unchecked from the object model. This allows us to control
  // the visibility of each single mesh.
  std::vector<ZMesh*> m_meshList;
  std::vector<ZMesh*> m_registeredMeshList;    // used for picking

  std::vector<glm::vec4> m_meshColors;
  std::vector<glm::vec4> m_meshPickingColors;

  ZEventListenerParameter m_selectMeshEvent;

  class SelectionStateMachine;
  std::shared_ptr<SelectionStateMachine> m_selectionStateMachine;

  // generate and save to speed up bound box rendering for big mesh
  std::map<const ZMesh*, ZBBox<glm::dvec3>> m_meshBoundboxMapper;

  std::shared_ptr<ZWidgetsGroup> m_widgetsGroup;
  bool m_dataIsInvalid;

  std::vector<ZMesh*> m_origMeshList;

  std::vector<glm::vec4> m_indexedColors;
  std::map<uint64_t, std::size_t> m_meshIDToColorIndex;
  std::function<std::size_t(uint64_t)> m_meshIDToColorIndexFunc = nullptr;
};

#endif // Z3DMESHFILTER_H
