#ifndef SIGN_UP_H
#define SIGN_UP_H

#include <QObject>

class Menu;
class User;

class Sign_up : public QObject
{
    Q_OBJECT
public:
    Sign_up(Menu* _menu, QObject* parent = nullptr);

public slots:
    void create_account();

private:
    Menu* menu;
    bool saveUserToFile(User& user);
    std::string hashPassword(QString& password);

    QString twoWayEncrypt(const QString &input);
};

#endif // SIGN_UP_H
