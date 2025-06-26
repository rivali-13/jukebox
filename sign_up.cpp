#include "sign_up.h"
#include "menu.h"
#include "ui_menu.h"
#include "user.h"
#include <functional>  // for std::hash
#include <QMessageBox>
#include <QFile>

Sign_up::Sign_up(Menu* _menu, QObject* parent): QObject(parent), menu(_menu){

}

QString Sign_up::twoWayEncrypt(const QString& input) {
    return QString(input.toUtf8().toBase64());
}


void Sign_up::create_account()
{
    bool isOk = true;
    menu->ui->errUpUser->setText("");
    menu->ui->errUpName->setText("");
    menu->ui->errUpPass->setText("");
    menu->ui->errUpEmail->setText("");

    if (menu->ui->upUsername->text().isEmpty()){
        menu->ui->errUpUser->setText("نام کاربری نمیتواند خالی باشد");
        isOk = false;
    }
    if (menu->ui->upName->text().isEmpty()){
        menu->ui->errUpName->setText("نام نمیتواند خالی باشد");
        isOk = false;
    }
    if (menu->ui->upPassword->text().isEmpty()){
        menu->ui->errUpPass->setText("رمزعبور نمیتواند خالی باشد");
        isOk = false;
    }
    if (menu->ui->upEmail->text().isEmpty()){
        menu->ui->errUpEmail->setText("ایمیل نمیتواند خالی باشد");
        isOk = false;
    }
    if (!isOk) return;
    // QMessageBox::critical(menu, "Error", "Invalid value");

    std::unique_ptr<User> u = std::make_unique<User>();
    u->set_user_name(twoWayEncrypt(menu->ui->upUsername->text()));
    u->set_name(twoWayEncrypt(menu->ui->upName->text()));
    QString rawPassword = menu->ui->upPassword->text();
    std::string hashedPassword = hashPassword(rawPassword);
    u->set_password(QString::fromStdString(hashedPassword));
    u->set_email(twoWayEncrypt(menu->ui->upEmail->text()));
    if (saveUserToFile(*u)) {
        menu->on_buttonCover_clicked();
        QMessageBox::information(menu, "Success", "User registered successfully!");
    }

    menu->ui->errUpUser->setText("");
    menu->ui->errUpName->setText("");
    menu->ui->errUpPass->setText("");
    menu->ui->errUpEmail->setText("");
}


bool Sign_up::saveUserToFile(User& user) {
    QFile file("users.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(menu, "Error", "Could not open file for writing");
        return false;
    }

    QTextStream out(&file);
    out << user.get_user_name() << ","
        << user.get_name() << ","
        << user.get_password() << ","
        << user.get_email() << "\n";

    file.close();
    return true;
}

std::string Sign_up::hashPassword(QString& password) {
    std::hash<std::string> hasher;
    std::string pwd = password.toStdString();
    size_t hash = hasher(pwd);
    return std::to_string(hash);
}

