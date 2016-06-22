#ifndef ZSTRING_H
#define ZSTRING_H

#include <string>
#if defined(_QT_GUI_USED_)
#include <QString>
#endif
#include <vector>

#include "tz_stdint.h"
#include "tz_string.h"

class ZString : public std::string
{
public:
  ZString();
  ZString(const ZString &str);
  ZString(const std::string& str);
  ZString ( const std::string& str, size_t pos, size_t n = npos );
  ZString ( const char * s, size_t n );
  ZString ( const char * s );
  ZString ( size_t n, char c );
#if defined(_QT_GUI_USED_)
  ZString(const QString &str);
#endif
  template<class InputIterator> ZString (
      InputIterator begin, InputIterator end) : std::string(begin, end)
  {
    init();
  }
  ~ZString();

public:
  static const char FileSeparator;

  enum ECaseSensitivity {
    CASE_SENSITIVE, CASE_INSENSITIVE
  };

  inline void init() { m_workspace = New_String_Workspace(); }

  static int firstInteger(const std::string &str);
  int firstInteger();
  int lastInteger();
  /*!
   * \brief Extract the last integer in a string
   * \param Input string.
   * \return Returns the last integer of \a str. It returns 0 if there is no
   *         integer in the string.
   */
  static int lastInteger(const std::string &str);
  double firstDouble();
  double lastDouble();
  uint64_t firstUint64();
  std::string firstQuotedWord();

  std::vector<int> toIntegerArray();
  std::vector<uint64_t> toUint64Array();
  std::vector<double> toDoubleArray();
  std::vector<std::string> toWordArray(const std::string &delim = ", \n");
  std::vector<std::string> tokenize(char c);

  bool readLine(FILE *fp);
  bool contains(const std::string &str);
  bool containsDigit();
  std::string& replace(const std::string &from, const std::string &to);
  std::string& replace(int from, const std::string &to);
  bool startsWith(const std::string &str, ECaseSensitivity cs = CASE_SENSITIVE) const;
  bool endsWith(const std::string &str, ECaseSensitivity cs = CASE_SENSITIVE) const;
  void trim();

  void toLower();
  void toUpper();

  std::string lower() const;
  std::string upper() const;

  void appendNumber(int num, int pad = 0);
  void appendNumber(uint64_t num, int pad = 0);

  /*!
   * \brief The directory path of a file
   *
   * The function assumes that \a path is a file path.
   */
  static ZString dirPath(const std::string &path);
  ZString dirPath();

  bool isAbsolutePath() const;
  static bool isAbsolutePath(const std::string &path);
  static std::string absolutePath(const std::string &dir,
                                  const std::string &relative);
  bool isRemotePath() const;

  /*!
   * \brief Get the relative path
   */
  static std::string relativePath(
      const std::string &path, const std::string &reference);

  static std::string fullPath(const std::string &dir, const std::string &fname,
                              const std::string &ext = "");
  static std::string fullPath(const std::string &dir, const std::string &part1,
                              const std::string &part2, const std::string &part3);
  static std::string fullPath(const std::vector<std::string> &parts);
  static std::string removeFileExt(const std::string &str);
  static std::string getBaseName(const std::string &str);

  ZString absolutePath(const std::string &dir) const;
  std::vector<std::string> fileParts() const;

  /*!
   * \brief Decompose a file path into node parts
   *
   * Example: "/home/test/data/test.tif" is decomposed into
   * {"/", "home", "test", "data", "test.tif"}
   */
  std::vector<std::string> decomposePath() const;
  static std::vector<std::string> decomposePath(const std::string &str);

  ZString toFileExt();
  ZString toFileName();

  ZString &operator= (const ZString &str);

  std::string changeExt(const std::string &newext) const;
  std::string changeDirectory(const std::string &newdir);

  inline const ZString& constRef() const {
    return *this;
  }

  static std::string num2str(int n);
  static std::string num2str(uint64_t n);

private:
  String_Workspace *m_workspace;
};

#endif // ZSTRING_H
