#ifndef connector_h
#define connector_h

#include "cuap/pdu/functions.h"
#include "cuap/pdu/session_sa.h"

//#include "sender_id.h"
#include "misc.h"
#include <cstring>

#include <non-std/function_view.h>

#include <asio/signal_set.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/buffer.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <asio/connect.hpp>

//#include <asio/ts/buffer.hpp>
//#include <asio/ts/internet.hpp>

using namespace asio;
using namespace asio::ip;

namespace wsg::net::tcp
{
   namespace concepts = wsg::concepts;
	using socket = ip::tcp::socket;

   class client
   {
      public:
         //client() = default;
         explicit client(io_service& );
         client(io_service&, bpstd::string_view );

         asio::error_code& close() { socket_.close(errc); return errc; }
         asio::error_code& connect_to(bpstd::string_view = "0.0.0.0", bpstd::string_view = "9981");

         asio::error_code& error() { return errc; }

         bool is_open() const { return socket_.is_open(); }

         //size_t recv(char_t, size_t len);
         //size_t send(char_t, size_t len);

         size_t recv(concepts::char_t data, size_t len)
         {
            return socket_.read_some(buffer(data, len), errc);
         }

         size_t send(concepts::char_t data, size_t len)
         {
            return  write(socket_, buffer(data, len), errc);
         }

         io_service& service() { return iosrv; }

         operator bool()
         {
            return !errc;
         }

      private:
         io_service&  iosrv;
         ip::tcp::socket socket_ = ip::tcp::socket{iosrv};
         asio::error_code errc;

   };

   class session
   {
      public:
         using session_ptr = std::unique_ptr<session> &&;

      public:
         session(tcp::socket &&sockets);

         asio::io_service& service() { return socket_.get_io_service(); }
         void start(session_ptr self);

      private:
         void do_read (session_ptr self);
         void do_write(session_ptr self, std::size_t length);

         inline void read_handler(session_ptr, const error_code& ec, size_t len);
         inline void read_handler(session_ptr, const error_code& ec, size_t len,
											 nonstd::function_view<void()>);
         inline void write_handler(session_ptr, const error_code& ec, size_t len);

         tcp::socket socket_;
         static constexpr short max_length = 384; /// Optimised for CUAP
         uint8_t data_[max_length]  = {0};
   };
	
	using client_t  = client;
	using session_t = session;
}

#endif//connector_h
