#ifndef ZABSTRACTMODELPRESENTER_H
#define ZABSTRACTMODELPRESENTER_H

#include <QObject>
#include <QVector>
#include <QString>
#include <QVariant>

class ZAbstractModelPresenter : public QObject
{
  Q_OBJECT

public:
  explicit ZAbstractModelPresenter(QObject *parent = 0);
  virtual ~ZAbstractModelPresenter();

  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole ) const;
  virtual int columnCount() const;
  const QString& getColumnName(int index) const;

signals:

public slots:

protected:
  QVector<QString> m_fieldList;
  QString m_emptyField;
};

#endif // ZABSTRACTMODELPRESENTER_H
