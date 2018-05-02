#ifndef sender_id_h
#define sender_id_h


#include <atomic>
#include <fstream>
#include <thread>
#include <bpstd/string_view.h>
#include <mib/io_fmt>

using bpstd::string_view;
using namespace std;

namespace sendid
{
   typedef atomic<uint32_t>  atomic_u32_t;

   static constexpr uint32_t SIZE = 1'00'000'0;
   static constexpr uint32_t defaultid = 1;

   constexpr const char SENDER_ID_FILE[] = "/tmp/send_ids";

   static atomic<uint32_t> m_index = {0};
   static atomic_bool  m_ids[SIZE];

   enum class AdvanceMode
   {
      /// Only go for the next available ID, Restart from the beginning when exhausted
      ForwardOnly,
      /// Search from beginning for available IDS, use the first encountered
      ReuseReleased
   };

   [[maybe_unused]] static uint16_t ids_to_return(const bpstd::string_view& data)
   {
      uint16_t i = data.find_first_of(':');
      bpstd::string_view ids(data.substr(i + 2));
      if (ids.size() > 0)
      {
         char* end;
         i = strtoul(ids, &end, 10);
      }
      else
         i = 0;
      return i;
   }

   template <class T>
   void save_to_file(const T& data)
   {
      ofstream outfile(SENDER_ID_FILE, ofstream::app);

      if (outfile)
         outfile << data << '\n';

      else
      {
         fmt::print("File Not Found, Creating...\n");
         ofstream ofs(SENDER_ID_FILE);
         if (ofs)
            outfile << data << '\n';
      }
      outfile.close();
   }

   /**
      Request:
          Firstline - ID: Number of ids needed. ids: 1

      Response
          Firstline - ID. Ids separated by comma
            0x00000001, 0x00000009 OR 1, 9, 2300, 3400, 5678
   */
   /// Sender or Receiver ID, Ranges from 0x00000001 to 0x7FFFFFFF. Default when NOT used: 0xFFFFFFFF
   struct send_id
   {
      send_id& operator++();

      inline bool crazy() { return m_index >= SIZE; }
      inline bool crazy(uint32_t ind) { return ind >= SIZE; }

      const uint32_t& default_id() const { return defaultid; }
      void set_default_id() { m_index.store(defaultid); }

      /// Returns true if m_ids[index] is false, vice versa
      bool available()           { return !m_ids[m_index]; }
      bool available(uint32_t i) { return !m_ids[i]; }
      atomic_u32_t& current()  { return m_index; }
      atomic_u32_t& next();
      atomic_u32_t& next_reuse();
      atomic_u32_t& next(AdvanceMode);

      void release(uint32_t);
      void release(const bpstd::string_view& );
      atomic_u32_t& reset();
      static void save_to_file() { sendid::save_to_file(m_index); }
   };

   [[maybe_unused]] static void usage_thrds()
   {
      static send_id id;
      auto func = [&]
      {
         for (int i = 0; i < 10; ++i)
         {
            id.next_reuse();
            //id.release("1 2 3 4 5 6");
            fmt::print("{}\n", id.current());
         }
      };

      vector<thread> thrds;
      for (uint32_t i = 0; i < 10; ++i)
         thrds.emplace_back(func);

      for (auto& t : thrds)
      {
         t.join();
      }
   }

   [[maybe_unused]] static void main()
   {
      for (uint32_t i = 0; i < 100; ++i)
         usage_thrds();
   }

}

#endif//sender_id_h
