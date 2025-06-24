#ifndef VIDEO_H
#define VIDEO_H

#include <QWidget>

namespace Ui {
class Video;
}

class Video : public QWidget
{
    Q_OBJECT

public:
    explicit Video(QWidget *parent = nullptr);
    ~Video();

private slots:
    void on_exit_clicked();

    void on_full_clicked();

private:
    Ui::Video *ui;
};

#endif // VIDEO_H
