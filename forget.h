#ifndef FORGET_H
#define FORGET_H

#include <QDialog>
#include <QMessageBox>
#include <QFile>
#include "login.h"

namespace Ui {
class Forget;
}

class Forget : public QDialog
{
    Q_OBJECT

public:
    explicit Forget(QWidget *parent = nullptr);
    ~Forget();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::Forget *ui;
    bool checkCredentials(const QString &user_name, const QString &email, const QString &pass);
    std::string hashPassword(QString &password);
};

#endif // FORGET_H
