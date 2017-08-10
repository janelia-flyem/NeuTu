#include "zmeshio.h"

#include "zmesh.h"
#include "zioutils.h"
#include "zexception.h"
#include "QsLog.h"
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <QFile>
#include <memory>

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
  pMesh->mNormals = new aiVector3D[pMesh->mNumVertices];
  const std::vector<glm::vec3>& vertices = mesh.vertices();
  const std::vector<glm::vec3>& normals = mesh.normals();
  CHECK(normals.size() >= vertices.size());
  memcpy(pMesh->mVertices, vertices.data(), sizeof(float) * 3 * vertices.size());
  memcpy(pMesh->mNormals, normals.data(), sizeof(float) * 3 * vertices.size());

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

  m_readFilter = QString("All Mesh files (*.") + m_readExts.join(" *.") + QString(")");

  Assimp::Exporter exporter;
  for (size_t i = 0; i < exporter.GetExportFormatCount(); ++i) {
    const aiExportFormatDesc* format = exporter.GetExportFormatDescription(i);
    m_writeExts.push_back(format->fileExtension);
    m_writeFormats.push_back(format->id);
    QString filter = format->description;
    filter += QString(" (*.%1)").arg(format->fileExtension);
    m_writeFilters.push_back(filter);
  }
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

void ZMeshIO::load(const QString& filename, ZMesh& mesh) const
{
  try {
    mesh.clear();
    mesh.setType(GL_TRIANGLES);
    if (filename.endsWith(".msh", Qt::CaseInsensitive)) {
      readAllenAtlasMesh(filename, mesh.m_normals, mesh.m_vertices, mesh.m_indices);
    } else {
      Assimp::Importer importer;
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

      aiMesh* msh = scene->mMeshes[0];
      mesh.m_vertices.resize(msh->mNumVertices);
      mesh.m_normals.resize(msh->mNumVertices);
      memcpy(mesh.m_vertices.data(), msh->mVertices, sizeof(float) * 3 * mesh.m_vertices.size());
      memcpy(mesh.m_normals.data(), msh->mNormals, sizeof(float) * 3 * mesh.m_vertices.size());

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
      for (int i = 0; i < m_writeExts.size(); ++i) {
        if (filename.endsWith(QString(".%1").arg(m_writeExts[i]), Qt::CaseInsensitive))
          format = m_writeFormats[i];
      }
    }
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
    aiReturn res = exporter.Export(sc.get(), format, QFile::encodeName(filename).constData());
    if (res != aiReturn_SUCCESS) {
      throw ZIOException(exporter.GetErrorString());
    }
  }
  catch (const ZException& e) {
    throw ZIOException(QString("Can not save mesh %1: %2").arg(filename).arg(e.what()));
  }
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
