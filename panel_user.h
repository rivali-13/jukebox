#ifndef PANEL_USER_H
#define PANEL_USER_H

#include <QWidget>
#include "user.h"
#include "info.h"
#include "home.h" //***

namespace Ui {
class Panel_User;
}

class Panel_User : public QWidget
{
    Q_OBJECT

public:
    explicit Panel_User(QWidget *parent = nullptr);
    ~Panel_User();
    void set_curuser(User& _user);
private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked(); //***

private:
    Ui::Panel_User *ui;
    User currentUser;
};

#endif // PANEL_USER_H
