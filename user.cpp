#include "user.h"
#include "ui_user.h"

User::User(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::User)
{
    ui->setupUi(this);
}

User::User(bool _male, QString _name, QString _user_name, QString _email, QString _password, QWidget *parent)
{
    name = _name;
    user_name = _user_name;
    password = _password;
    email = _email;
    male = _male;
}

bool User::get_male()
{
    return male;
}

QString User::get_name()
{
    return name;
}

QString User::get_user_name()
{
    return user_name;
}

QString User::get_email()
{
    return email;
}

QString User::get_password()
{
    return password;
}

void User::set_name(QString _name)
{
    name = _name;
}

void User::set_user_name(QString _user_name)
{
    user_name = _user_name;
}

void User::set_email(QString _email)
{
    email = _email;
}

void User::set_password(QString _password)
{
    password = _password;
}

void User::set_male(bool _male)
{
    male = _male;
}



User::~User()
{
    delete ui;
}
