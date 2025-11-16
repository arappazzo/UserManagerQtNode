#ifndef DBUSERMODEL_H
#define DBUSERMODEL_H

#include <QAbstractListModel>
#include "dbuser.h"
#include <QNetworkAccessManager>

#include "websocketclient.h"

using namespace  std;

class DbUserModel: public QAbstractListModel
{
    Q_OBJECT
public:
    explicit DbUserModel(QAbstractListModel *parent = nullptr);

    ~DbUserModel();

    enum DbUserRoles {
        IdFRole = Qt::UserRole + 1,
        nameRole,
        tableIdRole,
        ageRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::UserRole) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QHash<int, QByteArray> roleNames() const;


    Q_INVOKABLE void sendUserToServer(const QString &name, int age);
    Q_INVOKABLE void deleteUserFromServer(int id);

    static const QString SERVER_URL;
    static const QString WEBSOCKET_URL;

private:
    QList<DbUser*> mUserList{};

    QNetworkAccessManager *mpManager = nullptr;

    void getUsers();

    void createList(const QByteArray &jsonData);
	
	void addUser(QString name, int age, int tableId, bool insertRows = false);
	
    WebSocketClient *mpSocketClient = nullptr;

};

#endif // DBUSERMODEL_H
