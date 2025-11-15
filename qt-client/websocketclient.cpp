#include "websocketclient.h"
#include <QJsonDocument>
#include <QJsonObject>

WebSocketClient::WebSocketClient(const QUrl &url, QObject *parent)
    : QObject(parent), m_url(url)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

    m_reconnectTimer.setInterval(2000); // try connection every 2 sec
    m_reconnectTimer.setSingleShot(false);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::tryReconnect);
}

WebSocketClient::~WebSocketClient()
{
    m_webSocket.close();
    m_reconnectTimer.stop();
}

void WebSocketClient::start()
{
    tryReconnect();
}

void WebSocketClient::onConnected()
{
    qDebug() << "Connected to WebSocket server";
    m_reconnectTimer.stop(); // no need to further retry
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "WebSocket disconnected, will retry...";
    m_reconnectTimer.start(); // need to retry connection
}

void WebSocketClient::onTextMessageReceived(const QString &message)
{
    qDebug() << "Message:" << message;

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isObject())
    {
        QJsonObject obj = doc.object();
        if (obj["event"] == "serverOnline")
        {
            emit serverOnline();
        }
    }
}

void WebSocketClient::tryReconnect()
{
    if (m_webSocket.state() != QAbstractSocket::ConnectedState)
    {
        qDebug() << "Trying to connect to WebSocket server...";
        m_webSocket.open(m_url);
    }
}

