#ifndef ZDVIDANNOTATION_HPP
#define ZDVIDANNOTATION_HPP

#include "dvid/zdvidannotation.h"

template <typename InputIterator>
int ZDvidAnnotation::AddRelation(
    ZJsonObject &json, const InputIterator &first,
    const InputIterator &last, const std::string &rel)
{
  int count = 0;
  for (InputIterator iter = first; iter != last; ++iter) {
    if (AddRelation(json, *iter, rel)) {
      ++count;
    }
  }

  return count;
}

#endif // ZDVIDANNOTATION_HPP
