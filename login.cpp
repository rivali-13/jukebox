#include "login.h"
#include "ui_menu.h"
#include "menu.h"
#include <QMessageBox>
#include <functional>
#include <QFile>
#include "panel_user.h"

Login::Login(Menu* _menu, QObject* parent): QObject(parent), menu(_menu){

}

QString Login::twoWayEncrypt(const QString& input) {
    return QString(input.toUtf8().toBase64());
}


void Login::doLogin()
{
    menu->ui->errLogUser->setText("");
    menu->ui->errLogPass->setText("");

    QString user_name = menu->ui->logUsername->text();
    QString password = menu->ui->logPassword->text();

    if (user_name.isEmpty()) {
        //QMessageBox::warning(menu, "Error", "Please enter both username and password");
        menu->ui->errLogUser->setText("لطفا نام کاربری خود را وارد کنید");
        return;
    }
    else if(password.isEmpty()){
        menu->ui->errLogPass->setText("لطفا رمز عبور خود را وارد کنید");
        return;
    }

    if (checkCredentials(user_name, password)) {
        QMessageBox::information(menu, "Success", "Login successful!");
        Panel_User *p = new Panel_User();
        p->set_curuser(getCurrentUser());
        p->show();
        menu->close();
    } else {
        // QMessageBox::critical(menu, "Error", "Invalid username or password");
    }
}

bool Login::checkCredentials(const QString& username, const QString& password)
{
    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // QMessageBox::critical(menu, "Error", "Could not open users file");
        return false;
    }

    // Hash the input password for comparison
    std::hash<std::string> hasher;
    std::string pwd = password.toStdString();
    QString hashedPassword = QString::fromStdString(std::to_string(hasher(pwd)));

    // Encode the username for comparison (to match registration)
    QString encodedUsername = twoWayEncrypt(username);

    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        QStringList fields = line.split(",");

        if (fields.size() >= 4 && fields[0] == encodedUsername ){

            if (fields[2] == hashedPassword){
                currentUser.set_user_name(fields[0]);
                currentUser.set_name(fields[1]);
                currentUser.set_password(fields[2]);
                currentUser.set_email(fields[3]);

                file.close();
                return true;
            }
            else {
                menu->ui->errLogPass->setText("رمز عبور نادرست است");
                return false;
            }
        }
    }
    menu->ui->errLogUser->setText("کاربری با این نام پیدا نشد");

    file.close();
    return false;
}

User& Login::getCurrentUser()
{
    return currentUser;
}


