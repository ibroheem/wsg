#ifndef buffer_h
#define buffer_h

#include <array>
#include <string.h>
#include "nonstd/string_view.h"
#include "typedefs.h"

#ifdef CUAP_DEBUG
   #include "mib/io/fmt"
#endif // CUAP_DEBUG

namespace wsg::misc
{
   /// @file buffer.h
   /// @brief A buffer used for encoding PDUs.
   template <typename T, size_t SIZE>
   class static_buffer
   {
      public:
         //using array_t = misc::array<T, SIZE>;
         using type = wsg::misc::static_buffer<T, SIZE>;

         using iterator        = T*;
         using const_iterator  = const T*;
         using reference       = T&;
         using const_reference = const T&;

         //constexpr static_buffer() = default;
         constexpr static_buffer() = default;
         constexpr static_buffer(const T& val) { fill_n(SIZE, val); }
         constexpr static_buffer(const_iterator b, const_iterator e);
         template <typename T2, int32_t N>
         explicit static_buffer(T2(&arr)[N])
         {
            ///fmt::print("Using Array with.... Size\n");
            for (int32_t i = 0; i < N; ++i)
               memcpy_n_incr(arr[i]);
         }

         /*template <typename T2>
         constexpr static_buffer(const std::initializer_list<T2>& );*/

         template <typename T2>
         constexpr static_buffer(std::initializer_list<T2>&& );

         template <class T2, size_t N>
         constexpr static_buffer(std::array<T2, N>& init) { for (T2 v : init) memcpy_n_incr(v); }

         static_buffer(T&& val) { this->fill(val); }

         template <class T2, size_t N>
         static_buffer& operator=(std::array<T2, N>&&) noexcept;

         template <class T2, size_t N>
         static_buffer& operator=(std::array<T2, N>& );

         template <class T2>
         static_buffer& operator=(std::initializer_list<T2>&& );

         template <class T2>
         static_buffer& operator=(std::initializer_list<T2>& );

         static_buffer& append(const uint8_t val)   { *this += val; return *this; }
         static_buffer& append(const uint16_t val)  { return memcpy_n_incr(val); }
         static_buffer& append(const uint32_t val)  { return memcpy_n_incr(val); }
         //static_buffer& append(const auto val)    { return memcpy_n_incr(val); }
         static_buffer& append_sv(const bpstd::string_view&);

         /** Erases the given range, then copy values in @src to the range in @buffer */
         template <class T2 = std::make_unsigned_t<T>>
         void assign(const T* src, size_t b, size_t e)
         {
            erase(b, e);
            for (size_t i = 0; b < e; ++b, ++i)
               buffer[b] = src[i];
         }

         /// Assigns the value of src to the offset location in buffer
         /// @param \t src: value to be assigned
         /// @param \t _offset: start location in buffer
         void assign(uint8_t src, size_t _offset) { buffer[_offset] = src; }
         void assign(uint16_t src, size_t _offset) { memcpy(&buffer[_offset], &src, sizeof(uint16_t)); }
         void assign(uint32_t src, size_t _offset) { memcpy(&buffer[_offset], &src, sizeof(uint32_t)); }

         /** Erases the given range in buffer, then copy values in @src to the range in @buffer */
         template <class T2 = std::make_unsigned_t<T>>
         void assign_n(T2* src, size_t sz, size_t b, size_t e)
         {
            erase(b, e);
            for (size_t i = 0; ((i < sz) && (b < e)); ++b, ++i)
               buffer[b] = src[i];
         }

         iterator begin() { return &buffer[0]; }
         const_iterator begin()  const  { return begin(); }
         const_iterator cbegin() const  { return begin(); }

         iterator end() { return &buffer[offset]; }
         const_iterator end()  const  { return end(); }
         const_iterator cend() const  { return end(); }

         reference operator[](size_t i) { return buffer[i]; }
         const_reference operator[](size_t i) const { return buffer[i]; }

         void fill(T value) { this->fill_n(SIZE, value); }
         //void fill_n(size_t sz, T value) { std::fill_n(begin(), sz, value); }
         void fill_n(size_t sz, T value) { memset(buffer, value, sz * sizeof(T)); }

         void clear() { fill(0); offset = 0; }
         bool empty() const { return (SIZE == 0); }

         void erase(size_t b, size_t e)
         {
            for (; b < e; ++b) buffer[b] = 0;
         }

         void erase(iterator b, iterator e)
         {
            for (; b < e;)
               *b++ = 0;
         }

         template <class T2, size_t N>
         T2 get(size_t _offset)
         {
            T2 ret = 0;
            memcpy(&ret, &buffer[_offset], N);
            return ret;
         }

         uint16_t get_u16(size_t _offset) { return get<uint16_t, sizeof(uint16_t)>(_offset); }
         uint32_t get_u32(size_t _offset) { return get<uint32_t, sizeof(uint32_t)>(_offset); }

         static_buffer& operator+=(const uint8_t p);
         static_buffer& operator+=(const uint16_t p) { return memcpy_n_incr(p); }
         static_buffer& operator+=(const uint32_t p) { return memcpy_n_incr(p); }
         static_buffer& operator+=(bpstd::string_view& p) {  return append_sv(p); }

         operator T*() { return buffer;}
         operator T*() const { return buffer;}

         static_buffer& add_octet_array(const uint32_t& length, const uint8_t* arr);

         size_t buffer_offset() const { return offset; }
         T* data() { return buffer; }
         T* data() const { return buffer; }

         /// Returns Maximum size of the buffer
         constexpr size_t capacity() const { return SIZE; }
         constexpr size_t max_size() const { return SIZE; }

         /// Returns the number of elements in the buffer
         size_t size()    const { return offset; }

         /// Returns the number of elements in the buffer
         size_t length()  const { return offset; }

         void set_values(std::initializer_list<T> lst) {  for (T v : lst) memcpy_n_incr(static_cast<T>(v)); }

      public:
         size_t offset = 0;
         T buffer[SIZE];
         //uint32_t offset = 0;

      public:
         static_buffer& memcpy_n_incr(const wsg::concepts::char_t val)
         {
            ::memcpy(&buffer[offset], &val, sizeof(val));
            offset += sizeof(val);
            return *this;
         }
   };

   template <typename T, size_t SIZE>
   constexpr static_buffer<T, SIZE>::static_buffer(const_iterator b, const_iterator e)
   {
      for (; b != e;)
         memcpy_n_incr(*b++);
   }

   /*template <typename T, size_t SIZE>
   template <typename T2>
   constexpr static_buffer<T, SIZE>::static_buffer(const std::initializer_list<T2>& init)
   {
   #ifdef CUAP_DEBUG

      fmt::print("Using initializer_list \n");
      ///fmt::print("initializer_list Size: {}\n", sizeof(init));
      ///fmt::print("initializer_list elements: {}\n", init.size());
   #endif // CUAP_DEBUG
      for (T&& v : init)
         memcpy_n_incr(v);
   }*/

   template <typename T, size_t SIZE>
   template <typename T2>
   constexpr static_buffer<T, SIZE>::static_buffer(std::initializer_list<T2>&& init)
   {
   #ifdef CUAP_DEBUG
      fmt::print("Using initializer_list&& \n");
   #endif // CUAP_DEBUG
      for (T&& v : init)
         memcpy_n_incr(v);
   }

   template <typename T, size_t SIZE>
   template <class T2>
   static_buffer<T, SIZE>& static_buffer<T, SIZE>::operator=(std::initializer_list<T2>& init)
   {
      for (T&& v : init)
         memcpy_n_incr(v);

      return *this;
   }

   template <typename T, size_t SIZE>
   template <class T2>
   static_buffer<T, SIZE>& static_buffer<T, SIZE>::operator=(std::initializer_list<T2>&& init)
   {
      for (T&& v : init)
         memcpy_n_incr(v);

      return *this;
   }

   template <typename T, size_t SIZE>
   template <class T2, size_t N>
   static_buffer<T, SIZE>& static_buffer<T, SIZE>::operator=(std::array<T2, N>&& arr) noexcept
   {
#ifdef CUAP_DEBUG
      fmt::print("Array&& Size: {}\n", sizeof(arr));
#endif // CUAP_DEBUG
      for (T&& v : arr)
         memcpy_n_incr(v);

      return *this;
   }

   template <typename T, size_t SIZE>
   template <class T2, size_t N>
   static_buffer<T, SIZE>& static_buffer<T, SIZE>::operator=(std::array<T2, N>& arr)
   {
#ifdef CUAP_DEBUG
      fmt::print("Array Size: {}\n", sizeof(arr));
#endif
      for (T&& v : arr)
         memcpy_n_incr(v);

      return *this;
   }

   template <typename T, size_t SIZE>
   inline static_buffer<T, SIZE>& static_buffer<T, SIZE>::operator+=(const uint8_t p)
   {
      buffer[offset++] = p;
      return *this;
   }

   template <typename T, size_t SIZE>
   inline static_buffer<T, SIZE>& static_buffer<T, SIZE>::append_sv(const bpstd::string_view& p)
   {
      std::copy(p.begin(), p.end(), buffer + offset);
      offset += p.length();
      //buffer[offset - 1] = 0; // Null terminator
      return *this;
   }

   template <typename T, size_t SIZE>
   class static_buffer_char;
}

#endif//buffer_h

