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

    void LoadMusicFiles( QString Address = "/music");
    Ui::home* GetUi();


private slots:
    void on_addMusic_clicked();

private:
    Ui::home *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    void extractMetadata(const QString &filePath, int row);
};
#endif // home_H
