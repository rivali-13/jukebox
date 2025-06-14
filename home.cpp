#include "home.h"
#define address 3

home::home(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::home )
{
    ui->setupUi(this);
    this->showFullScreen();
    ui->tableMusic->setEditTriggers(QAbstractItemView::NoEditTriggers);
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    connect(player, &QMediaPlayer::positionChanged, this, &home::onPositionChanged);
    connect(player, &QMediaPlayer::durationChanged, this, &home::onDurationChanged);
    connect(ui->progressBar, &QSlider::sliderMoved, this, [=](int value){
        player->setPosition(value * 1000); // value is in seconds, position is in ms
    });
    connect(ui->volumeSlider, &QSlider::valueChanged, this, [=](int value){
        if (audioOutput) {
            audioOutput->setVolume(value / 100.0); // QAudioOutput expects 0.0 - 1.0
        }
    });




// **************  Picture Music  **********************
    QPixmap pix(":/JukeBox/Icon/cover.png");
    ui->cover->setPixmap(pix);
    ui->cover1->setPixmap(pix);
//********

//*****************************     music PlayList For Test **************************
    ui->playlist->setItemDelegate(new style_playlistitem(ui->playlist));
    QListWidgetItem* item1 = new QListWidgetItem();
    item1->setText("JukeBox");
    item1->setData(Qt::UserRole, "MP3 :: 44 kHz, 320 kbps, 7.4 MB, 3:13");
    ui->playlist->addItem(item1);

    QListWidgetItem* item2 = new QListWidgetItem();
    item2->setText("JukeBox");
    item2->setData(Qt::UserRole, "MP3 :: 44 kHz, 320 kbps, 7.4 MB, 3:13");
    ui->playlist->addItem(item2);
//***
}


home::~home()
{
    delete ui;
}


void home::extractMetadata(const QString &filePath, int row)
{
    QMediaPlayer tempPlayer;
    tempPlayer.setSource(QUrl::fromLocalFile(filePath));

    QEventLoop loop;
    QObject::connect(&tempPlayer, &QMediaPlayer::metaDataChanged, &loop, &QEventLoop::quit);
    loop.exec();

    QString artist = tempPlayer.metaData().value(QMediaMetaData::AlbumArtist).toString();
    QString title = tempPlayer.metaData().value(QMediaMetaData::Title).toString();
    qint64 duration = tempPlayer.duration();

    if (title.isEmpty()) {
        QFileInfo fileInfo(filePath);
        title = fileInfo.completeBaseName();
    }

    int minutes = duration / 60000;
    int seconds = (duration % 60000) / 1000;
    QString durationStr = QString("%1:%2")
                              .arg(minutes)
                              .arg(seconds, 2, 10, QChar('0'));

    ui->tableMusic->setItem(row, 4, new QTableWidgetItem(artist));
    ui->tableMusic->setItem(row, 1, new QTableWidgetItem(title));
    ui->tableMusic->setItem(row, 2, new QTableWidgetItem(durationStr));
}

void home::on_addMusic_clicked()
{
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        tr("Select Music Files"),
        QDir::homePath(),
        tr("Audio Files (*.mp3 *.wav)")
        );

    if (!files.isEmpty()) {
        foreach (const QString &filePath, files) {
            QFileInfo fileInfo(filePath);

            QString fileName = fileInfo.completeBaseName();
            QString format = fileInfo.suffix().toUpper();
            QString size = QString::number(fileInfo.size() / (1024.0 * 1024.0), 'f', 2) + "MB";

            int row = ui->tableMusic->rowCount();
            ui->tableMusic->insertRow(row);

            // همان ترتیبی که در LoadMusicFiles استفاده کردید
            //ui->tableMusic->setItem(row, 2, new QTableWidgetItem(fileName));
            ui->tableMusic->setItem(row, 3, new QTableWidgetItem(filePath));
            ui->tableMusic->setItem(row, 0, new QTableWidgetItem(format));
            ui->tableMusic->setItem(row, 5, new QTableWidgetItem(size));
            extractMetadata(filePath, row);
        }
        ui->tableMusic->resizeColumnsToContents();
    }
}

void home::on_exit_clicked()
{
    this->close();
}


void home::on_pushButton_2_clicked()
{
    if (!this->isFullScreen())
    this->showFullScreen();
    else this->showNormal();
}


void home::on_pushButton_3_clicked()
{
    this->showMinimized();
}

void home::playMusic(const QString &filePath)
{
    if (!audioOutput) {
        audioOutput = new QAudioOutput(this);
        player->setAudioOutput(audioOutput);
    }
    player->setSource(QUrl::fromLocalFile(filePath));
    player->play();
}

void home::on_pushButton_4_clicked()
{
    int row = ui->tableMusic->currentRow();

    // If no row is selected but table is not empty, play the first row
    if (row < 0 && ui->tableMusic->rowCount() > 0) {
        row = 0;
        ui->tableMusic->selectRow(row); // Optional: visually select the row
    }
    if (row < 0) return; // Still no valid row

    QString filePath = ui->tableMusic->item(row, address)->text();
    switch (player->playbackState()) {
    case QMediaPlayer::PlayingState:
        pauseMusic();
        break;
    case QMediaPlayer::PausedState:
        player->play(); // Continue from pause
        break;
    case QMediaPlayer::StoppedState:
    default:
        playMusic(filePath); // Start new song
        break;
    }

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/3.png")); // Pause icon
    } else {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
    }
}
void home::on_tableMusic_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row < 0) return;

    QString filePath = ui->tableMusic->item(row, address)->text();

    // If the same song is already loaded, toggle pause/play
    if (player->source().toLocalFile() == filePath) {
        switch (player->playbackState()) {
        case QMediaPlayer::PlayingState:
            pauseMusic();
            break;
        case QMediaPlayer::PausedState:
            player->play();
            break;
        case QMediaPlayer::StoppedState:
        default:
            playMusic(filePath);
            break;
        }
    } else {
        playMusic(filePath); // Play new song
    }

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/3.png")); // Pause icon
    } else {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
    }

}
void home::stopMusic()
{
    if (player) player->stop();
}

void home::pauseMusic()
{
    if (player) player->pause();
}

void home::onPositionChanged(qint64 position)
{
    // Only update if the user is not dragging
    if (!ui->progressBar->isSliderDown()){
        int minutes = position / 60000;
        int seconds = (position % 60000) / 1000;
        QString durationStr = QString("%1:%2")
                                  .arg(minutes, 2, 10, QChar('0'))
                                  .arg(seconds, 2, 10, QChar('0'));
        ui->label_2->setText(durationStr);
        ui->progressBar->setValue(static_cast<int>(position / 1000));
    }
}

void home::onDurationChanged(qint64 duration)
{
    ui->progressBar->setMaximum(static_cast<int>(duration / 1000));
}

void home::on_valume_clicked()
{
    if (!audioOutput) return;

    // Toggle mute
    bool isMuted = audioOutput->isMuted();
    audioOutput->setMuted(!isMuted);

    // Optionally, update the button icon to reflect mute state
    if (isMuted) {
        ui->valume->setIcon(QIcon(":/JukeBox/Icon/11.png")); // Unmuted icon
    } else {
        ui->valume->setIcon(QIcon(":/JukeBox/Icon/12.png"));     // Muted icon (add this to your resources)
    }
}

