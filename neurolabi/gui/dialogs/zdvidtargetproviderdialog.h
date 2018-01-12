#ifndef ZDVIDTARGETPROVIDERDIALOG_H
#define ZDVIDTARGETPROVIDERDIALOG_H

#include <QObject>
#include <QDialog>

#include "dvid/zdvidtarget.h"

class ZDvidTargetProviderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ZDvidTargetProviderDialog(QWidget *parent = 0);
    ~ZDvidTargetProviderDialog();

    virtual ZDvidTarget& getDvidTarget() = 0;

};

#endif // ZDVIDTARGETPROVIDERDIALOG_H
