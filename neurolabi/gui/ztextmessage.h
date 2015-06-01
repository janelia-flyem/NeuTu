#ifndef ZTEXTMESSAGE_H
#define ZTEXTMESSAGE_H

#include "neutube.h"
#include <QString>


class ZTextMessage
{
public:
  ZTextMessage();

private:
  NeuTube::EMessageType m_type;
  QString m_plainText;
};

#endif // ZTEXTMESSAGE_H
