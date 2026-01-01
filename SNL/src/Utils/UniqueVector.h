#pragma once

#include <vector>

namespace snl {
    template <typename T, typename Allocator = std::allocator<T>>
    class UniqueVector : public std::vector<T, Allocator>
    {
        using Base = std::vector<T, Allocator>;

    public:
        using Base::Base; // inherit constructors

        using Base::operator=;

        bool contains(const T& value) const
        {
            return std::find(this->begin(), this->end(), value) != this->end();
        }

        size_t find(const T& value) const
        {
            auto it = std::find(this->begin(), this->end(), value);
            if (it != this->end())
                return std::distance(this->begin(), it);

			throw std::runtime_error("Value not found in UniqueVector");
		}

        void push_back(const T& value)
        {
            if (!contains(value))
                Base::push_back(value);
        }

        void push_back(T&& value)
        {
            if (!contains(value))
                Base::push_back(std::move(value));
        }

        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            T value(std::forward<Args>(args)...);
            if (!contains(value))
                Base::push_back(std::move(value));
        }

        typename Base::iterator insert(typename Base::const_iterator pos,
            const T& value)
        {
            if (contains(value))
                return this->end();

            return Base::insert(pos, value);
        }

        typename Base::iterator insert(typename Base::const_iterator pos,
            T&& value)
        {
            if (contains(value))
                return this->end();

            return Base::insert(pos, std::move(value));
        }
    };
}