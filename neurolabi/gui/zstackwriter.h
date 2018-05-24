#ifndef ZSTACKWRITER_H
#define ZSTACKWRITER_H

#include <string>

#include "tz_image_lib_defs.h"

class ZStack;

class ZStackWriter
{
public:
  ZStackWriter();

  enum ECompressHint {
    COMPRESS_NONE = 0, COMPRESS_DEFAULT = 1
  };

  void write(const std::string &filePath, const Stack *stack);
  void write(const std::string &filePath, const Mc_Stack *stack);
  void write(const std::string &filePath, const ZStack *stack);

  void setCompressHint(ECompressHint compress);
  void setMeta(const std::string &meta);
  void setMeta(const char* meta);

  void writeRaw(const std::string &filePath, const Mc_Stack *stack);

private:
  const char* getMetaStr() const;
  int getTifCompressHint() const;

private:
  ECompressHint m_compressHint;
  std::string m_meta;
};

#endif // ZSTACKWRITER_H
