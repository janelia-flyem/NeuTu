#include "zrandomgenerator.h"
#include <cstdlib>
#include <ctime>
#include <algorithm>

//using namespace std;

int ZRandomGenerator::count = 0;

ZRandomGenerator::ZRandomGenerator()
{
  srand(UniqueSeed());
}

ZRandomGenerator::ZRandomGenerator(int seed)
{
  srand(seed);
}

int ZRandomGenerator::UniqueSeed()
{
  return (unsigned)time(0) + count++;
}

void ZRandomGenerator::setSeed(int seed) const
{
  srand(seed);
}

int ZRandomGenerator::rndint(int maxValue) const
{
  return rand() % maxValue + 1;
}

int ZRandomGenerator::rndint(int minValue, int maxValue) const
{
  return minValue + rndint(maxValue - minValue + 1) - 1;
}

struct IntLessThan {
    bool operator() (const std::pair<int, int> &p1, const std::pair<int, int> &p2)
    {
        if (p1.first < p2.first) {
            return true;
        }

        return false;
    }
};

std::vector<int> ZRandomGenerator::randperm(int n) const
{
  std::vector<std::pair<int, int> > randArray(n);
  for (int i = 0; i < n; i++) {
    randArray[i].first = rndint(n * 5);
    randArray[i].second = i + 1;
  }

  std::sort(randArray.begin(), randArray.end(), IntLessThan());

  std::vector<int> permArray(n);
  for (int i = 0; i < n; i++) {
    permArray[i] = randArray[i].second;
  }

  return permArray;
}

std::vector<int> ZRandomGenerator::Randperm(int n)
{
  return GetInstance().randperm(n);
}

int ZRandomGenerator::Rndint(int maxValue)
{
  return GetInstance().rndint(maxValue);
}

int ZRandomGenerator::Rndint(int minValue, int maxValue)
{
  return GetInstance().rndint(minValue, maxValue);
}

void ZRandomGenerator::SetSeed(int seed)
{
  return GetInstance().setSeed(seed);
}
