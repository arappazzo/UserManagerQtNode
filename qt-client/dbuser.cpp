#include "dbuser.h"

DbUser::DbUser(QObject *parent):
    QObject(parent)
{

}

void DbUser::setName(const QString &val)
{
    mName = val;
    emit nameChanged();
}

void DbUser::setAge(const int &val)
{
    mAge = val;
    emit ageChanged();
}

void DbUser::setTableId(const int &val)
{
    mTableId = val;
    emit tableIdChanged();
}
