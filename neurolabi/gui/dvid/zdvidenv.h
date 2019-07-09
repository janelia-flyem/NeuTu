#ifndef ZDVIDENV_H
#define ZDVIDENV_H

#include <map>
#include <vector>

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

  enum class ERole {
    GRAYSCALE, SEGMENTATION
  };

  bool isValid() const;

  void clear();

  void set(const ZDvidTarget &target);

  void loadJsonObject(const ZJsonObject &obj);
  ZJsonObject toJsonObject() const;


  ZDvidTarget& getMainTarget();
  std::vector<ZDvidTarget>& getTargetList(ERole role);

  const ZDvidTarget& getMainTarget() const;
  const std::vector<ZDvidTarget>& getTargetList(ERole role) const;

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
  std::map<ERole, std::vector<ZDvidTarget>> m_targetMap;

  static const char *KEY_GRAYSCALE;
  static const char *KEY_SEGMENTATION;
};

#endif // ZDVIDENV_H
