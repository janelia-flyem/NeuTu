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
    void getROIs(Z3DWindow *window,
                 ZDvidInfo &dvidInfo,
                 std::vector<std::string> roiList,
                 std::vector<ZObject3dScan> loadedROIs,
                 std::vector<std::string> roiSourceList);
    void makeGUI();

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

public:
    //
    Z3DWindow *m_window;
    ZDvidInfo m_dvidInfo;

    //
    std::vector<std::string> m_roiList;
    std::vector<ZObject3dScan> m_loadedROIs;
    QColor defaultColor;
    std::vector<std::string> m_roiSourceList;
    std::vector<bool> colorModified;
    std::vector<bool> m_checkStatus;

    //
    QCheckBox *selectAll;

    QSpinBox *m_dsIntvWidget;
    QPushButton *m_updateButton;
    QLabel *l_opacity;
    QSlider *s_opacity;
    QTableWidget *tw_ROIs;

    //
    ZROIObjsModel *m_objmodel;
};


#endif // ZROIWIDGET_H
