#ifndef BODYPREFETCHQUEUE_H
#define BODYPREFETCHQUEUE_H

#include <QObject>

class BodyPrefetchQueue : public QObject
{
    Q_OBJECT
public:
    explicit BodyPrefetchQueue(QObject *parent = nullptr);

signals:

public slots:
};

#endif // BODYPREFETCHQUEUE_H