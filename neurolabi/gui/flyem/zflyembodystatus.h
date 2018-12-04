#ifndef ZFLYEMBODYSTATUS_H
#define ZFLYEMBODYSTATUS_H

#include <string>

class ZJsonObject;

class ZFlyEmBodyStatus
{
public:
  ZFlyEmBodyStatus(const std::string &status = "");

  std::string getName() const;
  bool isAccessible() const;
  bool isExpertStatus() const;
  bool isFinal() const;
  int getProtectionLevel() const;
  int getPriority() const;

  void setProtectionLevel(int level);
  void setPriority(int p);
  void setExpert(bool on);
  void setFinal(bool on);

  std::string getStatusKey() const;

  static bool IsAccessible(const std::string &status);

  static std::string GetExpertStatus();

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void print() const;

  void reset();

public:
  static const char *KEY_NAME;
  static const char *KEY_PRIORITY;
  static const char *KEY_PROTECTION;
  static const char *KEY_EXPERT;
  static const char *KEY_FINAL;

private:
  std::string m_status;
  int m_priority = 999;
  int m_protection = 0; //0: visible to all
  bool m_isExpertStatus = false;
  bool m_isFinal = false;
};

#endif // ZFLYEMBODYSTATUS_H
