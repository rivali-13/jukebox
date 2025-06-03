#ifndef SIGN_IN_H
#define SIGN_IN_H

#include <QDialog>
#include "user.h"
namespace Ui {
class Sign_in;
}

class Sign_in : public QDialog
{
    Q_OBJECT

public:
    explicit Sign_in(QWidget *parent = nullptr);
    ~Sign_in();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Sign_in *ui;
    bool saveUserToFile(User& user);
    std::string hashPassword(QString& password);

    QString twoWayEncrypt(const QString &input);
};

#endif // SIGN_IN_H
