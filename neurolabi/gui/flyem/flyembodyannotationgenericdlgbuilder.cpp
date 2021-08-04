#include "flyembodyannotationgenericdlgbuilder.h"

#include "dialogs/zgenericbodyannotationdialog.h"
#include "zflyembodyannotation.h"
#include "neutube.h"

FlyEmBodyAnnotationGenericDlgBuilder::FlyEmBodyAnnotationGenericDlgBuilder()
{

}

FlyEmBodyAnnotationGenericDlgBuilder&
FlyEmBodyAnnotationGenericDlgBuilder::getDialogFrom(
    std::function<ZGenericBodyAnnotationDialog*()> f)
{
  m_dlgGetter = f;

  return *this;
}

ZSegmentAnnotationBuilder&
FlyEmBodyAnnotationGenericDlgBuilder::fromOldAnnotation(const ZJsonObject &obj)
{
  if (m_dlgGetter) {
    ZGenericBodyAnnotationDialog *dlg = m_dlgGetter();
    if (dlg) {
      dlg->loadJsonObject(obj);
      std::string lastModifiedBy =
          ZFlyEmBodyAnnotation::GetLastModifiedBy(obj);
      std::string oldNamingUser =
          ZFlyEmBodyAnnotation::GetNamingUser(obj);
      QString label =
          (m_bodyId > 0) ? QString("Body ID: <b>%1</b>").arg(m_bodyId) : "";
      QString separator = "<br>";
      if (!lastModifiedBy.empty()) {
        label += separator +
            QString("Previously annotated by <em>%1</em>").arg(
              lastModifiedBy.c_str());
        separator = " ;  ";
      }
      if (!oldNamingUser.empty()) {
        label += separator +
            QString("Named by <em>%1</em>").arg(oldNamingUser.c_str());
      }
      if (!label.isEmpty()) {
        dlg->setLabel(label);
      }

      if (dlg->exec()) {
        m_annotation = dlg->toJsonObject();
        std::string user = neutu::GetCurrentUserName();
        ZFlyEmBodyAnnotation::UpdateUserFields(
              m_annotation, user, obj);
      } else {
        m_annotation = ZJsonObject::MakeNull();
      }
    }
  }

  return *this;
}
