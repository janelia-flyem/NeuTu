#ifndef ZFLYEMMISC_H
#define ZFLYEMMISC_H

#include <string>
#include <vector>

class ZMatrix;

namespace ZFlyEmMisc {
void NormalizeSimmat(ZMatrix &simmat);

class HackathonEvaluator {
public:
  HackathonEvaluator(const std::string &sourceDir,
                     const std::string &workDir);
  void processNeuronTypeFile();
  void evalulate();

  const std::string& getSourceDir() const { return m_sourceDir; }
  const std::string& getWorkDir() const { return m_workDir; }
  std::string getNeuronTypeFile() const;
  std::string getNeuronInfoFile() const;
  std::string getSimmatFile() const;
  int getAccurateCount() const { return m_accuracy; }
  int getNeuronCount() const { return (int) m_idArray.size(); }

private:
  std::string m_sourceDir;
  std::string m_workDir;
  int m_accuracy;
  std::vector<int> m_idArray;
};

}
#endif // ZFLYEMMISC_H
