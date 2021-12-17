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

class HighlightDebug {
public:
  HighlightDebug();

  static HighlightDebug& GetInstance() {
    static HighlightDebug hd;

    return hd;
  }

  std::string getIcon(const std::string &topic) const;

  template<typename T>
  HighlightDebug& operator<< (T &&v) {
    if (m_isActive) {
      std::cout << std::forward<T>(v);
    }
    return *this;
  }

  HighlightDebug& operator <<(std::ostream& (*os)(std::ostream&));

  /*!
   * \brief Set topic filter
   *
   * It uses a simple format to set up a filter:
   * 1. Topics are semi-colon separated;
   * 2. If \a filter is prefixed with '~', then it means disabled topics.
   *
   * For simplicity, neither ';' nor '~' is expected in a topic.
   *
   * This function will clear cached icon mapping.
   */
  void setTopicFilter(const std::string &filter);

  /*!
   * \brief Activate output
   *
   * Debugging messages will not be output to the screen if the activation
   * flag is off.
   */
  void activate(bool on);

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
  std::unordered_set<std::string> m_enabledTopicSet;
  bool m_isActive = true;
};


HighlightDebug& HLDebug(const std::string &key);

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
