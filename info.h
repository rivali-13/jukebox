#ifndef INFO_H
#define INFO_H

#include <QWidget>
#include "user.h"

namespace Ui {
class Info;
}

class Info : public QWidget
{
    Q_OBJECT

public:
    explicit Info(QWidget *parent = nullptr);
    ~Info();
    void set_info(User& u);
private slots:
    void on_pushButton_clicked();

private:
    Ui::Info *ui;
    QString twoWayDecrypt(const QString &input);
};

#endif // INFO_H
