#include "forget.h"
#include "ui_forget.h"
//#include <QDebug>

Forget::Forget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Forget)
{
    ui->setupUi(this);
}

Forget::~Forget()
{
    delete ui;
}

QString Forget::twoWayEncrypt(const QString& input) {
    return QString(input.toUtf8().toBase64());
}

void Forget::on_buttonBox_accepted()
{
    QString user_name = ui->lineEdit->text();
    QString email = ui->lineEdit_2->text();
    QString rawPassword = ui->lineEdit_3->text();
    std::string hashedPasswordStd = hashPassword(rawPassword);
    QString hashedPassword = QString::fromStdString(hashedPasswordStd);

    QString encodedUserName = twoWayEncrypt(user_name);
    QString encodedEmail = twoWayEncrypt(email);


    if (checkCredentials(encodedUserName, encodedEmail, hashedPassword)){
        QMessageBox::information(this, "Success", "password changed successfully!");
    } else {
        QMessageBox::critical(this, "Error", "Username and email do not match any account");
    }

}

bool Forget::checkCredentials(const QString &user_name, const QString &email, const QString &pass)
{
    QFile inputFile("users.txt");
    QFile tempFile("users_temp.txt");
    bool updated = false;

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not open users file");
        return false;
    }
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error", "Could not open temporary file");
        inputFile.close();
        return false;
    }

    QTextStream in(&inputFile);
    QTextStream out(&tempFile);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        QStringList fields = line.split(",");
        for (QString &field : fields) field = field.trimmed();
        //qDebug() << "Comparing:" << fields[0] << user_name << fields[3] << email;
        if (fields.size() >= 4 &&
            fields[0] == user_name &&
            fields[3] == email) {
            fields[2] = pass; // Update password
            updated = true;
        }
        out << fields.join(",") << "\n";
    }

    inputFile.close();
    tempFile.close();

    if (updated) {
        // Replace original file with updated temp file
        if (!QFile::remove("users.txt") || !QFile::rename("users_temp.txt", "users.txt")) {
            QMessageBox::critical(this, "Error", "Could not update users file");
            return false;
        }
    } else {
        QFile::remove("users_temp.txt");
    }

    return updated;
}

std::string Forget::hashPassword(QString& password) {
    std::hash<std::string> hasher;
    std::string pwd = password.toStdString();
    size_t hash = hasher(pwd);
    return std::to_string(hash);
}
