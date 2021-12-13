#ifndef FLYEMBATCHBODYANNOTATIONDIALOG_H
#define FLYEMBATCHBODYANNOTATIONDIALOG_H

#include <QCheckBox>

#include "zgenericbodyannotationdialog.h"

/*!
 * \brief The dialog for annotating bodies in batch
 */
class FlyEmBatchBodyAnnotationDialog : public ZGenericBodyAnnotationDialog
{
  Q_OBJECT
public:
  FlyEmBatchBodyAnnotationDialog(QWidget *parent = nullptr);

protected:
  void postProcess(ZJsonObject &obj) const override;

private:
  QCheckBox *m_ignoringEmptyField = nullptr;
};

#endif // FLYEMBATCHBODYANNOTATIONDIALOG_H
