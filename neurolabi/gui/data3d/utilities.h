#ifndef DATA3D_UTILITIES_H
#define DATA3D_UTILITIES_H

#include <string>
#include <vector>
#include <set>
#include <functional>
#include <iostream>

#include "defs.h"

namespace neutu {

std::string ToString(neutu::data3d::ETarget target);

namespace data3d {

std::vector<ETarget> GetTargetList();

std::vector<ETarget> GetTargetList(
    std::function<std::vector<ETarget>()> getFullList,
    std::function<bool(ETarget)> pred);

std::vector<ETarget> GetTargetList(
    std::function<std::vector<ETarget>()> getFullList,
    const std::set<ETarget> & excluded);

std::vector<ETarget> GetTarget2dList();
std::vector<ETarget> GetTarget2dList(const std::set<ETarget> & excluded);

std::vector<ETarget> GetTarget2dObjectCanvasList();
std::vector<ETarget> GetTarget2dObjectCanvasList(
    const std::set<ETarget> & excluded);

std::vector<ETarget> GetTargetSettled2dObjectCanvasList();

bool IsSettled2dObjectCanvas(ETarget target);
bool IsNonblocking(ETarget target);

}
}

std::ostream& operator<<(std::ostream &stream, neutu::data3d::ETarget target);

#endif // UTILITIES_H
