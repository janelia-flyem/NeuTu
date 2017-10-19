#ifndef BODYPREFETCHQUEUE_H
#define BODYPREFETCHQUEUE_H

#include <QObject>
#include <QAtomicInteger>
#include <QSet>

class BodyPrefetchQueue : public QObject
{
    Q_OBJECT
public:
    explicit BodyPrefetchQueue(QObject *parent = nullptr);

    uint64_t get();
    bool isEmpty();

signals:

public slots:    
    void add(uint64_t bodyID);
    void add(QSet<uint64_t> bodyIDs);
    void remove(QSet<uint64_t> bodyIDs);
    void clear();

private:
    QAtomicInteger<uint64_t> m_bodyID;
};

#endif // BODYPREFETCHQUEUE_H
