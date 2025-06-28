#ifndef MUSICNETWORK_H
#define MUSICNETWORK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>

class MusicNetwork : public QObject
{
    Q_OBJECT

public:
    explicit MusicNetwork(QObject* parent = nullptr);
    ~MusicNetwork();

    // سرور
    void startServer(quint16 port = 12345);
    // کلاینت
    void connectToServer(const QString& host, quint16 port = 12345);
    // ارسال پیام به سرور یا کل کلاینت‌ها
    void broadcastToClients(const QJsonObject& msg);
    void sendToServer(const QJsonObject& msg);
    bool isServer() const;

    QStringList getConnectedClients() const;         // گرفتن لیست کلاینت‌های متصل
    bool isConnected() const;
signals:
    void playPauseFromNetwork(const QString& songTitle, const QString& action, qint64 pos);
    void clientConnected(const QString& address);    // وقتی کلاینت جدید وصل میشه
    void clientDisconnected(const QString& address); // وقتی کلاینت قطع میشه
    void networkError(const QString& error);
    void serverStarted(quint16 port);
private slots:
    void handleIncomingMessage(QTcpSocket* socket);
    void handleSocketError(QAbstractSocket::SocketError socketError);
private:
    QTcpServer* server = nullptr;
    QList<QTcpSocket*> clientSockets;
    QTcpSocket* clientSocket = nullptr;
};

#endif // MUSICNETWORK_H
