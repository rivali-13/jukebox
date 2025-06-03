#include "home.h"

home::home(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::home )
{
    ui->setupUi(this);
    ui->centralwidget->setStyleSheet("#centralwidget { background-color: #201f1d; border-radius: 20px; border: 1px solid #0632a2 } ");
    ui->LeftPageMusic->setStyleSheet("#LeftPageMusic {background-color: #232220; border-top-left-radius: 20px }");

// **************  Picture Music  **********************
    QPixmap pix(":/JukeBox/Icon/cover.png");
    ui->cover->setPixmap(pix);
    ui->cover1->setPixmap(pix);
    ui->cover->setStyleSheet("#cover { border:1px solid #c9cfdd} " );
//********
    LoadMusicFiles();

    QPixmap sound(":JukeBox/Icon/audio.png");
    ui->sound->setPixmap(sound);

    QIcon addMusic(":JukeBox/Icon/add.png");
    ui->addMusic->setIcon(addMusic);

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


Ui::home* home::GetUi()
{
    return ui;
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
