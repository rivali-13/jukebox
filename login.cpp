#include "login.h"
#include "ui_login.h"
#include <QMessageBox>
#include <functional>
#include <QFile>
#include "panel_user.h"
#include "forget.h"

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

QString Login::twoWayEncrypt(const QString& input) {
    return QString(input.toUtf8().toBase64());
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

    // Encode the username for comparison (to match registration)
    QString encodedUsername = twoWayEncrypt(username);

    while (!file.atEnd()) {
        QString line = file.readLine().trimmed();
        QStringList fields = line.split(",");

        if (fields.size() >= 4 &&
            fields[0] == encodedUsername &&
            fields[2] == hashedPassword) {

            currentUser.set_user_name(fields[0]);
            currentUser.set_name(fields[1]);
            currentUser.set_password(fields[2]);
            currentUser.set_email(fields[3]);

            file.close();
            return true;
        }
    }

    file.close();
    return false;
}

User& Login::getCurrentUser()
{
    return currentUser;
}

void Login::on_pushButton_clicked()
{
    Forget* f = new Forget (this);
    f->show();
}

