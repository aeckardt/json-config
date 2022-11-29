#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "configitem.h"

ConfigItem::ConfigItem()
    : JsonTreeItem(),
      m_extendedData(nullptr),
      m_extendedType(None)
{
}

void ConfigItem::clearExtended()
{
    switch (m_extendedType)
    {
    case StringMap:
        // m_extendedData is a pointer to a QMap<QString, QString> object
        freeExtendedData<StringMap>();
        break;
    case StringList:
        // m_extendedData is a pointer to a QStringList object
        freeExtendedData<StringList>();
        break;
    case IntList:
        // m_extendedData is a pointer to a QList<int> object
        freeExtendedData<IntList>();
        break;
    default:
        break;
    }

    m_extendedType = None;
    m_extendedData = nullptr;
}

ConfigItem::ValueType<ConfigItem::StringMap> &ConfigItem::stringMap()
{
    if (m_extendedType != StringMap) {
        allocExtendedData<StringMap>();

        auto &object = forceAsType<ParentType<StringMap>>();
        for (JsonTreeItem *dictItem : qAsConst(object))
            asExtendedType<StringMap>()[dictItem->key()] = dictItem->value().toString();
        object.clear();
    }

    return asExtendedType<StringMap>();
}

ConfigItem::ValueType<ConfigItem::StringList> &ConfigItem::stringList()
{
    if (m_extendedType != StringList) {
        allocExtendedData<StringList>();

        auto &array = forceAsType<ParentType<StringList>>();
        for (JsonTreeItem *strItem : qAsConst(array))
            asExtendedType<StringList>().push_back(strItem->value().toString());
        array.clear();
    }

    return asExtendedType<StringList>();
}

ConfigItem::ValueType<ConfigItem::IntList> &ConfigItem::intList()
{
    if (m_extendedType != IntList) {
        allocExtendedData<IntList>();

        auto &array = forceAsType<ParentType<IntList>>();
        for (JsonTreeItem *intItem : qAsConst(array))
            asExtendedType<IntList>().push_back(intItem->value().toInt());
        array.clear();
    }

    return asExtendedType<IntList>();
}

void ConfigItem::finalizeForExport()
{
    switch (m_extendedType) {
    case StringMap: {
        auto &object = forceAsType<ParentType<StringMap>>();
        object.clear();

        const QMap<QString, QString> &stringMap = asExtendedType<StringMap>();
        const QStringList keys = stringMap.keys();
        for (const QString &key : keys) {
            ConfigItem *item = newItem();
            item->setKey(key);
            item->value() = stringMap[key];
            object.push_back(item);
        }
        break;
    }
    case StringList: {
        auto &array = forceAsType<ParentType<StringList>>();
        array.clear();

        const QStringList &stringList = asExtendedType<StringList>();
        for (const QString &str : stringList) {
            ConfigItem *item = newItem();
            item->value() = str;
            array.push_back(item);
        }
        break;
    }
    case IntList: {
        auto &array = forceAsType<ParentType<IntList>>();
        array.clear();

        const QList<int> &intArray = asExtendedType<IntList>();
        for (int i : intArray) {
            ConfigItem *item = newItem();
            item->value() = i;
            array.push_back(item);
        }
        break;
    }
    default:
        break;
    }
}
