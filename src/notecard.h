#ifndef NOTECARD_H
#define NOTECARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class NoteCard : public QWidget {
    Q_OBJECT
public:
    explicit NoteCard(int id, const QString &title, const QString &content, QWidget *parent = nullptr);

signals:
    void editRequested(int id);
    void deleteRequested(int id);

protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    int m_id;
    bool m_isHovered;
    void updateStyle();
};

#endif // NOTECARD_H