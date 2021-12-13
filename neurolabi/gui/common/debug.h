#ifndef _NEUTU_DEBUG_H
#define _NEUTU_DEBUG_H

#include <iostream>
#include <unordered_map>
#include <unordered_set>
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
  bool topicDisabled(const std::string &key) const;

private:
  mutable std::mutex m_mutex;
  std::vector<std::string> m_iconList{
    OUTPUT_HIGHLIGHT_1, OUTPUT_HIGHLIGHT_2, OUTPUT_HIGHLIGHT_3,
    OUTPUT_HIGHLIGHT_4
  };
  mutable std::unordered_map<std::string, std::string> m_iconMap;
  std::unordered_set<std::string> m_disabledTopicSet;
};


HighligthDebug& HLDebug(const std::string &key);

#ifdef _DEBUG_
#  define HLDEBUG(key) HLDebug(key)
#  define HLDEBUG_FUNC(key) HLDebug(key) << __PRETTY_FUNCTION__
#  define HLDEBUG_FUNC_LN(key) HLDEBUG_FUNC(key) << std::endl
#else
#  define HLDEBUG(key) if (1) {} else std::cout
#  define HLDEBUG_FUNC(key) if (1) {} else std::cout
#  define HLDEBUG_FUNC_LN(key) if (1) {} else std::cout
#endif

#endif // DEBUG_H
