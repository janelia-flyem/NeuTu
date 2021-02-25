#ifndef ZSTACKDOCOBJECTMONITOR_H
#define ZSTACKDOCOBJECTMONITOR_H

#include <QObject>
#include <functional>

class ZStackObjectInfoSet;
class ZStackDoc;

class ZStackDocObjectMonitor : public QObject
{
  Q_OBJECT
public:
  explicit ZStackDocObjectMonitor(QObject *parent = nullptr);

  void monitor(ZStackDoc *doc);

signals:

private slots:
  void processObjectModified(const ZStackObjectInfoSet &infoSet);

private:
  std::function<void(const ZStackObjectInfoSet &infoSet)>
  m_processObjectModified;
};

#endif // ZSTACKDOCOBJECTMONITOR_H
