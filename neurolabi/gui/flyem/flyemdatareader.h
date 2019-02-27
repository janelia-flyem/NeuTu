#ifndef FLYEMDATAREADER_H
#define FLYEMDATAREADER_H

class ZDvidReader;
class FlyEmDataConfig;

/*!
 * \brief The class for wrapping functions of reading flyem data.
 */
class FlyEmDataReader
{
public:
  FlyEmDataReader();

public:
  static FlyEmDataConfig readDataConfig(const ZDvidReader &reader);
};

#endif // FLYEMDATAREADER_H
