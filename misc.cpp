#include "misc.h"

namespace wsg::misc
{
   session::session() : m_uuid(wsg::misc::make_uuid())
   {
      //
   }

   inline void session::start()
   {
      m_start = steady_clock::now();
   }

   inline void session::stop()
   {
      m_end   = steady_clock::now();
   }

   /// Calculates time elapsed, stop() should be called before using this
   template <class ConvertTo>
   inline size_t session::time_elapsed()
   {
      return duration_cast<ConvertTo>(m_end - m_start).count();
   }

   /// Calculates time elapsed. Uses its steady_clock::now() - start, no end_timer need to be called
   /// Coverts to specified duration type, chrono::microseconds is default
   template <class ConvertTo>
   inline size_t session::stop_w_end() const
   {
      return duration_cast<ConvertTo>(steady_clock::now() - m_start).count();
   }

   const char* session::id() { return m_uuid.to_cstr(); }
   uuid& session::uuid() { return m_uuid; }

}

namespace wsg::misc
{
   void tests()
   {
      char buf[10], data[5];
      for (auto c : data)
         fmt::print("{} ", c);

      fmt::print("\n");

      wsg::misc::set_null(buf, data);
      for (int8_t c : buf)
         fmt::print("{} ", c);

      fmt::print("\n");

      for (int8_t c : data)
         fmt::print("{} ", c);
   }
}
