/*/
 *  A ready-made way to iterate over *dynamic C arrays* in c++98 -->
 * (There is no support in bog-standard STL for this, unless the
 * size of the array is known at compile-time (in which case it is not
 * a dynamic array)).
 *
 * You can iterate forward or backwards over the array, just as if it
 * were, say, a vector. My use-case does not require const_iterator, but if
 * you want to add it, a quick read of the class guts should give you
 * many hints on how to do it.
 *
 * Example use (in practice the array will probably come from a C library)
 * Iteration is via the 'ptrs' class which implements the iterators.
 *
 *
    short* short_array* = new short[256];
    // <-- you don't need to know this size @ compile time -->
    typedef myptrs<short> shorts; shorts::iterator it = shorts.begin();
    shorts::iterator it = shorts.end();
    while (it < e){
        cout << *it++ << endl;
    }
    For more example code, see the test() code at the bottom of this file.

/*/
#pragma once

#ifndef MY_ITERATOR_H
#define MY_ITERATOR_H

#include <cstddef> // ptrdiff_t. Should be in namespace std, but not in MSVC98! (it's in the global namespace)
#include <cstring> // size_t
#include <iterator> // iterator, reverse_iterator

#include <cstdio>

#ifdef _DEBUG
#include <iostream> // cout & friends.
#include <cassert> // assert().
#include <numeric> // std::accumulate.
#include <algorithm> // std::reverse
#include <vector>
#endif

namespace my {

template <bool B> class CompileTimeAssert {};
template <> class CompileTimeAssert<true> {
    public:
    static inline void Check() {}
};

namespace iterator {
    template <typename T> struct ptrs {
        /*/
         * Liberal sprinkling of 'inline' as some old compilers
         * (and this is for c++98 after all!) really need it.
         * Source: Stepanov during Amazon lectures on YouTube,
         * 'Efficient use of components'.
        /*/
        inline ptrs(T* beg, T* end) : m_beg(beg), m_end(end) {}
        inline ptrs(T* beg, size_t sz) : m_beg(beg) { m_end = beg + sz; }

        struct iterator {

            typedef std::random_access_iterator_tag iterator_category;
            typedef T value_type;
            typedef int difference_type;
            typedef T* pointer;
            typedef T& reference;
            typedef const T* const_pointer;
            typedef const T& const_reference;
            // in MSVC98, ptrdiff_t is not in std namespace? WTF?

            inline iterator() : m_start(), m_end(), m_cur() {}
            inline iterator(T* b, T* e) : m_start(b), m_end(e), m_cur(b) {}
            inline iterator(T* b, T* e, T* cur) : m_start(b), m_end(e), m_cur(cur) {}

            inline iterator(const iterator& rhs)
                : m_start(rhs.m_start), m_end(rhs.m_end), m_cur(rhs.m_cur) {}

            iterator(const T* x) : m_cur(x) {}
            iterator& operator=(const T* x) {
                m_cur = x;
                return *this;
            }
            inline iterator& operator++() {
                m_cur++;
                return *this;
            }

            inline iterator operator++(int) const {
                iterator old = *this;
                m_cur++;
                return old;
            }
            inline iterator& operator--() {
                m_cur--;
                return *this;
            }
            inline iterator operator--(int) {
                iterator old = *this;
                m_cur--;
                return old;
            }
			inline iterator& operator+=(int n){
				
				m_cur+=n;
				return *this;
			}
			inline iterator& operator-=(int n){
				
				m_cur-=n;
				return *this;
			}



            // operator T&() const { return m_cur; }
            operator T*() const { return m_cur; }
            operator T*() { return m_cur; }

            T* operator->() { return m_cur; }

            iterator operator-(difference_type n) const {

                iterator Tmp = *this;
                return (Tmp -= n);
            }

            iterator& operator-=(const iterator& rhs) {
                m_cur -= rhs.m_cur;
                return *this; // return the result by reference
            }

            iterator& operator-=(const difference_type& rhs) {
                m_cur -= rhs;
                return *this; // return the result by reference
            }

            inline bool operator==(const iterator& rhs) const {
                return m_cur == rhs.m_cur;
            }

            inline bool operator!=(const iterator& rhs){
               return (m_cur != rhs.m_cur);
			}


            inline bool operator<(const iterator& rhs) const {
                return m_cur < rhs.m_cur;
            };

            inline reference end() { return m_end; }
            inline reference begin() { return m_start; }
#ifdef _MSC_VER

            typedef std::reverse_iterator<iterator, value_type> reverse_iterator;

#else
            typedef std::reverse_iterator<iterator> reverse_iterator;
#endif
            inline reverse_iterator rbegin() const { return reverse_iterator(end()); }
            inline reverse_iterator rend() const { return reverse_iterator(begin()); }

            private:
            mutable T* m_start;
            mutable T* m_end;
            mutable T* m_cur;
        };

        typedef int difference_type;
#ifdef _MSC_VER
        typedef std::reverse_iterator<iterator, T> reverse_iterator;
#else
        typedef std::reverse_iterator<iterator> reverse_iterator;
#endif
        typedef T value_type;
        typedef T* pointer;
        typedef T& reference;
        typedef const T* const_pointer;
        typedef const T& const_reference;

        inline reverse_iterator rbegin() const { return reverse_iterator(end()); }
        inline reverse_iterator rend() const { return reverse_iterator(begin()); }

        inline iterator begin() const { return iterator(m_beg, m_end); }
        inline iterator end() const { return iterator(m_beg, m_end, m_end); }
        inline iterator begin() { return iterator(m_beg, m_end); }
        inline iterator end() { return iterator(m_beg, m_end, m_end); }

        inline const ptrs& operator*() const { return *this; }
        inline ptrs& operator*() { return *this; }

        inline size_t size() const { return static_cast<size_t>(m_end - m_beg); }
        inline int isize() const { return static_cast<int>(m_end - m_beg); }
        inline typename iterator::difference_type ptrdiff_size() const {
            return std::distance(m_end, m_beg);
        }
        T operator[](size_t i) const { return *(m_beg + i); }
        T& operator[](size_t i) { return *(m_beg + i); }

        // ptrs& operator+=(const ptrs& rhs) {
        /* addition of rhs to *this takes place here */
        //   m_cur += rhs.m_cur;
        //  return *this; // return the result by reference
        //}

        // ptrs& operator-=(const ptrs& rhs) {
        /* addition of rhs to *this takes place here */
        //   m_cur -= rhs.m_cur;
        //  return *this; // return the result by reference
        // }

        private:
        mutable T* m_beg;
        mutable T* m_end;
    };

#ifdef _DEBUG
    namespace test {
        using namespace std;
        typedef ptrs<short> ptrs_t;

        static void iterate_normally(ptrs_t& myptrs) {
            ptrs_t::iterator it = myptrs.begin();
            const ptrs_t::iterator e = myptrs.end();
            while (it < e) {
                cout << *it << endl;
                ++it;
            }
            --it;
        }

        static void iterate_backwards(ptrs_t& myptrs) {
            ptrs_t::reverse_iterator it = myptrs.rbegin();
            const ptrs_t::reverse_iterator e = myptrs.rend();
            while (it < e) {
                cout << *it << endl;
                ++it;
            }
            --it;
        }

        static inline void run(size_t ar_size = 10) {

            if (ar_size < 10) ar_size = 10;
            short* short_array = new short[ar_size];
            short* p = short_array;
            short* paend = p + ar_size;
            short x = 0;
            while (p < paend) {
                *p++ = x++;
            };

            p = short_array++;
            assert(*p == 0);
            assert(*(++p) == 1);
            assert(*(--paend) == 9);
            short_array--;
            ptrs_t myptrs(short_array, ar_size);
            assert(myptrs.size() == ar_size);

            ptrs_t::iterator myit = myptrs.begin();
            assert(*myit == 0);
            ptrs_t::reverse_iterator rit = myptrs.rbegin();
            short rtiv = *rit;
            assert(rtiv == 9);
            cout << "reverse iterator output: " << endl << endl;
            iterate_backwards(myptrs);

            cout << "reverse_iterator complete." << endl << endl;

            const int sd1 = myptrs.end() - myptrs.begin();
            const int sd2 = myptrs.rend() - myptrs.rbegin();
            cout << "size diff is:       " << sd1 << endl;
            cout << "size diff (rev) is: " << sd2 << endl;
            assert(sd1 == sd2);
            cout << "normal iterator output: " << endl;
            iterate_normally(myptrs);
            cout << "normal iterator complete." << endl << endl;

            cout << "Modifyfing contents (backwards) ... " << endl;
            rit = myptrs.rbegin();
            ptrs_t::reverse_iterator re = myptrs.rend();
            int c = 0;

            while (rit < re) {
                *rit = c++;
                ++rit;
            }

            cout << "Modifyfing contents (backwards) complete." << endl;

            cout << "Testing access, forwards, by index ... " << endl;
            int item = myptrs[myptrs.size() - 1];
            assert(item == 0 && "array reversed failed.");
            int pos = myptrs.size() - 1;
            int expected = 9;
            int asize = myptrs.isize();
            pos = 0;

            while (pos < asize) {
                int val = myptrs[pos];
                assert(val == expected--);
                cout << "value " << val << "   at index " << pos << endl;
                ++pos;
            }

            cout << "Modifying (backwards)iterator complete." << endl;
            cout << "Iterating normally ... " << endl;

            iterate_normally(myptrs);

            cout << "Modifyfing contents (forwards) ... " << endl;
            ptrs_t::iterator it = myptrs.begin();
            ptrs_t::iterator e = myptrs.end();
            c = 1000;
            while (it < e) {
                *it = c++;
                ++it;
            }

            cout << "Displaying modified contents ..." << endl;
            iterate_normally(myptrs);
            cout << endl << endl;

            cout << "Using STL to reverse() the array ... " << endl;
            ptrs_t::reverse_iterator tit = myptrs.rbegin();
            ptrs_t::reverse_iterator eit = myptrs.rend();

            std::reverse(tit, eit);
            cout << "reverse values are ..." << endl;
            iterate_normally(myptrs);
            int what = myptrs[0];
            assert(what == 1009);

            int what2 = *(myptrs.end() - 1);
            assert(what2 == 1000);
            cout << "std::reverse completed." << endl;
            delete[] short_array;
            cout << endl;
            cout << "--------------- my::iterator ---------------" << endl;
            cout << "        Tests complete. All passed!" << endl;
            cout << "--------------- my::iterator ---------------" << endl;
#ifdef _MSC_VER 
			::OutputDebugStringA("****** iterator tests all passed *****************\n");
#endif
            cout << endl;
        }

    } // namespace test
#endif
} // namespace iterator
} // namespace my

#endif // MY_ITERATOR_H
