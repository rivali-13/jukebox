#include "home.h"
#define c_address 4
#define c_size 5
#define c_artist 2
#define c_length 1
#define c_title 0
#define c_format 3

home* home::address = nullptr;
home* home::single()
{
    if(!address){
        address = new home();
    }
    return address;
}

home::home(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::home )
{
    srand (time(0));
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

    connect(player, &QMediaPlayer::playbackStateChanged, this, &home::updatePlayButtonIcon);

    connect(player, &QMediaPlayer::metaDataChanged, this, &home::set_info);





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
    updatePlayButtonIcon(); // Call the helper function
}

void home::on_pushButton_4_clicked()
{
    // --- IMPROVEMENT START ---
    if (ui->tableMusic->rowCount() == 0) {
        player->stop();
        ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
        ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
        // Explicitly clear references to any active playlist/queue
        is_list = false;
        is_queue = false;
        listWidget = nullptr; // Clear the reference
        tabWidget = nullptr;  // Clear the reference
        return; // No music to play
    }
    // --- IMPROVEMENT END ---

    // Ensure state flags and pointers are cleared when playing from tableMusic
    is_list = false;
    is_queue = false;
    listWidget = nullptr; // Explicitly clear the reference
    tabWidget = nullptr;  // Explicitly clear the reference

    int row = ui->tableMusic->currentRow();

    if (row < 0) {
        row = 0;
        ui->tableMusic->selectRow(row);
    }
    // ... rest of the function remains the same
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

    set_info();
}

void home::on_tableMusic_doubleClicked(const QModelIndex &index)
{
    // --- IMPROVEMENT START ---
    if (ui->tableMusic->rowCount() == 0) {
        player->stop();
        ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
        ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
        // Explicitly clear references to any active playlist/queue
        is_list = false;
        is_queue = false;
        listWidget = nullptr; // Clear the reference
        tabWidget = nullptr;  // Clear the reference
        return;
    }
    // --- IMPROVEMENT END ---

    is_list = false;
    is_queue = false;
    listWidget = nullptr; // Explicitly clear the reference
    tabWidget = nullptr;  // Explicitly clear the reference

    int row = index.row();
    if (row < 0) return;

    QString filePath = ui->tableMusic->item(row, c_address)->text();

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
    QMenu* add_to_queue = menu.addMenu("Add to Queue");

    for (int i = 0; i < ui->tab_playlist->count(); ++i) {
        QString tabName = ui->tab_playlist->tabText(i);
        QAction* action = add_to_list->addAction(tabName);
        action->setData(i);
    }

    for (int i = 0; i < ui->tab_queue->count(); ++i) {
        QString tabName = ui->tab_queue->tabText(i);
        QAction* action = add_to_queue->addAction(tabName);
        action->setData(i);
    }

    QPoint globalPos = ui->tableMusic->viewport()->mapToGlobal(pos);
    QAction *selectedAction = menu.exec(globalPos);

    if (selectedAction && selectedAction->parent() == add_to_list) {
        int tabIndex = selectedAction->data().toInt();
        QString itemText = ui->tableMusic->item(index.row(), c_address)->text();
        add_to_playlist(tabIndex, itemText);
    }

    if (selectedAction && selectedAction->parent() == add_to_queue) {
        int tabIndex = selectedAction->data().toInt();
        QString itemText = ui->tableMusic->item(index.row(), c_address)->text();
        home::add_to_queue(tabIndex, itemText);
    }
}

void home::set_info()
{
    QString filePath = player->source().toString(); // The QUrl source of the playing media
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    // --- NEW LOGIC FOR COVER ART ---
    QPixmap pixmapToDisplay;
    QVariant coverArtData = player->metaData().value(QMediaMetaData::CoverArtImage);

    // Check if valid cover art data exists and is not null
    if (coverArtData.isValid() && !coverArtData.isNull()) {
        qDebug() << "in cover if";
        QImage coverImage = coverArtData.value<QImage>(); // QMediaMetaData::CoverArtImage returns a QImage
        if (!coverImage.isNull()) {
            // Scale the image to fit the QLabel's size, maintaining aspect ratio
            // and using smooth transformation for better quality
            pixmapToDisplay = QPixmap::fromImage(coverImage).scaled(ui->cover->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    // If no cover art was found or extracted (pixmapToDisplay is null), set the default cover
    if (pixmapToDisplay.isNull()) {
        pixmapToDisplay = QPixmap(":/JukeBox/Icon/cover.png"); // Path to your default cover image
    }

    // Set the pixmap to your QLabel widgets
    ui->cover->setPixmap(pixmapToDisplay);
    ui->cover1->setPixmap(pixmapToDisplay);
    // --- END NEW LOGIC FOR COVER ART ---


    // The existing logic to find and display text metadata remains the same
    for (int i = 0; i < ui->tableMusic->rowCount(); ++i) {
        // Compare "file:///" prefixed path from player source with stored file path
        if ("file:///" + ui->tableMusic->item(i, c_address)->text() == filePath) {
            qDebug() << "in loop";
            QString artist = ui->tableMusic->item(i, c_artist)->text();
            QString title = ui->tableMusic->item(i, c_title)->text();
            QString duration = ui->tableMusic->item(i, c_length)->text();
            QString format = ui->tableMusic->item(i, c_format)->text();
            QString size = ui->tableMusic->item(i, c_size)->text();

            ui->label_4->setText(artist);
            ui->artist->setText(artist);
            ui->title->setText(title);

            QString info = "<br><table border='0' cellspacing='4' cellpadding='4'>"
                           "<tr><td><b>Format:</b></td><td>" + format + "</td></tr>"
                                      "<tr><td><b>Size:</b></td><td>" + size + "</td></tr>"
                                    "<tr><td><b>Length:</b></td><td>" + duration + "</td></tr>"
                                        "</table>";
            ui->infoMusic->setText(info);
            return; // Found the song, so exit the loop
        }
    }
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
    tabWidget = ui->tab_playlist->widget(tabIndex);
    if (!tabWidget) {
        return;
    }

    listWidget = tabWidget->findChild<QListWidget*>();
    if (!listWidget) {
        return;
    }

    disconnect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_list_play);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_list_play);
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

void home::on_pushButton_8_clicked()
{
    player->stop();
    updatePlayButtonIcon(); // Call the helper function
    ui->label_4->setText("");
    ui->artist->setText("");
    ui->title->setText("");
    ui->infoMusic->setText("");
    ui->progressBar->setValue(0);
}

int random(int count){
    int temp = rand();
    return temp % count;
}

void home::on_pushButton_5_clicked()
{
    if (!is_list && !is_queue){ // Currently playing from tableMusic
        // No change needed for this block regarding tab management for tableMusic
        // as it's not in a tab.
        // Ensure the main table has rows before proceeding
        if (ui->tableMusic->rowCount() == 0) {
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);
            return;
        }

        if (!is_shuffle || (is_shuffle && ui->tableMusic->rowCount() < 2)){ // Changed listWidget->count() to ui->tableMusic->rowCount() and < 3 to < 2 for meaningful shuffle
            int cur_row = ui->tableMusic->currentRow();
            int next_row = (cur_row <= 0) ? ui->tableMusic->rowCount() - 1 : cur_row - 1; // Loop to end if at start
            ui->tableMusic->setCurrentCell(next_row, ui->tableMusic->currentColumn());
        }
        else {
            int cur_row = ui->tableMusic->currentRow();
            int r;
            // Ensure we pick a different row if there's more than one
            while (ui->tableMusic->rowCount() > 1 && (r = random(ui->tableMusic->rowCount())) == cur_row){
                // Loop until a different random row is picked
            }
            ui->tableMusic->setCurrentCell(r, ui->tableMusic->currentColumn());
        }
        player->stop();
        on_pushButton_4_clicked(); // This will play the newly selected song and set info
    }
    else if(is_list && !is_queue) { //playlist
        if (!listWidget || listWidget->count() == 0) return; // Safety check

        if (!is_shuffle || (is_shuffle && listWidget->count() < 2)){ // < 2 for meaningful shuffle
            int cur_row = listWidget->currentRow();
            int next_row = (cur_row <= 0) ? listWidget->count() - 1 : cur_row - 1; // Loop to end if at start
            listWidget->setCurrentRow(next_row);
        }
        else {//shuffle
            int cur_row = listWidget->currentRow();
            int r;
            while (listWidget->count() > 1 && (r = random(listWidget->count())) == cur_row){
                // Loop until a different random row is picked
            }
            listWidget->setCurrentRow(r);
        }
        player->stop();
        play_list_play(listWidget->currentItem());
    }
    else if(!is_list && is_queue){//queue

        int cur_row = listWidget->currentRow();
        if (cur_row < 0) { // If no item is current (e.g., first play of queue)
            if (listWidget->count() > 0) {
                listWidget->setCurrentRow(0); // Select first item
                cur_row = 0;
            } else {
                return; // Should be caught by the above empty check, but safety
            }
        }

        // QListWidgetItem* item_to_remove = listWidget->currentItem(); // Not needed for "previous"

        if (!is_shuffle || (is_shuffle && listWidget->count() < 2)){ // < 2 for meaningful shuffle
            if (cur_row == 0) {
                listWidget->setCurrentRow(listWidget->count() - 1); // Loop to last
            } else {
                listWidget->setCurrentRow(cur_row - 1);
            }
        }
        else {
            int r;
            while (listWidget->count() > 1 && (r = random(listWidget->count())) == cur_row){
                // Loop until a different random row is picked
            }
            listWidget->setCurrentRow(r);
        }
        player->stop();
        QListWidgetItem* next_item = listWidget->currentItem();
        if (next_item) {
            play_queue(next_item);
        } else {
            // Fallback: This should ideally not be reached if listWidget->count() > 0
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        }

        if (!listWidget || listWidget->count() == 0) {
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);

            int currentQueueTabIndex = ui->tab_queue->indexOf(tabWidget);
            if (currentQueueTabIndex != -1) {
                QWidget* widgetToDelete = ui->tab_queue->widget(currentQueueTabIndex);
                ui->tab_queue->removeTab(currentQueueTabIndex);
                if (widgetToDelete) {
                    delete widgetToDelete;
                }
            }
            // Transition to tableMusic
            is_queue = false;
            is_list = false;
            listWidget = nullptr; // Clear references
            tabWidget = nullptr;  // Clear references
            return;
        }
    }
}

void home::on_pushButton_9_clicked()
{
    if (!is_list && !is_queue){ // Currently playing from tableMusic
        // No change needed for this block regarding tab management for tableMusic
        // Ensure the main table has rows before proceeding
        if (ui->tableMusic->rowCount() == 0) {
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);
            return;
        }

        if (!is_shuffle || (is_shuffle && ui->tableMusic->rowCount() < 2)){ // Changed listWidget->count() to ui->tableMusic->rowCount() and < 3 to < 2 for meaningful shuffle
            int cur_row = ui->tableMusic->currentRow();
            int next_row = (cur_row >= ui->tableMusic->rowCount() - 1) ? 0 : cur_row + 1; // Loop to start if at end
            ui->tableMusic->setCurrentCell(next_row, ui->tableMusic->currentColumn());
        }
        else{
            int cur_row = ui->tableMusic->currentRow();
            int r;
            while (ui->tableMusic->rowCount() > 1 && (r = random(ui->tableMusic->rowCount())) == cur_row){
                // Loop until a different random row is picked
            }
            ui->tableMusic->setCurrentCell(r, ui->tableMusic->currentColumn());
        }
        player->stop();
        on_pushButton_4_clicked(); // This will play the newly selected song and set info
    }
    else if (is_list && !is_queue){//playlist
        if (!listWidget || listWidget->count() == 0) return; // Safety check

        int cur_row = listWidget->currentRow();
        if (!is_shuffle || (is_shuffle && listWidget->count() < 2)){ // < 2 for meaningful shuffle
            int next_row = (cur_row >= listWidget->count() - 1) ? 0 : cur_row + 1; // Loop to start if at end
            listWidget->setCurrentRow(next_row);
        }
        else {
            int r;
            while (listWidget->count() > 1 && (r = random(listWidget->count())) == cur_row){
                // Loop until a different random row is picked
            }
            listWidget->setCurrentRow(r);
        }
        player->stop();
        play_list_play(listWidget->currentItem());
    }
    else if (is_queue && !is_list){//queue

        QListWidgetItem* item_to_remove = listWidget->currentItem();
        int cur_row = listWidget->currentRow();

        if (cur_row < 0) { // If no item is current (e.g., first play of queue)
            if (listWidget->count() > 0) {
                listWidget->setCurrentRow(0); // Select first item
                cur_row = 0;
            } else {
                return; // Should be caught by the above empty check, but safety
            }
        }

        if (!is_shuffle || (is_shuffle && listWidget->count() < 2)){ // < 2 for meaningful shuffle
            if (cur_row == listWidget->count() - 1) {
                listWidget->setCurrentRow(0);
            } else {
                listWidget->setCurrentRow(cur_row + 1);
            }
        }
        else {
            int r;
            while (listWidget->count() > 1 && (r = random(listWidget->count())) == cur_row){
                r = random(listWidget->count()); // Regenerate random number
            }
            listWidget->setCurrentRow(r);
        }

        player->stop();
        QListWidgetItem* next_item = listWidget->currentItem();
        if (next_item) {
            play_queue(next_item);
        } else {
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        }

        // Now remove the *old* item (item_to_remove) after the new one starts playing
        if (item_to_remove) {
            int old_row_to_remove = -1;
            for (int i = 0; i < listWidget->count(); ++i) {
                if (listWidget->item(i) == item_to_remove) {
                    old_row_to_remove = i;
                    break;
                }
            }
            if (old_row_to_remove != -1) {
                QListWidgetItem* takenItem = listWidget->takeItem(old_row_to_remove);
                if (takenItem) {
                    delete takenItem;
                }
            }
        }

        // --- IMPROVEMENT START ---
        if (!listWidget || listWidget->count() == 0) {
            player->stop();
            ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
            ui->infoMusic->setText(""); ui->progressBar->setValue(0);

            int currentQueueTabIndex = ui->tab_queue->indexOf(tabWidget);
            if (currentQueueTabIndex != -1) {
                QWidget* widgetToDelete = ui->tab_queue->widget(currentQueueTabIndex);
                ui->tab_queue->removeTab(currentQueueTabIndex);
                if (widgetToDelete) {
                    delete widgetToDelete;
                }
            }
            // Transition to tableMusic
            is_queue = false;
            is_list = false;
            listWidget = nullptr; // Clear references
            tabWidget = nullptr;  // Clear references
            return;
        }
        // --- IMPROVEMENT END ---
    }
}

void home::play_list_play(QListWidgetItem* item){
    if (!item) {
        player->stop();
        updatePlayButtonIcon();
        ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
        ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        return;
    }

    QString filePath = item->data(Qt::UserRole + 1).toString();

    playMusic(filePath); // Always play the music associated with this item

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/3.png")); // Pause icon
    } else {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
    }
    is_queue = false;
    is_list = true;
    set_info();
    qDebug() << "in list";

    // --- NEW LOGIC FOR CHANGING TAB ---
    // Get the QListWidget that the item belongs to
    listWidget = item->listWidget();
    if (listWidget) {
        // Get the parent QWidget of the QListWidget (which is the tab page itself)
        tabWidget = listWidget->parentWidget();
        if (tabWidget) {
            // Find the index of this parentTabWidget within ui->tab_playlist
            int tabIndex = ui->tab_playlist->indexOf(tabWidget);
            if (tabIndex != -1) {
                // Set the current index of the QTabWidget to bring the tab to front
                ui->tab_playlist->setCurrentIndex(tabIndex);
            }
        }
    }
    // --- END NEW LOGIC ---
}


void home::on_pushButton_7_clicked()
{
    if (is_shuffle) {
        is_shuffle = false;
        ui->pushButton_7->setIcon(QIcon(":/JukeBox/Icon/30.png"));
    }
    else {
        is_shuffle = true;
        ui->pushButton_7->setIcon(QIcon(":/JukeBox/Icon/16.png"));
    }
}

void home::on_pushButton_clicked()
{
    New_queue* queue = new New_queue(this);
    connect(queue, &New_queue::new_name, this, &home::creat_queue);
    queue->show();
}

void home::creat_queue(const QString &name)
{
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
    ui->tab_queue->addTab(newtab, name);
}

void home::add_to_queue(int tabIndex, const QString& itemText){

    tabWidget = ui->tab_queue->widget(tabIndex);
    if (!tabWidget) {
        return;
    }

    listWidget = tabWidget->findChild<QListWidget*>();
    if (!listWidget) {
        return;
    }

    disconnect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_queue);
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_queue);
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

void home::play_queue(QListWidgetItem* item){
    if (!item) {
        player->stop();
        updatePlayButtonIcon();
        ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
        ui->infoMusic->setText(""); ui->progressBar->setValue(0);
        return;
    }

    QString filePath = item->data(Qt::UserRole + 1).toString();

    playMusic(filePath); // Always play the music associated with this item

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/3.png")); // Pause icon
    } else {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
    }
    is_queue = true;
    is_list = false;
    set_info();
    qDebug() << "in queue";

    // --- NEW LOGIC FOR CHANGING TAB ---
    // Get the QListWidget that the item belongs to
    listWidget = item->listWidget();
    if (listWidget) {
        // Get the parent QWidget of the QListWidget (which is the tab page itself)
        tabWidget = listWidget->parentWidget();
        if (tabWidget) {
            // Find the index of this parentTabWidget within ui->tab_queue
            int tabIndex = ui->tab_queue->indexOf(tabWidget);
            if (tabIndex != -1) {
                // Set the current index of the QTabWidget to bring the tab to front
                ui->tab_queue->setCurrentIndex(tabIndex);
            }
        }
    }
    // --- END NEW LOGIC ---
}

void home::updatePlayButtonIcon()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/3.png")); // Pause icon
    } else {
        ui->pushButton_4->setIcon(QIcon(":/JukeBox/Icon/1.png")); // Play icon
    }
}
