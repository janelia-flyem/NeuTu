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

protected:
  void info(const std::string &title, const std::string &description) const;
  void warn(const std::string &title, const std::string &description) const;
  void error(const std::string &title, const std::string &description) const;
  void debug(const std::string &title, const std::string &description) const;

private:
  std::string composeMessage(
      const std::string &title, const std::string &description) const;

private:
  bool m_forceUpdate;
};

#endif // ZCOMMANDMODULE_H
