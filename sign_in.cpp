#include "sign_in.h"
#include "ui_sign_in.h"
#include "user.h"
#include <functional>  // for std::hash
#include <QMessageBox>
#include <QFile>

Sign_in::Sign_in(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Sign_in)
{
    ui->setupUi(this);
    ui->lineEdit->setFocus();
}

Sign_in::~Sign_in()
{
    delete ui;
}

void Sign_in::on_buttonBox_accepted()
{
    if (ui->lineEdit->text().isEmpty()||
        ui->lineEdit_2->text().isEmpty()||
        ui->lineEdit_3->text().isEmpty()||
        ui->lineEdit_4->text().isEmpty()){
        QMessageBox::critical(this, "Error", "Invalid value");
        return;
    }
    
    std::unique_ptr<User> u = std::make_unique<User>();
    u->set_user_name(ui->lineEdit->text());
    u->set_name(ui->lineEdit_2->text());
    QString rawPassword = ui->lineEdit_3->text();
    std::string hashedPassword = hashPassword(rawPassword);
    u->set_password(QString::fromStdString(hashedPassword));
    u->set_email(ui->lineEdit_4->text());
    if (saveUserToFile(*u)) {
        QMessageBox::information(this, "Success", "User registered successfully!");
        this->close();
    }
}

bool Sign_in::saveUserToFile(User& user) {
    QFile file("users.txt");
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open file for writing");
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

std::string Sign_in::hashPassword(QString& password) {
    std::hash<std::string> hasher;
    std::string pwd = password.toStdString();
    size_t hash = hasher(pwd);
    return std::to_string(hash);
}

