#include "zdvidtile.h"

#include <QTransform>
#include <QtConcurrentRun>
#include <QElapsedTimer>

#include "widgets/zimagewidget.h"
#include "neutubeconfig.h"
#include "zstack.hxx"
#include "zstackfactory.h"
#include "zdvidreader.h"
#include "zimage.h"
#include "zpainter.h"
#include "zdvidbufferreader.h"
#include "zdvidurl.h"
#include "zdvidtileinfo.h"
#include "zdvidreader.h"
#include "zstackview.h"
#include "zrect2d.h"
#include "libdvidheader.h"

ZDvidTile::ZDvidTile() : m_ix(0), m_iy(0), m_z(0),
  m_view(NULL)
{
  setTarget(ZStackObject::TARGET_OBJECT_CANVAS);
  m_type = ZStackObject::TYPE_DVID_TILE;
  m_image = NULL;
}

ZDvidTile::~ZDvidTile()
{
  clear();
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidTile)

void ZDvidTile::clear()
{
  m_dvidTarget.clear();
  delete m_image;
  m_image = NULL;
}

void ZDvidTile::loadDvidSlice(const uchar *buf, int length, int z)
{
  bool loading = true;
  if (m_view != NULL) {
    if (m_view->getZ(NeuTube::COORD_STACK) != z) {
      loading = false;
    }
  }

  bool modified = false;
  if (loading) {
    if (m_image == NULL) {
      m_image = new ZImage;
    }

//#ifdef _DEBUG_
    std::cout << "Decoding tile ..." << std::endl;
    std::cout << "data length: " << length << std::endl;
//#endif
    m_image->loadFromData(buf, length);
    std::cout << m_image->width() << "x" << m_image->height() << std::endl;

    m_image->setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
    m_image->setOffset(-getX(), -getY());

    modified = true;

    m_image->enhanceContrast(
          hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST));

#ifdef _DEBUG_
    std::cout << "Format: " << m_image->format() << std::endl;
    setVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
#endif
    m_z = z;
  }

  if (modified) {
    updatePixmap();
  }
}

void ZDvidTile::updatePixmap()
{
  m_pixmap.cleanUp();
  m_pixmap.convertFromImage(*m_image);
  std::cout << m_image->width() << "x" << m_image->height() << std::endl;

  m_pixmap.setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
  m_pixmap.setOffset(-getX(), -getY());

  std::cout << "saving pixmap" << std::endl;
  std::cout << m_pixmap.width() << "x" << m_pixmap.height() << std::endl;
  m_pixmap.save("/lhome/yy/work/tools/dvid/neutube/neurolabi/build/test.tif");
}

void ZDvidTile::enhanceContrast(bool high)
{
  if (high != hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST)) {
    if (high) {
      addVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    } else {
      removeVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST);
    }

    if (m_image != NULL) {
      m_image->enhanceContrast(
            hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST));
      updatePixmap();
    }
  }
}
/*
void ZDvidTile::setImageData(const uint8_t *data, int width, int height)
{
  if (width <= 0 || height <= 0) {
    return;
  }

  if (m_image != NULL) {
    if (m_image->width() != width || m_image->height() != height) {
      delete m_image;
      m_image = NULL;
    }
  }

  if (m_image == NULL) {
    m_image = new ZImage(width, height);
  }

  m_image->setData(data);
  m_image->setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
  m_image->setOffset(-getX(), -getY());
}
*/
void ZDvidTile::loadDvidSlice(const QByteArray &buffer, int z)
{
  loadDvidSlice((const uchar *) buffer.data(), buffer.length(), z);
#if 0
  bool loading = true;
  if (m_view != NULL) {
    if (m_view->getZ(NeuTube::COORD_STACK) != z) {
      loading = false;
    }
  }
  if (loading) {
#ifdef _DEBUG_2
      std::cout << z << " Loaded." << std::endl;
#endif


    m_image.loadFromData(buffer);
//    m_image.enhanceContrast();

//    m_image.setOffset();
    m_z = z;
  }
#endif

//  m_image.save((GET_TEST_DATA_DIR + "/test.tif").c_str());
}

void ZDvidTile::display(
    ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  bool isProj = false;
  int z = painter.getZOffset() + slice;
  if (slice < 0) {
    isProj = true;
    z = painter.getZOffset() - slice - 1;
  }
  //if (!m_image.isNull()) {
//  bool isProj = (slice < 0);

//  int z = painter.getZOffset() + slice;
  m_latestZ = z;

//  tic();
  const_cast<ZDvidTile&>(*this).update(z);
//  std::cout << "tile update time: " << toc() << std::endl;

  if ((z == m_z)  && (m_image != NULL)) {
#ifdef _DEBUG_2
    std::cout << "Display tile: " << z << std::endl;
#endif
    //      ZImage image = getImage();
    //int dx = getX() - painter.getOffset().x();
    //int dy = getY() - painter.getOffset().y();

    //      QRect sourceRect = QRect(0, 0, m_image.width(), m_image.height());
    //      QRect targetRect = QRect(getX(), getY(), m_image.width() * m_res.getScale(),
    //                         m_image.height() * m_res.getScale());
#if 0
    if (m_res.getScale() == 1) {
      m_image.save((GET_DATA_DIR + "/test.tif").c_str());
    }
#endif

//    m_image->enhanceContrast(
//          hasVisualEffect(NeuTube::Display::Image::VE_HIGH_CONTRAST));
//    QElapsedTimer timer;
//    timer.start();
//    tic();
    painter.drawPixmap(getX(), getY(), m_pixmap);
//    painter.drawImage(getX(), getY(), *m_image);
//    std::cout << "Draw image time: " << toc() << std::endl;
//    std::cout << "Draw image time: " << timer.elapsed() << std::endl;

//      ZIntPoint pt = m_offset - painter.getOffset().toIntPoint();

//      painter.save();

//      QTransform transform;

//      transform.scale(m_res.getScale(), m_res.getScale());
//      transform.translate(getX(), getY());

//      //transform.translate(pt.x(), pt.y());
//      painter.setTransform(transform);
//      painter.drawImage(m_image);

//      painter.restore();
    //}
  }
}
#if 0
void ZDvidTile::update(int x, int y, int z, int width, int height)
{

  bool updating = false;
  if (m_stack == NULL) {
    m_stack = ZStackFactory::makeZeroStack(GREY, width, height, 1);
    m_stack->setOffset(x, y, z);
    updating = true;
  } else if (m_stack->getOffset().getZ() != z ||
             m_stack->getOffset().getX() != x ||
             m_stack->getOffset().getZ() != z ||
             m_stack->width() != width || m_stack->height() != height) {
    updating = true;
  }

  if (updating) {
    ZDvidReader reader;
    if (reader.open(m_dvidTarget)) {
      Stack *stack = reader.readTile(x, y, z, width, heigth, m_res.getLevel());
    }
  }

}
#endif
void ZDvidTile::setTileIndex(int ix, int iy)
{
  m_ix = ix;
  m_iy = iy;
}

void ZDvidTile::update(int z)
{
  if (m_z != z || m_image == NULL) {
#if defined(_ENABLE_LIBDVIDCPP_2)
    std::vector<int> offset(3);
    offset[0] = m_ix;
    offset[1] = m_iy;
    offset[2] = z;

    tic();
    try {
      libdvid::DVIDNodeService service(getDvidTarget().getAddressWithPort(),
                                       getDvidTarget().getUuid());
      libdvid::BinaryDataPtr data =
          service.get_tile_slice_binary(
            "tiles", libdvid::XY, m_res.getLevel(), offset);
      if (data->length() > 0) {
        loadDvidSlice(data->get_raw(), data->length(), z);
        m_image->setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
        m_image->setOffset(-getX(), -getY());
      }
    } catch (std::exception &e) {
      std::cout << e.what() << std::endl;
    }
    std::cout << "Tile level: " << m_res.getLevel() << std::endl;
    std::cout << "Tile reading time: " << toc() << std::endl;
#else
    ZDvidUrl dvidUrl(getDvidTarget());
    ZDvidBufferReader bufferReader;

    /*
    QFuture<void> result = QtConcurrent::run(
          &bufferReader, &ZDvidBufferReader::read,
          QString(dvidUrl.getTileUrl("graytiles", m_res.getLevel(), m_ix, m_iy, z).c_str()));
    result.waitForFinished();
    */

    tic();
    bufferReader.read(
          dvidUrl.getTileUrl(getDvidTarget().getMultiscale2dName(),
                             m_res.getLevel(), m_ix, m_iy, z).c_str(), true);
    QByteArray buffer = bufferReader.getBuffer();
    std::cout << "Tile reading time: " << toc() << std::endl;

//    ZDvidTileInfo tileInfo = readTileInfo("graytiles");

    if (!buffer.isEmpty()) {
      loadDvidSlice(buffer, z);
//      m_image->setScale(1.0 / m_res.getScale(), 1.0 / m_res.getScale());
//      m_image->setOffset(-getX(), -getY());
      //      setResolutionLevel(m_res.getLevel());
    }
#endif
  }
  //m_z = z;
}
#if 0
void ZDvidTile::setTileOffset(int x, int y, int z)
{
  m_offset.set(x, y, z);
}
#endif

void ZDvidTile::printInfo() const
{
  std::cout << "Dvid tile: " << std::endl;
  m_res.print();
  std::cout << "Offset: " << getX() << ", " << getY() << ", " << getZ() << std::endl;
  if (m_image != NULL) {
    std::cout << "Size: " << m_image->width() << " x " << m_image->height()
              << std::endl;
  }

}

void ZDvidTile::setResolutionLevel(int level)
{
  m_res.setLevel(level);
}

void ZDvidTile::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  if (!m_tilingInfo.isValid()) {
    ZDvidReader reader;
    if (reader.open(target)) {
      m_tilingInfo = reader.readTileInfo(target.getMultiscale2dName());
    }
  }
}

int ZDvidTile::getX() const
{
  return m_ix * m_tilingInfo.getWidth() * m_res.getScale();
}

int ZDvidTile::getWidth() const
{
  return m_tilingInfo.getWidth() * m_res.getScale();
}

int ZDvidTile::getHeight() const
{
  return m_tilingInfo.getHeight() * m_res.getScale();
}

int ZDvidTile::getY() const
{
  return m_iy * m_tilingInfo.getHeight() * m_res.getScale();
}

int ZDvidTile::getZ() const
{
  return m_z;
}

void ZDvidTile::attachView(ZStackView *view)
{
  m_view = view;
}

ZRect2d ZDvidTile::getBoundBox() const
{
  ZRect2d rect;
  rect.set(getX(), getY(), getWidth(), getHeight());

  return rect;
}


