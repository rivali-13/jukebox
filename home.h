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
#include <QPixmap>
#include <QImage>
#include <QScreen>
#include <QMenu>
#include <QTimer> // Add this include
#include <QPen>   // Add this include
#include <QColor>// Add this include
#include <QMouseEvent>
#include "new_playlist.h"
#include "new_queue.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QStandardPaths>

enum state{
    disable = 1,
    once,
    list
};

QT_BEGIN_NAMESPACE
namespace Ui {
class home;
}
QT_END_NAMESPACE

class home : public QMainWindow
{
    Q_OBJECT

public:
    ~home();
    void playMusic(const QString &filePath);
    static home* single();

    void add_to_playlist(int tabIndex, const QString& itemText);
    void add_to_queue(int tabIndex, const QString &itemText);
    void set_username(QString un);
    void saveUserData(const QString& username);
    void loadUserData(const QString& username);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override; // Override eventFilter


private slots:

    void on_moje_clicked(); // New slot for handling clicks on moje label

    void onPositionChanged(qint64 position);

    void onDurationChanged(qint64 duration);

    void on_addMusic_clicked();

    void on_pushButton_4_clicked();

    void on_tableMusic_doubleClicked(const QModelIndex &index);

    void stopMusic();

    void pauseMusic();

    void on_valume_clicked();

    void showContextMenu(const QPoint &pos);
    void on_new_playlist_clicked();
    void creat_list(const QString &name);
    void on_pushButton_8_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_9_clicked();

    void play_list_play(QListWidgetItem* item);
    void on_pushButton_7_clicked();

    void on_pushButton_clicked();

    void creat_queue(const QString &name);
    void play_queue(QListWidgetItem *item);

    void onMediaStatusChanged(QMediaPlayer::MediaStatus status); // Add this line

    void on_pushButton_6_clicked();


private:
    home(QWidget *parent = nullptr);
    static home* address;
    QListWidget* listWidget = nullptr;
    int tabindex = -1;
    QWidget* tabWidget = nullptr;
    Ui::home *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    int repeat = disable;
    bool is_list = false;
    bool is_shuffle = false;
    bool is_queue = false;
    void extractMetadata(const QString &filePath, int row);
    void set_progressbar(int parts);
    void tab_text(QString name);
    void set_cover();
    void set_info();
    void updatePlayButtonIcon();


    QString username = "";
    QTimer *visualizerTimer = nullptr;
    void updateVisualizer(); // New slot for drawing
    // You might also want to store some visualizer parameters
    int visualizerBarCount = 80;
    int visualizerMaxHeight = 185;
    QColor currentVisualizerColor; // New member to store the current color
    bool waveModel = false; // false for bars, true for sine wave (or another model)



};
#endif // home_H
