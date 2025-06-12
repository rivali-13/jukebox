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


// **************  Picture Music  **********************
    QPixmap pix(":/JukeBox/Icon/cover.png");
    ui->cover->setPixmap(pix);
    ui->cover1->setPixmap(pix);
//********
    LoadMusicFiles();

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


//************ Read music for Folder ( Default = "/music" ) ***********************
void home::LoadMusicFiles(QString Address)
{
    QDir musicDir(QCoreApplication::applicationDirPath() + "/../../.." + Address);

    QStringList nameFilters = {"*.mp3", "*.wav"};
    QFileInfoList files  = musicDir.entryInfoList(nameFilters, QDir::Files);

    for ( const QFileInfo& fileInfo : files)
    {
        qDebug() << fileInfo.fileName();
        QString fileName = fileInfo.completeBaseName();
        QString filePath = fileInfo.absoluteFilePath();
        QString format = fileInfo.suffix().toUpper();
        QString size = QString::number(fileInfo.size() / (1024.0 * 1024.0), 'f', 2) + "MB";


        int currentRow = ui->tableMusic->rowCount();
        ui->tableMusic->insertRow(currentRow);

        ui->tableMusic->setItem(currentRow, 2, new QTableWidgetItem(fileName));
        ui->tableMusic->setItem(currentRow, 4, new QTableWidgetItem(filePath));
        ui->tableMusic->setItem(currentRow, 1, new QTableWidgetItem(format));
        ui->tableMusic->setItem(currentRow, 7, new QTableWidgetItem(size));

    }
    ui->tableMusic->resizeColumnsToContents();
}
// ***


void home::extractMetadata(const QString &filePath, int row)
{
    player->setSource(QUrl::fromLocalFile(filePath));

    // منتظر می‌مانیم تا متادیتا لود شود
    QEventLoop loop;
    connect(player, &QMediaPlayer::metaDataChanged, &loop, &QEventLoop::quit);
    loop.exec();

    QString artist = player->metaData().value(QMediaMetaData::AlbumArtist).toString();
    QString title = player->metaData().value(QMediaMetaData::Title).toString();
    qint64 duration = player->duration(); // میلی‌ثانیه

    if (title.isEmpty()) {
        QFileInfo fileInfo(filePath);
        title = fileInfo.completeBaseName();
    }

    // تبدیل مدت زمان به فرمت mm:ss
    int minutes = duration / 60000;
    int seconds = (duration % 60000) / 1000;
    QString durationStr = QString("%1:%2")
                              .arg(minutes)
                              .arg(seconds, 2, 10, QChar('0'));

    // اضافه کردن به جدول
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
    if (row < 0) return;

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
}
void home::stopMusic()
{
    if (player) player->stop();
}

void home::pauseMusic()
{
    if (player) player->pause();
}
