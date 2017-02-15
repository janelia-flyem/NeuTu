%include thread_manage.dot

class QRunnable;
class QObject;
class ZProgressable;
class ZTask:QRunnable:QObject;
class ZTaskManager:QObject:ZProgressable;

connect ZTask::SIGNAL(finished()), ZTaskManager::process();
function ZTaskManager::start;

stm retrieve_result "retrieve result";
stm dequeue_neuron "dequeue neuron";
stm restart_calculator "restart calculator";
connect ZTaskManager::@finished, xxx::retrieveResult;

cmp ZTaskManager::ZTask;


