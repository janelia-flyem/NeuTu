#ifndef ZDVIDTARGETFACTORY_H
#define ZDVIDTARGETFACTORY_H

#include <string>
#include <QString>

class ZDvidTarget;
class ZJsonObject;

class ZDvidTargetFactory
{
public:
  ZDvidTargetFactory();

  /*!
   * \brief Make DVID target from a source string
   *
   * A source string is an deprecated way of encoding a dvid target with the
   * format [scheme:]host:port::uuid[:segmentation][:grayscale].
   */
  static ZDvidTarget MakeFromSourceString(const std::string &sourceString);
  static ZDvidTarget MakeFromSourceString(const QString &sourceString);
  static ZDvidTarget MakeFromSourceString(const char *sourceString);

  /*!
   * \brief Make DVID target from URL
   *
   * The URL is supposed to be like
   * <scheme>://<host>:<port>?uuid=<>[&segmentation=<>][&grayscale=<>][&admintoken=<>]
   * , in which the order of the query strings does not matter.
   */
  static ZDvidTarget MakeFromUrlSpec(const std::string &urlSpec);
  static ZDvidTarget MakeFromUrlSpec(const QString &urlSpec);
  static ZDvidTarget MakeFromUrlSpec(const char *urlSpec);


  /*!
   * \brief A more general way of making DVID targret
   *
   * The url can take either a source string or a url and automatically figure
   * out how to parse the string.
   */
  static ZDvidTarget MakeFromSpec(const std::string &spec);
  static ZDvidTarget MakeFromSpec(const QString &spec);
  static ZDvidTarget MakeFromSpec(const char *spec);

  static ZDvidTarget Make(const ZJsonObject &obj);

  static ZDvidTarget MakeFromJsonSpec(const std::string &spec);

};

#endif // ZDVIDTARGETFACTORY_H
