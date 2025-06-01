#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <functional>
#include <QFile>
#include "panel_user.h"

Login::Login(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Login)
    , currentUser(nullptr)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

Login::~Login()
{
    delete ui;
}

void Login::on_buttonBox_accepted()
{
    QString user_name = ui->lineEdit->text();
    QString password = ui->lineEdit_2->text();

    if (user_name.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter both username and password");
        return;
    }

    if (checkCredentials(user_name, password)) {
        QMessageBox::information(this, "Success", "Login successful!");
        Panel_User *p = new Panel_User();
        p->set_curuser(getCurrentUser());
        p->show();
        this->parentWidget()->close();
    } else {
        QMessageBox::critical(this, "Error", "Invalid username or password");
    }
}

bool Login::checkCredentials(const QString& username, const QString& password)
{
    QFile file("users.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open users file");
        return false;
    }

    // Hash the input password for comparison
    std::hash<std::string> hasher;
    std::string pwd = password.toStdString();
    QString hashedPassword = QString::fromStdString(std::to_string(hasher(pwd)));

    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        QStringList fields = line.split(",");

        if (fields.size() >= 6 &&  // Check for all required fields
            fields[0] == username &&
            fields[2] == hashedPassword) {

            currentUser.set_user_name(fields[0]);
            currentUser.set_name(fields[1]);
            currentUser.set_password(fields[2]);
            currentUser.set_email(fields[3]);
            currentUser.set_age(fields[4].toInt());
            currentUser.set_male(fields[5] == "1" ? true : false);

            file.close();
            return true;  // Found matching user
        }
    }

    file.close();
    return false;  // No matching user found
}

User& Login::getCurrentUser()
{
    return currentUser;
}
