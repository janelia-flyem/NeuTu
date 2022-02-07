#include "zskeletonizeservice.h"

#include <exception>

#include "neutubeconfig.h"
#include "common/utilities.h"
#include "dvid/zdvidtarget.h"
#include "neutuse/task.h"
#include "neutuse/taskfactory.h"

void ZSkeletonizeService::requestSkeletonize(
    const ZDvidTarget &target, uint64_t bodyId)
{
  if (GET_FLYEM_CONFIG.neutuseAvailable(
        neutu::UsingLocalHost(target.getAddress()))) {
    neutuse::Task task = neutuse::TaskFactory::MakeDvidTask(
          "skeletonize", target, bodyId, true);
    task.setPriority(5);
    GET_FLYEM_CONFIG.getNeutuseWriter().uploadTask(task);
    if (GET_FLYEM_CONFIG.getNeutuseWriter().getStatusCode() != 200) {
      throw std::runtime_error("Failed to upload skeletonization task");
    }
  } else {
    throw std::runtime_error(
          "Skeletonization service unavailable for " + target.getAddress());
  }
}

