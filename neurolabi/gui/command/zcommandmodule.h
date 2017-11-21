#ifndef ZCOMMANDMODULE_H
#define ZCOMMANDMODULE_H

#include <vector>
#include <string>

class ZJsonObject;

class ZCommandModule
{
public:
  ZCommandModule();
  virtual ~ZCommandModule() {}

  virtual int run(
      const std::vector<std::string> &input, const std::string &output,
      const ZJsonObject &config);

  void setForceUpdate(bool on);
  bool forcingUpdate() const;

private:
  bool m_forceUpdate;
};

#endif // ZCOMMANDMODULE_H
