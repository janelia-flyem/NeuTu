#ifndef ZROIWIDGET_H
#define ZROIWIDGET_H

#include <QTableWidget>
#include <QDockWidget>

#include "flyem/zflyemproofdoc.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QPushButton;
class QTableWidget;
class QTableWidgetItem;
QT_END_NAMESPACE

//
class ZROIWidget : public QDockWidget
{
    Q_OBJECT

public:
    ZROIWidget(QWidget *parent = 0);
    ZROIWidget(const QString & title, QWidget * parent = 0, Qt::WindowFlags flags = 0);
    ~ZROIWidget();

public:
    void getROIs(Z3DWindow *window, const ZDvidInfo &dvidInfo, const ZDvidTarget &dvidTarget);
    void makeGUI();

public slots:
    void updateROIRendering(int row, int column);
    void updateROIRendering(QTableWidgetItem* item);

public:
    //
    Z3DWindow *m_window;
    ZDvidInfo m_dvidInfo;
    ZDvidTarget m_dvidTarget;
    ZDvidReader reader;

    //
    std::vector<std::string> roiList;

    //
    QTableWidget *tw_ROIs;
};


#endif // ZROIWIDGET_H
