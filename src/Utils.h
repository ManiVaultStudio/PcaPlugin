#ifndef UTILS_H
#define UTILS_H

#include <chrono>       // high_resolution_clock, milliseconds
#include <iterator>     // std::forward_iterator_tag
#include <cstddef>      // std::ptrdiff_t
#include <type_traits>
#include <typeinfo>
#include <iostream>

namespace utils {

    /// ////// ///
    /// TIMING ///
    /// ////// ///

    // Call like:
    /*
    utils::timer([&]() {
         <CODE YOU WANT TO TIME>
        },
        "<DESCRIPTION>");
    */
    template <typename F>
    void timer(F myFunc, std::string name) {
        using clock = std::chrono::high_resolution_clock;
        const auto time_start = clock::now();
        myFunc();
        std::cout << "Timing " << name << ": " << std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - time_start).count() << "ms" << std::endl;
    }

    /// /////// ///
    /// Looping ///
    /// /////// ///

    // Motivated by 
    // https://github.com/klmr/cpp11-range
    // https://isocpp.org/blog/2020/12/writing-a-custom-iterator-in-modern-cpp
    // cpp11-range does not define a forward_iterator but only a input_iterator
    // and can therefor not be used with std::execution in e.g. std::for_each
    // Other resources: https://github.com/VinGarcia/Simple-Iterator-Template
    //
    // Is pyrange iterator is to be used with c++17
    // With c++20 you might as well use: for (int num : std::views::iota(START_NUM, END_NUM))

    template <typename T>
    struct pyrange {
        static_assert(std::is_integral<T>::value, "Integral type required.");

        // see https://en.cppreference.com/w/cpp/iterator/iterator_traits
        struct pyrange_iter
        {
            using iterator_category = std::forward_iterator_tag;    // c++17 style, c++20 would use std::forward_iterator
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = value_type*;
            using reference = value_type&;

            pyrange_iter() : _val(0) {}
            pyrange_iter(value_type val) : _val(val) {}

            reference operator*() { return _val; }
            pointer operator->() { return &_val; }

            pyrange_iter& operator++() { _val++; return *this; }                                // prefix increment
            pyrange_iter  operator++(int) { pyrange_iter tmp = *this; ++(*this); return tmp; }  // postix increment

            friend bool operator== (const pyrange_iter& a, const pyrange_iter& b) { return a._val == b._val; };
            friend bool operator!= (const pyrange_iter& a, const pyrange_iter& b) { return a._val != b._val; };

        private:
            value_type _val;
        };

        pyrange(T end) : _begin(0), _end(end) { }
        // if end < begin, don't throw an exception but instead don't iterate at all
        pyrange(T begin, T end) : _begin(begin), _end(end) { if (end < begin) _end = _begin; }

        pyrange_iter begin() { return _begin; }
        pyrange_iter end() { return _end; }

        const char* typeinfo() const { return typeid(T).name(); }

    private:
        pyrange_iter _begin;
        pyrange_iter _end;

    };
 
}

#endif UTILS_H