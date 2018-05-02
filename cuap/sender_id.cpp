
#include "sender_id.h"

using namespace std;

namespace sendid
{
   /// Gets the next id without any checks for available preceeding space
   atomic_u32_t& send_id::next()
   {
      ++*this;
      if (crazy())
         m_index.store(1);

      return m_index;
   }

   atomic_u32_t& send_id::next_reuse()
   {
      if (m_index == SIZE - 1)
      {
         return reset();
      }

      for(uint32_t i = 1; i < SIZE; ++i)
      {
         if (m_ids[i].load() == false) /// not used
         {
            m_index.store(i);
            m_ids[m_index.load()].store(true);
            return m_index;
         }
      }
      fmt::print("Resetting...!\n");
      return reset(); /// No ID is available
   }

   void send_id::release(uint32_t index)
   {
      if (!crazy())
         m_ids[index] = false;
   }

   void send_id::release(const bpstd::string_view& data)
   {
      static uint8_t BASE10 = 10;
      size_t i = data.find_first_of(':');
      bpstd::string_view ids(data.substr(i + 1));
      if (ids.size() > 0)
      {
         char* end;
         i = strtoul(ids, &end, BASE10);
         if (errno != ERANGE)
            m_index.store(i);

         for (; ids.data() != end; i = strtoul(ids.data(), &end, BASE10))
         {
            ids = end;
            if (errno == ERANGE)
            {
               fmt::print("Range error, got: {}\n", i);
               errno = 0;
               continue;
            }

            if(!crazy(i) && m_ids[i])
            {
               m_ids[i].store(false);
               fmt::print("Released: {}\n", i );
            }
         }
      }
   }

   atomic_u32_t& send_id::reset()
   {
      m_index.store(0);
      memset(m_ids, false, SIZE);
      m_ids[0].store(true);
      return m_index;
   }

   send_id& send_id::operator++()
   {
      if (m_index == SIZE - 1)
         reset();
      else
      {
         m_index.fetch_add(1);
         m_ids[m_index.load()].store(true);
      }
      return *this;
   }
}

