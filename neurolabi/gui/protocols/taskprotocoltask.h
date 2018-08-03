#ifndef TASKPROTOCOLTASK_H
#define TASKPROTOCOLTASK_H

#include <QObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSet>

class QMenu;

class TaskProtocolTask: public QObject
{
    Q_OBJECT

public:
    TaskProtocolTask();
    virtual ~TaskProtocolTask() {}

    // Where the task JSON was loaded from, for logging purposes only.
    static void setJsonSource(const QString &source);
    static QString jsonSource();

    bool completed() const;
    void setCompleted(bool completed);
    const QSet<uint64_t> & visibleBodies();
    const QSet<uint64_t> & selectedBodies();

    virtual void beforeNext();
    virtual void beforePrev();
    virtual void onLoaded();
    virtual void beforeDone();

    bool loadJson(QJsonObject json);
    QJsonObject toJson();

    void addTag(QString tag);
    void removeTag(QString tag);
    bool hasTag(QString tag);
    QStringList getTags();
    void clearTags();

    virtual QString tasktype() = 0;
    virtual QString actionString() = 0;
    virtual QString targetString() = 0;    
    virtual bool skip();
    virtual QWidget * getTaskWidget();
    virtual QMenu * getTaskMenu();
    virtual bool usePrefetching();
    virtual bool allowCompletion();

signals:
    void bodiesUpdated();
    void browseGrayscale(double x, double y, double z, const QHash<uint64_t, QColor> &idToColor);
    void updateGrayscaleColor(const QHash<uint64_t, QColor>& idToColor);

protected:
    static const QString KEY_COMPLETED;
    static const QString KEY_TAGS;
    static const QString KEY_VISIBLE;
    static const QString KEY_SELECTED;

    bool m_completed;
    QSet<uint64_t> m_visibleBodies;
    QSet<uint64_t> m_selectedBodies;
    QSet<QString> m_tags;

    QString objectToString(QJsonObject json);

    void updateBodies(const QSet<uint64_t> &visible, const QSet<uint64_t> &selected);

private:
    bool loadStandard(QJsonObject json);
    virtual bool loadSpecific(QJsonObject json) = 0;
    virtual QJsonObject addToJson(QJsonObject json) = 0;
    virtual void onCompleted();
};

#endif // TASKPROTOCOLTASK_H
