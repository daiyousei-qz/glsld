#pragma once
#include <vector>
#include <iterator>
#include <algorithm>

template <typename T>
class FlatSet
{
public:
    using VectorType        = std::vector<T>;
    using IteratorType      = typename VectorType::iterator;
    using ConstIteratorType = typename VectorType::const_iterator;

    constexpr FlatSet()
    {
    }
    constexpr FlatSet(std::initializer_list<T> ilist)
    {
        data_.reserve(ilist.size());
        data_.insert(data_.end(), ilist.begin(), ilist.end());
        std::ranges::sort(data_);
    }
    constexpr ~FlatSet()
    {
    }

    constexpr auto Data() -> T*
    {
        return data_.data();
    }
    constexpr auto Data() const -> T*
    {
        return data_.data();
    }
    constexpr auto Size() -> size_t
    {
        return data_.size();
    }

    constexpr auto Insert(const T& x) -> void
    {
        // auto it = std::ranges::lower_bound(data_, x);
        // if (it == data_.end() || *it != x) {
        //     data_.insert(it, x);
        // }
        // TODO: this is a workaround
        auto i_lb = std::lower_bound(Data(), Data() + Size(), x) - Data();
        data_.push_back(x);
        std::rotate(Data() + i_lb, Data() + Size() - 1, Data() + Size());
    }

    constexpr auto Erase(const T& x) -> void
    {
        auto it = std::ranges::find(data_, x);
        if (it != data_.end()) {
            data_.erase(it);
        }
    }

    constexpr auto Clear() -> void
    {
        return data_.clear();
    }

    constexpr auto begin() -> IteratorType
    {
        return data_.begin();
    }
    constexpr auto begin() const -> ConstIteratorType
    {
        return data_.begin();
    }
    constexpr auto end() -> IteratorType
    {
        return data_.end();
    }
    constexpr auto end() const -> ConstIteratorType
    {
        return data_.end();
    }

    template <typename T>
    constexpr auto operator==(const FlatSet<T>& other) const -> bool
    {
        return data_ == other.data_;
    }

    template <typename T>
    constexpr auto operator<=>(const FlatSet<T>& other) const
    {
        return data_ <=> other.data_;
    }

public:
    std::vector<T> data_;
};
