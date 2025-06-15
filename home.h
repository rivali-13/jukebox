#ifndef HOME_H
#define HOME_H

#include "ui_home.h"
#include "style_playlistitem.h"
#include <QMainWindow>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QListWidget>
#include <QFileDialog>
#include <QAudioOutput>
#include <QMediaPlayer>
#include <QScreen>


QT_BEGIN_NAMESPACE
namespace Ui {
class home;
}
QT_END_NAMESPACE

class home : public QMainWindow
{
    Q_OBJECT

public:
    home(QWidget *parent = nullptr);
    ~home();
    void playMusic(const QString &filePath);


private slots:
    void onPositionChanged(qint64 position);

    void onDurationChanged(qint64 duration);

    void on_addMusic_clicked();

    void on_exit_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_tableMusic_doubleClicked(const QModelIndex &index);

    void stopMusic();

    void pauseMusic();

    void on_valume_clicked();

private:
    Ui::home *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    void extractMetadata(const QString &filePath, int row);
    void set_progressbar(int parts);

};
#endif // home_H
