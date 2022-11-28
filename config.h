#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include "jsontree.h"

namespace ConfigItemData {

// Definiere verfügbare Key-Datenwerte
enum Type {
    None,
    StringMap,  // Spezialtyp von Object, wo alle Werte Strings sind
                // Der Vorteil dieser Differenzierung ist, dass die
                // Werte als Referenz editiert werden können!
    StringList, // Spezialtyp von Array, wo alle Werte Strings sind
                // Der Vorteil der Benutzung ist analog zu StringMap!
    IntList
};

template<Type>
struct TypeTraits;

// Definiere TypeTraits für Datentypen
template<> struct TypeTraits<StringMap> {
    using Type = QMap<QString, QString>;
    static constexpr JsonTreeItem::DataType ParentType = JsonTreeItem::Object;
};

template<> struct TypeTraits<StringList> {
    using Type = QStringList;
    static constexpr JsonTreeItem::DataType ParentType = JsonTreeItem::Array;
};

template<> struct TypeTraits<IntList> {
    using Type = QList<int>;
    static constexpr JsonTreeItem::DataType ParentType = JsonTreeItem::Array;
};

}

// Definiere Klasse ConfigItem
class ConfigItem : public JsonTreeItem
{
public:
    using ExtendedType = ConfigItemData::Type;

    template<ExtendedType _T>
    using ValueType = typename ConfigItemData::TypeTraits<_T>::Type;

    template<ExtendedType _T>
    static constexpr JsonTreeItem::DataType ParentType = ConfigItemData::TypeTraits<_T>::ParentType;

    template<ExtendedType _T>
    using ParentValueType = typename JsonTreeItem::ValueType<ParentType<_T>>;

    // Hier sind die Werte redundant deklariert
    static constexpr ExtendedType None       = ConfigItemData::None;
    static constexpr ExtendedType StringMap  = ConfigItemData::StringMap;
    static constexpr ExtendedType StringList = ConfigItemData::StringList;
    static constexpr ExtendedType IntList    = ConfigItemData::IntList;

    ConfigItem();
    ~ConfigItem() override { clearExtended(); }

    void clearExtended();

    // Lade und/oder manipuliere einfaches Dictionary
    QMap<QString, QString> &stringMap();
    QMap<QString, QString> &stringMap(const QString &key) { return itemAt(key)->stringMap(); }
    QMap<QString, QString> &stringMap(const QString &objPath, const QString &key) { return itemAt(objPath, key)->stringMap(); }

    // Lade und/oder manipuliere QStringList Variable
    QStringList &stringList();
    QStringList &stringList(const QString &key) { return itemAt(key)->stringList(); }
    QStringList &stringList(const QString &objPath, const QString &key) { return itemAt(objPath, key)->stringList(); }

    // Lade und/oder manipuliere Integer Liste
    QList<int> &intList();
    QList<int> &intList(const QString &key) { return itemAt(key)->intList(); }
    QList<int> &intList(const QString &objPath, const QString &key) { return itemAt(objPath, key)->intList(); }

    ConfigItem *objectAt(const QString &objPath) { return static_cast<ConfigItem *>(JsonTreeItem::objectAt(objPath)); }
    const ConfigItem *objectAt(const QString &objPath) const { return static_cast<const ConfigItem *>(JsonTreeItem::objectAt(objPath)); }

    ConfigItem *itemAt(const QString &key) { return static_cast<ConfigItem *>(JsonTreeItem::itemAt(key)); }
    ConfigItem *itemAt(const QString &objPath, const QString &key) { return static_cast<ConfigItem *>(JsonTreeItem::itemAt(objPath, key)); }

protected:
    ConfigItem *newItem() const override { return new ConfigItem(); }

private:
    void *m_extendedData;
    ExtendedType m_extendedType;

    void finalizeForExport() override;

    // Funktionen zum Ansteuern (und Bearbeiten) von Werten im aktuellen Knoten
    // Dass der Typ _T mit m_extendedType übereinstimmt wird beim Aufruf vorausgesetzt!
    template<ExtendedType _T>
    ValueType<_T> &asExtendedType()
    { return *static_cast<ValueType<_T> *>(m_extendedData); }

    template<ExtendedType _T>
    const ValueType<_T> &asExtendedType() const
    { return *static_cast<const ValueType<_T> *>(m_extendedData); }

    // Allokiere Datenfeld mit gewähltem Typ
    template<ExtendedType _T>
    void allocExtendedData()
    {
        if (m_extendedType != None)
            clearExtended();
        m_extendedType = _T;
        m_extendedData = new ValueType<_T>;
    }

    // Lösche Daten mit gewähltem Typ
    // Dass der Typ _T mit m_type übereinstimmt wird beim Aufruf vorausgesetzt!
    template<ExtendedType _T>
    void freeExtendedData()
    { delete static_cast<ValueType<_T> *>(m_extendedData); }
};

#endif // CONFIGITEM_H
