#include "musicnetwork.h"

MusicNetwork::MusicNetwork(QObject* parent)
    : QObject(parent)
{
}

MusicNetwork::~MusicNetwork()
{
    if (server) {
        server->close();
        delete server;
    }
    if (clientSocket) {
        clientSocket->close();
        delete clientSocket;
    }
}

void MusicNetwork::connectToServer(const QString& host, quint16 port)
{
    if (clientSocket)
        return;
    clientSocket = new QTcpSocket(this);
    clientSocket->connectToHost(host, port);
    connect(clientSocket, &QTcpSocket::readyRead, this, [=]() {
        handleIncomingMessage(clientSocket);
    });
}

void MusicNetwork::broadcastToClients(const QJsonObject& msg)
{
    QByteArray data = QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n";
    for (QTcpSocket* sock : clientSockets) {
        sock->write(data);
    }
}

void MusicNetwork::sendToServer(const QJsonObject& msg)
{
    if (clientSocket && clientSocket->state() == QAbstractSocket::ConnectedState) {
        clientSocket->write(QJsonDocument(msg).toJson(QJsonDocument::Compact) + "\n");
    }
}

bool MusicNetwork::isServer() const
{
    return server != nullptr;
}

QStringList MusicNetwork::getConnectedClients() const
{
    QStringList clients;
    for (QTcpSocket* socket : clientSockets) {
        clients << socket->peerAddress().toString() + ":" +
                       QString::number(socket->peerPort());
    }
    return clients;
}

bool MusicNetwork::isConnected() const
{
    if (server) {
        return server->isListening();
    } else if (clientSocket) {
        return clientSocket->state() == QAbstractSocket::ConnectedState;
    }
    return false;
}

// بهبود تابع handleIncomingMessage
void MusicNetwork::handleIncomingMessage(QTcpSocket* socket)
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine();
        QJsonDocument doc = QJsonDocument::fromJson(line);

        if (!doc.isObject()) {
            emit networkError("Invalid message format received");
            continue;
        }

        QJsonObject obj = doc.object();

        try {
            QString action = obj["action"].toString();
            QString songTitle = obj["song_title"].toString();
            qint64 position = obj["position"].toVariant().toLongLong();

            // اعتبارسنجی داده‌ها
            if (action.isEmpty() || songTitle.isEmpty()) {
                emit networkError("Invalid message data");
                continue;
            }

            // ارسال به همه کلاینت‌ها (اگر سرور هستیم)
            if (server && socket != nullptr) {
                broadcastToClients(obj);
            }

            // ارسال سیگنال برای پخش
            emit playPauseFromNetwork(songTitle, action, position);

        } catch (const std::exception& e) {
            emit networkError(QString("Error processing message: %1").arg(e.what()));
        }
    }
}

// بهبود تابع startServer
void MusicNetwork::startServer(quint16 port)
{
    // اگر قبلاً سرور فعال است، اول آن را ببندیم
    if (server) {
        qDebug() << "Server already running, stopping previous instance...";
        server->close();
        delete server;
        server = nullptr;
        clientSockets.clear();
    }

    // اگر به عنوان کلاینت متصل هستیم، ابتدا قطع کنیم
    if (clientSocket) {
        qDebug() << "Client connection detected, disconnecting...";
        clientSocket->disconnectFromHost();
        delete clientSocket;
        clientSocket = nullptr;
    }

    // ایجاد سرور جدید
    server = new QTcpServer(this);

    // تنظیم سرور برای گوش دادن به پورت مشخص شده
    if (!server->listen(QHostAddress::Any, port)) {
        QString errorMsg = QString("Could not start server on port %1: %2")
        .arg(port)
            .arg(server->errorString());
        qDebug() << "Server Error:" << errorMsg;
        emit networkError(errorMsg);

        delete server;
        server = nullptr;
        return;
    }

    // اتصال سیگنال برای کلاینت‌های جدید
    connect(server, &QTcpServer::newConnection, this, [this]() {
        while (server->hasPendingConnections()) {
            QTcpSocket* socket = server->nextPendingConnection();
            if (!socket) continue;

            QString clientAddress = QString("%1:%2")
                                        .arg(socket->peerAddress().toString())
                                        .arg(socket->peerPort());

            qDebug() << "New client connected:" << clientAddress;

            // اضافه کردن سوکت به لیست کلاینت‌ها
            clientSockets.append(socket);

            // اتصال سیگنال‌های سوکت
            connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
                handleIncomingMessage(socket);
            });

            connect(socket, &QTcpSocket::disconnected, this, [this, socket, clientAddress]() {
                qDebug() << "Client disconnected:" << clientAddress;
                clientSockets.removeOne(socket);
                emit clientDisconnected(clientAddress);
                socket->deleteLater();
            });

            // استفاده از سینتکس ساده‌تر برای اتصال سیگنال error
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(handleSocketError(QAbstractSocket::SocketError)));

            // ارسال سیگنال اتصال موفق
            emit clientConnected(clientAddress);
        }
    });

    qDebug() << "Server successfully started on port" << port;
    emit serverStarted(port);
}

void MusicNetwork::handleSocketError(QAbstractSocket::SocketError socketError)
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (socket) {
        QString clientAddress = QString("%1:%2")
        .arg(socket->peerAddress().toString())
            .arg(socket->peerPort());

        QString errorMsg = QString("Client %1 error: %2")
                               .arg(clientAddress)
                               .arg(socket->errorString());
        qDebug() << errorMsg;
        emit networkError(errorMsg);
    }
}
