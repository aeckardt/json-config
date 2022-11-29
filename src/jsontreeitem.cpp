#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

#include "jsontreeitem.h"

JsonTreeItem::JsonTreeItem()
    : m_type(None),
      m_data(nullptr)
{
}

JsonTreeItem::~JsonTreeItem()
{
    clear();
}

void JsonTreeItem::loadFromFile(const QString &filename)
{
    if (!QFile::exists(filename))
        return;

    QFile file(filename);
    file.open(QFile::ReadOnly);
    loadFromJson(file.readAll());
    file.close();
}

void JsonTreeItem::saveToFile(const QString &filename)
{
    QFile file(filename);
    file.open(QFile::WriteOnly);
    file.write(saveToJson());
    file.close();
}

void JsonTreeItem::loadFromJson(const QByteArray &json)
{
    reset();

    QJsonDocument doc = QJsonDocument::fromJson(json);

    if (!doc.isObject() && !doc.isArray())
        return;

    if (doc.isObject())
        import(doc.object());
    else if (doc.isArray())
        import(doc.array());
}

QByteArray JsonTreeItem::saveToJson()
{
    if (m_type != Object && m_type != Array)
        return QByteArray();

    QJsonDocument doc;

    if (m_type == Object)
        doc.setObject(exportObject());
    else if (m_type == Array)
        doc.setArray(exportArray());

    return doc.toJson();
}

void JsonTreeItem::appendJson(const QByteArray &json)
{
    QJsonDocument doc = QJsonDocument::fromJson(json);

    if (!doc.isObject() && !doc.isArray())
        return;

    if (doc.isObject())
        append(doc.object());
    else if (doc.isArray())
        append(doc.array());
}

void JsonTreeItem::clear()
{
    switch (m_type)
    {
    case Value:
        // m_data is a pointer to a QVariant object
        freeData<Value>();
        break;
    case Object:
        // Delete all child nodes, before deleting the QVector pointer
        qDeleteAll(asType<Object>());
        // m_data is a pointer to a QVector<ConfigTree *> object
        freeData<Object>();
        break;
    case Array:
        // Delete all child nodes, before deleting the QVector pointer
        qDeleteAll(asType<Array>());
        // m_data is a pointer to a QVector<ConfigTree *> object
        freeData<Array>();
        break;
    default:
        break;
    }

    m_type = None;
    m_data = nullptr;
}

bool JsonTreeItem::contains(const QString &objPath, const QString &key) const
{
    const JsonTreeItem *ct = objectAt(objPath);
    if (!ct)
        return false;
    return ct->find(key) != nullptr;
}

void JsonTreeItem::removeItem(const QString &objPath, const QString &key)
{
    JsonTreeItem *ct = objectAt(objPath);
    JsonTreeItem *ctKey = ct->find(key);
    if (ctKey) {
        ct->asType<Object>().removeOne(ctKey);
        delete ctKey;
    }
}

JsonTreeItem *JsonTreeItem::itemAt(const QString &objPath, const QString &key)
{
    JsonTreeItem *obj = objectAt(objPath);
    JsonTreeItem *item = obj->find(key);
    if (!item) {
        item = newItem();
        item->m_key = key;
        obj->object().push_back(item);
    }
    return item;
}

JsonTreeItem *JsonTreeItem::objectAt(const QString &objPath)
{
    if (m_type != Object)
        reset();

    if (objPath.isEmpty())
        return this;

    QStringList dirs = objPath.split("/", Qt::SkipEmptyParts);
    const QString key0 = dirs[0];
    dirs.removeAt(0);
    QString restPath;
    if (dirs.size() > 0)
        restPath = dirs.join("/");

    return forceKeyAsType<Object>(key0)->objectAt(restPath);
}

const JsonTreeItem *JsonTreeItem::objectAt(const QString &objPath) const
{
    if (m_type != Object)
        return nullptr;

    if (objPath.isEmpty())
        return this;

    QStringList dirs = objPath.split("/", Qt::SkipEmptyParts);
    const QString key0 = dirs[0];
    dirs.removeAt(0);
    QString restPath;
    if (dirs.size() > 0)
        restPath = dirs.join("/");

    const JsonTreeItem *child = find(key0);
    if (!child)
        return nullptr;
    return child->objectAt(restPath);
}

JsonTreeItem *JsonTreeItem::find(const QString &key)
{
    if (m_type != Object)
        return nullptr;

    for (JsonTreeItem *ct : qAsConst(asType<Object>())) {
        if (ct->m_key == key)
            return ct;
    }

    return nullptr;
}

const JsonTreeItem *JsonTreeItem::find(const QString &key) const
{
    if (m_type != Object)
        return nullptr;

    for (const JsonTreeItem *ct : qAsConst(asType<Object>())) {
        if (ct->m_key == key)
            return ct;
    }

    return nullptr;
}

void JsonTreeItem::import(const QJsonObject &obj)
{
    allocData<Object>();

    const QStringList objKeys = obj.keys();
    for (const QString &key : objKeys) {
        JsonTreeItem *ct = newItem();
        ct->m_key = key;
        ct->import(obj[key]);
        asType<Object>().push_back(ct);
    }
}

void JsonTreeItem::import(const QJsonArray &arr)
{
    allocData<Array>();

    for (const QJsonValue &val : arr) {
        JsonTreeItem *ct = newItem();
        ct->m_key = QString();
        ct->import(val);
        asType<Array>().push_back(ct);
    }
}

void JsonTreeItem::import(const QJsonValue &val)
{
    if (val.isString() || val.isDouble() || val.isBool() || val.isNull() || val.isUndefined()) {
        allocData<Value>();
        if (val.isString() || val.isDouble() || val.isBool())
            asType<Value>() = val.toVariant();
    } else {
        if (val.isObject())
            import(val.toObject());
        else if (val.isArray())
            import(val.toArray());
    }
}

void JsonTreeItem::append(const QJsonObject &obj)
{
    if (m_type != Object)
        // If the element alreadys exists with a different type, overwrite it
        allocData<Object>();

    const QStringList objKeys = obj.keys();
    for (const QString &key : objKeys) {
        // Check, if the key already exists
        JsonTreeItem *ct = find(key);
        if (!ct) {
            // If the key doesn't exists, create a new one
            ct = newItem();
            ct->m_key = key;
            asType<Object>().push_back(ct);
        }
        ct->append(obj[key]);
    }
}

void JsonTreeItem::append(const QJsonArray &arr)
{
    if (m_type != Array)
        // If the element alreadys exists with a different type, overwrite it
        allocData<Array>();

    for (const QJsonValue &val : arr) {
        JsonTreeItem *ct = newItem();
        ct->m_key = QString();
        ct->import(val);
        asType<Array>().push_back(ct);
    }
}

void JsonTreeItem::append(const QJsonValue &val)
{
    if (val.isString() || val.isDouble() || val.isBool() || val.isNull() || val.isUndefined()) {
        // Overwrite value, if already exists
        allocData<Value>();
        if (val.isString() || val.isDouble() || val.isBool())
            asType<Value>() = val.toVariant();
    } else {
        if (val.isObject())
            append(val.toObject());
        else if (val.isArray())
            append(val.toArray());
    }
}

QJsonObject JsonTreeItem::exportObject()
{
    finalizeForExport();

    if (m_type != Object)
        return QJsonObject();

    QJsonObject obj;
    for (JsonTreeItem *item : qAsConst(asType<Object>())) {
        switch (item->m_type) {
        case Value:
            obj.insert(item->m_key, item->exportValue());
            break;
        case Object:
            obj.insert(item->m_key, item->exportObject());
            break;
        case Array:
            obj.insert(item->m_key, item->exportArray());
            break;
        default:
            break;
        }
    }

    return obj;
}

QJsonArray JsonTreeItem::exportArray()
{
    finalizeForExport();

    if (m_type != Array)
        return QJsonArray();

    QJsonArray arr;
    for (JsonTreeItem *item : qAsConst(asType<Array>())) {
        switch (item->m_type) {
        case Value:
            arr.append(item->exportValue());
            break;
        case Object:
            arr.append(item->exportObject());
            break;
        case Array:
            arr.append(item->exportArray());
            break;
        default:
            break;
        }
    }

    return arr;
}

QJsonValue JsonTreeItem::exportValue()
{
    finalizeForExport();

    if (m_type != Value)
        return QJsonValue();

    return QJsonValue::fromVariant(asType<Value>());
}
