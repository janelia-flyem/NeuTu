#ifndef ZWATERSHEDMODULE
#define ZWATERSHEDMODULE
#include"zsandboxmodule.h"
#include<QWidget>
class QLineEdit;
class QPushButton;
class QVBoxLayout;
class QLabel;
class ZWaterShedGUI:public QWidget
{
  Q_OBJECT
  public:
    explicit ZWaterShedGUI(QWidget* parent=0);
    ~ZWaterShedGUI();
    void initGui();
    void reset();
  protected:
    void closeEvent(QCloseEvent* event);
  signals:
  private slots:
    void onStartSegment();
    void onAddSeed();
    void onMousePressed(QMouseEvent* event);
  private:
    std::vector<QLineEdit*> m_line_edits;
    std::vector<QLabel*>    m_labels;
    QPushButton* m_add_seed_btn;
    int m_max_seed_index;
    int m_cur_seed_index;
    QVBoxLayout* m_lay;
};

class ZWaterShedModule:public ZSandboxModule
{
  Q_OBJECT
  public:
  explicit ZWaterShedModule(QObject *parent = 0);
    ~ZWaterShedModule();
  signals:
  private slots:
    void execute();
  private:
    void init();
  private:
    ZWaterShedGUI* m_gui;
};

#endif // ZWATERSHEDMODULE

