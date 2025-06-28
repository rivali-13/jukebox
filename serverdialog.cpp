#include "serverdialog.h"

ServerDialog::ServerDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Server Status");
    setMinimumSize(350, 300);

    QVBoxLayout* layout = new QVBoxLayout(this);
    statusLabel = new QLabel("Server not started.", this);
    clientListWidget = new QListWidget(this);
    QPushButton* closeBtn = new QPushButton("Close", this);

    layout->addWidget(statusLabel);
    layout->addWidget(new QLabel("Connected Clients:", this));
    layout->addWidget(clientListWidget);
    layout->addWidget(closeBtn);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void ServerDialog::setStatus(const QString& status)
{
    statusLabel->setText(status);
}

void ServerDialog::setClients(const QStringList& clients)
{
    clientListWidget->clear();
    clientListWidget->addItems(clients);
}

void ServerDialog::addClient(const QString& clientAddress)
{
    // اضافه کردن کلاینت جدید به لیست
    if (!clientListWidget->findItems(clientAddress, Qt::MatchExactly).isEmpty()) {
        return; // اگر قبلاً اضافه شده، برگرد
    }

    QListWidgetItem* item = new QListWidgetItem(clientAddress);
    item->setIcon(QIcon(":/JukeBox/Icon/user.png")); // یک آیکون برای کلاینت
    clientListWidget->addItem(item);

    updateClientCount();
}

void ServerDialog::removeClient(const QString& clientAddress)
{
    // حذف کلاینت از لیست
    QList<QListWidgetItem*> items =
        clientListWidget->findItems(clientAddress, Qt::MatchExactly);

    for (QListWidgetItem* item : items) {
        delete clientListWidget->takeItem(clientListWidget->row(item));
    }

    updateClientCount();
}

void ServerDialog::updateClientCount()
{
    int count = clientListWidget->count();
    statusLabel->setText(QString("Connected Clients: %1").arg(count));
    emit clientCountChanged(count);
}
