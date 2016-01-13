#include "ztilegraphicsitem.h"
#include "c_stack.h"
#include "zimage.h"

bool ZTileGraphicsItem::loadJsonObject(const ZJsonObject &obj, QString tileFilePath)
{
  if (m_tileInfo.loadJsonObject(obj,tileFilePath)) {
    setOffset(m_tileInfo.getOffset().x(), m_tileInfo.getOffset().y());
    Stack *stack;
    stack = C_Stack::readSc(m_tileInfo.getImageSource().c_str());

    if (stack != NULL) {
      ZImage image(C_Stack::width(stack), C_Stack::height(stack));
      Image_Array ima;
      ima.array = stack->array;
      switch (C_Stack::kind(stack)) {
      case GREY:
        image.setData(ima.array8);
        break;
      case COLOR:
        image.setData(ima.arrayc);
        break;
      default:
        break;
      }
      C_Stack::kill(stack);

      QPixmap pmap = QPixmap::fromImage(image);

      //.load(QString::fromStdString((m_tileInfo.getImageSource())));
      setPixmap(pmap);

      //_DEBUG_
#ifdef _DEBUG_
      std::cout << m_tileInfo.getOffset().toString() << std::endl;
      std::cout << m_tileInfo.getImageSource() << std::endl;
      std::cout << image.width() << " " <<  image.height() << std::endl;
      std::cout << pmap.size().width() << " " <<  pixmap().size().height() << std::endl;
#endif

      setToolTip(m_tileInfo.getSource().c_str());
      return true;
    }
  }
  return false;
}
