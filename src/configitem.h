#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include "jsontreeitem.h"

namespace ConfigItemData {

// Define available key datatypes
enum Type {
    None,
    StringMap,  
    StringList, 
    IntList
};

template<Type>
struct TypeTraits;

// Define type traits for data types
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

    // Load and / or manipulate string to string dictionary
    QMap<QString, QString> &stringMap();
    QMap<QString, QString> &stringMap(const QString &key) { return itemAt(key)->stringMap(); }
    QMap<QString, QString> &stringMap(const QString &objPath, const QString &key) { return itemAt(objPath, key)->stringMap(); }

    // Load and / or manipulate string list
    QStringList &stringList();
    QStringList &stringList(const QString &key) { return itemAt(key)->stringList(); }
    QStringList &stringList(const QString &objPath, const QString &key) { return itemAt(objPath, key)->stringList(); }

    // Load and / or manipulate int list
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

    template<ExtendedType _T>
    ValueType<_T> &asExtendedType()
    { return *static_cast<ValueType<_T> *>(m_extendedData); }

    template<ExtendedType _T>
    const ValueType<_T> &asExtendedType() const
    { return *static_cast<const ValueType<_T> *>(m_extendedData); }

    // Allocate datafield with selected type
    template<ExtendedType _T>
    void allocExtendedData()
    {
        if (m_extendedType != None)
            clearExtended();
        m_extendedType = _T;
        m_extendedData = new ValueType<_T>;
    }

    // Delete data with selected type
    template<ExtendedType _T>
    void freeExtendedData()
    { delete static_cast<ValueType<_T> *>(m_extendedData); }
};

#endif // CONFIGITEM_H
