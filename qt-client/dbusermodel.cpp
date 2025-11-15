#include "dbusermodel.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QNetworkReply>

const QString DbUserModel::SERVER_URL = "http://localhost:3000/api/users";
const QString DbUserModel::WEBSOCKET_URL = "ws://localhost:3001";

DbUserModel::DbUserModel(QAbstractListModel *parent):
    QAbstractListModel(parent)
{
    mpManager = new QNetworkAccessManager();
    getUsers();

    mpSocketClient = new WebSocketClient(QUrl(WEBSOCKET_URL));

    connect(mpSocketClient, &WebSocketClient::serverOnline, [this](){
        getUsers();
    });
    mpSocketClient->start();
}

DbUserModel::~DbUserModel()
{
    foreach (DbUser *el, mUserList)
        delete el;

    mUserList.clear();

    delete mpManager;

    delete mpSocketClient;
}

QHash<int, QByteArray> DbUserModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[nameRole] = "Name";
    roles[ageRole] = "Age";
    roles[tableIdRole] = "TableId";

    return roles;
}

void DbUserModel::addUser(QString name, int age, int tableId, bool insertRows)
{
    auto it = find_if(mUserList.begin(), mUserList.end(), [=](DbUser* el){
        return el->age() == age && el->name() == name;
    });

    if (it == mUserList.end())
    {
        if (insertRows)
            beginInsertRows(QModelIndex(), rowCount(), rowCount());

        DbUser* el = new DbUser();
        el->setName(name);
        el->setAge(age);
        el->setTableId(tableId);
        mUserList.append(el);

        if (insertRows)
            endInsertRows();
    }
}

void DbUserModel::deleteUser(QString name, int age)
{
    auto it = find_if(mUserList.begin(), mUserList.end(), [=](DbUser* el){
        return el->age() == age && el->name() == name;
    });

    if (it != mUserList.end())
    {
        int index = it - mUserList.begin();
        deleteUserByIndex(index);
    }
}

void DbUserModel::deleteUserByIndex(int index)
{
    beginRemoveRows(QModelIndex(), index, index);

    auto iterator= mUserList.begin()+index;
    mUserList.erase(iterator);

    endRemoveRows();
}

void DbUserModel::createList(const QByteArray &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    if (!doc.isArray())
    {
        qWarning() << "Expected a JSON array!";
        return;
    }

    QJsonArray array = doc.array();
    if (array.size() < 0)
        return;

    beginResetModel();

    mUserList.clear();
    for (const QJsonValue &val : array)
    {
        if (val.isObject())
        {
            QJsonObject obj = val.toObject();
            QString name = obj["name"].toString();
            int age = obj["age"].toInt();
            int tableId = obj["id"].toInt();

            addUser(name, age, tableId);
        }
    }

    endResetModel();
}

int DbUserModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mUserList.count();
}

QVariant DbUserModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() > mUserList.size())
        return QVariant();

    if(role <= Qt::UserRole && role > ageRole)
        return QVariant();

    DbUser *el = mUserList[index.row()];

    if (el == nullptr)
        return QVariant();

    if (role == nameRole)
        return el->name();
    else if (role == ageRole)
        return el->age();
    else if (role == tableIdRole)
        return el->tableId();

    return QVariant();
}

bool DbUserModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() < 0 || index.row() > mUserList.size())
        return false;

    if(role <= Qt::UserRole && role > ageRole)
        return false;

   DbUser *el = mUserList[index.row()];

    if (el == nullptr)
        return false;

    bool ret = false;

    if(role > Qt::UserRole && role <= ageRole)
    {
        if (role == nameRole)
        {
            el->setName(value.toString());
        }
        else if (role == ageRole)
        {
            el->setAge(value.toInt());
        }
        else if (role == tableIdRole)
        {
            el->setTableId(value.toInt());
        }

        emit dataChanged(index,index);
        ret = true;
    }

    return ret;
}

void DbUserModel::sendUserToServer(const QString &name, int age)
{

    qDebug() << "DbUserModel::sendUserToServer, name = " << name << " age  = " << age;

    QUrl url(SERVER_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject userJson;
    userJson["name"] = name;
    userJson["age"] = age;

    QNetworkReply *reply = mpManager->sendCustomRequest(
        request,
        "POST",
        QJsonDocument(userJson).toJson()
    );


    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray responseData = reply->readAll();
            qDebug() << "Server response:" << responseData;

            QJsonDocument doc = QJsonDocument::fromJson(responseData);
            QJsonObject obj = doc.object();
            QString name = obj["name"].toString();
            int age = obj["age"].toInt();
            int tableId = obj["id"].toInt();

            addUser(name, age, tableId, true);
        }
        else
        {
            qWarning() << "Error:" << reply->errorString();
        }

        reply->deleteLater();
    });
}

void DbUserModel::deleteUserFromServer(int id)
{
    // DELETE non deve avere un body → molti server rifiutano DELETE + body ---> ID va messo nell’URL

    // Costruisco l’URL con l'ID
    QUrl url(QString("%1/%2").arg(SERVER_URL).arg(id));

    QNetworkRequest request(url);

    // DELETE non necessita di body!
    QNetworkReply *reply = mpManager->sendCustomRequest(
        request,
        "DELETE"
        );

    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            qDebug() << "User deleted.";

            // chiedi la lista aggiornata
            getUsers();
        }
        else
        {
            qWarning() << "Error:" << reply->errorString();
        }

        reply->deleteLater();
    });
}

void DbUserModel::getUsers()
{
    QUrl url(SERVER_URL);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;

    QNetworkReply *reply = mpManager->sendCustomRequest(
        request,
        "GET",
        QJsonDocument(json).toJson()
        );

    QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray responseData = reply->readAll();
            qDebug() << "Server response:" << responseData;

            createList(responseData);
        }
        else
        {
            qWarning() << "Error:" << reply->errorString();
        }

        reply->deleteLater();
    });
}
