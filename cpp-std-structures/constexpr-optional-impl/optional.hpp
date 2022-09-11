#pragma once

#include <memory>
#include <type_traits>
#include <iostream>


namespace sigmadt {

    template<typename T>
    struct is_literal_type {
        inline static constexpr bool value = std::is_trivially_destructible_v<T>;
    };
    template<typename T>
    constexpr bool is_literal_type_v = is_literal_type<T>::value;


    template<class S, class = void>
    class optional;



    template<class S>
    class optional<S, std::enable_if_t<!is_literal_type_v<S>>>{
    public:
        optional() : is_inside(false) {}

        optional(const S& el) : is_inside(true), content() {
            new(&content) S(el);
        }

        optional(S&& el) : is_inside(true), content() {
            new(&content) S(std::move(el));
        }

        optional(const optional& other) : is_inside(other.is_inside), content()  {
            if (other.is_inside) {
                new(&content) S(*reinterpret_cast<const S*>(&other.content));
            }
        }

        optional(optional&& other) noexcept : is_inside(other.is_inside), content()  {
            if (other.is_inside) {
                new(&content) S(std::move(*reinterpret_cast<S*>(&other.content)));
            }
        }

        ~optional() {
            if (is_inside) {
                reinterpret_cast<const S*>(&content)->~S();
            }
        }

        void reset() {
            if (is_inside) {
                is_inside = false;
                reinterpret_cast<S*>(&content)->~S();
            }
        }

        optional& operator=(const optional& other) {
            if (static_cast<const void*>(&content) == static_cast<const void*>(&other.content)) {
                return *this;
            }

            reset();

            if (other.is_inside) {
                new(&content) S(*reinterpret_cast<const S*>(&other.content));
                is_inside = true;
            }

            return *this;
        }

        optional& operator=(optional&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            reset();

            if (other.is_inside) {
                new(&content) S(std::move(*reinterpret_cast<S*>(&other.content)));
                is_inside = true;
            }

            return *this;
        }

        explicit operator bool() const {
            return is_inside;
        }

        const S& operator*() const {
            return *reinterpret_cast<const S*>(&content);
        }

        S& operator*() {
            return *reinterpret_cast<S*>(&content);
        }

        const S* operator->() const {
            return reinterpret_cast<const S*>(&content);
        }

        S* operator->() {
            return reinterpret_cast<S*>(&content);
        }

        const S& value() const {
            return operator*();
        }

        S& value() {
            return operator*();
        }

        S value_or(const S& el) const {
            return is_inside ? value() : el;
        }

        friend bool operator==(const optional& x, const optional& y) {
            if (!x.is_inside && !y.is_inside) {
                return true;
            }
            else if (!x.is_inside || !y.is_inside) {
                return false;
            }

            return *reinterpret_cast<const S*>(&x.content) == *reinterpret_cast<const S*>(&y.content);
        }

        friend bool operator==(const optional& x, const S& el) {
            if (!x.is_inside) {
                return false;
            }

            return *reinterpret_cast<const S*>(&x.content) == *reinterpret_cast<const S*>(&el);
        }

        friend bool operator==(const S& el, const optional& x) {
            return x == el;
        }

    private:
        bool is_inside;
        std::aligned_storage_t<sizeof(S), alignof(S)> content;


    };
}

namespace sigmadt {

    template<class S>
    class optional<S, std::enable_if_t<is_literal_type_v<S>>> {
    public:
        constexpr optional() : is_inside(false), support()  {}

        constexpr optional(const S& el) : is_inside(true), content(el)  {}
        constexpr optional(S&& el) : is_inside(true), content(std::move(el))  {}

        constexpr optional(const optional& other) : is_inside(other.is_inside), content(other.content)  {}
        constexpr optional(optional&& other) noexcept : is_inside(other.is_inside), content(std::move(other.content)) {}

        constexpr optional& operator=(const optional& other) {
            if (this == &other) {
                return *this;
            }

            is_inside = other.is_inside;
            content = other.content;
            return *this;
        }

        constexpr optional& operator=(optional&& other) noexcept {
            if (this == &other) {
                return *this;
            }

            is_inside = std::move(other.is_inside);
            content = std::move(other.content);
            return *this;
        }

        constexpr optional& operator=(const S& el) {
            if (&content == &el) {
                return *this;
            }

            is_inside = true;
            content = el;
            return *this;
        }

        constexpr optional& operator=(S&& el) noexcept {
            if (&content == &el) {
                return *this;
            }

            is_inside = true;
            content = std::move(el);
            return *this;
        }

        explicit constexpr operator bool() const {
            return is_inside;
        }

        constexpr const S& operator*() const {
            return content;
        }

        constexpr S& operator*() {
            return content;
        }

        constexpr void reset() {
            if (is_inside) {
                is_inside = false;

                reinterpret_cast<S*>(&content)->~S();
            }
        }

        constexpr const S* operator->() const {
            return &content;
        }

        constexpr S* operator->() {
            return &content;
        }

        constexpr const S& value() const {
            return operator*();
        }

        constexpr S& value() {
            return operator*();
        }

        constexpr S value_or(const S& el) const {
            return is_inside ? content : el;
        }

        constexpr friend bool operator==(const optional& x, const optional& y) {
            return x.content == y.content;
        }

        constexpr friend bool operator==(const optional& x, const S& el) {
            return x.content == el;
        }

        constexpr friend bool operator==(const S& el, const optional& x) {
            return x == el;
        }



    private:
        bool is_inside;
        union {
            S content;
            char support;
        };

    };
}

