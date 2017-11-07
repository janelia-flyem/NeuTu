#ifndef BODYPREFETCHQUEUE_H
#define BODYPREFETCHQUEUE_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSet>
#include <QWaitCondition>

class BodyPrefetchQueue : public QObject
{
    Q_OBJECT
public:
    explicit BodyPrefetchQueue(QObject *parent = nullptr);

    uint64_t get();
    bool isEmpty();

signals:
    void finished();

public slots:    
    void add(uint64_t bodyID);
    void add(QSet<uint64_t> bodyIDs);
    void remove(QSet<uint64_t> bodyIDs);
    void clear();
    void finish();

private:
    QQueue<uint64_t> m_queue;
    QMutex m_queueLock;
    QWaitCondition m_queueHasItems;

};

#endif // BODYPREFETCHQUEUE_H
