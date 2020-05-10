#ifndef ZRANDOMGENERATOR_H
#define ZRANDOMGENERATOR_H

#include <vector>

class ZRandomGenerator
{
public:
  ZRandomGenerator();
  ZRandomGenerator(int seed);

public:
  /*!
   * \brief Permute 1...n randomly
   */
  std::vector<int> randperm(int n) const;
  int rndint(int maxValue) const;
  int rndint(int minValue, int maxValue) const;
  void setSeed(int seed) const;

public:
  static ZRandomGenerator& GetInstance() {
    static ZRandomGenerator g;
    return g;
  }

  static std::vector<int> Randperm(int n);
  static int Rndint(int maxValue);
  static int Rndint(int minValue, int maxValue);
  static void SetSeed(int seed);
  static int UniqueSeed();

  static int count;
};

#endif // ZRANDOMGENERATOR_H
