#ifndef misc_h
#define misc_h

#include <cstdlib>

#include <mib/io/fmt>
#include <mib/timer>

#include <bpstd/string_view.h>

#include "uuid.h"
#include "typedefs.h"

namespace wsg::misc
{
   struct session
   {
      enum init_mode { delayed, immediate };

      /// Initializes session object
      session();

      ///Initializes session object and start session
      session(init_mode) : m_start(steady_clock::now()) { }

      void start();
      void stop();

      /// Calculates time elapsed, stop() should be called before using this
      template <class ConvertTo = chrono::microseconds>
      size_t time_elapsed();

      /// Calculates time elapsed. Uses its steady_clock::now() - start, no end_timer need to be called
      /// Coverts to specified duration type, chrono::microseconds is default
      template <class ConvertTo = chrono::microseconds>
      size_t stop_w_end() const;

      steady_clock::time_point m_start, m_end;

      const char* id();
      misc::uuid& uuid();
      misc::uuid m_uuid;
   };
}

namespace wsg::misc::conversion
{
   template <typename T, typename T2>
   constexpr T cast(T2&& src)
   {
      return static_cast<T>(src);
   }
}

namespace wsg::misc
{
   template <int16_t N1, int16_t N2>
   bool is_equal(auto(&lhs)[N1], auto(&rhs)[N2])
   {
      bool b = true;

      for (int16_t i = 0; i < N1; ++i)
      {
         if (lhs[i] != rhs[i])
         {
            b = false;
            break;
         }
      }
      return b;
   }

   bool is_equal(concepts::uchar_t lhs, int16_t N1, concepts::uchar_t rhs)
   {
      bool b = true;

      for (int16_t i = 0; i < N1; ++i)
      {
         if (lhs[i] != rhs[i])
         {
            b = false;
            break;
         }
      }
      return b;
   }

   bool is_equal(concepts::uchar_t lhs, int16_t N1, concepts::uchar_t rhs, int16_t N2)
   {
      bool b = true;

      for (int16_t i = 0; i < N1 && i < N2; ++i)
      {
         if (lhs[i] != rhs[i])
         {
            b = false;
            break;
         }
      }
      return b;
   }

   template <typename T>
   constexpr void set_null(T* data, size_t SZ)
   {
      memset(data, 0, SZ);
   }

   template <typename T>
   constexpr void set_null(T* data, size_t b, size_t e)
   {
      for (; b < e; ++b)
         data[b] = '\0';
   }

   template <typename T, size_t...N>
   constexpr void set_null(T(&...data)[N])
   {
      (memset(data, 0, N), ...);
   }

   /// TODO Remove Non standard (GNU) : auto spec in param
   constexpr size_t strlen(const auto *s)
   {
      size_t i = 0;
      for (; s[i]; ++i)
         ;
      return i;
   }

   void tests();
}

#endif//misc_h
