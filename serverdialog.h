#ifndef SERVERDIALOG_H
#define SERVERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTimer>

class ServerDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ServerDialog(QWidget* parent = nullptr);

    void setStatus(const QString& status);
    void setClients(const QStringList& clients);
signals:
    void clientCountChanged(int count);
public slots:
    void addClient(const QString& clientAddress);
    void removeClient(const QString& clientAddress);
    void updateClientCount();
private:
    QLabel* statusLabel;
    QListWidget* clientListWidget;
    QTimer* updateTimer;
};

#endif // SERVERDIALOG_H
