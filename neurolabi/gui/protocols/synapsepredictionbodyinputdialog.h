#ifndef SYNAPSEPREDICTIONBODYINPUTDIALOG_H
#define SYNAPSEPREDICTIONBODYINPUTDIALOG_H

#include <QDialog>

namespace Ui {
class SynapsePredictionBodyInputDialog;
}

class SynapsePredictionBodyInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SynapsePredictionBodyInputDialog(QWidget *parent = 0);
    ~SynapsePredictionBodyInputDialog();

    uint64_t getBodyID();
    bool hasBodyID();
    std::string getMode();

private:
    Ui::SynapsePredictionBodyInputDialog *ui;
};

#endif // SYNAPSEPREDICTIONBODYINPUTDIALOG_H
