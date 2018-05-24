%include task_manage.dot

class QRunnable;
//class QObject;
class ZProgressable;
class ZTask:QRunnable;
class ZTaskManager:ZProgressable;

connect ZTask::SIGNAL(finished()), ZTaskManager::process();
function ZTaskManager::start;
function ZTask::@finished;
function ZTask::execute;
function ZTaskManager::prepare();
function QRunnable::run;
function ZTask::run;
function ZTaskManager::postProcess();
function ZTaskManager::@finished;
function ZTask::prepare;
function ZTaskManager::addTask;

connect ZTaskManager::@finished, xxx::retrieveResult;

cmp ZTaskManager::ZTask;


