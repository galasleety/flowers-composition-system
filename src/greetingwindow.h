#ifndef GREETINGWINDOW_H
#define GREETINGWINDOW_H

#include <QWidget>

class GreetingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GreetingWindow(const QString &username, bool isLogin, bool isAdmin = false, QWidget *parent = nullptr);
    ~GreetingWindow();

signals:
    void changeBackgroundRequested();
    void finished();

protected:
    void paintEvent(QPaintEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    void fadeOutAndClose();

private:
    bool m_isAdmin;
};

#endif // GREETINGWINDOW_H