#include "zstackwriter.h"
#include "zfiletype.h"
#include "tz_image_io.h"
#include "c_stack.h"
#include "zstack.hxx"
#include "tz_constant.h"

ZStackWriter::ZStackWriter()
{
  m_compressHint = COMPRESS_DEFAULT;
}

void ZStackWriter::setCompressHint(ECompressHint compress)
{
  m_compressHint = compress;
}

void ZStackWriter::setMeta(const char *meta)
{
  m_meta = meta;
}

void ZStackWriter::setMeta(const std::string &meta)
{
  m_meta = meta;
}

const char* ZStackWriter::getMetaStr() const
{
  const char *metaStr = NULL;
  if (!m_meta.empty()) {
    metaStr = m_meta.c_str();
  }

  return metaStr;
}

int ZStackWriter::getTifCompressHint() const
{
  if (m_compressHint == COMPRESS_DEFAULT) {
    return 1;
  }

  return 0;
}

void ZStackWriter::write(const std::string &filePath, const Stack *stack)
{
  Write_Stack_U(filePath.c_str(), stack, getMetaStr(), getTifCompressHint());
}

void ZStackWriter::writeRaw(const std::string &filePath, const Mc_Stack *stack)
{
  if (stack == NULL) {
    return;
  }

  FILE *fp = fopen(filePath.c_str(), "w");
  if (fp != NULL) {
    int magicNumber = MRAW_MAGIC_NUMBER;
    int reserved = 0;
    fwrite(&magicNumber, 4, 1, fp);
    fwrite(&(stack->kind), 4, 1, fp);
    fwrite(&(stack->width), 4, 1, fp);
    fwrite(&(stack->height), 4, 1, fp);
    fwrite(&(stack->depth), 4, 1, fp);
    fwrite(&(stack->nchannel), 4, 1, fp);
    fwrite(&reserved, 4, 1, fp);
    size_t byteNumber = C_Stack::volumeByteNumber(stack);
    fwrite(stack->array, 1, byteNumber, fp);
    fclose(fp);
  }
}

void ZStackWriter::write(const std::string &filePath, const Mc_Stack *stack)
{
  if (stack == NULL) {
    return;
  }

  ZFileType::EFileType fileType = ZFileType::FileType(filePath) ;

  switch (fileType) {
  case ZFileType::MC_STACK_RAW_FILE:
    writeRaw(filePath, stack);
    break;
  default:
    Write_Mc_Stack(filePath.c_str(), stack, getMetaStr(), getTifCompressHint());
    break;
  }
}

void ZStackWriter::write(const std::string &filePath, const ZStack *stack)
{
  if (stack == NULL || filePath.empty()) {
    return;
  }

  if (!stack->hasData()) {
    return;
  }

  std::string resultFilePath = filePath;
  if ((stack->channelNumber() > 1 && stack->kind() != GREY && stack->kind() != GREY16) ||
      (stack->getVoxelNumber() - 1 > (size_t) MAX_INT32)) { //save as raw
    if (ZFileType::FileType(filePath) != ZFileType::V3D_RAW_FILE ||
        ZFileType::FileType(filePath) != ZFileType::MC_STACK_RAW_FILE) {
      std::cout << "Unsupported data format for " << resultFilePath << std::endl;
      resultFilePath += ".raw";
      std::cout << resultFilePath << " saved instead." << std::endl;
    }
  }

  ZStackWriter writer;
  writer.setCompressHint(m_compressHint);
  std::string meta;
  ZIntPoint offset = stack->getOffset();
  if (offset.getX() != 0 || offset.getY() != 0 || offset.getZ() != 0) {
    meta = "@offset ";
    meta += offset.toString();
  }
  writer.setMeta(meta);

  writer.write(filePath, stack->data());
}
