#include "zioutils.h"

#include "zutils.h"
#include <QFile>
#include <QTextStream>
#include <vector>

void openFileStream(std::ifstream& fs, const QString& filename, std::ios_base::openmode mode)
{
#ifdef _MSC_VER
  fs.open(filename.toStdWString().c_str(), mode);   // use msvc extension
#else
  fs.open(QFile::encodeName(filename).constData(), mode);
#endif
  if (!fs.is_open()) {
    throw ZIOException("Can not open file for reading.");
  }
}

void openFileStream(std::ofstream& fs, const QString& filename, std::ios_base::openmode mode)
{
#ifdef _MSC_VER
  fs.open(filename.toStdWString().c_str(), mode);   // use msvc extension
#else
  fs.open(QFile::encodeName(filename).constData(), mode);
#endif
  if (!fs.is_open()) {
    throw ZIOException("Can not open file for writing.");
  }
}

void readStream_impl(std::istream& fs, char* buf, size_t count)
{
#if defined(__APPLE__)
  if (count < 1024_usize * 1024 * 1024 * 2) {
    if (!fs.read(buf, count)) {
      throw ZIOException(QString("Expect %1 bytes, only read %2 bytes.").arg(count).arg(fs.gcount()));
    }
    return;
  }
  static size_t chunkSize = 1024_usize * 1024 * 1024;
  size_t bytesRemaining = count;
  while (bytesRemaining > 0) {
    size_t bytesToRead = std::min(bytesRemaining, chunkSize);
    if (!fs.read(buf, bytesToRead)) {
      throw ZIOException(QString("Expect %1 bytes, only read %2 bytes.").arg(bytesToRead).arg(fs.gcount()));
    }
    bytesRemaining -= bytesToRead;
    buf += bytesToRead;
  }
#else
  if (!fs.read(buf, count)) {
    throw ZIOException(QString("Expect %1 bytes, only read %2 bytes.").arg(count).arg(fs.gcount()));
  }
#endif
}

void writeStream_impl(std::ostream& fs, const char* buf, size_t count)
{
  if (!fs.write(buf, count)) {
    throw ZIOException("File write failed.");
  }
}

#ifdef _MSC_VER

std::unique_ptr<std::FILE, decltype(&std::fclose)> openFile(const QString& filename, const QString& mode)
{
  errno = 0;
  std::FILE *tmpf = nullptr;
  if (_wfopen_s(&tmpf, filename.toStdWString().c_str(), mode.toStdWString().c_str()) != 0) {
    throw ZIOException("Can not open file");
  }
  return std::unique_ptr<std::FILE, decltype(&std::fclose)>(tmpf, std::fclose);
}

#else

std::unique_ptr<std::FILE, decltype(&std::fclose)> openFile(const QString& filename, const char* mode)
{
  errno = 0;
  std::FILE* tmpf = std::fopen(QFile::encodeName(filename).constData(), mode);
  if (!tmpf) {
    throw ZIOException("Can not open file");
  }
  return std::unique_ptr<std::FILE, decltype(&std::fclose)>(tmpf, std::fclose);
}

#endif
