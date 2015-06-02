#ifndef NEUTUBE_H
#define NEUTUBE_H

#include <string>

class ZMessageReporter;
class ZLogMessageReporter;

namespace NeuTube {

enum EDocumentableType {
  Documentable_SWC, Documentable_PUNCTUM, Documentable_OBJ3D,
  Documentable_STROKE, Documentable_LOCSEG_CHAIN, Documentable_CONN,
  Documentable_SPARSE_OBJECT, Documentable_Circle
};

namespace Document {
enum ETag {
  NORMAL, BIOCYTIN_PROJECTION, BIOCYTIN_STACK, FLYEM_BODY, FLYEM_STACK,
  FLYEM_SPLIT, FLYEM_ROI, FLYEM_MERGE, SEGMENTATION_TARGET, FLYEM_DVID,
  FLYEM_BODY_DISPLAY, FLYEM_PROOFREAD
};
}

enum EImageBackground {
  IMAGE_BACKGROUND_BRIGHT, IMAGE_BACKGROUND_DARK
};

enum ESizeHintOption {
  SIZE_HINT_DEFAULT, SIZE_HINT_CURRENT_BEST, SIZE_HINT_TAKING_SPACE
};

enum EAxis {
  X_AXIS, Y_AXIS, Z_AXIS
};

enum ECoordinateSystem {
  COORD_WIDGET, COORD_SCREEN, COORD_RAW_STACK, COORD_STACK,
  COORD_WORLD, COORD_CANVAS
};

enum EColor {
  RED, GREEN, BLUE
};

enum EWindowConfig {
  WINDOW_2D, WINDOW_3D
};

enum EMessageType {
  MSG_INFORMATION, MSG_WARING, MSG_ERROR
};

enum EBodyLabelType {
  BODY_LABEL_ORIGINAL, BODY_LABEL_MAPPED
};


ZMessageReporter *getMessageReporter();
ZLogMessageReporter* getLogMessageReporter();

std::string getErrorFile();
std::string getWarnFile();
std::string getInfoFile();


std::string GetUserName();
}



#endif // NEUTUBE_H
