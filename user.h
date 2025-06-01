#ifndef USER_H
#define USER_H

#include <QWidget>

namespace Ui {
class User;
}

class User : public QWidget
{
    Q_OBJECT

public:
    explicit User(QWidget *parent = nullptr);
    User(bool _male, QString _name, QString _user_name, QString _email, QString _password, QWidget *parent = nullptr);
    bool get_male();
    QString get_name();
    QString get_user_name();
    QString get_email();
    QString get_password();
    void set_name(QString _name);
    void set_user_name(QString _user_name);
    void set_email(QString _email);
    void set_password(QString _password);
    void set_male(bool _male);
    ~User();

private:
    Ui::User *ui;
    QString name;
    QString user_name;
    QString email;
    QString password;
    bool male;
};

#endif // USER_H
