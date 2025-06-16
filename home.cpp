#include "home.h"
#define c_address 4
#define c_size 5
#define c_artist 2
#define c_length 1
#define c_title 0
#define c_format 3

home::home(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::home )
{
    ui->setupUi(this);
    ui->label_4->setText("");
    ui->artist->setText("");
    ui->title->setText("");
    ui->infoMusic->setText("");
    ui->progressBar->setValue(0);

    ui->tableMusic->setEditTriggers(QAbstractItemView::NoEditTriggers);
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->showFullScreen();
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
//*****



    ui->tableMusic->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableMusic, &QTableWidget::customContextMenuRequested,
            this, &home::showContextMenu);
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
                              .arg(minutes, 2, 10, QChar('0'))
                              .arg(seconds, 2, 10, QChar('0'));


    ui->tableMusic->setItem(row, c_artist, new QTableWidgetItem(artist));
    ui->tableMusic->setItem(row, c_title, new QTableWidgetItem(title));
    ui->tableMusic->setItem(row, c_length, new QTableWidgetItem(durationStr));

}

void home::tab_text(QString name)
{
    ui->tab_playlist->setTabText(0,name);
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
            ui->tableMusic->setItem(row, c_address, new QTableWidgetItem(filePath));
            ui->tableMusic->setItem(row, c_format, new QTableWidgetItem(format));
            ui->tableMusic->setItem(row, c_size, new QTableWidgetItem(size));
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
    {
        ui->centralwidget->setStyleSheet("#centralwidget { background-color: #201f1d;	border-radius: 0px;    border: none }");
        ui->LeftPageMusic->setStyleSheet("#LeftPageMusic { background-color: #232220;   border-top-left-radius: 0px }");
        this->showFullScreen();
    }
    else
    {
        this->showNormal();
        ui->centralwidget->setStyleSheet("#centralwidget {  background-color: #201f1d;	border-radius: 20px;    border: 1px solid #0632a2; }");
        ui->LeftPageMusic->setStyleSheet("#LeftPageMusic { background-color: #232220;   border-top-left-radius: 20px }");
    }
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

    QString filePath = ui->tableMusic->item(row, c_address)->text();
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
    set_info();
}

void home::on_tableMusic_doubleClicked(const QModelIndex &index)
{
    int row = index.row();
    if (row < 0) return;

    QString filePath = ui->tableMusic->item(row, c_address)->text();

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
    set_info();
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

void home::on_valume_clicked(){
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

void home::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->tableMusic->indexAt(pos);
    if (!index.isValid() || ui->tableMusic->item(index.row(), index.column()) == nullptr)
        return;

    QMenu menu(this);
    QMenu* add_to_list = menu.addMenu("Add to Playlist");

    for (int i = 0; i < ui->tab_playlist->count(); ++i) {
        QString tabName = ui->tab_playlist->tabText(i);
        QAction* action = add_to_list->addAction(tabName);
        action->setData(i);
    }

    QPoint globalPos = ui->tableMusic->viewport()->mapToGlobal(pos);
    QAction *selectedAction = menu.exec(globalPos);

    if (selectedAction && selectedAction->parent() == add_to_list) {
        int tabIndex = selectedAction->data().toInt();
        QString itemText = ui->tableMusic->item(index.row(), c_address)->text();
        add_to_playlist(tabIndex, itemText);
    }
}

void home::set_info(){
    int cur_row = ui->tableMusic->currentRow();
    if (cur_row < 0) return;
    QString artist = ui->tableMusic->item(cur_row, c_artist)->text();
    QString title = ui->tableMusic->item(cur_row, c_title)->text();
    QString duration = ui->tableMusic->item(cur_row, c_length)->text();
    ui->label_4->setText(artist);
    ui->artist->setText(artist);
    ui->title->setText(title);
    ui->title->autoFillBackground();
    QString format = ui->tableMusic->item(cur_row, c_format)->text();
    QString size = ui->tableMusic->item(cur_row, c_size)->text();
    QString info = "<br><table border='0' cellspacing='4' cellpadding='4'>"
                   "<tr><td><b>Format:</b></td><td>" + format + "</td></tr>"
                              "<tr><td><b>Size:</b></td><td>" + size + "</td></tr>"
                            "<tr><td><b>Length:</b></td><td>" + duration + "</td></tr>"
                                "</table>";

    ui->infoMusic->setText(info);
}

void home::on_new_playlist_clicked()
{
    New_playlist *new_list = new New_playlist(this);
    connect(new_list, &New_playlist::new_name, this, &home::creat_list);
    new_list->show();
}

void home::creat_list(const QString &name)
{
    // ساختن ویجت مادر (صفحه تب)
    QWidget* newtab = new QWidget();

    // می‌تونیم یک layout برای صفحه تب تنظیم کنیم تا widget ها داخلش جا بگیرند
    QVBoxLayout* layout = new QVBoxLayout(newtab);

    // ساختن QListWidget
    QListWidget* new_list = new QListWidget();

    // (اختیاری ولی بسیار مفید) ست کردن objectName برای پیدا کردن راحت‌تر
    new_list->setObjectName("playlistListWidget");

    // اضافه کردن لیست به layout
    layout->addWidget(new_list);

    // اضافه کردن تب به QTabWidget
    ui->tab_playlist->addTab(newtab, name);
}

void home::add_to_playlist(int tabIndex, const QString& itemText)
{
    QWidget* tabWidget = ui->tab_playlist->widget(tabIndex);
    if (!tabWidget) {
        return;
    }

    QListWidget* listWidget = tabWidget->findChild<QListWidget*>();
    if (!listWidget) {
        return;
    }

    // بررسی آهنگ تکراری بر اساس آدرس (itemAddress)
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem* existingItem = listWidget->item(i);
        QString existingAddress = existingItem->data(Qt::UserRole + 1).toString();
        if (itemText == existingAddress) {
            return; // آهنگ تکراری است، اضافه نکن
        }
    }

    listWidget->setItemDelegate(new style_playlistitem(listWidget));

    QListWidgetItem* item = new QListWidgetItem();

    // جستجوی اطلاعات آهنگ در جدول اصلی
    for (int i = 0; i < ui->tableMusic->rowCount(); ++i) {
        if (itemText == ui->tableMusic->item(i, c_address)->text()) {
            QString title = ui->tableMusic->item(i, c_title)->text();
            QString format = ui->tableMusic->item(i, c_format)->text();
            QString size = ui->tableMusic->item(i, c_size)->text();
            QString length = ui->tableMusic->item(i, c_length)->text();

            item->setText(title);
            item->setData(Qt::UserRole, format + " :: " + size + ", " + length);
            item->setData(Qt::UserRole + 1, itemText); // ذخیره آدرس در data جهت بررسی تکراری بودن

            break;
        }
    }

    listWidget->addItem(item);
}

