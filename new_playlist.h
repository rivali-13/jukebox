#ifndef NEW_PLAYLIST_H
#define NEW_PLAYLIST_H

#include <QDialog>

namespace Ui {
class New_playlist;
}

class New_playlist : public QDialog
{
    Q_OBJECT

public:
    explicit New_playlist(QWidget *parent = nullptr);
    ~New_playlist();
    QString get_name();

private slots:
    void on_buttonBox_accepted();

private:
    Ui::New_playlist *ui;
    QString name;
signals:
    void new_name(const QString &name);
};

#endif // NEW_PLAYLIST_H
