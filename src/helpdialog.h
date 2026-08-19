#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class HelpDialog : public QDialog {
    Q_OBJECT

public:
    enum Mode { UserMode, AdminMode };

    explicit HelpDialog(Mode mode, QWidget *parent = nullptr);
    static void execWithOverlay(Mode mode, QWidget *parent);

protected:

private slots:
    void onMenuToggled(int id, bool checked);
    void fadeOutAndClose(); // НОВЫЙ СЛОТ ДЛЯ ПЛАВНОГО ЗАКРЫТИЯ

private:
    void setupUI();
    void setupUserContent();
    void setupAdminContent();

    QPushButton* createMenuButton(const QString &text);
    QWidget* createContentPage(const QString &title, const QString &contentHtml);

    Mode m_mode;
    QStackedWidget *m_contentStack;
    QButtonGroup *m_menuGroup;
    QVBoxLayout *m_menuLayout;
};

#endif // HELPDIALOG_H