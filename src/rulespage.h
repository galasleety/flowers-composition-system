#ifndef RULESPAGE_H
#define RULESPAGE_H

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QFrame>
#include <QTextEdit>

class RulesPage : public QWidget {
    Q_OBJECT
public:
    explicit RulesPage(QWidget *parent = nullptr);
    void loadRules();

private slots:
    void onSearchTextChanged(const QString &text);
    void onRuleSelected();
    void onDeleteRuleClicked();
    void onEditRuleClicked();
    void onAddRuleClicked();
    void onItemAChanged(int index);
    void onItemBChanged(int index);
    void onSliderValueChanged(int value);

private:
    void setupUI();
    void loadItemsIntoCombos();
    void updateItemImage(int itemId, QLabel *imgLabel, int size);
    QPixmap getRoundedPixmap(const QString &path, int logicalSize);
    QWidget* createRuleItemWidget(const QString &imgA, const QString &imgB, const QString &title, const QString &desc, int compLevel);
    void showOverlay();
    void hideOverlay();

    QWidget *m_overlay = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_rulesList = nullptr;
    QPushButton *m_btnAddRule = nullptr;

    QWidget *m_rightPanel = nullptr;
    QScrollArea *m_mainScroll = nullptr;
    QWidget *m_scrollContent = nullptr;

    QFrame *m_headerCard = nullptr;
    QLabel *m_ruleTitleLabel = nullptr;
    QLabel *m_ruleIdLabel = nullptr;
    QPushButton *m_btnEditRule = nullptr;
    QPushButton *m_btnDeleteRule = nullptr;

    QFrame *m_formulaCard = nullptr;
    QLabel *m_imgItemA = nullptr;
    QLabel *m_nameItemA = nullptr;
    QComboBox *m_comboItemA = nullptr;
    QLabel *m_imgItemB = nullptr;
    QLabel *m_nameItemB = nullptr;
    QComboBox *m_comboItemB = nullptr;

    QFrame *m_paramsCard = nullptr;
    QTextEdit *m_textEditFullDesc = nullptr;
    QSlider *m_sliderComp = nullptr;
    QLabel *m_lblCompValue = nullptr;

    bool m_isEditing = false;
    bool m_isAddingNew = false;
};

#endif // RULESPAGE_H