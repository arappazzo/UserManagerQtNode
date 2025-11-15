#ifndef DBUSER_H
#define DBUSER_H

#include <QObject>

class DbUser: public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(int age READ age WRITE setAge NOTIFY ageChanged)
    Q_PROPERTY(int tableId READ tableId WRITE setTableId NOTIFY tableIdChanged)

public:

    explicit DbUser(QObject *parent = nullptr);
    virtual ~DbUser(){};

    QString name() const {return mName;}
    int age() const {return mAge;}
    int tableId() const {return mTableId;}

public slots:
    void setName(const QString &val);
    void setAge(const int& val);
    void setTableId(const int& val);

private:
    QString mName = "";
    int mAge = -1;
    int mTableId = -1;

signals:
    void nameChanged();
    void ageChanged();
    void tableIdChanged();
};

#endif // DBUSER_H
