#ifndef BODYPREFETCHQUEUE_H
#define BODYPREFETCHQUEUE_H

#include <QObject>
#include <QMutex>
#include <QQueue>
#include <QSet>
#include <QWaitCondition>

#include "dvid/zdvidreader.h"

class ZFlyEmBody3dDoc;

class BodyPrefetchQueue : public QObject
{
    Q_OBJECT
public:
    explicit BodyPrefetchQueue(QObject *parent = nullptr);
    ~BodyPrefetchQueue() override;

    uint64_t get();
    bool isEmpty();

    void setDocument(ZFlyEmBody3dDoc *doc);

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

    ZFlyEmBody3dDoc *m_doc = NULL;
    ZDvidReader m_reader;
};

#endif // BODYPREFETCHQUEUE_H
