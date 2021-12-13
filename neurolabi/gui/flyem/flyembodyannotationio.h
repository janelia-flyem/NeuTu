#ifndef FLYEMBODYANNOTATIONIO_H
#define FLYEMBODYANNOTATIONIO_H

#include <cstdint>
#include <utility>
#include <vector>
#include <functional>

class ZJsonObject;

class FlyEmBodyAnnotationIO
{
public:
  FlyEmBodyAnnotationIO();
  virtual ~FlyEmBodyAnnotationIO() {}

  virtual ZJsonObject readBodyAnnotation(uint64_t bodyId) = 0;
  virtual void deleteBodyAnnotation(uint64_t bodyId) = 0;
  virtual void writeBodyAnnotation(uint64_t bodyId, const ZJsonObject &obj) = 0;
  virtual bool hasBodyAnnotation(uint64_t bodyId) = 0;

  /*!
   * \brief Read a list of body annotations.
   *
   * The returned vector has one-to-one correspondence to \a bodyIds until it
   * reaches the end. i.e. if the returned vector is shorter than the input
   * vector, its elements correspond to those of \a bodyIds one by one from
   * start to end.
   */
  virtual std::vector<ZJsonObject> readBodyAnnotations(
      const std::vector<uint64_t> &bodyIds);

  /*!
   * \brief Write body annotations.
   *
   * It tries to write a list of body annotations and any failed writing will be
   * process by \a handleError if it is not null, which takes the failed body ID
   * and error message as its parameters.
   */
  void writeBodyAnnotations(
      const std::vector<std::pair<uint64_t, ZJsonObject>> &annotations,
      std::function<void(uint64_t, const std::string&)> handleError = nullptr);
};

#endif // FLYEMBODYANNOTATIONIO_H
