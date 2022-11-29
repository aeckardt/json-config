#ifndef JSONTREEITEM_H
#define JSONTREEITEM_H

#include <QString>
#include <QVariant>

class QJsonArray;
class QJsonObject;
class QJsonValue;
class JsonTreeItem;

namespace JsonTreeItemData {

// Define available key datatypes
enum Type {
    None,
    Value,
    Object,
    Array,
};

template<Type>
struct TypeTraits;

// Define type traits for datatypes
template<> struct TypeTraits<Value>  { using Type = QVariant; };
template<> struct TypeTraits<Object> { using Type = QVector<JsonTreeItem *>; };
template<> struct TypeTraits<Array>  { using Type = QVector<JsonTreeItem *>; };

}

class JsonTreeItem
{
public:
    using DataType = JsonTreeItemData::Type;

    template<DataType _T>
    using ValueType = typename JsonTreeItemData::TypeTraits<_T>::Type;

    static constexpr DataType None   = JsonTreeItemData::None;
    static constexpr DataType Value  = JsonTreeItemData::Value;
    static constexpr DataType Object = JsonTreeItemData::Object;
    static constexpr DataType Array  = JsonTreeItemData::Array;

    JsonTreeItem();
    virtual ~JsonTreeItem();

    // Serialization and deserialization to a file
    void loadFromFile(const QString &filename);
    void saveToFile(const QString &filename);

    // Serialization and deserializiation to a JSON byte array
    void loadFromJson(const QByteArray &json);
    QByteArray saveToJson();

    // Append the structure in the byte array to the current tree
    // The structures of the two trees are being merged!
    void appendJson(const QByteArray &json);

    // Delete all nodes recursively
    void clear();

    // Delete everything and reset type to Object
    void reset() { allocData<Object>(); }

    bool contains(const QString &key) const { return contains(QString(), key); }
    bool contains(const QString &objPath, const QString &key) const;

    // Remove key from tree
    void removeItem(const QString &key) { removeItem(QString(), key); }
    void removeItem(const QString &objPath, const QString &key);

    // Access functions
    const QString &key() const { return m_key; }
    void setKey(const QString &key) { m_key = key; }

    DataType type() const { return m_type; }

    bool isNull() const { return m_type == None; }

    template<DataType _T>
    void setType()
    { if (m_type != _T) allocData<_T>(); }

    // Load and / or manipulate QVariant variable
    QVariant &value() { return forceAsType<Value>(); }
    QVariant &value(const QString &key) { return itemAt(key)->forceAsType<Value>(); }
    QVariant &value(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Value>(); }

    // Load and / or manipulate Array/Object with child nodes
    QVector<JsonTreeItem *> &array() { return forceAsType<Array>(); }
    QVector<JsonTreeItem *> &array(const QString &key) { return itemAt(key)->forceAsType<Array>(); }
    QVector<JsonTreeItem *> &array(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Array>(); }

    QVector<JsonTreeItem *> &object() { return forceAsType<Object>(); }
    QVector<JsonTreeItem *> &object(const QString &key) { return itemAt(key)->forceAsType<Object>(); }
    QVector<JsonTreeItem *> &object(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Object>(); }

    // The recursive function objectAt walks the nodes in the tree to find / or create an object with the
    // given path, while "/" is interpreted as a separator.
    // If an object does not exist with the specified path, one is created. That applies even if
    // a key of another type exists at the specified path (except if you call the const function).
    // The const function does not throw an error, instead you get a nullptr if the path could not be found.
    JsonTreeItem *objectAt(const QString &objPath);
    const JsonTreeItem *objectAt(const QString &objPath) const;

    // The recursive function itemAt walks the nodes and returns whatever tree node is at the specified path.
    // If an object path is specified, which does not exist, it is created.
    JsonTreeItem *itemAt(const QString &key) { return itemAt(QString(), key); }
    JsonTreeItem *itemAt(const QString &objPath, const QString &key);

protected:
    virtual JsonTreeItem *newItem() const { return new JsonTreeItem; }

    virtual void finalizeForExport() {}

    template<DataType _T>
    ValueType<_T> &forceAsType()
    {
        if (m_type != _T)
            allocData<_T>();
        return asType<_T>();
    }

private:
    QString m_key;
    DataType m_type;
    void *m_data;

    // Find element with a specified key
    JsonTreeItem *find(const QString &key);
    const JsonTreeItem *find(const QString &key) const;

    // Functions for importing from JSON
    void import(const QJsonObject &obj);
    void import(const QJsonArray &arr);
    void import(const QJsonValue &val);

    // Functions for appending and merging with JSON-format
    void append(const QJsonObject &obj);
    void append(const QJsonArray &arr);
    void append(const QJsonValue &val);

    // Functions for exporting to JSON-format
    QJsonObject exportObject();
    QJsonArray exportArray();
    QJsonValue exportValue();

    // Function for control of values in the current node
    // The template parameter _T must be the current DataType! Otherwise the program might crash
    template<DataType _T>
    ValueType<_T> &asType()
    { return *static_cast<ValueType<_T> *>(m_data); }

    template<DataType _T>
    const ValueType<_T> &asType() const
    { return *static_cast<const ValueType<_T> *>(m_data); }

    // Allocate datafield with specified type
    template<DataType _T>
    void allocData()
    {
        if (m_type != None)
            clear();
        m_type = _T;
        m_data = new ValueType<_T>;
    }

    // Delete data of specified type
    // The template parameter _T must be the current DataType! Otherwise the program might crash
    template<DataType _T>
    void freeData()
    { delete static_cast<ValueType<_T> *>(m_data); }

    // Find item with a specific key and type and create the item, if it is not available with the
    // desired type
    template<DataType _T>
    JsonTreeItem *forceKeyAsType(const QString &key)
    {
        JsonTreeItem *ct = find(key);
        if (!ct) {
            ct = newItem();
            ct->m_key = key;
            ct->allocData<_T>();
            asType<Object>().push_back(ct);
        } else if (ct->m_type != _T)
            ct->allocData<_T>();
        return ct;
    }
};

#endif // JSONTREEITEM_H
