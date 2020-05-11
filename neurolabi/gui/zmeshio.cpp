#include "zmeshio.h"

#include <utility>

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <draco/mesh/mesh.h>
#include <draco/point_cloud/point_cloud.h>
#include <draco/compression/decode.h>
#include <draco/compression/encode.h>
#include <QFile>
#include <memory>

#include "zmesh.h"
#include "zioutils.h"
#include "qt/core/zexception.h"
#include "logging/zqslog.h"
#include "common/memorystream.h"

#undef COLOR

namespace {

void createMaterials(aiScene* pScene)
{
  pScene->mNumMaterials = 1;
  pScene->mMaterials = new aiMaterial* [pScene->mNumMaterials];
  for (unsigned int matIndex = 0; matIndex < pScene->mNumMaterials; matIndex++) {
    auto mat = new aiMaterial;
    pScene->mMaterials[matIndex] = mat;
  }
}

aiMesh* createAIMesh(const ZMesh& mesh)
{
  auto pMesh = new aiMesh;
  pMesh->mMaterialIndex = 0;
  pMesh->mNumFaces = mesh.numTriangles();
  pMesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
  if (pMesh->mNumFaces > 0) {
    pMesh->mFaces = new aiFace[pMesh->mNumFaces];
    for (size_t i = 0; i < pMesh->mNumFaces; ++i) {
      aiFace* pFace = &pMesh->mFaces[i];
      pFace->mNumIndices = 3;
      pFace->mIndices = new unsigned int[pFace->mNumIndices];
      glm::uvec3 indices = mesh.triangleIndices(i);
      pFace->mIndices[0] = indices.x;
      pFace->mIndices[1] = indices.y;
      pFace->mIndices[2] = indices.z;
    }
  }
  pMesh->mNumVertices = mesh.numVertices();
  pMesh->mVertices = new aiVector3D[pMesh->mNumVertices];
  const std::vector<glm::vec3>& vertices = mesh.vertices();
  const std::vector<glm::vec3>& normals = mesh.normals();
  if (!normals.empty()) {
    pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];
    CHECK(normals.size() >= vertices.size());
  }
  memcpy(pMesh->mVertices, vertices.data(), sizeof(float) * 3 * vertices.size());
  if (!normals.empty()) {
    memcpy(pMesh->mNormals, normals.data(), sizeof(float) * 3 * vertices.size());
  }

  return pMesh;
}

// Appends this node to the parent node
void appendChildToParentNode(aiNode* pParent, aiNode* pChild)
{
  // Checking preconditions
  CHECK(pParent && pChild);

  // Assign parent to child
  pChild->mParent = pParent;

  // If already children was assigned to the parent node, store them in a
  std::vector<aiNode*> temp;
  if (pParent->mChildren != nullptr) {
    CHECK(0 != pParent->mNumChildren);
    for (size_t index = 0; index < pParent->mNumChildren; index++) {
      temp.push_back(pParent->mChildren[index]);
    }
    delete[] pParent->mChildren;
  }

  // Copy node instances into parent node
  pParent->mNumChildren++;
  pParent->mChildren = new aiNode* [pParent->mNumChildren];
  for (size_t index = 0; index < pParent->mNumChildren - 1; index++) {
    pParent->mChildren[index] = temp[index];
  }
  pParent->mChildren[pParent->mNumChildren - 1] = pChild;
}

// Creates all nodes of the model
aiNode* createNodes(const ZMesh& mesh, aiNode* pParent, aiScene* pScene, std::vector<aiMesh*>& MeshArray)
{
  // Store older mesh size to be able to computes mesh offsets for new mesh instances
  const size_t oldMeshSize = MeshArray.size();
  auto pNode = new aiNode;

  pNode->mName = "meshName";
  appendChildToParentNode(pParent, pNode);

  for (unsigned int i = 0; i < 1; ++i) {
    std::unique_ptr<aiMesh> pMesh(createAIMesh(mesh));
    if (pMesh->mNumVertices > 0) {
      MeshArray.push_back(pMesh.release());
    }
  }

  // Set mesh instances into scene- and node-instances
  const size_t meshSizeDiff = MeshArray.size() - oldMeshSize;
  if (meshSizeDiff > 0) {
    pNode->mMeshes = new unsigned int[meshSizeDiff];
    pNode->mNumMeshes = static_cast<unsigned int>( meshSizeDiff );
    size_t index = 0;
    for (size_t i = oldMeshSize; i < MeshArray.size(); ++i) {
      pNode->mMeshes[index] = pScene->mNumMeshes;
      pScene->mNumMeshes++;
      index++;
    }
  }

  return pNode;
}

using namespace draco;

void makeDracoMesh(const ZMesh &in, Mesh *out)
{
  const std::vector<glm::vec3>& vertices = in.vertices();
  size_t vertexDataLength = vertices.size() * 3;
  float *vertexDataBuffer = new float[vertexDataLength];
  float *vertexDataBufferIter = vertexDataBuffer;
  for (size_t i = 0; i < vertices.size(); ++i) {
    *vertexDataBufferIter++ = vertices[i].x;
    *vertexDataBufferIter++ = vertices[i].y;
    *vertexDataBufferIter++ = vertices[i].z;
  }

  DataBuffer buffer;
  buffer.Update(vertexDataBuffer, sizeof(float) * vertexDataLength);

  GeometryAttribute va;
  va.Init(GeometryAttribute::POSITION, &buffer, 3, DT_FLOAT32,
          false, sizeof(float) * 3, 0);

  out->AddAttribute(va, true, vertices.size());

  delete []vertexDataBuffer;
}

void getDracoVertices(const PointCloud& pc, std::vector<glm::vec3>& vertices)
{
  const PointAttribute *const att = pc.GetNamedAttribute(GeometryAttribute::POSITION);
  if (att == nullptr || att->size() == 0)
    throw ZIOException("no vertices found in draco file");
  vertices.resize(pc.num_points());
  for (PointIndex i(0); i < pc.num_points(); ++i) {
    if (!att->ConvertValue<float, 3>(att->mapped_index(i), &vertices[i.value()][0])) {
      vertices.clear();
      throw ZIOException("can not decode draco vertices");
    }
  }
}

void getDracoNormals(const PointCloud& pc, std::vector<glm::vec3>& normals)
{
  const PointAttribute *const att = pc.GetNamedAttribute(GeometryAttribute::NORMAL);
  if (att == nullptr || att->size() == 0) {
    return; // no normal
  }
  if (att->num_components() == 3) {
    PointIndex num_points(pc.num_points());
    normals.resize(num_points.value());
    for (PointIndex i(0); i < num_points; ++i) {
      if (!att->ConvertValue<float, 3>(att->mapped_index(i), &normals[i.value()][0])) {
        normals.clear();
        throw ZIOException("can not decode draco normals");
      }
    }
  } else {
    LOG(WARNING) << "ignore normals that are not 3 components";
  }
}

void getDracoColors(const PointCloud& pc, std::vector<glm::vec4>& colors)
{
  const PointAttribute *const att = pc.GetNamedAttribute(GeometryAttribute::COLOR);
  if (att == nullptr || att->size() == 0) {
    return; // no color
  }
  if (att->num_components() == 1) {
    colors.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 1>(att->mapped_index(i), &colors[i.value()][0])) {
        colors.clear();
        throw ZIOException("can not decode draco 1 component colors");
      }
    }
    for (auto& c : colors) {
      c.a = 1.f;
    }
  } else if (att->num_components() == 2) {
    colors.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 2>(att->mapped_index(i), &colors[i.value()][0])) {
        colors.clear();
        throw ZIOException("can not decode draco 2 component colors");
      }
    }
    for (auto& c : colors) {
      c.a = 1.f;
    }
  } else if (att->num_components() == 3) {
    colors.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 3>(att->mapped_index(i), &colors[i.value()][0])) {
        colors.clear();
        throw ZIOException("can not decode draco 3 component colors");
      }
    }
    for (auto& c : colors) {
      c.a = 1.f;
    }
  } else if (att->num_components() == 4) {
    colors.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 4>(att->mapped_index(i), &colors[i.value()][0])) {
        colors.clear();
        throw ZIOException("can not decode draco 4 component colors");
      }
    }
  } else {
    LOG(WARNING) << "texture coordinate with" << att->num_components() << "components are not supported";
  }
}

void getDracoTextureCoordinates(const PointCloud& pc,
                                std::vector<float>& textureCoordinates1D,
                                std::vector<glm::vec2>& textureCoordinates2D,
                                std::vector<glm::vec3>& textureCoordinates3D)
{
  const PointAttribute *const att = pc.GetNamedAttribute(GeometryAttribute::TEX_COORD);
  if (att == nullptr || att->size() == 0) {
    return; // no textureCoordinates2D
  }
  if (att->num_components() == 1) {
    textureCoordinates1D.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 1>(att->mapped_index(i), &textureCoordinates1D[i.value()])) {
        textureCoordinates1D.clear();
        throw ZIOException("can not decode draco textureCoordinates1D");
      }
    }
  } else if (att->num_components() == 2) {
    textureCoordinates2D.resize(pc.num_points());
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      if (!att->ConvertValue<float, 2>(att->mapped_index(i), &textureCoordinates2D[i.value()][0])) {
        textureCoordinates2D.clear();
        throw ZIOException("can not decode draco textureCoordinates2D");
      }
    }
  } else if (att->num_components() == 3) {
    for (PointIndex i(0); i < pc.num_points(); ++i) {
      textureCoordinates3D.resize(pc.num_points());
      if (!att->ConvertValue<float, 3>(att->mapped_index(i), &textureCoordinates3D[i.value()][0])) {
        textureCoordinates3D.clear();
        throw ZIOException("can not decode draco textureCoordinates3D");
      }
    }
  } else {
    LOG(WARNING) << "texture coordinate with" << att->num_components() << "components are not supported";
  }
}

void getDracoFaces(const Mesh& msh, std::vector<GLuint>& faces)
{
  faces.resize(msh.num_faces() * 3);
//  const PointAttribute *const pos_att_ = msh.GetNamedAttribute(GeometryAttribute::POSITION);
  for (FaceIndex i(0); i < msh.num_faces(); ++i) {
    for (int j = 0; j < 3; ++j) {
      const PointIndex vert_index = msh.face(i)[j];
      faces[i.value()*3+j] = vert_index.value();
    }
  }
}

}  // namespace


ZMeshIO& ZMeshIO::instance()
{
  static ZMeshIO meshIO;
  return meshIO;
}

ZMeshIO::ZMeshIO()
{
  std::string tmp;
  Assimp::Importer importer;
  importer.GetExtensionList(tmp);
  QString exts = QString::fromStdString(tmp);
  exts.replace("*.", "");
  m_readExts = exts.split(";", QString::SkipEmptyParts);
  m_readExts.push_back("msh");
  m_readExts.push_back("drc");
  m_readExts.push_back("ngmesh");

  m_readFilter = QString("All Mesh files (*.") + m_readExts.join(" *.") + QString(")");

  Assimp::Exporter exporter;
  for (size_t i = 0; i < exporter.GetExportFormatCount(); ++i) {
    const aiExportFormatDesc* format = exporter.GetExportFormatDescription(i);
    addWriteFormat(format->id, format->fileExtension, format->description);
    /*
    m_writeExts.push_back(format->fileExtension);
    m_writeFormats.push_back(format->id);
    QString filter = format->description;
    filter += QString(" (*.%1)").arg(format->fileExtension);
    m_writeFilters.push_back(filter);
    */
  }
  addWriteFormat("ngmesh", "ngmesh", "NG Mesh");
//  m_writeExts.push_back("ngmesh");
//  m_writeFormats.push_back("ngmesh");
}

void ZMeshIO::addWriteFormat(
    const std::string &format, const std::string &ext,
    const std::string &description)
{
  m_writeFormats.push_back(format);
  m_writeExts.push_back(ext.c_str());
  m_writeFilters.push_back(
        QString("%1 (*.%2").arg(description.c_str()).arg(ext.c_str()));
}

bool ZMeshIO::canReadFile(const QString& filename)
{
  for (int i = 0; i < m_readExts.size(); ++i) {
    if (filename.endsWith(QString(".%1").arg(m_readExts[i]), Qt::CaseInsensitive))
      return true;
  }
  return false;
}

bool ZMeshIO::canWriteFile(const QString& filename)
{
  for (int i = 0; i < m_writeExts.size(); ++i) {
    if (filename.endsWith(QString(".%1").arg(m_writeExts[i]), Qt::CaseInsensitive))
      return true;
  }
  return false;
}

void ZMeshIO::getQtWriteNameFilter(QStringList& filters, QList<std::string>& formats)
{
  filters = m_writeFilters;
  formats = m_writeFormats;
}

void ZMeshIO::loadMesh(const aiScene *scene, ZMesh &mesh) const
{
  aiMesh* msh = scene->mMeshes[0];
  mesh.m_vertices.resize(msh->mNumVertices);
  mesh.m_normals.resize(msh->mNumVertices);
  memcpy(mesh.m_vertices.data(), msh->mVertices,
         sizeof(float) * 3 * mesh.m_vertices.size());
  memcpy(mesh.m_normals.data(), msh->mNormals,
         sizeof(float) * 3 * mesh.m_vertices.size());

  for (size_t i = 0; i < msh->mNumFaces; ++i) {
    if (msh->mFaces[i].mNumIndices != 3)
      continue;
    mesh.m_indices.push_back(msh->mFaces[i].mIndices[0]);
    mesh.m_indices.push_back(msh->mFaces[i].mIndices[1]);
    mesh.m_indices.push_back(msh->mFaces[i].mIndices[2]);
  }

  for (size_t j = 1; j < scene->mNumMeshes; ++j) {
    size_t numV = mesh.numVertices();
    msh = scene->mMeshes[j];
    mesh.m_vertices.resize(numV + msh->mNumVertices);
    mesh.m_normals.resize(numV + msh->mNumVertices);
    memcpy(&mesh.m_vertices[numV], msh->mVertices, sizeof(float) * 3 * msh->mNumVertices);
    memcpy(&mesh.m_normals[numV], msh->mNormals, sizeof(float) * 3 * msh->mNumVertices);

    for (size_t i = 0; i < msh->mNumFaces; ++i) {
      if (msh->mFaces[i].mNumIndices != 3)
        continue;
      mesh.m_indices.push_back(msh->mFaces[i].mIndices[0] + numV);
      mesh.m_indices.push_back(msh->mFaces[i].mIndices[1] + numV);
      mesh.m_indices.push_back(msh->mFaces[i].mIndices[2] + numV);
    }
  }
}

void ZMeshIO::initImporter(Assimp::Importer &importer) const
{
  importer.SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE,
                              aiPrimitiveType_POINT |      //remove points and
                              aiPrimitiveType_LINE);      //lines
  importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS,
                              aiComponent_CAMERAS |
                              aiComponent_LIGHTS |
                              aiComponent_BONEWEIGHTS |
                              aiComponent_COLORS |
                              aiComponent_TANGENTS_AND_BITANGENTS |
                              aiComponent_ANIMATIONS |
                              aiComponent_MATERIALS |
                              0);
}

ZMesh* ZMeshIO::loadFromMemory(
    const QByteArray &buffer, const std::string &format) const
{
  ZMesh *mesh = new ZMesh();
  loadFromMemory(buffer, *mesh, format);
  if (mesh->empty()) {
    delete mesh;
    mesh = NULL;
  }

  return mesh;
}

void ZMeshIO::loadFromMemory(
    const QByteArray &buffer, ZMesh &mesh, const std::string &format) const
{
  try {
    if (format == "drc") {
      readDracoMeshFromMemory(buffer.constData(), buffer.size(), mesh);
    } else if (format == "ngmesh") {
      readNgMeshFromMemory(buffer.constData(), buffer.size(), mesh);
    } else {
      mesh.clear();
      mesh.setType(GL_TRIANGLES);
      Assimp::Importer importer;
      initImporter(importer);
//      qDebug() << "Meshes:" << buffer;
      const aiScene* scene = importer.ReadFileFromMemory(buffer.data(), buffer.size(),                                               aiProcess_GenSmoothNormals |
                                                         aiProcess_JoinIdenticalVertices |
                                                         aiProcess_ImproveCacheLocality |
                                                         aiProcess_PreTransformVertices |
                                                         aiProcess_RemoveRedundantMaterials |
                                                         aiProcess_Triangulate |
                                                         aiProcess_GenUVCoords |
                                                         aiProcess_TransformUVCoords |
                                                         aiProcess_SortByPType |
                                                         aiProcess_FindDegenerates |
                                                         aiProcess_FindInvalidData |
                                                         aiProcess_RemoveComponent |
                                                         0);

      if (!scene) {
        throw ZIOException(importer.GetErrorString());
      }

      if (scene->mNumMeshes == 0) {
        LOG(WARNING) << "Failed to load mesh data.";
        return;
      }

      loadMesh(scene, mesh);
    }

  }
  catch (const ZException& e) {
    throw ZIOException(QString("Can not load mesh: %2").arg(e.what()));
  }
}

void ZMeshIO::load(const QString& filename, ZMesh& mesh) const
{
  try {
    mesh.clear();
    mesh.setType(GL_TRIANGLES);
    if (filename.endsWith(".msh", Qt::CaseInsensitive)) {
      readAllenAtlasMesh(filename, mesh.m_normals, mesh.m_vertices, mesh.m_indices);
    } else if (filename.endsWith(".drc", Qt::CaseInsensitive)) {
      readDracoMesh(filename, mesh);
    } else if (filename.endsWith(".ngmesh", Qt::CaseInsensitive)) {
      readNgMesh(filename, mesh);
    } else {
      Assimp::Importer importer;
      initImporter(importer);
      const aiScene* scene = importer.ReadFile(QFile::encodeName(filename).constData(),
                                               aiProcess_GenSmoothNormals |
                                               aiProcess_JoinIdenticalVertices |
                                               aiProcess_ImproveCacheLocality |
                                               aiProcess_PreTransformVertices |
                                               aiProcess_RemoveRedundantMaterials |
                                               aiProcess_Triangulate |
                                               aiProcess_GenUVCoords |
                                               aiProcess_TransformUVCoords |
                                               aiProcess_SortByPType |
                                               aiProcess_FindDegenerates |
                                               aiProcess_FindInvalidData |
                                               aiProcess_RemoveComponent |
                                               0);

      if (!scene) {
        throw ZIOException(importer.GetErrorString());
      }

      if (scene->mNumMeshes == 0) {
        LOG(WARNING) << "File " << filename << " does not contain any mesh.";
        return;
      }

      loadMesh(scene, mesh);
      //mesh.generateNormals();

      //throw ZIOException("Not supported mesh format");
    }
  }
  catch (const ZException& e) {
    throw ZIOException(QString("Can not load mesh %1: %2").arg(filename).arg(e.what()));
  }
}

void ZMeshIO::save(const ZMesh& mesh, const QString& filename, std::string format) const
{
  try {
    if (format.empty()) {
      if (filename.endsWith(".drc")) {
        format = "drc";
      } else {
        for (int i = 0; i < m_writeExts.size(); ++i) {
          if (filename.endsWith("." + m_writeExts[i], Qt::CaseInsensitive)) {
            format = m_writeFormats[i];
            break;
          }
        }
      }
    }

    if (format == "drc") {
      writeDracoMesh(filename, mesh);
    } else if (format == "ngmesh") {
      QByteArray data = mesh.writeToMemory(format);
      QFile file(filename);
      file.open(QIODevice::WriteOnly);
      file.write(data);
      file.close();
    } else {
      CHECK(m_writeFormats.contains(format));

      auto sc = std::make_unique<aiScene>();
      sc->mRootNode = new aiNode;
      sc->mRootNode->mName.Set("modelName");

      // Create nodes for the whole scene
      std::vector<aiMesh*> meshArray;
      createNodes(mesh, sc->mRootNode, sc.get(), meshArray);

      // Create mesh pointer buffer for this scene
      if (sc->mNumMeshes > 0) {
        sc->mMeshes = new aiMesh* [meshArray.size()];
        for (size_t index = 0; index < meshArray.size(); index++) {
          sc->mMeshes[index] = meshArray[index];
        }
      }

      //
      createMaterials(sc.get());

      Assimp::Exporter exporter;
      aiReturn res = exporter.Export(
            sc.get(), format, QFile::encodeName(filename).constData());
      if (res != aiReturn_SUCCESS) {
        throw ZIOException(exporter.GetErrorString());
      }
    }
  }
  catch (const ZException& e) {
    throw ZIOException(QString("Can not save mesh %1: %2").arg(filename).arg(e.what()));
  }
}

QByteArray ZMeshIO::writeToMemory(const ZMesh& mesh, std::string format) const
{
  QByteArray result;

  try {
    if (format == "drc") {
      draco::Mesh *dmesh = ToDracoMesh(mesh, nullptr);
      if (dmesh) {
        draco::Encoder encoder;
        draco::EncoderBuffer buffer;
        encoder.EncodeMeshToBuffer(*dmesh, &buffer);
        result.append(buffer.data(), buffer.size());
        delete dmesh;
      }
    } else if (format == "ngmesh") {
      if (mesh.type() == GL_TRIANGLES) { //only support GL_TRIANGLES for now
        QDataStream stream(&result, QIODevice::WriteOnly);
        //The stream operator doesn't work as expected
//        stream.setByteOrder(QDataStream::LittleEndian); //QDataStream uses BigEndian by default!
        const std::vector<glm::vec3> &vertices = mesh.vertices();
        uint32_t numVertices = uint32_t(vertices.size());
        stream.writeRawData(reinterpret_cast<char*>(&numVertices), 4);
//        stream << numVertices;
        for (const auto &vertex : vertices) {
          float x = vertex[0];
          float y = vertex[1];
          float z = vertex[2];

          stream.writeRawData(reinterpret_cast<char*>(&x), 4);
          stream.writeRawData(reinterpret_cast<char*>(&y), 4);
          stream.writeRawData(reinterpret_cast<char*>(&z), 4);
//          stream << float(vertex[0]) << float(vertex[1]) << float(vertex[2]);
        }

        const std::vector<GLuint> &indices = mesh.indices();
        if (indices.empty()) {
          for(uint32_t i = 0; i < vertices.size(); ++i) {
            stream << i;
          }
        } else {
          for (auto index : indices) {
            uint32_t v = index;
            stream.writeRawData(reinterpret_cast<char*>(&v), 4);
//            stream << uint32_t(index);
          }
        }
      } else {
        throw std::runtime_error("Unsupported mesh type.");
      }
    } else {
      CHECK(m_writeFormats.contains(format));

      auto sc = std::make_unique<aiScene>();
      sc->mRootNode = new aiNode;
      sc->mRootNode->mName.Set("modelName");

      // Create nodes for the whole scene
      std::vector<aiMesh*> meshArray;
      createNodes(mesh, sc->mRootNode, sc.get(), meshArray);

      // Create mesh pointer buffer for this scene
      if (sc->mNumMeshes > 0) {
        sc->mMeshes = new aiMesh* [meshArray.size()];
        for (size_t index = 0; index < meshArray.size(); index++) {
          sc->mMeshes[index] = meshArray[index];
        }
      }

      //
      createMaterials(sc.get());

      Assimp::Exporter exporter;
      const aiExportDataBlob *blob = exporter.ExportToBlob(sc.get(), format);
      if (blob == NULL) {
        throw ZIOException(exporter.GetErrorString());
      } else {
        result.append(static_cast<const char*>(blob->data), blob->size);
      }
    }
  } catch (const ZException& e) {
    throw ZIOException(QString("Can not save mesh: %1").arg(e.what()));
  }

  return result;
}

void ZMeshIO::readAllenAtlasMesh(const QString& filename, std::vector<glm::vec3>& normals,
                                 std::vector<glm::vec3>& vertices, std::vector<GLuint>& indices) const
{
  std::ifstream inputFileStream;
  openFileStream(inputFileStream, filename, std::ios::in | std::ios::binary);

  uint32_t numPoints;
  readStream(inputFileStream, &numPoints, 4);

  normals.resize(numPoints);
  vertices.resize(numPoints);

  //read points
  // 3 floats for normals followed by 3 floats for coordinates per point
  for (uint32_t i = 0; i < numPoints; ++i) {
    readStream(inputFileStream, &normals[i][0], 3 * 4);
    readStream(inputFileStream, &vertices[i][0], 3 * 4);
  }

  uint32_t numTriangleStrips = 0;
  readStream(inputFileStream, &numTriangleStrips, 4);
  std::vector<std::vector<uint32_t>> allStrips;

  // read triangle strips
  for (uint32_t i = 0; i < numTriangleStrips; ++i) {
    //read number of points in the strip
    uint32_t numPointsInStrip = 0;
    readStream(inputFileStream, &numPointsInStrip, 2);

    if (numPointsInStrip > 0) {
      std::vector<uint32_t> strip(numPointsInStrip);
      readStream(inputFileStream, strip.data(), numPointsInStrip * 4);
      allStrips.push_back(strip);
    }
  }
  inputFileStream.close();

  size_t numTriangles = 0;
  for (uint32_t i = 0; i < numTriangleStrips; ++i) {
    numTriangles += allStrips[i].size() - 2;
  }
  indices.resize(numTriangles * 3);

  size_t triIdx = 0;
  for (uint32_t i = 0; i < numTriangleStrips; ++i) {
    for (size_t j = 0; j < allStrips[i].size() - 2; ++j) {
      //Indicies in the triStripIndices are like: ABCDEFG
      //We need to change them to be ABC CBD CDE EDF EFG(note swapping of ordering)
      if (j % 2 == 0) {
        indices[triIdx * 3] = allStrips[i][j];
        indices[triIdx * 3 + 1] = allStrips[i][j + 1];
        indices[triIdx * 3 + 2] = allStrips[i][j + 2];
        ++triIdx;
      } else {
        indices[triIdx * 3] = allStrips[i][j + 1];
        indices[triIdx * 3 + 1] = allStrips[i][j];
        indices[triIdx * 3 + 2] = allStrips[i][j + 2];
        ++triIdx;
      }
    }
  }
}

void ZMeshIO::readNgMesh(std::istream &stream, ZMesh &mesh) const
{
  try {
    uint32_t numVertices = 0;
    stream.read(reinterpret_cast<char*>(&numVertices), 4);
    std::vector<glm::dvec3> vertices;
    for (uint32_t i = 0; i < numVertices; ++i) {
      float x = 0;
      float y = 0;
      float z = 0;
      stream.read(reinterpret_cast<char*>(&x), 4);
      stream.read(reinterpret_cast<char*>(&y), 4);
      stream.read(reinterpret_cast<char*>(&z), 4);
      vertices.emplace_back(x, y, z);
    }

    std::vector<GLuint> indices;
    while (stream.good()) {
      uint32_t index = 0;
      stream.read(reinterpret_cast<char*>(&index), 4);
      if (stream.good()) {
        indices.push_back(index);
      }
    }

    mesh.setVertices(vertices);
    mesh.setIndices(indices);
  } catch (std::exception &e) {
    LERROR() << e.what();
    throw std::runtime_error("Failed to decode NG mesh");
  }
}

/*NG mesh format
 * 4B: #vertices
 * #vertices * 4 * 3B: vertices with float for each coordinate.
 *    (x,y,z), (x,y,z), (x,y,z), ...
 * Remain: indices, each 4 bytes as uint32
 */
void ZMeshIO::readNgMeshFromMemory(
    const char *data, size_t size, ZMesh &mesh) const
{
  ZMemoryInputStream stream(data, size);
  readNgMesh(stream, mesh);

  /*
  try {
    ZMemoryInputStream stream(data, size);
    uint32_t numVertices = 0;
    stream >> numVertices;
    std::vector<glm::dvec3> vertices;
    for (uint32_t i = 0; i < numVertices; ++i) {
      float x = 0;
      float y = 0;
      float z = 0;
      stream >> x >> y >> z;
      vertices.emplace_back(x, y, z);
    }

    size_t numIndices = (size - numVertices * 12 - 4) / 4;
    std::vector<GLuint> indices(numIndices);
    for (size_t i = 0; i < numIndices; ++i) {
      uint32_t index = 0;
      stream >> index;
      indices[i] = index;
    }

    mesh.setVertices(vertices);
    mesh.setIndices(indices);
  } catch (std::exception &e) {
    LERROR() << e.what();
    throw std::runtime_error("Failed to decode NG mesh");
  }
  */
}

void ZMeshIO::readDracoMeshFromMemory(
    const char *data, size_t size, ZMesh &mesh) const
{
  draco::DecoderBuffer buffer;
  buffer.Init(data, size);

  // Decode the input data into a geometry.
  std::unique_ptr<draco::PointCloud> pc;
  draco::Mesh *msh = nullptr;
  auto type_statusor = draco::Decoder::GetEncodedGeometryType(&buffer);
  if (!type_statusor.ok()) {
    throw ZIOException(QString("failed to decode the draco file %1").arg(type_statusor.status().error_msg()));
  }
  const draco::EncodedGeometryType geom_type = type_statusor.value();
  if (geom_type == draco::TRIANGULAR_MESH) {
    draco::Decoder decoder;
    auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
    if (!statusor.ok()) {
      LERROR() << statusor.status().error_msg_string();
      throw ZIOException(QString("failed to decode the draco file %1").arg(type_statusor.status().error_msg()));
    }
    std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();
    if (in_mesh) {
      msh = in_mesh.get();

      // Draco encoding may cause duplication of vertices data.
      // De-duplicate after decoding.
      // Note: These functions are not defined unless you build with a special preprocessor definition:
      //       Make sure NeuTu is built with -D DRACO_ATTRIBUTE_DEDUPLICATION_SUPPORTED=1
      msh->DeduplicateAttributeValues();
      msh->DeduplicatePointIds();

      pc = std::move(in_mesh);
    }
  } else if (geom_type == draco::POINT_CLOUD) {
    // Failed to decode it as mesh, so let's try to decode it as a point cloud.
    draco::Decoder decoder;
    auto statusor = decoder.DecodePointCloudFromBuffer(&buffer);
    if (!statusor.ok()) {
      LERROR() << statusor.status().error_msg_string();
      throw ZIOException(QString("failed to decode the draco file %1").arg(type_statusor.status().error_msg()));
    }
    pc = std::move(statusor).value();
  }

  if (!pc) {
    throw ZIOException("failed to decode the draco file");
  }

  getDracoVertices(*pc, mesh.m_vertices);
  getDracoColors(*pc, mesh.m_colors);
  getDracoNormals(*pc, mesh.m_normals);
  getDracoTextureCoordinates(
        *pc, mesh.m_1DTextureCoordinates, mesh.m_2DTextureCoordinates,
        mesh.m_3DTextureCoordinates);
  if (msh) {
    getDracoFaces(*msh, mesh.m_indices);
  }
}

draco::Mesh* ZMeshIO::ToDracoMesh(const ZMesh &zmesh, draco::Mesh *dmesh)
{
  if (dmesh == nullptr) {
    dmesh = new draco::Mesh;
  }

  const std::vector<glm::vec3>& vertices = zmesh.vertices();
  draco::GeometryAttribute va;
  va.Init(draco::GeometryAttribute::POSITION, nullptr, 3, draco::DT_FLOAT32,
          false, sizeof(float) * 3, 0);
  int attId = dmesh->AddAttribute(va, true, vertices.size());
  dmesh->SetAttributeElementType(attId, draco::MESH_VERTEX_ATTRIBUTE);
  dmesh->set_num_points(vertices.size());

  for (size_t i = 0; i < vertices.size(); ++i) {
    float val[3];
    for (size_t j = 0; j < 3; ++j) {
      val[j] = vertices[i][j];
    }
    dmesh->attribute(attId)->SetAttributeValue(draco::AttributeValueIndex(i), val);
  }

  for (size_t i = 0; i < zmesh.numTriangles(); ++i) {
    glm::uvec3 triangle = zmesh.triangleIndices(i);
    draco::Mesh::Face face;
    face[0] = draco::PointIndex(triangle[0]);
    face[1] = draco::PointIndex(triangle[1]);
    face[2] = draco::PointIndex(triangle[2]);
    dmesh->AddFace(face);
  }

  return dmesh;
}

void ZMeshIO::writeDracoMesh(const QString &filename, const ZMesh &mesh) const
{
  std::ofstream stream;
  stream.open(filename.toStdString(), std::ios::binary);
  if (stream.good()) {
    draco::Mesh *dmesh = ToDracoMesh(mesh, nullptr);
    if (dmesh) {
      draco::Encoder encoder;
      draco::EncoderBuffer buffer;
      encoder.EncodeMeshToBuffer(*dmesh, &buffer);
      if (buffer.size() > 0) {
        stream.write(buffer.data(), buffer.size());
      }
      delete dmesh;
    }
  }
}

void ZMeshIO::readDracoMesh(const QString& filename, ZMesh& mesh) const
{
  std::ifstream inputFileStream;
  openFileStream(inputFileStream, filename, std::ios::in | std::ios::binary | std::ios::ate);
  std::streamsize size = inputFileStream.tellg();
  if (size <= 0) {
    throw ZIOException("can not get draco file size or empty file");
  }
  inputFileStream.seekg(0, std::ios::beg);
  std::vector<char> data(size);
  readStream(inputFileStream, data.data(), size);
  inputFileStream.close();

  readDracoMeshFromMemory(data.data(), data.size(), mesh);
}

void ZMeshIO::readNgMesh(const QString &filename, ZMesh &mesh) const
{
  std::ifstream stream;
  openFileStream(stream, filename, std::ios::in | std::ios::binary);

  readNgMesh(stream, mesh);
}
