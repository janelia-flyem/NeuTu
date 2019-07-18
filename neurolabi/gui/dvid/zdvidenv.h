#ifndef ZDVIDENV_H
#define ZDVIDENV_H

#include <map>
#include <vector>
#include <string>

#include "zdvidtarget.h"

class ZJsonObject;

/*!
 * \brief The class for more flexible DVID settings compared to ZDvidTarget.
 *
 * This class wraps ZDvidTarget to facilitate loading data from multiple sources.
 * It has a main target that provides the main settings of the DVID data.
 * Only data specified in the main DVID target are subjected to modification.
 * Therefore, if it is set to proofreading certain segmentation data, the
 * corresponding segmentation name must be included in the main target.
 *
 */
class ZDvidEnv
{
public:
  ZDvidEnv();
  explicit ZDvidEnv(const ZDvidTarget &target);

  enum class ERole {
    GRAYSCALE, SEGMENTATION
  };

  bool isValid() const;

  void clear();

  void set(const ZDvidTarget &target);
  void setHost(const std::string &host);
  void setPort(int port);
  void setUuid(const std::string &uuid);
  void setSegmentation(const std::string &name);

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;

  void setReadOnly(bool on);

  void setMainTarget(const ZDvidTarget &target);

  ZDvidTarget& getMainTarget();
  std::vector<ZDvidTarget>& getTargetList(ERole role);

  const ZDvidTarget& getMainTarget() const;
  const std::vector<ZDvidTarget>& getTargetList(ERole role) const;
  const ZDvidTarget& getMainGrayscaleTarget() const;

  /*!
   * \brief Main target with main grayscale data if available
   */
  ZDvidTarget getFullMainTarget() const;

  void appendValidDvidTarget(const ZDvidTarget &target, ERole role);

private:
  void appendDvidTarget(
      std::vector<ZDvidTarget> &targetList, const ZJsonArray &arrayObj);

  /*!
   * \brief Enable a role in the env (for developers)
   *
   * When a new role is added to ERole, it must be passed into this function
   * in the constructor to make the role work properly.
   */
  void enableRole(ERole role);

private:
  ZDvidTarget m_mainTarget;
  ZDvidTarget m_emptyTarget;
  std::map<ERole, std::vector<ZDvidTarget>> m_targetMap;

  static const char *KEY_GRAYSCALE;
  static const char *KEY_SEGMENTATION;
};

#endif // ZDVIDENV_H
