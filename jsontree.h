#ifndef JSONTREE_H
#define JSONTREE_H

#include <QString>
#include <QVariant>

class QJsonArray;
class QJsonObject;
class QJsonValue;
class JsonTreeItem;

namespace JsonTreeItemData {

// Definiere verfügbare Key-Datenwerte
enum Type {
    None,
    Value,
    Object,
    Array,
};

template<Type>
struct TypeTraits;

// Definiere Type traits für Datentypen
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

    // Hier sind die Werte redundant deklariert
    // Das ist leider wegen dem aktuellen g++ Compiler notwendig
    static constexpr DataType None   = JsonTreeItemData::None;
    static constexpr DataType Value  = JsonTreeItemData::Value;
    static constexpr DataType Object = JsonTreeItemData::Object;
    static constexpr DataType Array  = JsonTreeItemData::Array;

    JsonTreeItem();
    virtual ~JsonTreeItem();

    // Serialisierung und Deserialiserung über Datei
    void loadFromFile(const QString &filename);
    void saveToFile(const QString &filename);

    // Serialisierung und Deserialisierung direkt über JSON
    void loadFromJson(const QByteArray &json);
    QByteArray saveToJson();

    // Füge JSON zu diesem Baum hinzu
    // Die Strukturen der zwei Bäume werden verschmolzen (merge)!
    void appendJson(const QByteArray &json);

    // Lösche alle Einträge und Kindknoten
    void clear();

    // Lösche alles und setze Typ auf Objekt
    void reset() { allocData<Object>(); }

    bool contains(const QString &key) const { return contains(QString(), key); }
    bool contains(const QString &objPath, const QString &key) const;

    // Lösche Key
    void removeItem(const QString &key) { removeItem(QString(), key); }
    void removeItem(const QString &objPath, const QString &key);

    // Zugriffs-Funktionen
    const QString &key() const { return m_key; }
    void setKey(const QString &key) { m_key = key; }

    DataType type() const { return m_type; }

    bool isNull() const { return m_type == None; }

    template<DataType _T>
    void setType()
    { if (m_type != _T) allocData<_T>(); }

    // Lade und/oder manipuliere QVariant Variable
    QVariant &value() { return forceAsType<Value>(); }
    QVariant &value(const QString &key) { return itemAt(key)->forceAsType<Value>(); }
    QVariant &value(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Value>(); }

    // Lade und/oder manipuliere Array/Object mit Child-Knoten
    QVector<JsonTreeItem *> &array() { return forceAsType<Array>(); }
    QVector<JsonTreeItem *> &array(const QString &key) { return itemAt(key)->forceAsType<Array>(); }
    QVector<JsonTreeItem *> &array(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Array>(); }

    QVector<JsonTreeItem *> &object() { return forceAsType<Object>(); }
    QVector<JsonTreeItem *> &object(const QString &key) { return itemAt(key)->forceAsType<Object>(); }
    QVector<JsonTreeItem *> &object(const QString &objPath, const QString &key) { return itemAt(objPath, key)->forceAsType<Object>(); }

    // Funktion zum rekursiven Ablaufen des ConfigTrees
    // "/" werden als Trennzeichen interpretiert
    // Erzeugt Object in objPath, falls es noch nicht existiert,
    // oder falls es von einem anderen Typ ist (dies gilt nicht für die const Variante)
    JsonTreeItem *objectAt(const QString &objPath);
    const JsonTreeItem *objectAt(const QString &objPath) const;

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

    // Finde Element mit einem bestimmten Key
    JsonTreeItem *find(const QString &key);
    const JsonTreeItem *find(const QString &key) const;

    // Funktionen zum Importieren von JSON-Format
    void import(const QJsonObject &obj);
    void import(const QJsonArray &arr);
    void import(const QJsonValue &val);

    // Funktionen zum Hinzufügen und Verschmelzen von JSON-Format
    void append(const QJsonObject &obj);
    void append(const QJsonArray &arr);
    void append(const QJsonValue &val);

    // Funktionen zum Exportieren in JSON-Format
    QJsonObject exportObject();
    QJsonArray exportArray();
    QJsonValue exportValue();

    // Funktionen zum Ansteuern (und Bearbeiten) von Werten im aktuellen Knoten
    // Dass der Typ _T mit m_type übereinstimmt wird beim Aufruf vorausgesetzt!
    template<DataType _T>
    ValueType<_T> &asType()
    { return *static_cast<ValueType<_T> *>(m_data); }

    template<DataType _T>
    const ValueType<_T> &asType() const
    { return *static_cast<const ValueType<_T> *>(m_data); }

    // Allokiere data-Feld mit gewähltem Typ
    template<DataType _T>
    void allocData()
    {
        if (m_type != None)
            clear();
        m_type = _T;
        m_data = new ValueType<_T>;
    }

    // Lösche Daten mit gewähltem Typ
    // Dass der Typ _T mit m_type übereinstimmt wird beim Aufruf vorausgesetzt!
    template<DataType _T>
    void freeData()
    { delete static_cast<ValueType<_T> *>(m_data); }

    // Finde Element mit einem bestimmten Key und Typ und erzeuge dieses,
    // wenn nicht mit gewünschtem Typ vorhanden
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

#endif // JSONTREE_H
