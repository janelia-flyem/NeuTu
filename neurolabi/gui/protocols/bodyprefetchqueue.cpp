#include "bodyprefetchqueue.h"

/*
 * NOTE: this implementation is a quick-and-dirty version; the API
 * is queue-like, but internally, it only stores one body ID, not
 * a set of them; this was done to get it out as soon as possible;
 * it's intended to update this in the future to handle multiple
 * items in the queue with some reasonable behavior
 */
BodyPrefetchQueue::BodyPrefetchQueue(QObject *parent) : QObject(parent)
{

    clear();

}

/*
 * returns the top body ID in the queue (ie, the body
 * that should be fetched next); if body ID 0 is returned,
 * there are no bodies in the queue
 */
uint64_t BodyPrefetchQueue::get() {
    // for the single value case, we're just returning the stored
    //  value and storing zero again
    return m_bodyID.fetchAndStoreOrdered(0);
}

/*
 * are there any items in the queue?  this is sketchy, not really
 * safe, but maybe useful?
 */
bool BodyPrefetchQueue::isEmpty() {
    return m_bodyID.loadAcquire() > 0;
}

/*
 * add body IDs to the prefetch queue; order is
 * arbitrary within the input set
 */
void BodyPrefetchQueue::add(QSet<uint64_t> bodyIDs) {
    // for the single-item implementation, we take an
    //  arbitrary item from the set and ignore the others
    m_bodyID.storeRelease(bodyIDs.values().first());
}

/*
 * add the body ID to the prefetch queue
 */
void BodyPrefetchQueue::add(uint64_t bodyID) {
    // for the single-item implementation, this is easy;
    //  we overwrite whatever is there currently
    m_bodyID.storeRelease(bodyID);
}

/*
 * remove the input body IDs from the queue; they are
 * assumed not to be needed anymore and should not be
 * fetched
 */
void BodyPrefetchQueue::remove(QSet<uint64_t> bodyIDs) {
    // for single-item implementation, we're just comparing each value
    //  in the set to the current value; in this case, since there's
    //  only one case, we can break the loop if we actually to find the value
    foreach(uint64_t bodyID, bodyIDs) {
        if (m_bodyID.testAndSetAcquire(bodyID, 0)) {
            break;
        }
    }
}

/*
 * clear the prefetch queue
 */
void BodyPrefetchQueue::clear() {
    m_bodyID.storeRelease(0);
}
