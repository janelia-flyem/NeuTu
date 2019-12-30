#ifndef TODOREVIEWPROTOCOL_H
#define TODOREVIEWPROTOCOL_H

#include <QDialog>
#include <QStandardItemModel>

#include "protocoldialog.h"
#include "todosearcher.h"

#include "zjsonobject.h"


namespace Ui {
class ToDoReviewProtocol;
}

class ToDoReviewProtocol : public ProtocolDialog
{
    Q_OBJECT

public:
    explicit ToDoReviewProtocol(QWidget *parent = 0);
    ~ToDoReviewProtocol();   
    bool initialize();

signals:
    void protocolCompleting();
    void protocolExiting();
    void requestSaveProtocol(ZJsonObject data);

public slots:
    void loadDataRequested(ZJsonObject data);

private slots:
    void onExitButton();
    void onCompleteButton();

private:
    static const std::string KEY_VERSION;
    static const std::string KEY_PARAMETERS;
    static const int fileVersion;

    enum TodoTableColumns {
        ITEM_COLUMN,
        CHECKED_COLUMN
    };

    void saveState();
    void inputErrorDialog(QString message);

    Ui::ToDoReviewProtocol *ui;
    ToDoSearcher m_searcher;
    QStandardItemModel * m_sitesModel;
    // QList<ZIntPoint> m_pendingList;
    // QList<ZIntPoint> m_finishedList;
};

#endif // TODOREVIEWPROTOCOL_H
