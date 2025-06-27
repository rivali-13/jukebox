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
    srand (time(nullptr));
    ui->setupUi(this);
    ui->label_4->setText("");
    ui->artist->setText("");
    ui->title->setText("");
    ui->infoMusic->setText("");
    ui->progressBar->setValue(0);


    ui->moje->setText(""); // Clear any default text

    visualizerTimer = new QTimer(this);
    connect(visualizerTimer, &QTimer::timeout, this, &home::updateVisualizer);
    currentVisualizerColor = QColor::fromRgb(rand() % 236 + 30, rand() % 216 + 40, rand() % 200 + 56);
    ui->moje->installEventFilter(this); // Install event filter on moje label
    waveModel = false;

    // Start the timer when music plays and stop when it stops
    // For now, let's start it always, you can tie it to player->playbackState()


    ui->tableMusic->setEditTriggers(QAbstractItemView::NoEditTriggers);
    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->showMaximized();
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

    connect(player, &QMediaPlayer::mediaStatusChanged, this, &home::onMediaStatusChanged);


    connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState state){
        if (state == QMediaPlayer::PlayingState) {
            visualizerTimer->start(100);
        } else {
            visualizerTimer->stop();
            updateVisualizer(); // Update one last time to show the fixed pattern
        }
    });

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit,
        this, [this]() {
        player->stop();
        saveUserData(username);
    });

    connect(player, &QMediaPlayer::playbackStateChanged, this, [this](QMediaPlayer::PlaybackState){
        sendNetworkPlaybackState();
    });

    connect(ui->progressBar, &QSlider::sliderReleased, this, [this](){
        if (!isRemoteChange && musicNetwork) {
            QJsonObject msg;
            msg["action"] = "seek";
            msg["song_title"] = ui->title->text();
            msg["position"] = player->position();
            if (musicNetwork->isServer())
                musicNetwork->broadcastToClients(msg);
            else
                musicNetwork->sendToServer(msg);
        }
    });

    connect(musicNetwork, &MusicNetwork::clientConnected,
            this, [this](const QString& addr) {
                updateNetworkStatus("Client connected: " + addr);
            });

    connect(musicNetwork, &MusicNetwork::clientDisconnected,
            this, [this](const QString& addr) {
                updateNetworkStatus("Client disconnected: " + addr);
            });

    // **************  Picture Music  **********************
    QPixmap pix(":/JukeBox/Icon/cover.png");
    ui->moje->setPixmap(pix);
    //*****



    ui->tableMusic->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableMusic, &QTableWidget::customContextMenuRequested, this, &home::showContextMenu);
}


home::~home()
{
    delete ui;
    delete player;
    delete audioOutput;
    delete visualizerTimer;
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
    QWidget* newTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(newTab);
    QListWidget* listWidget = new QListWidget(newTab);
    listWidget->setObjectName("playlistListWidget"); // این خط را اضافه یا اصلاح کنید
    layout->addWidget(listWidget);
    newTab->setLayout(layout);
    ui->tab_playlist->addTab(newTab, name);
    ui->tab_playlist->setCurrentWidget(newTab);

    // اتصال سیگنال double-click
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_list_play);

    // اعمال delegate
    listWidget->setItemDelegate(new style_playlistitem(listWidget));
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
    QWidget* newTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(newTab);
    QListWidget* listWidget = new QListWidget(newTab);
    listWidget->setObjectName("queueListWidget"); // این خط را اضافه یا اصلاح کنید (یا همان "playlistListWidget" اگر می‌خواهید یکسان باشند)
    layout->addWidget(listWidget);
    newTab->setLayout(layout);
    ui->tab_queue->addTab(newTab, name);
    ui->tab_queue->setCurrentWidget(newTab);

    // اتصال سیگنال double-click
    connect(listWidget, &QListWidget::itemDoubleClicked, this, &home::play_queue);

    // اعمال delegate
    listWidget->setItemDelegate(new style_playlistitem(listWidget));
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

void home::set_username(QString un)
{
    username = un;
}

// In home.cpp
void home::onMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        qDebug() << "End of Media detected. Repeat mode: " << repeat;

        switch (repeat) {
        case disable: // Disable (default): Stop playback
            player->stop();
            // Clear info and reset UI
            ui->label_4->setText("");
            ui->artist->setText("");
            ui->title->setText("");
            ui->infoMusic->setText("");
            ui->progressBar->setValue(0);
            updatePlayButtonIcon(); // Ensure play button shows 'play'
            break;
        case once: // Once: Replay the current song
            player->play(); // Will restart from the beginning
            break;
        case list: // List: Play the next song in the current list/queue
            // This is where you call your 'next song' logic
            if (is_list) { // Currently playing from a playlist
                if (listWidget && listWidget->count() > 0) {
                    int currentRow = listWidget->currentRow();
                    int nextRow = (currentRow >= listWidget->count() - 1) ? 0 : currentRow + 1;
                    listWidget->setCurrentRow(nextRow);
                    play_list_play(listWidget->currentItem());
                } else {
                    // No more items in playlist, or list is empty, stop
                    player->stop();
                    ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
                    ui->infoMusic->setText(""); ui->progressBar->setValue(0);
                    updatePlayButtonIcon();
                }
            } else if (is_queue) { // Currently playing from a queue
                if (listWidget && listWidget->count() > 0) {
                    QListWidgetItem* item_to_remove = listWidget->currentItem();
                    int currentRow = listWidget->currentRow();

                    // Remove the finished song from the queue (as it's played)
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

                    // After removing, get the next item (which will be at the current row if previous was deleted)
                    if (listWidget->count() > 0) {
                        // If the list is not empty, ensure a valid row is selected for next play
                        // If the last item was removed and no loop-back, set current row to 0
                        if (currentRow >= listWidget->count()) { // If the last item was played and removed
                            listWidget->setCurrentRow(0);
                        } else { // Otherwise, the next item is now at the current index
                            listWidget->setCurrentRow(currentRow);
                        }
                        play_queue(listWidget->currentItem());
                    } else {
                        // Queue is now empty, clean up and stop
                        player->stop();
                        ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
                        ui->infoMusic->setText(""); ui->progressBar->setValue(0);
                        updatePlayButtonIcon();

                        // Remove the empty queue tab
                        int currentQueueTabIndex = ui->tab_queue->indexOf(tabWidget);
                        if (currentQueueTabIndex != -1) {
                            QWidget* widgetToDelete = ui->tab_queue->widget(currentQueueTabIndex);
                            ui->tab_queue->removeTab(currentQueueTabIndex);
                            if (widgetToDelete) {
                                delete widgetToDelete;
                            }
                        }
                        is_queue = false; // Transition out of queue mode
                        listWidget = nullptr; // Clear references
                        tabWidget = nullptr;  // Clear references
                    }
                } else {
                    // Queue is empty, stop
                    player->stop();
                    ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
                    ui->infoMusic->setText(""); ui->progressBar->setValue(0);
                    updatePlayButtonIcon();
                }
            } else { // Playing from main table
                if (ui->tableMusic->rowCount() > 0) {
                    int currentRow = ui->tableMusic->currentRow();
                    int nextRow = (currentRow >= ui->tableMusic->rowCount() - 1) ? 0 : currentRow + 1;
                    ui->tableMusic->setCurrentCell(nextRow, ui->tableMusic->currentColumn());
                    playMusic(ui->tableMusic->item(nextRow, c_address)->text());
                } else {
                    // No more items in main table, stop
                    player->stop();
                    ui->label_4->setText(""); ui->artist->setText(""); ui->title->setText("");
                    ui->infoMusic->setText(""); ui->progressBar->setValue(0);
                    updatePlayButtonIcon();
                }
            }
            break;
        }
    }
}

void home::on_pushButton_6_clicked()
{
    if (repeat == disable){
        repeat = once;
        ui->pushButton_6->setIcon(QIcon(":/JukeBox/Icon/26.png"));
        return;
    }
    else if (repeat == once){
        repeat = list;
        ui->pushButton_6->setIcon(QIcon(":/JukeBox/Icon/27.png"));
        return;
    }
    else if (repeat == list){
        repeat = disable;
        ui->pushButton_6->setIcon(QIcon(":/JukeBox/Icon/28.png"));
        return;
    }
}

void home::updateVisualizer()
{
    QSize labelSize = ui->moje->size();
    if (labelSize.isEmpty()) {
        return;
    }

    QPixmap pixmap(labelSize);
    pixmap.fill(Qt::black); // Fill background with black

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QPen pen(currentVisualizerColor);
    pen.setWidth(2);
    painter.setPen(pen);

    int barWidth = labelSize.width() / visualizerBarCount;
    if (barWidth == 0) barWidth = 1;

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        if (waveModel == false) { // Model 1: Vertical bars (existing)
            for (int i = 0; i < visualizerBarCount; ++i) {
                int barHeight = rand() % visualizerMaxHeight + 1;
                int x = i * barWidth + (barWidth / 2);
                painter.drawLine(x, labelSize.height(), x, labelSize.height() - barHeight);
            }
        } else { // Model 2: Waveform similar to the provided image (Simplified approach)
            QVector<QPoint> points;
            int centerY = labelSize.height() / 2;
            double maxAmplitude = visualizerMaxHeight / 2.0; // Max deviation from center

            // تعداد "پیک" یا نقاطی که می خواهیم در عرض ویژوالایزر داشته باشیم
            int numPeaks = visualizerBarCount * 2; // مثلا 2 برابر تعداد میله ها

            for (int i = 0; i < numPeaks; ++i) {
                double x_pos = (double)i / (numPeaks - 1) * labelSize.width();

                // Generate a "peak" height for this segment
                // A random value for the peak, simulating audio variations
                double peakHeightRatio = (rand() % 100) / 100.0; // 0.0 to 1.0
                double currentAmplitude = maxAmplitude * (0.3 + peakHeightRatio * 0.7); // Varies from 30% to 100% of max

                // Use an inverted V-shape or triangle for each peak/segment
                // This creates the spiky look from the image.
                // We'll draw two points per segment: one for the positive peak, one for the negative.

                // Top part of the wave
                int y_top = (int)(centerY - currentAmplitude);
                points.append(QPoint(x_pos, y_top));

                // Bottom part of the wave (symmetrical to top)
                int y_bottom = (int)(centerY + currentAmplitude);
                points.append(QPoint(x_pos, y_bottom));
            }

            // Draw the polyline, connecting all calculated points.
            // Note: This simple approach draws vertical lines between top and bottom points for each peak.
            // If you want a continuous line that goes up and down,
            // you'd need to connect (x_i, y_top_i) to (x_i+1, y_bottom_i) and then to (x_i+1, y_top_i+1)
            // For now, let's connect the top points and bottom points separately to create two lines.

            // To get a single continuous wave like the image, a slightly different approach:
            QVector<QPoint> topWavePoints;
            QVector<QPoint> bottomWavePoints;

            for (int i = 0; i < numPeaks; ++i) {
                double x_pos = (double)i / (numPeaks - 1) * labelSize.width();
                double peakHeightRatio = (rand() % 100) / 100.0;
                double currentAmplitude = maxAmplitude * (0.3 + peakHeightRatio * 0.7);

                int y_top = (int)(centerY - currentAmplitude);
                int y_bottom = (int)(centerY + currentAmplitude);

                topWavePoints.append(QPoint(x_pos, y_top));
                bottomWavePoints.append(QPoint(x_pos, y_bottom));
            }

            // Draw the top wave
            if (topWavePoints.size() > 1) {
                painter.drawPolyline(topWavePoints.data(), topWavePoints.size());
            }

            // Draw the bottom wave (mirror of top wave)
            if (bottomWavePoints.size() > 1) {
                painter.drawPolyline(bottomWavePoints.data(), bottomWavePoints.size());
            }
        }
    } else { // If not playing, display fixed state for the *current* waveModel
        int fixedHeight = visualizerMaxHeight / 3;
        if (waveModel == false) { // Fixed bars
            for (int i = 0; i < visualizerBarCount; ++i) {
                int x = i * barWidth + (barWidth / 2);
                painter.drawLine(x, labelSize.height(), x, labelSize.height() - fixedHeight);
            }
        } else { // Fixed Waveform (simpler, less dynamic when stopped, similar to image in resting state)
            QVector<QPoint> topWavePoints;
            QVector<QPoint> bottomWavePoints;
            int centerY = labelSize.height() / 2;
            double fixedAmplitude = visualizerMaxHeight / 2.0 * 0.2; // Smaller amplitude when stopped
            int numPoints = labelSize.width(); // Use more points for a smooth line

            for (int i = 0; i < numPoints; ++i) {
                double x_pos = (double)i / (numPoints - 1) * labelSize.width();
                double value = qSin(x_pos * 0.05); // A simple sine wave for the fixed state

                int y_top = (int)(centerY - value * fixedAmplitude);
                int y_bottom = (int)(centerY + value * fixedAmplitude);

                topWavePoints.append(QPoint(x_pos, y_top));
                bottomWavePoints.append(QPoint(x_pos, y_bottom));
            }

            if (topWavePoints.size() > 1) {
                painter.drawPolyline(topWavePoints.data(), topWavePoints.size());
            }
            if (bottomWavePoints.size() > 1) {
                painter.drawPolyline(bottomWavePoints.data(), bottomWavePoints.size());
            }
        }
    }

    ui->moje->setPixmap(pixmap);
}

void home::on_moje_clicked()
{
    waveModel = !waveModel;
    updateVisualizer();
}


bool home::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->moje && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            on_moje_clicked();
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void home::saveUserData(const QString& username)
{
    if (username.isEmpty()) {
        qDebug() << "Cannot save data: Username is empty.";
        QMessageBox::warning(this, "خطا در ذخیره", "نام کاربری وارد نشده است.");
        return;
    }

    QJsonObject userData;
    userData["username"] = username;

    // ذخیده table_music
    QJsonArray musicTableFilePathsArray;
    if (ui->tableMusic) { // اطمینان از اینکه music_table وجود دارد
        for (int row = 0; row < ui->tableMusic->rowCount(); ++row) {
            // فرض می‌کنیم که آدرس فایل در ستون 0 (اولین ستون) است.
            QTableWidgetItem* filePathItem = ui->tableMusic->item(row, c_address); // ستون آدرس فایل

            if (filePathItem) {
                musicTableFilePathsArray.append(filePathItem->text());
            }
        }
    }
    // آدرس‌های فایل را با کلید "music_file_paths" در JSON اصلی ذخیره می‌کنیم
    userData["music_file_paths"] = musicTableFilePathsArray;
    // --- پایان ذخیره اطلاعات QTableWidget music_table ---


    // ذخیره پلی‌لیست‌ها
    QJsonArray playlistsArray;
    for (int i = 0; i < ui->tab_playlist->count(); ++i) {
        QJsonObject playlistObject;
        QString tabName = ui->tab_playlist->tabText(i);
        playlistObject["name"] = tabName;

        QWidget* tabWidget = ui->tab_playlist->widget(i);
        // اینجا باید مطمئن شوید که QListWidget شما objectName مناسبی دارد
        QListWidget* listWidget = tabWidget->findChild<QListWidget*>("playlistListWidget"); // فرض شده objectName برای QListWidgetها "playlistListWidget" است
        if (listWidget) {
            QJsonArray musicFilesArray;
            for (int j = 0; j < listWidget->count(); ++j) {
                QListWidgetItem* item = listWidget->item(j);
                QString filePath = item->data(Qt::UserRole + 1).toString(); // مسیر فایل را از Qt::UserRole + 1 می‌خواند
                musicFilesArray.append(filePath);
            }
            playlistObject["musicFiles"] = musicFilesArray;
        }
        playlistsArray.append(playlistObject);
    }
    userData["playlists"] = playlistsArray;

    // ذخیره صف‌های پخش
    QJsonArray queuesArray;
    for (int i = 0; i < ui->tab_queue->count(); ++i) {
        QJsonObject queueObject;
        QString tabName = ui->tab_queue->tabText(i);
        queueObject["name"] = tabName;

        QWidget* tabWidget = ui->tab_queue->widget(i);
        // اینجا هم باید مطمئن شوید که QListWidget شما objectName مناسبی دارد
        QListWidget* listWidget = tabWidget->findChild<QListWidget*>("queueListWidget"); // فرض شده objectName برای QListWidgetهای صف "queueListWidget" است. یا همان "playlistListWidget" اگر منطق یکسان است.
        if (listWidget) {
            QJsonArray musicFilesArray;
            for (int j = 0; j < listWidget->count(); ++j) {
                QListWidgetItem* item = listWidget->item(j);
                QString filePath = item->data(Qt::UserRole + 1).toString();
                musicFilesArray.append(filePath);
            }
            queueObject["musicFiles"] = musicFilesArray;
        }
        queuesArray.append(queueObject);
    }
    userData["queues"] = queuesArray;

    QJsonDocument saveDoc(userData);

    // تعیین مسیر ذخیره فایل (مسیر استاندارد برای داده‌های برنامه)
    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appDataLocation);
    if (!dir.exists()) {
        dir.mkpath("."); // ایجاد دایرکتوری در صورت عدم وجود
    }

    QString fileName = appDataLocation + "/" + username + "_music_data.json";
    qDebug() << "JSON file path:" << fileName;
    QFile saveFile(fileName);
    if (!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file %s for writing: %s", qPrintable(fileName), qPrintable(saveFile.errorString()));
        QMessageBox::warning(this, "خطا در ذخیره", "امکان ذخیره اطلاعات کاربر وجود ندارد.");
        return;
    }

    saveFile.write(saveDoc.toJson(QJsonDocument::Indented)); // ذخیره با فرمت خواناتر
    saveFile.close();
    qDebug() << "User data saved to:" << fileName;
    QMessageBox::information(this, "ذخیره موفق", "اطلاعات کاربر با موفقیت ذخیره شد!");
}

void home::loadUserData(const QString& username)
{
    if (username.isEmpty()) {
        qDebug() << "Cannot load data: Username is empty.";
        QMessageBox::warning(this, "خطا در بارگذاری", "نام کاربری وارد نشده است.");
        return;
    }

    QString appDataLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString fileName = appDataLocation + "/" + username + "_music_data.json";
    QFile loadFile(fileName);

    if (!loadFile.exists()) {
        qDebug() << "No saved data found for user:" << username;
        QMessageBox::information(this, "بارگذاری", "هیچ اطلاعات ذخیره شده‌ای برای این کاربر پیدا نشد. تنظیمات پیش‌فرض اعمال می‌شود.");
        // می‌توانید اینجا پلی‌لیست‌ها و صف‌های پیش‌فرض را ایجاد کنید
        return;
    }

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open load file %s for reading: %s", qPrintable(fileName), qPrintable(loadFile.errorString()));
        QMessageBox::warning(this, "خطا در بارگذاری", "امکان بارگذاری اطلاعات کاربر وجود ندارد.");
        return;
    }

    QByteArray savedData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(savedData));

    if (loadDoc.isNull() || !loadDoc.isObject()) {
        qWarning() << "Failed to parse JSON document or it's not an object.";
        QMessageBox::warning(this, "خطا در بارگذاری", "فرمت فایل ذخیره شده نامعتبر است.");
        loadFile.close();
        return;
    }

    QJsonObject userData = loadDoc.object();

    // بارگذاری table_music
    QJsonArray tableArray = userData["music_file_paths"].toArray();
    int cur_row;
    ui->tableMusic->setRowCount(0);
    for (const QJsonValue &filePathValue : tableArray) {
        QString filePath = filePathValue.toString();
        QFileInfo fileInfo(filePath);

        QString fileName = fileInfo.completeBaseName();
        QString format = fileInfo.suffix().toUpper();
        QString size = QString::number(fileInfo.size() / (1024.0 * 1024.0), 'f', 2) + "MB";
        cur_row = ui->tableMusic->rowCount();
        ui->tableMusic->insertRow(cur_row);
        ui->tableMusic->setItem(cur_row, c_address, new QTableWidgetItem(filePath));
        ui->tableMusic->setItem(cur_row, c_format, new QTableWidgetItem(format));
        ui->tableMusic->setItem(cur_row, c_size, new QTableWidgetItem(size));
        ui->tableMusic->setItem(cur_row, c_address, new QTableWidgetItem(filePath));
        extractMetadata(filePath, cur_row);
    }
    ui->tableMusic->resizeColumnsToContents();

    // بارگذاری پلی‌لیست‌ها
    QJsonArray playlistsArray = userData["playlists"].toArray();
    for (const QJsonValue &playlistValue : playlistsArray) {
        QJsonObject playlistObject = playlistValue.toObject();
        QString playlistName = playlistObject["name"].toString();
        QJsonArray musicFilesArray = playlistObject["musicFiles"].toArray();

        // ایجاد تب پلی‌لیست و QListWidget مربوطه
        creat_list(playlistName); // این تابع باید تب و QListWidget را ایجاد کند و objectName را تنظیم کند
        // پیدا کردن QListWidget که تازه ایجاد شده است
        QWidget* newTabWidget = ui->tab_playlist->widget(ui->tab_playlist->count() - 1);
        QListWidget* currentListWidget = newTabWidget->findChild<QListWidget*>("playlistListWidget");

        if (currentListWidget) {
            // اطمینان از اتصال سیگنال itemDoubleClicked
            disconnect(currentListWidget, &QListWidget::itemDoubleClicked, this, &home::play_list_play);
            connect(currentListWidget, &QListWidget::itemDoubleClicked, this, &home::play_list_play);
            currentListWidget->setItemDelegate(new style_playlistitem(currentListWidget)); // اعمال Delegate

            for (const QJsonValue &filePathValue : musicFilesArray) {
                QString filePath = filePathValue.toString();
                // اضافه کردن آهنگ به پلی‌لیست
                QListWidgetItem* item = new QListWidgetItem();
                bool foundInTable = false;
                for (int i = 0; i < ui->tableMusic->rowCount(); ++i) {
                    // مسیر فایل را از tableMusic با c_address مقایسه می‌کند
                    if (filePath == ui->tableMusic->item(i, c_address)->text()) {
                        QString title = ui->tableMusic->item(i, c_title)->text();
                        QString format = ui->tableMusic->item(i, c_format)->text();
                        QString size = ui->tableMusic->item(i, c_size)->text();
                        QString length = ui->tableMusic->item(i, c_length)->text();

                        item->setText(title);
                        item->setData(Qt::UserRole, format + " :: " + size + ", " + length);
                        item->setData(Qt::UserRole + 1, filePath); // مسیر فایل را در UserRole + 1 ذخیره می‌کند
                        foundInTable = true;
                        break;
                    }
                }
                if (foundInTable) {
                    currentListWidget->addItem(item);
                } else {
                    // اگر فایل در tableMusic پیدا نشد، می‌توانید آن را به صورت "Missing File" یا مشابه اضافه کنید
                    // یا کلاً اضافه نکنید.
                    qWarning() << "Music file not found in tableMusic:" << filePath;
                    delete item; // حذف آیتم اگر استفاده نشود
                }
            }
        } else {
            qWarning() << "Could not find QListWidget in playlist tab:" << playlistName;
        }
    }

    // بارگذاری صف‌های پخش (مشابه پلی‌لیست‌ها)
    QJsonArray queuesArray = userData["queues"].toArray();
    for (const QJsonValue &queueValue : queuesArray) {
        QJsonObject queueObject = queueValue.toObject();
        QString queueName = queueObject["name"].toString();
        QJsonArray musicFilesArray = queueObject["musicFiles"].toArray();

        creat_queue(queueName); // این تابع باید تب و QListWidget را ایجاد کند
        QWidget* newTabWidget = ui->tab_queue->widget(ui->tab_queue->count() - 1);
        QListWidget* currentListWidget = newTabWidget->findChild<QListWidget*>("queueListWidget"); // یا "playlistListWidget"

        if (currentListWidget) {
            disconnect(currentListWidget, &QListWidget::itemDoubleClicked, this, &home::play_queue);
            connect(currentListWidget, &QListWidget::itemDoubleClicked, this, &home::play_queue);
            currentListWidget->setItemDelegate(new style_playlistitem(currentListWidget)); // اعمال Delegate

            for (const QJsonValue &filePathValue : musicFilesArray) {
                QString filePath = filePathValue.toString();
                QListWidgetItem* item = new QListWidgetItem();
                bool foundInTable = false;
                for (int i = 0; i < ui->tableMusic->rowCount(); ++i) {
                    if (filePath == ui->tableMusic->item(i, c_address)->text()) {
                        QString title = ui->tableMusic->item(i, c_title)->text();
                        QString format = ui->tableMusic->item(i, c_format)->text();
                        QString size = ui->tableMusic->item(i, c_size)->text();
                        QString length = ui->tableMusic->item(i, c_length)->text();

                        item->setText(title);
                        item->setData(Qt::UserRole, format + " :: " + size + ", " + length);
                        item->setData(Qt::UserRole + 1, filePath);
                        foundInTable = true;
                        break;
                    }
                }
                if (foundInTable) {
                    currentListWidget->addItem(item);
                } else {
                    qWarning() << "Music file not found in tableMusic (for queue):" << filePath;
                    delete item;
                }
            }
        } else {
            qWarning() << "Could not find QListWidget in queue tab:" << queueName;
        }
    }
    loadFile.close();
    qDebug() << "User data loaded for:" << username;
    QMessageBox::information(this, "بارگذاری موفق", "اطلاعات کاربر با موفقیت بارگذاری شد!");
}

void home::handlePlayPauseFromNetwork(const QString &songTitle, const QString &action, qint64 pos)
{
    if (!musicNetwork) return;

    isRemoteChange = true;

    // پیدا کردن آهنگ در جدول
    int foundRow = -1;
    for (int row = 0; row < ui->tableMusic->rowCount(); ++row) {
        if (ui->tableMusic->item(row, c_title) &&
            ui->tableMusic->item(row, c_title)->text() == songTitle) {
            foundRow = row;
            break;
        }
    }

    if (foundRow != -1) {
        QString filePath = ui->tableMusic->item(foundRow, c_address)->text();

        // اگر آهنگ عوض شده
        if (player->source().toLocalFile() != filePath) {
            player->setSource(QUrl::fromLocalFile(filePath));
        }

        // تنظیم موقعیت پخش
        player->setPosition(pos);

        // اجرای اکشن
        if (action == "play") {
            player->play();
        } else if (action == "pause") {
            player->pause();
        } else if (action == "stop") {
            player->stop();
        }

        set_info();
        updatePlayButtonIcon();
    }

    isRemoteChange = false;
}

void home::on_pushButton_3_clicked()
{
    if (!musicNetwork) {
        musicNetwork = new MusicNetwork(this);
        connect(musicNetwork, &MusicNetwork::playPauseFromNetwork,
                this, &home::handlePlayPauseFromNetwork);
    }
    // فرض: localhost و پورت 12345 (یا هر آی‌پی مورد نظر)
    musicNetwork->connectToServer("127.0.0.1", 12345);
    // (اختیاری: پیام موفقیت یا نمایش دیالوگ)
}

void home::sendNetworkPlaybackState()
{
    if (!musicNetwork || isRemoteChange) return;

    QJsonObject msg;
    QMediaPlayer::PlaybackState state = player->playbackState();

    if (state == QMediaPlayer::PlayingState)
        msg["action"] = "play";
    else if (state == QMediaPlayer::PausedState)
        msg["action"] = "pause";
    else if (state == QMediaPlayer::StoppedState)
        msg["action"] = "stop";
    else
        return; // Unknown state

    msg["song_title"] = ui->title->text();
    msg["position"] = player->position();

    // فرض: musicNetwork->isServer() متدی است که سرور بودن را تشخیص می‌دهد
    if (musicNetwork->isServer())
        musicNetwork->broadcastToClients(msg);
    else
        musicNetwork->sendToServer(msg);
}

void home::updateNetworkStatus(const QString& status)
{

    ui->status->setText(status);

    // نمایش پیام به مدت 3 ثانیه
    QTimer::singleShot(3000, this, [this]() {
        if (musicNetwork && musicNetwork->isConnected()) {
            if (musicNetwork->isServer()) {
                ui->status->setText("Server: Running");
            } else {
                ui->status->setText("Client: Connected");
            }
        } else {
            ui->status->setText("Not Connected");
        }
    });
}

// بهبود تابع‌های مربوط به دکمه‌های سرور و کلاینت
void home::on_pushButton_2_clicked() // Server button
{
    if (!musicNetwork) {
        musicNetwork = new MusicNetwork(this);

        // اتصال سیگنال‌های شبکه
        connect(musicNetwork, &MusicNetwork::clientConnected,
                this, [this](const QString& addr) {
                    updateNetworkStatus("Client connected: " + addr);
                });

        connect(musicNetwork, &MusicNetwork::clientDisconnected,
                this, [this](const QString& addr) {
                    updateNetworkStatus("Client disconnected: " + addr);
                });

        connect(musicNetwork, &MusicNetwork::networkError,
                this, [this](const QString& error) {
                    updateNetworkStatus("Error: " + error);
                    QMessageBox::warning(this, "Network Error", error);
                });

        connect(musicNetwork, &MusicNetwork::playPauseFromNetwork,
                this, &home::handlePlayPauseFromNetwork);
    }

    // راه‌اندازی سرور
    musicNetwork->startServer(12345);

    // نمایش دیالوگ سرور
    ServerDialog* dlg = new ServerDialog(this);
    dlg->setStatus("Server running on port 12345");
    dlg->setClients(musicNetwork->getConnectedClients());

    // به‌روزرسانی خودکار لیست کلاینت‌ها
    connect(musicNetwork, &MusicNetwork::clientConnected,
            dlg, &ServerDialog::addClient);
    connect(musicNetwork, &MusicNetwork::clientDisconnected,
            dlg, &ServerDialog::removeClient);

    connect(musicNetwork, &MusicNetwork::clientConnected,
            this, &home::sendCurrentPlaylist);


    dlg->exec();
    dlg->deleteLater();
}

void home::sendCurrentPlaylist()
{
    if (!musicNetwork || !musicNetwork->isServer()) return;

    QJsonObject msg;
    msg["action"] = "playlist_sync";

    QJsonArray songs;
    for (int row = 0; row < ui->tableMusic->rowCount(); ++row) {
        QJsonObject song;
        song["title"] = ui->tableMusic->item(row, c_title)->text();
        song["artist"] = ui->tableMusic->item(row, c_artist)->text();
        song["path"] = ui->tableMusic->item(row, c_address)->text();
        songs.append(song);
    }

    msg["songs"] = songs;
    musicNetwork->broadcastToClients(msg);
}

// و این را به سیگنال clientConnected در on_pushButton_2_clicked اضافه کنید:
