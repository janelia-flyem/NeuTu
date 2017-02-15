class QObject;
class ZMessageManager:QObject;
class ZMessageProcessor;
class ZMainWindowMessageProcessor:ZMessageProcessor;

cmp ZMessageManager::ZMessageProcessor;
function ZMessageManager::processMessage;
function ZMessageManager::dispatchMessage;
function ZMessageManager::reportMessage;

function ZMessageManager::registerWidget;
function ZMessageProcessor::processMessage;
function ZMessageManager::getRootManager;




