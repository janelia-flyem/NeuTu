#ifndef ZDVIDURL_H
#define ZDVIDURL_H

#include <string>

#include "dvid/zdvidtarget.h"
#include "dvid/zdviddata.h"

class ZDvidUrl
{
public:
  ZDvidUrl();
  ZDvidUrl(const std::string &serverAddress, const std::string &uuid, int port);
  ZDvidUrl(const ZDvidTarget &target);

  std::string getNodeUrl() const;
  std::string getDataUrl(const std::string &dataName) const;
  std::string getDataUrl(ZDvidData::ERole role) const;
  std::string getDataUrl(
      ZDvidData::ERole role, ZDvidData::ERole prefixRole,
      const std::string &prefixName);
  std::string getInfoUrl(const std::string &dataName) const;
  std::string getHelpUrl() const;
  std::string getServerInfoUrl() const;
  std::string getApiUrl() const;
  std::string getRepoUrl() const;
  std::string getInstanceUrl() const;

  std::string getSkeletonUrl(const std::string &bodyLabelName) const;
  std::string getSkeletonUrl(
      int bodyId, const std::string &bodyLabelName) const;

//  std::string getThumbnailUrl(const std::string &bodyLableName) const;
//  std::string getThumbnailUrl(int bodyId) const;

  std::string getThumbnailUrl(const std::string &bodyLabelName) const;
  std::string
  getThumbnailUrl(int bodyId, const std::string &bodyLabelName) const;

  std::string getSp2bodyUrl() const;
  std::string getSp2bodyUrl(const std::string &suffix) const;

//  std::string getSparsevolUrl() const;
//  std::string getSparsevolUrl(int bodyId) const;

  std::string getSparsevolUrl(const std::string &dataName) const;
  std::string getSparsevolUrl(int bodyId, const std::string &dataName) const;

//  std::string getCoarseSparsevolUrl() const;
//  std::string getCoarseSparsevolUrl(int bodyId) const;

  std::string getCoarseSparsevolUrl(const std::string &dataName) const;
  std::string getCoarseSparsevolUrl(int bodyId, const std::string &dataName) const;


  std::string getGrayscaleUrl() const;
  std::string getGrayscaleUrl(int sx, int sy, int x0, int y0, int z0) const;
  std::string getGrayscaleUrl(int sx, int sy, int sz, int x0, int y0, int z0)
   const;

  std::string getLabels64Url(
      const std::string &name,
      int sx, int sy, int sz, int x0, int y0, int z0) const;

  std::string getKeyUrl(const std::string &name, const std::string &key) const;
  std::string getKeyRangeUrl(
      const std::string &name,
      const std::string &key1, const std::string &key2) const;

  std::string getAnnotationUrl(const std::string &bodyLabelName) const;
  std::string getAnnotationUrl(
      int bodyId, const std::string &bodyLabelName) const;

//  std::string getBodyInfoUrl() const;
//  std::string getBodyInfoUrl(int bodyId) const;

  std::string getBodyInfoUrl(const std::string &dataName) const;
  std::string getBodyInfoUrl(int bodyId, const std::string &dataName) const;

  std::string getBoundBoxUrl() const;
  std::string getBoundBoxUrl(int z) const;

  std::string getBodyLabelUrl() const;
  std::string getBodyLabelUrl(const std::string &dataName) const;
  std::string getBodyLabelUrl(const std::string &dataName,
      int x0, int y0, int z0, int width, int height, int depth) const;
  std::string getBodyLabelUrl(
      int x0, int y0, int z0, int width, int height, int depth) const;

  std::string getBodyListUrl(int minSize) const;
  std::string getBodyListUrl(int minSize, int maxSize) const;

  std::string getMaxBodyIdUrl() const;

  std::string getSynapseListUrl() const;
  std::string getSynapseAnnotationUrl(const std::string &name) const;

  std::string getMergeUrl(const std::string &dataName) const;

  std::string getTileUrl(const std::string &dataName) const;
  std::string getTileUrl(const std::string &dataName, int resLevel) const;
  std::string getTileUrl(
      const std::string &dataName, int resLevel,
      int xi0, int yi0, int z0) const;



private:
  ZDvidTarget m_dvidTarget;
};

#endif // ZDVIDURL_H
