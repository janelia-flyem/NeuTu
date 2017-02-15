class ProtocolSwitcher;
class SynapsePredicionProtocol:ProtocolDialog;
class ProtocolChooser;
class ProtocolMetaData;
class ProtocolDialog;
class SynapsePredictionInputDialog;
class ZFlyEmProofMvc;

function SynapsePredictionInputDialog::getVolume;
function SynapsePredictionInputDialog::getRoI;
function ProtocolSwitcher::openProtocolDialogRequested;

agg ProtocolSwitcher::ProtocolChooser;
cmp ProtocolSwitcher::ProtocolDialog;
cmp ProtocolSwitcher::ProtocolMetaData;
cmp ZFlyEmProofMvc::ProtocolSwitcher;

connect ProtocolChooser::requestStartProtocol, ProtocolSwitcher::startProtocolRequested;
connect ProtocolChooser::requestLoadProtocolKey, ProtocolSwitcher::loadProtocolKeyRequested;
connect ProtocolSwitcher::requestDisplaySavedProtocols,  ProtocolChooser::displaySavedProtocolKeys;
call ProtocolSwitcher::dvidTargetChanged, ProtocolSwitcher::loadProtocolRequested;
call ProtocolSwitcher::dvidTargetChanged, ProtocolMetadata::ReadProtocolMetadata;
connect ProtocolDialog, SIGNAL(protocolExiting()), ProtocolSwitcher, SLOT(exitProtocolRequested());
connect ProtocolDialog::protocolCompleting, ProtocolSwitcher::completeProtocolRequested;
connect ProtocolDialog::requestSaveProtocol, ProtocolSwitcher::saveProtocolRequested;
connect ProtocolSwitcher::requestLoadProtocol, ProtocolDialog::loadDataRequested;
connect ProtocolDialog::requestDisplayPoint, ProtocolSwitcher::displayPointRequested;




