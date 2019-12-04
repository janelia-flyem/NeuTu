#ifndef ZFLYEMUTILITIES_H
#define ZFLYEMUTILITIES_H

#include <string>
#include <set>
#include <cstdint>

class ZIntPoint;
class ZStack;
class ZSwcTree;

namespace flyem
{

double GetFlyEmRoiMarkerRadius(double s);
double GetFlyEmRoiMarkerRadius(double width, double height);
std::set<uint64_t> LoadBodySet(const std::string &filePath);


//ZIntPoint FindClosestBg(const ZStack *stack, int x, int y, int z);

/*!
 * \brief Get mutation ID of a skeleton
 *
 * The mutation ID is stored in the comment of \a tree. If the information does
 * not exist, it returns 0 (considered as from the initial segmentation).
 */
int64_t GetMutationId(const ZSwcTree *tree);
int64_t GetMutationId(const ZSwcTree &tree);
void SetMutationId(ZSwcTree *tree, int64_t mid);
}

#endif // ZFLYEMUTILITIES_H
