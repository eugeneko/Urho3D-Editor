#pragma once

#include <QString>
#include <QVector>
#include <QPair>

namespace Urho3DEditor
{

/// Ordered map from type name to object.
template <class T>
class TypeMap
{
public:
    /// Iterator.
    using Iterator = typename QVector<QPair<QString, T>>::ConstIterator;

    /// Insert new value.
    bool Insert(const QString& typeName, const T& value)
    {
        if (map_.contains(typeName))
            return false;
        map_.insert(typeName, storage_.size());
        storage_.push_back(qMakePair(typeName, value));
        return true;
    }
    /// Find value by type name.
    const T* Find(const QString& typeName) const
    {
        const int index = map_.value(typeName, -1);
        return index >= 0 && index < storage_.size() ? &storage_[index].second : nullptr;
    }
    /// Get value by type name.
    T Get(const QString& typeName, const T& defaultValue = T()) const
    {
        const T* value = Find(typeName);
        return value ? *value : defaultValue;
    }
    /// Begin iterator.
    Iterator Begin() const { return storage_.cbegin(); }
    /// End iterator.
    Iterator End() const { return storage_.cend(); }

private:
    /// Vector storage.
    QVector<QPair<QString, T>> storage_;
    /// Map.
    QHash<QString, int> map_;
};

template <class T> typename TypeMap<T>::Iterator begin(const TypeMap<T>& map) { return map.Begin(); }

template <class T> typename TypeMap<T>::Iterator end(const TypeMap<T>& map) { return map.End(); }

}

