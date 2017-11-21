#include "z3darrowrenderer.h"

Z3DArrowRenderer::Z3DArrowRenderer(Z3DRendererBase& rendererBase) :
  Z3DConeRenderer(rendererBase)
{
}

void Z3DArrowRenderer::setArrowData(std::vector<glm::vec4>* tailPosAndTailRadius,
                                    std::vector<glm::vec4>* headPosAndHeadRadius,
                                    float headLengthProportion)
{
  m_arrowConeBaseAndBaseRadius.clear();
  m_arrowConeAxisAndTopRadius.clear();

  for (size_t i = 0; i < tailPosAndTailRadius->size(); ++i) {
    glm::vec3 tail = (*tailPosAndTailRadius)[i].xyz();
    glm::vec3 head = (*headPosAndHeadRadius)[i].xyz();
    glm::vec3 cutPos = glm::mix(head, tail, headLengthProportion);
    m_arrowConeBaseAndBaseRadius.emplace_back(cutPos, (*tailPosAndTailRadius)[i].w);
    m_arrowConeAxisAndTopRadius.emplace_back(tail - cutPos, (*tailPosAndTailRadius)[i].w);
    m_arrowConeBaseAndBaseRadius.emplace_back(head, 0.f);
    m_arrowConeAxisAndTopRadius.emplace_back(cutPos - head, (*headPosAndHeadRadius)[i].w);
  }

  setData(&m_arrowConeBaseAndBaseRadius, &m_arrowConeAxisAndTopRadius);
}

void Z3DArrowRenderer::setFixedHeadLengthArrowData(std::vector<glm::vec4>* tailPosAndTailRadius,
                                                   std::vector<glm::vec4>* headPosAndHeadRadius,
                                                   float fixedHeadLength)
{
  m_arrowConeBaseAndBaseRadius.clear();
  m_arrowConeAxisAndTopRadius.clear();

  for (size_t i = 0; i < tailPosAndTailRadius->size(); ++i) {
    glm::vec3 tail = (*tailPosAndTailRadius)[i].xyz();
    glm::vec3 head = (*headPosAndHeadRadius)[i].xyz();
    float totalLength = glm::length(head - tail);
    glm::vec3 cutPos = head + glm::normalize(tail - head) *
                              (fixedHeadLength < totalLength ? fixedHeadLength : .5f * totalLength);
    m_arrowConeBaseAndBaseRadius.emplace_back(cutPos, (*tailPosAndTailRadius)[i].w);
    m_arrowConeAxisAndTopRadius.emplace_back(tail - cutPos, (*tailPosAndTailRadius)[i].w);
    m_arrowConeBaseAndBaseRadius.emplace_back(head, 0.f);
    m_arrowConeAxisAndTopRadius.emplace_back(cutPos - head, (*headPosAndHeadRadius)[i].w);
  }

  setData(&m_arrowConeBaseAndBaseRadius, &m_arrowConeAxisAndTopRadius);
}

void Z3DArrowRenderer::setArrowColors(std::vector<glm::vec4>* arrowColors)
{
  m_arrowConeColors.clear();

  for (auto color : *arrowColors) {
    m_arrowConeColors.push_back(color);
    m_arrowConeColors.push_back(color);
  }

  setDataColors(&m_arrowConeColors);
}

void Z3DArrowRenderer::setArrowColors(std::vector<glm::vec4>* arrowTailColors, std::vector<glm::vec4>* arrowHeadColors)
{
  m_arrowConeColors.clear();

  for (size_t i = 0; i < arrowTailColors->size(); ++i) {
    m_arrowConeColors.push_back((*arrowTailColors)[i]);
    m_arrowConeColors.push_back((*arrowHeadColors)[i]);
  }

  setDataColors(&m_arrowConeColors);
}

void Z3DArrowRenderer::setArrowPickingColors(std::vector<glm::vec4>* arrowPickingColors)
{
  m_arrowConePickingColors.clear();

  if (!arrowPickingColors) {
    setDataPickingColors(nullptr);
    return;
  }

  for (auto color : *arrowPickingColors) {
    m_arrowConePickingColors.push_back(color);
    m_arrowConePickingColors.push_back(color);
  }

  setDataPickingColors(&m_arrowConePickingColors);
}
