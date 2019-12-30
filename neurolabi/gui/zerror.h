#ifndef ZERROR_H
#define ZERROR_H

#include <iostream>
#include <sstream>

#include "neutubeconfig.h"
#include "common/neutudefs.h"

#define RECORD_ERROR(condition, msg)					\
{                                                       \
  if (condition) {                                      \
    std::ostringstream stream;                          \
    stream << "Error in the function " << __func__ ;		\
    std::ostringstream stream2;         \
    stream2 << __FILE__ << ":"  << __LINE__ << ": " << msg; \
    NeutubeConfig::getInstance().getMessageReporter()->report(\
      stream.str(), stream2.str(), neutu::EMessageType::ERROR); \
    std::cerr << msg << std::endl; \
  }									\
}

#define RECORD_ERROR_UNCOND(msg)					\
{                                                       \
  std::ostringstream stream;                          \
  stream << "Error in the function " << __func__ ;		\
  std::ostringstream stream2;         \
  stream2 << __FILE__ << ":"  << __LINE__ << ": " << msg; \
  NeutubeConfig::getInstance().getMessageReporter()->report(\
  stream.str(), stream2.str(), neutu::EMessageType::ERROR); \
  std::cerr << msg << std::endl; \
}

#define PROCESS_ERROR(condition, msg, action)					\
{                                                       \
  if (condition) {                                      \
    std::cerr << msg << std::endl; \
    std::ostringstream stream;                          \
    stream << "Error in the function " << __func__;		\
    std::ostringstream stream2;         \
    stream2 << __FILE__ << ":"  << __LINE__ << ": " << msg; \
    NeutubeConfig::getInstance().getMessageReporter()->report(\
      stream.str(), stream2.str(), neutu::EMessageType::ERROR); \
    action; \
  }									\
}

#define RECORD_WARNING_UNCOND(msg) \
{ \
  std::ostringstream stream;                          \
  stream << "Warning in the function " << __func__;		\
  std::ostringstream stream2;         \
  stream2 << __FILE__ << ":"  << __LINE__ << ": " << msg; \
  NeutubeConfig::getInstance().getMessageReporter()->report(\
    stream.str(), stream2.str(), neutu::EMessageType::WARNING); \
  std::cerr << msg << std::endl; \
}

#define RECORD_WARNING(condition, msg)					\
{                                                       \
  if (condition) {                                      \
    RECORD_WARNING_UNCOND(msg); \
  }									\
}

#define PROCESS_WARNING(condition, msg, action)					\
{  \
  if (condition) {  \
   RECORD_WARNING_UNCOND(msg); \
    action; \
  } \
}

#define RECORD_TITLED_INFORMATION(title, msg) \
{ \
  NeutubeConfig::getInstance().getMessageReporter()->report(\
    title, msg, neutu::EMessageType::INFORMATION); \
}

#define RECORD_INFORMATION(msg) \
{ \
  std::ostringstream stream;                          \
  stream << __FILE__ << ":"  << __func__ << ": ";		\
  NeutubeConfig::getInstance().getMessageReporter()->report(\
    stream.str(), msg, neutu::EMessageType::INFORMATION); \
}

#endif // ZERROR_H
