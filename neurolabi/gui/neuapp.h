#ifndef NEUAPP_H
#define NEUAPP_H

#include <QApplication>

class NeuApp : public QApplication
{
  Q_OBJECT
public:
  NeuApp(int& argc, char** argv);
  bool notify(QObject* receiver, QEvent* event) override;
};

#endif // NEUAPP_H
