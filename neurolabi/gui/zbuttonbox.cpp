#include "zbuttonbox.h"
#include <QHBoxLayout>

const ZButtonBox::TRole ZButtonBox::ROLE_NONE = 0;
const ZButtonBox::TRole ZButtonBox::ROLE_YES = 1;
const ZButtonBox::TRole ZButtonBox::ROLE_NO = 2;
const ZButtonBox::TRole ZButtonBox::ROLE_CONTINUE = 4;
const ZButtonBox::TRole ZButtonBox::ROLE_PAUSE = 8;

ZButtonBox::ZButtonBox(QWidget *parent) :
  QWidget(parent), m_yesButton(NULL), m_noButton(NULL), m_continueButton(NULL),
  m_paushButton(NULL)
{
  m_layout = new QHBoxLayout;
  setLayout(m_layout);
}

QPushButton* ZButtonBox::activate(ZButtonBox::TRole role)
{
  QPushButton *button = NULL;

  if (role & ROLE_YES) {
    if (m_yesButton == NULL) {
      m_yesButton = new QPushButton(this);
      m_yesButton->setText("OK");
      button = m_yesButton;

      connect(m_yesButton, SIGNAL(clicked()), this, SIGNAL(clickedYes()));

      m_layout->addWidget(m_yesButton);
    }
  }

  if (role & ROLE_NO) {
    if (m_noButton == NULL) {
      m_noButton = new QPushButton(this);
      m_noButton->setText("Cancel");
      button = m_noButton;

      connect(m_noButton, SIGNAL(clicked()), this, SIGNAL(clickedNo()));

      m_layout->addWidget(m_noButton);
    }
  }

  if (role & ROLE_CONTINUE) {
    if (m_continueButton == NULL) {
      m_continueButton = new QPushButton(this);
      m_continueButton->setText("Continue");
      button = m_continueButton;

      connect(m_continueButton, SIGNAL(clicked()),
              this, SIGNAL(clickedContinue()));

      m_layout->addWidget(m_continueButton);
    }
  }

  return button;
}
