#ifndef MENU_H
#define MENU_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QPropertyAnimation>
#include <QPoint>
#include <QObject>
#include <QLineEdit>

#include <QPainter> // Add this include for QPainter
#include <QPaintEvent> // Add this include for QPaintEvent
#include <QPixmap> // Add this include for QPixmap
#include <QResizeEvent>

class Login;
class Sign_up;

QT_BEGIN_NAMESPACE
namespace Ui {
class Menu;
}
QT_END_NAMESPACE

class Menu : public QMainWindow
{
    Q_OBJECT

public:
    Menu(QWidget *parent = nullptr);
    ~Menu();
    Ui::Menu *ui;
public slots:

    void on_buttonCover_clicked();
protected:
    void paintEvent(QPaintEvent *) override;

private slots:
    void on_login_clicked();

    void on_signUp_clicked();

    void on_buttonForget_clicked();

private:
    QPropertyAnimation *animation   ;
    bool isLoginMode = true;
    QPoint startPos;
    QPoint endPos;
    void setInfoCover();
    Login* login;
    Sign_up* sign_up;
    void cleanLabels();
};
#endif // MENU_H
