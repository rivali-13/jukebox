#ifndef NEW_QUEUE_H
#define NEW_QUEUE_H

#include <QDialog>

namespace Ui {
class New_queue;
}

class New_queue : public QDialog
{
    Q_OBJECT

public:
    explicit New_queue(QWidget *parent = nullptr);
    ~New_queue();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::New_queue *ui;
signals:
    void new_name(const QString &name);
};

#endif // NEW_QUEUE_H
