#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(const QUrl &url, QObject *parent = nullptr);
    ~WebSocketClient();

    void start();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void tryReconnect();

private:
    QWebSocket m_webSocket;
    QUrl m_url;
    QTimer m_reconnectTimer;

signals:
    void serverOnline();
};

#endif // WEBSOCKETCLIENT_H
