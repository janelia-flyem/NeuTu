%include match.dot

class ZSwcTreeMatchThread:QThread;

connect ZSwcTreeMatchThread::SIGNAL(finished()), ZSwcBatchMatcher::process();
stm retrieve_result "retrieve result";
stm dequeue_neuron "dequeue neuron";
stm restart_calculator "restart calculator";
connect ZSwcBatchMatcher::@finished, xxx::retrieveResult;

agg ZSwcBatchMatcher::ZFlyEmDataBundle;
cmp ZSwcTreeMatchThread::ZSwcTreeMatcher;
cmp ZSwcBatchMatcher::ZSwcTreeMatchThread;


