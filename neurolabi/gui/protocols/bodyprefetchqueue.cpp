#include "bodyprefetchqueue.h"

#include <QMutexLocker>

#include "logging/zqslog.h"
#include "logging/zlog.h"

#include "zstackobjectsourcefactory.h"
#include "zmesh.h"

#include "mvc/zstackdocdatabuffer.h"

#include "flyem/zflyembody3ddoc.h"

/*
 * this queue should be moved into its own thread after creation; see
 * TaskProtocolWindow, where it's used
 *
 * incoming add/remove item requests come in through queued signal/slot;
 * request for item is blocking if there are none
 *
 * I followed these examples:
 * - http://code.jamming.com.ua/classic-producer-consumer-in-qtc/
 * - https://mayaposch.wordpress.com/2011/11/01/how-to-really-truly-use-qthreads-the-full-explanation/
 */
BodyPrefetchQueue::BodyPrefetchQueue(QObject *parent) : QObject(parent)
{

    clear();

}

BodyPrefetchQueue::~BodyPrefetchQueue()
{
  ZINFO(neutu::TOPIC_NULL) << "Destroying BodyPrefetchQueue";
}

/*
 * returns the top body ID in the queue (ie, the body
 * that should be fetched next); blocks if there are no items
 */
/*
uint64_t BodyPrefetchQueue::get() {
    QMutexLocker locker(&m_queueLock);

    while (m_queue.isEmpty()) {
        m_queueHasItems.wait(&m_queueLock);
    }
    LINFO() << "BodyPrefetchQueue: returning body:" << m_queue.head();
    return m_queue.dequeue();
}
*/

/*
 * are there any items in the queue?
 */
bool BodyPrefetchQueue::isEmpty() {
    return m_queue.isEmpty();
}

/*
 * add body IDs to the prefetch queue; order is
 * arbitrary within the input set
 */
void BodyPrefetchQueue::add(QSet<uint64_t> bodyIDs) {
    QMutexLocker locker(&m_queueLock);

    bool wasEmpty = m_queue.isEmpty();
    foreach (uint64_t bodyID, bodyIDs) {
      if (!m_queue.contains(bodyID)) {
        m_queue.enqueue(bodyID);
        LINFO() << "BodyPrefetchQueue: added body:" << bodyID;

        if (!ZFlyEmBodyManager::EncodesTar(bodyID)) {
          ZMesh *mesh = m_doc->readMesh(m_reader, bodyID, 0);

          if (mesh != NULL) {
            auto source = ZStackObjectSourceFactory::MakeFlyEmBodySource(
                  bodyID, 0, flyem::EBodyType::MESH);
            mesh->setSource(source);

            m_doc->getDataBuffer()->addUpdate(
                  mesh, ZStackDocObjectUpdate::EAction::ADD_BUFFER);
          }
        } else{
          ZFlyEmBodyConfig config(bodyID);
          config.setAddBuffer();
          m_doc->addBody(config);
        }
      }
    }

    if (wasEmpty) {
//        m_queueHasItems.wakeAll();
    }
}

/*
 * add the body ID to the prefetch queue
 */
void BodyPrefetchQueue::add(uint64_t bodyID) {
    QMutexLocker locker(&m_queueLock);

    bool wasEmpty = m_queue.isEmpty();
    m_queue.enqueue(bodyID);
    LINFO() << "BodyPrefetchQueue: added body:" << bodyID;
    if (wasEmpty) {
//        m_queueHasItems.wakeAll();
    }
}

void BodyPrefetchQueue::remove(uint64_t bodyID) {
    QMutexLocker locker(&m_queueLock);
    LINFO() << "BodyPrefetchQueue: removed body:" << bodyID;
    m_queue.removeAll(bodyID);
}

/*
 * remove the input body IDs from the queue; they are
 * assumed not to be needed anymore and should not be
 * fetched
 */
void BodyPrefetchQueue::remove(QSet<uint64_t> bodyIDs) {
    QMutexLocker locker(&m_queueLock);

    foreach (uint64_t bodyID, bodyIDs) {
        LINFO() << "BodyPrefetchQueue: removed body:" << bodyID;
        m_queue.removeAll(bodyID);
    }
}

/*
 * clear the prefetch queue
 */
void BodyPrefetchQueue::clear() {
    QMutexLocker locker(&m_queueLock);
    LINFO() << "BodyPrefetchQueue: clearing queue";
    m_queue.clear();
}

/*
 * finish up and signal the thread it can die
 */
void BodyPrefetchQueue::finish() {
    // do I need to grab the lock here?
    emit finished();
}


void BodyPrefetchQueue::setDocument(ZFlyEmBody3dDoc *doc)
{
  m_doc = doc;
  m_reader.open(m_doc->getDvidTarget());
}
