#ifndef TODOSEARCHER_H
#define TODOSEARCHER_H

#include <QObject>

#include "dvid/zdvidreader.h"

#include "zjsonobject.h"

class ToDoSearcher : public QObject
{
    Q_OBJECT
public:
    explicit ToDoSearcher(QObject *parent = nullptr);

    static const std::string KEY_BODYID;

    void setBodyID(uint64_t bodyID);
    void search(ZDvidReader reader);
    ZJsonObject toJson();
    void fromJson(ZJsonObject object);

signals:

public slots:

private:
    uint64_t m_bodyID;

};

#endif // TODOSEARCHER_H
