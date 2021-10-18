#ifndef FLYEMSYNAPSEDVIDSOURCE_H
#define FLYEMSYNAPSEDVIDSOURCE_H

#include "flyemsynapsesource.h"
#include "dvid/zdvidwriter.h"

class FlyEmSynapseDvidSource : public FlyEmSynapseSource
{
public:
  FlyEmSynapseDvidSource();

  void setDvidTarget(const ZDvidTarget &target);

  std::vector<ZDvidSynapse> getData(const ZIntCuboid &box) const override;
  ZIntCuboid getRange() const override;

  void saveItem(const ZDvidSynapse &item) override;
  void removeItem(const ZIntPoint &pos) override;
  void updatePartner(ZDvidSynapse *item) const override;
//  void updateItem(ZDvidSynapse *item) const override;
  ZDvidSynapse getItem(const ZIntPoint &pos) const override;
  void moveItem(const ZIntPoint &from, const ZIntPoint &to) override;

private:
  ZDvidWriter m_writer;
};

#endif // FLYEMSYNAPSEDVIDSOURCE_H
