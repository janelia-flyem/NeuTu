#include "flyemdatareader.h"

#include "dvid/zdvidreader.h"

#include "flyemdataconfig.h"

FlyEmDataReader::FlyEmDataReader()
{

}

FlyEmDataConfig FlyEmDataReader::readDataConfig(const ZDvidReader &reader)
{
  FlyEmDataConfig config;
  config.loadContrastProtocol(reader.readContrastProtocal());
  config.loadBodyStatusProtocol(reader.readBodyStatusV2());

  return config;
}
