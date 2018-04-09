#ifndef ZMAINWINDOWCONTROLLER_H
#define ZMAINWINDOWCONTROLLER_H

#include <string>

class ZJsonObject;
class ZProofreadWindow;
class MainWindow;

/*!
 * \brief An experimental class for controlling GUI from the root level.
 */
class ZMainWindowController
{
public:
  ZMainWindowController();

public:
//  static void StartTestTask(const std::string &taskKey);
  static void StartTestTask(ZProofreadWindow *window);
  static void StartTestTask(MainWindow *mainWin);
};

#endif // ZMAINWINDOWCONTROLLER_H
