#ifndef LOGIN_H
#define LOGIN_H

#include <QDialog>
#include "user.h"

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = nullptr);
    ~Login();
    bool checkCredentials(const QString &username, const QString &password);
    virtual User& getCurrentUser();
private slots:
    void on_buttonBox_accepted();

    void on_pushButton_clicked();

private:
    Ui::Login *ui;
    User currentUser;
};

#endif // LOGIN_H
