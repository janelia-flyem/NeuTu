#ifndef ZROIWIDGET_H
#define ZROIWIDGET_H

#include <QTableWidget>
#include <QDockWidget>

#include "flyem/zflyemproofdoc.h"
#include "zcolorscheme.h"
#include "zobjsmodel.h"
#include "z3dsurfacefilter.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QSpinBox;
class QLabel;
class QDoubleSpinBox;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

//
class ZROIObjsModel : public ZObjsModel
{
    Q_OBJECT
public:
    ZROIObjsModel(QObject *parent = 0);
    ~ZROIObjsModel();

public slots:

protected:
    virtual void setModelIndexCheckState(const QModelIndex &index, Qt::CheckState cs);
    virtual bool needCheckbox(const QModelIndex &index) const;

};

//
class ZROIWidget : public QDockWidget
{
    Q_OBJECT

public:
    ZROIWidget(QWidget *parent = 0);
    ZROIWidget(const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~ZROIWidget();

public:
    void loadROIs(Z3DWindow *window,
//                  const ZDvidInfo &dvidInfo,
                  std::vector<std::string> roiList,
                  std::vector<ZSharedPointer<ZMesh> > loadedROIs,
                  std::vector<std::string> roiSourceList);
    void loadROIs(std::vector<std::string> roiList,
                  std::vector<ZSharedPointer<ZMesh> > loadedROIs,
                  std::vector<std::string> roiSourceList);

    Z3DWindow* getParentWindow() const {
      return m_window;
    }

signals:
    void toBeClosed();

public slots:
    void updateROIs();
    void updateROISelections(int row, int column);
    void updateROIColors(int row, int column);
    void updateROIRendering(QTableWidgetItem* item);
    void updateSelection();
    void updateROISelections(QModelIndex idx);
    void updateSelectedROIs();
    void updateOpacity(double v);
    void updateSlider(int v);


protected:
    void closeEvent(QCloseEvent * event);

private:
    int getDsIntv() const;
    void makeGUI();
    void setCheckStatus(int row, bool on);
    void toggleCheckStatus(int row);
    void updateRoiTable();
    void updateOpacityLabel(double v);

public:
    //
    Z3DWindow *m_window;
//    ZDvidInfo m_dvidInfo;

    //
    std::vector<std::string> m_roiList;
    std::vector<ZSharedPointer<ZMesh> > m_loadedROIs;
    QColor m_defaultColor;
    std::vector<std::string> m_roiSourceList;
    std::vector<bool> m_colorModified;
    std::vector<bool> m_checkStatus;

    //
    QCheckBox *m_selectAll;

//    QSpinBox *m_dsIntvWidget = NULL;
//    QPushButton *m_updateButton;
    QLabel *m_opacityLabel;
    QSlider *m_opacitySlider;
    QTableWidget *tw_ROIs;

    //
//    ZROIObjsModel *m_objmodel;
};


#endif // ZROIWIDGET_H
