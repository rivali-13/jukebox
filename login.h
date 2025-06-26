#ifndef LOGIN_H
#define LOGIN_H

#include <QObject>
#include "user.h"
class Menu;
class User;

class Login: public QObject
{
    Q_OBJECT
public:
    Login(Menu* _menu,  QObject* parent = nullptr);
    bool checkCredentials(const QString &username, const QString &password);
    User& getCurrentUser();

public slots:
    void doLogin();


private:
    Menu* menu;
    User currentUser;
    QString twoWayEncrypt(const QString &input);

};

#endif // LOGIN_H
