#ifndef ZMESSAGEFACTORY_H
#define ZMESSAGEFACTORY_H

class ZMessage;
class QWidget;

class ZMessageFactory
{
public:
  ZMessageFactory();

  static ZMessage Make3DVisMessage(QWidget *source);
  static void Make3DVisMessage(ZMessage &message);
  static void MakeFlyEmSplit3DVisMessage(ZMessage &message);
  /*!
   * \brief Make quick 3dvis message
   *
   * \param coarseLevel higher value means more more coarse
   */
  static void MakeQuick3DVisMessage(ZMessage &message, int coarseLevel);
};

#endif // ZMESSAGEFACTORY_H
