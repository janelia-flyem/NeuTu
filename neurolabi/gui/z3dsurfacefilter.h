#ifndef Z3DSURFACEFILTER_H
#define Z3DSURFACEFILTER_H

#include <QObject>
#include <vector>
#include "z3dgeometryfilter.h"
#include "tz_geo3d_scalar_field.h"
#include "tz_graph.h"
#include "zwidgetsgroup.h"
#include "z3dcuberenderer.h"

class ZObject3d;

//
class Z3DSurfaceFilter : public Z3DGeometryFilter
{
    Q_OBJECT

public:
    explicit Z3DSurfaceFilter();
    virtual ~Z3DSurfaceFilter();

    virtual void initialize();
    virtual void deinitialize();

    void prepareData();
    void addData(const Z3DCube &cube);
    void addData(ZCubeArray *cubes);
    void clearData();
    void clearSources();

    std::vector<double> boundBox();

    virtual void process(Z3DEye);

    virtual void render(Z3DEye eye);

    ZWidgetsGroup *getWidgetsGroup();

    bool isReady(Z3DEye eye) const;

    void setVisible(bool v);
    bool isVisible() const;

    void initRenderers(size_t n);
    void invalidateRenderer(const std::string &source);

signals:
    void opacityValueChanged(double);


public slots:
    void updateSurfaceVisibleState();
    void indicateOpacityChanged(double v);

private:
    ZBoolParameter m_showCube;

    std::vector<Z3DCube> m_cubeArray;
    std::vector<ZCubeArray> m_cubeArrayList;
    Z3DCubeRenderer *m_cubeRenderer;
    std::vector<std::string> m_sourceList;
    std::vector<Z3DCubeRenderer*> m_cubeRenderers;
    std::vector<bool> m_renderCubes;

    bool m_dataIsInvalid;
    bool m_initialized;

    ZWidgetsGroup *m_widgetsGroup;
    size_t m_nSources;
    bool m_sourceSet;

};

#endif // Z3DSURFACEFILTER_H
