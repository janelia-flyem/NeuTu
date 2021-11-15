#ifndef _NEUTU_DEBUG_H
#define _NEUTU_DEBUG_H

#include <iostream>
#include <unordered_map>
#include <mutex>
#include <vector>

#define OUTPUT_HIGHLIGHT_1 "♦️ "
#define OUTPUT_HIGHLIGHT_2 "🦋 "
#define OUTPUT_HIGHLIGHT_3 "🙏 "
#define OUTPUT_HIGHLIGHT_4 "👉 "

class HighligthDebug {
public:
  HighligthDebug();

  static HighligthDebug& GetInstance() {
    static HighligthDebug hd;

    return hd;
  }

  std::string getIcon(const std::string &key) const;

  template<typename T>
  HighligthDebug& operator<< (T &&v) {
    std::cout << std::forward<T>(v);
    return *this;
  }

  HighligthDebug& operator <<(std::ostream& (*os)(std::ostream&));

private:
  mutable std::mutex m_mutex;
  std::vector<std::string> m_iconList{
    OUTPUT_HIGHLIGHT_1, OUTPUT_HIGHLIGHT_2, OUTPUT_HIGHLIGHT_3,
    OUTPUT_HIGHLIGHT_4
  };
  mutable std::unordered_map<std::string, std::string> m_iconMap;
};


HighligthDebug& HLDebug(const std::string &key);

#endif // DEBUG_H
