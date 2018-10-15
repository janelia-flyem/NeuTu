#include "taskprotocoltask.h"

#include "zqslog.h"
#include "protocoltaskconfig.h"

/*
 * this is the abstract base class for tasks used by the TaskProtocolWindow in Neu3; your
 * create a new task by subclassing this class
 *
 * pure virtual methods you must implement:
 * -- taskType() should return the string that identifies the class type in the json
 * -- actionString() and targetString() will appear in the task UI; eg, "Review body:" and "12345"
 * -- loadSpecific() and addToJson() are how the task reads and writes its particular data;
 *      note that the task takes care of standard values for you (eg, completed status, tags)
 *
 * optional methods you may implement:
 * -- onCompleted() is called when the user completes the task, in case the task
 *      wants to do something, check something, etc.
 * -- getTaskWidget(): optionally return a QWidget that the task will insert into
 *      the layout below the generic task interface, in case the task needs more UI
 *
 * other things you should do:
 * -- document the json that the task expects, preferably both in the task
 *      cpp file top comments, and on the wiki page (if you're at Janelia)
 *
 */
TaskProtocolTask::TaskProtocolTask()
{

    m_completed = false;

}

namespace {
static QString s_jsonSource;
}

void TaskProtocolTask::setJsonSource(const QString &source)
{
  s_jsonSource = source;
}

QString TaskProtocolTask::jsonSource()
{
  return s_jsonSource;
}

// constants
const QString TaskProtocolTask::KEY_COMPLETED = "completed";
const QString TaskProtocolTask::KEY_TAGS = "tags";
const QString TaskProtocolTask::KEY_VISIBLE = "visible";
const QString TaskProtocolTask::KEY_SELECTED= "selected";

bool TaskProtocolTask::completed() const
{
    return m_completed;
}

const QSet<uint64_t> & TaskProtocolTask::visibleBodies() {
    return m_visibleBodies;
}

const QSet<uint64_t> & TaskProtocolTask::selectedBodies() {
    return m_selectedBodies;
}

void TaskProtocolTask::setCompleted(bool completed)
{
    m_completed = completed;

    // call the method that subclasses might implement
    if (completed) {
        onCompleted();
    }
}

/*
 * subclasses may optionally implement this method to
 * disable prefetching (e.g., because the task uses only low resolution bodies)
 */
bool TaskProtocolTask::usePrefetching() {
    return true;
}

/*
 * subclasses may optionally implement this method to
 * check internal state and disallow completion of the task
 */
bool TaskProtocolTask::allowCompletion() {
    return true;
}

/*
 * subclasses may optionally implement this method to
 * do something when the task is marked "complete"
 */
void TaskProtocolTask::onCompleted() {
    // nothing
}

/*
 * subclasses may optionally implement this method to
 * do something when a task is about to be moved away from
 * via "next" button
 */
void TaskProtocolTask::beforeNext() {
    // nothing
}

/*
 * subclasses may optionally implement this method to
 * do something when a task is about to be moved away from
 * via "prev" button
 */
void TaskProtocolTask::beforePrev() {
    // nothing
}

/*
 * subclasses may optionally implement this method to
 * do something when a task is about to be loaded as the current task
 * via the "next" or "prev" button
 */
void TaskProtocolTask::beforeLoading() {
    // nothing
}

/*
 * subclasses may optionally implement this method to
 * do something when the bodies from the previous task have been unloaded
 * and the bodies from this task have been loaded
 */
void TaskProtocolTask::onLoaded() {
    // nothing
}

/*
 * subclasses may optionally implement this method to
 * do something in the task that is active when the "done" button
 * is pressed
 */
void TaskProtocolTask::beforeDone() {
  // nothing
}

/*
 * subclasses may optionally implement this method to
 * inidicate that a particular task instance should be skipped
 * (e.g., because when the task loaded itself it discovered that
 * it was redundant in some sense)
 */
bool TaskProtocolTask::skip()
{
  return false;
}

/*
 * subclasses may optionally implement this method to
 * add UI below the standard UI
 */
QWidget * TaskProtocolTask::getTaskWidget() {
    return nullptr;
}

/*
 * subclasses may optionally implement this method to
 * add a menu to the main menu bar
 */
QMenu * TaskProtocolTask::getTaskMenu() {
    return nullptr;
}

// tag methods: standard add, remove, has, get all, clear

void TaskProtocolTask::addTag(QString tag) {
    m_tags.insert(tag);
}

void TaskProtocolTask::removeTag(QString tag) {
    m_tags.remove(tag);
}

bool TaskProtocolTask::hasTag(QString tag) {
    return m_tags.contains(tag);
}

QStringList TaskProtocolTask::getTags() {
    QStringList tags;
    foreach (QString tag, m_tags) {
        tags.append(tag);
    }
    return tags;
}

void TaskProtocolTask::clearTags() {
    m_tags.clear();
}

/*
 * load data from json
 */
bool TaskProtocolTask::loadJson(QJsonObject json) {

    // take care of standard fields
    if (!loadStandard(json)) {
        return false;
    }

    // and fields provided by subclasses
    if (!loadSpecific(json)) {
        return false;
    }

    return true;
}

/*
 * parse out the standard fields that are used in all tasks
 */
bool TaskProtocolTask::loadStandard(QJsonObject json) {
    // these keys are optional, so no need to fail if missing;
    //  we'll take the default values

    // completed:
    if (json.contains(KEY_COMPLETED)) {
        m_completed = json[KEY_COMPLETED].toBool();
    }

    // visible and selected bodies:
    m_visibleBodies.clear();
    if (json.contains(KEY_VISIBLE)) {
        foreach (QJsonValue value, json[KEY_VISIBLE].toArray()) {
            m_visibleBodies.insert(value.toDouble());
        }
    }

    m_selectedBodies.clear();
    if (json.contains(KEY_SELECTED)) {
        foreach (QJsonValue value, json[KEY_SELECTED].toArray()) {
            m_selectedBodies.insert(value.toDouble());
        }
    }

    // if there are selected or visible bodies, though, we
    //  have to check them
    // this is a little iffy; our body IDs are 64-bit; json numbers
    //  are floats and technnically not limited; however, Qt only
    //  converts them to doubles; that means if we have a body ID
    //  above about 2^52, we won't be able to convert it without
    //  losing precision; that seems unlikely in the near future,
    //  so test and log, but don't handle it yet
    // alternate possibility is to store the body IDs as strings are parse,
    //  but that seems just as inelegant
    QSet<uint64_t> allBodies;
    allBodies.unite(m_visibleBodies);
    allBodies.unite(m_selectedBodies);
    if (allBodies.size() > 0) {
        foreach (uint64_t bodyID, allBodies) {
            if (bodyID == 0) {
                // 0 indicates a conversion failure; we don't
                //  anticipate reviewing body 0
                LINFO() << "error converting task json; body ID = 0 not allowed";
                return false;
            }
            if (bodyID > 4503599627370496) {
                // that number is 2^52
                LINFO() << "error converting task json; found body ID > 2^52";
                return false;
            }
        }
    }

    // tags:
    clearTags();
    if (json.contains(KEY_TAGS)) {
        foreach (QJsonValue value, json[KEY_TAGS].toArray()) {
            addTag(value.toString());
        }
    }

    return true;
}

/*
 * utility; when having trouble with input json, nice
 * to embed that json in the error
 */
QString TaskProtocolTask::objectToString(QJsonObject json) {
    QJsonDocument doc(json);
    QString jsonString(doc.toJson(QJsonDocument::Compact));
    return jsonString;
}

void TaskProtocolTask::updateBodies(const QSet<uint64_t> &visible,
                                    const QSet<uint64_t> &selected)
{
  m_visibleBodies = visible;
  m_selectedBodies = selected;
  emit bodiesUpdated();
}

void TaskProtocolTask::allowNextPrev(bool allow)
{
  emit nextPrevAllowed(allow);
}

void TaskProtocolTask::notify(const ZWidgetMessage &msg)
{
  emit messageGenerated(msg);
}

/*
 * produce the json that holds all the task data
 */
QJsonObject TaskProtocolTask::toJson() {
    QJsonObject taskJson;

    // completed:
    taskJson[KEY_COMPLETED] = m_completed;

    // visible and selected bodies:
    // see note in loadJson() re: precision; because we
    //  know the source of the body IDs, the conversions
    //  below should be OK
    if (m_visibleBodies.size() > 0) {
        QJsonArray array;
        foreach (uint64_t bodyID, m_visibleBodies) {
            array.append(static_cast<double>(bodyID));
        }
        taskJson[KEY_VISIBLE] = array;
    }
    if (m_selectedBodies.size() > 0) {
        QJsonArray array;
        foreach (uint64_t bodyID, m_selectedBodies) {
            array.append(static_cast<double>(bodyID));
        }
        taskJson[KEY_SELECTED] = array;
    }

    // tags:
    QJsonArray tags;
    foreach (QString tag, getTags()) {
        tags.append(tag);
    }
    if (tags.size() > 0)  {
        taskJson[KEY_TAGS] = tags;
    }

    // allow subclass to add fields
    taskJson = addToJson(taskJson);

    return taskJson;
}

ProtocolTaskConfig TaskProtocolTask::getTaskConfig() const
{
  ProtocolTaskConfig config;
  config.setTaskType(taskType());
  config.setDefaultTodo(neutube::EToDoAction::TO_SPLIT);

  return config;
}

bool TaskProtocolTask::allowingSplit(uint64_t /*bodyId*/) const
{
  return false;
}
