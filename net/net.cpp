#include "cuap/sender_id.h"
#include "cuap/packet_samples.h"
#include "net.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

using namespace asio;
using namespace asio::ip;
using namespace rapidjson;
using namespace std;

using namespace wsg::cuap;
using namespace wsg::cuap::pdu;
using namespace wsg::concepts;

namespace wsg::net::tcp
{
   client::client(io_service& io) : iosrv(io)
   {
   }

   client::client(io_service& io, bpstd::string_view /*addr*/) : iosrv(io)
   {

   }

   asio::error_code& client::connect_to(bpstd::string_view addr,
         bpstd::string_view port)
   {
      ip::tcp::resolver resolver(iosrv);
      asio::connect(socket_, resolver.resolve({addr.data(), port.data()}), errc);
      return errc;
   }

}

namespace wsg::cuap
{
   namespace verifiers
   {
      constexpr bool bind_response(bind_resp& lhs, bind_resp& rhs)
      {
         using namespace cuap::header;
         using namespace cuap::body::bind_msg;

         char sysid[11] {};
         system_id(rhs, sysid);

         bool ret =  (command_len(lhs) == command_len(rhs) &&
                      command_id(lhs)  == command_id(rhs) &&
                      command_status(lhs) == command_status(rhs) &&
                      sender_id(lhs) == sender_id(rhs) &&
                      receiver_id(lhs) == receiver_id(rhs) &&
                      misc::is_equal(sysid, "test"));

         memset(sysid, 0x0, 11);
         system_id(lhs, sysid);
         return (ret && misc::is_equal(sysid, "test"));
      }

      constexpr int16_t expected_size(CommandIDs command_ids)
      {
         int16_t ret = 0;
         switch (command_ids)
         {
            case CommandIDs::Bind:
            {
               ret =  256;
            }
            break;

            case CommandIDs::Shake:
            {
               ret = 256;
            }
            break;

            /*case 3:
            {

            } break;*/

            default:
               break;
         }
         return ret;
      }

   }
}

namespace wsg::cuap::login
{
   using namespace net;
   using net::tcp::client_t;
   using net::tcp::session_t;

   struct login_t
   {
         bind_msg  login_msg;
         bind_resp msg_hdr;
         uint16_t  command_len = 20;
   };

   void build_bind_msg(login_t& lgn)
   {
      lgn.login_msg.set_command_id(htobe32_mod((uint8_t)CommandIDs::Bind));
      lgn.login_msg.set_command_status(0);
      lgn.login_msg.set_sender_id(htobe32_mod(OxFFFFFFFF));
      lgn.login_msg.set_receiver_id(htobe32_mod(OxFFFFFFFF));

      uint8_t id []   = "test";
      uint8_t pass [] = "test";
      uint8_t type [] = "USSD";

      lgn.login_msg.set_system_id(id, sizeof(id));
      lgn.login_msg.set_password(pass, sizeof(pass));
      lgn.login_msg.set_system_type(type, sizeof(type));
      lgn.login_msg.set_interface_ver(htobe32_mod(0x00000010));

      lgn.command_len += sizeof(id) + sizeof(pass) + sizeof(type) + 4; /// All Body

      lgn.login_msg.set_command_len(htobe32_mod(lgn.command_len));
   }

   bool bind_with(login_t& lgn, client_t& client)
   {
      auto st = client.send(lgn.login_msg.data(), lgn.command_len);
      if (st > 0)
      {
         lgn.msg_hdr.clear();
         st = client.recv(lgn.msg_hdr.data(), lgn.msg_hdr.max_size());
         if (st > 0)
            return lgn.msg_hdr.is_BindResp();
      }
      return false;
   }

   void header_builder(login_t& lgn, CommandIDs cmd_id)
   {
      lgn.msg_hdr.clear();

      lgn.command_len = 20;
      lgn.msg_hdr.set_command_len(htobe32_mod(lgn.command_len));
      lgn.msg_hdr.set_command_id(htobe32_mod((uint8_t)cmd_id));
      lgn.msg_hdr.set_command_status(0);
      lgn.msg_hdr.set_sender_id(htobe32_mod(OxFFFFFFFF));
      lgn.msg_hdr.set_receiver_id(htobe32_mod(OxFFFFFFFF));
   }

   bool transmit_header(login_t& lgn, client_t& sock)
   {
      auto st = sock.send(lgn.msg_hdr.data(), lgn.command_len);
      if (st > 0)
      {
         lgn.msg_hdr.clear();
         st = sock.recv(lgn.msg_hdr.data(), lgn.msg_hdr.max_size());
         if (st > 0)
            return lgn.msg_hdr.is_ShakeResp() || lgn.msg_hdr.is_UnBindResp();
      }
      return false;
   }

   bool shake_with(login_t& lgn, client_t& sock)
   {
      header_builder(lgn, CommandIDs::Shake);
      return transmit_header(lgn, sock);
   }

   bool unbind_with(login_t& lgn, client_t& sock)
   {
      header_builder(lgn, CommandIDs::UnBind);
      return transmit_header(lgn, sock);
   }

   bool exec(client_t& cl)
   {
      login_t lgn;
      build_bind_msg(lgn);

      bool ret;
      if (ret = bind_with(lgn, cl); ret)
      {
         fmt::print_cyan("[Read Handler]: Binded\n");
         if (ret = shake_with(lgn, cl); ret)
         {
            fmt::print_cyan("[Read Handler]: Shake Successful\n");
         }
         else
         {
            fmt::print_red("[Read Handler]: Shake UnSuccessful\n");
         }
      }
      else
      {
         fmt::print_red("[Read Handler]: Unable to Bind!\n");
      }

      return ret;
   }
}

namespace wsg::cuap::dispatcher
{
   using net::tcp::client_t;

   struct dispatcher_t
   {
      Document doc;
      session  sess;
      uint8_t  cuap_msg[384] = {0};
   };

   uint8_t command_id_from(const char* msg)
   {
      if (misc::is_equal("begin", 5, msg))
         return (uint8_t)CommandIDs::Begin;

      else if (misc::is_equal("continue", 8, msg))
         return (uint8_t)CommandIDs::Contniue;

      else if (misc::is_equal("end", 3, msg))
         return (uint8_t)CommandIDs::End;

      else if (misc::is_equal("abort", 5, msg))
         return (uint8_t)CommandIDs::Abort;

      return (uint8_t)0xFE; /// Error Occured
   }

   uint32_t get_sender_id_old()
   {
      uint32_t ret = 0;

      static io_service iosvc;
      static net::tcp::client sck(iosvc);

      auto fn = [&]() -> uint32_t
      {
         sck.send("ids: 1", 6);
         sck.recv(&ret, 4);
         return ret;
      };

      if (!sck.is_open())
      {
         sck.connect_to("0.0.0.0", "9982");
         return fn();
      }
      else
      {
        return fn();
      }

      fmt::print("[Dispatcher][Diagnostics]: {}\n", sck.error().message());

      return 0;
   }

   uint32_t make_sender_id()
   {
      static sendid::send_id sids;
      return sids.next_reuse().load();
   }

   bool release_sender_id()
   {
      return false;
   }

   cchar* get_session_str(uchar_t recv_buf)
   {
      using namespace cuap;

      uint32_t cmd_id = header::command_id(recv_buf);
      cmd_id = be32toh(cmd_id);

      switch (cmd_id)
      {
         case static_cast<uint8_t>(CommandIDs::Abort):
            return "abort";
            break;

         case static_cast<uint8_t>(CommandIDs::Begin):
            return "begin";
            break;

         case static_cast<uint8_t>(CommandIDs::End):
            return "end";
            break;

         case static_cast<uint8_t>(CommandIDs::Contniue):
            return "contniue";
            break;

         default:
            //
            break;
      }
      return "";
   }

   bool is_begin_family(uint8_t cmd_id)
   {
      return ((cmd_id == (uint8_t)CommandIDs::Begin)    ||
              (cmd_id == (uint8_t)CommandIDs::Contniue) ||
              (cmd_id == (uint8_t)CommandIDs::End)      ||
              (cmd_id == (uint8_t)CommandIDs::Abort));
   }

   constexpr bool is_cuap_msg(uchar_t msg)
   {
      return cuap::header::is_cuap_msg(msg);
   }

   bool is_json_msg(dispatcher_t& dsp)
   {
      return dsp.doc["msisdn"].IsString() &&
             dsp.doc["op_type"].IsString() &&
             dsp.doc["session"].IsString() &&
             dsp.doc["session-id"].IsString() &&
             dsp.doc["ussd_code"].IsString() &&
             dsp.doc["message"].IsString();
   }

   bool build_cuap(dispatcher_t& dsp, concepts::uchar_t incomming)
   {
      using namespace cuap::header;
      using namespace cuap::body::begin_msg;

      //doc.Clear();

      if(ParseResult ok = dsp.doc.Parse<kParseCommentsFlag, UTF8<uint8_t>>(incomming);
            ok && is_json_msg(dsp))
      {
         memset(dsp.cuap_msg, 0x00, sizeof(dsp.cuap_msg));

         uint8_t cmd_id = command_id_from(dsp.doc["session"].GetString());

         set_command_id(dsp.cuap_msg,     htobe32_mod(cmd_id));
         set_command_status(dsp.cuap_msg, htobe32_mod(Ox00000000));
         set_sender_id(dsp.cuap_msg,      htobe32_mod(make_sender_id()));
         set_sender_id(dsp.cuap_msg,      htobe32_mod(make_sender_id()));
         set_receiver_id(dsp.cuap_msg,    htobe32_mod(OxFFFFFFFF));

         if (is_begin_family(cmd_id))
            set_command_status(dsp.cuap_msg, htobe32_mod(Ox00000000));

         set_service_code(dsp.cuap_msg, dsp.doc["ussd_code"].GetString());
         set_msisdn(dsp.cuap_msg,       dsp.doc["msisdn"].GetString());
         set_ussd_content(dsp.cuap_msg, dsp.doc["message"].GetString());
         set_command_len(dsp.cuap_msg,  strlen(dsp.doc["message"].GetString()) + 20);
         return true;
      }

      fmt::print_yellow("[Binding App]: Wrong JSON Format, Cannot Build CUAP\n");

      return false;
   }

   bool build_json(dispatcher_t& dsp, concepts::uchar_t incomming)
   {
      using namespace cuap::body::begin_msg;
      char _ussd_content[182] {0},
           _msisdn[21] {0},
           _ussd[21] {0};

      //bool is_cuap;

      dsp.doc.SetObject();
      Document::AllocatorType& allocator = dsp.doc.GetAllocator();

      msisdn(incomming, _msisdn);
      service_code(incomming, _msisdn);
      ussd_content(_ussd_content);

      dsp.doc.AddMember("misisdn",  StringRef(_msisdn), allocator);
      dsp.doc.AddMember("session",  StringRef(get_session_str(incomming)), allocator);
      dsp.doc.AddMember("op_type",  "menu", allocator);
      dsp.doc.AddMember("session_id", StringRef(dsp.sess.id()), allocator);
      dsp.doc.AddMember("ussd_code",  StringRef(_ussd), allocator);
      dsp.doc.AddMember("message",    StringRef(_ussd_content), allocator);

      return true;
   }

   bool dispatch(dispatcher_t& dsp,
                 client_t& client, uchar_t incoming)
   {
      bool ret = false;
      if(ret = login::exec(client); ret)
      {
         if (ret = build_cuap(dsp, incoming); ret)
         {
            fmt::print_blue("Building CUAP msg...\n");
            client.send(dsp.cuap_msg, header::command_len(dsp.cuap_msg));
            memset(dsp.cuap_msg, 0x0, sizeof(dsp.cuap_msg));
            client.recv(dsp.cuap_msg, sizeof(dsp.cuap_msg));
         }
         else if(ret = build_json(dsp, incoming); ret)
         {
            //
         }
      }
      return ret;
   }

   inline bool exec(dispatcher_t& dsp,
                    client_t& client, uchar_t incoming)
   {
      return dispatch(dsp, client, incoming);
   }
}

namespace wsg::net::tcp
{

   namespace login = wsg::cuap::login;
   //using namespace service_app;

   session::session(tcp::socket &&socket) : socket_(std::move(socket))
   {
      //static int n = 0;
      //fmt::print_cyan("{}. [session]: New session Constructed!\n", ++n);
   }

   void session::start(std::unique_ptr<session> &&self)
   {
      do_read(std::move(self));
   }

   void session::read_handler(session_ptr self, const error_code& ec, size_t len)
   {
      using namespace samples;

      if (!ec)
      {
         client cl(self->service());
         cl.connect_to("0.0.0.0", "9981");
         if(!cl.error())
         {/// dispather code starts from here
            login::login_t           lgn;
            dispatcher::dispatcher_t dsp;

            dispatcher::exec(dsp, cl, data_); /// And dispather code ends here

            if (!cl.error())
            {
               do_write(std::move(self), sizeof(data_));
               memset(data_, 0, sizeof(data_));
            }
            else
               fmt::print_red("[Read Handler][TCP Client]: {}\n", cl.error().message());
         }
         else
            fmt::print_red("[Read Handler]: {}\n", cl.error().message());
      }
   }

   void session::write_handler(session_ptr self, const error_code& ec, size_t len)
   {
      if (!ec)
      {
         do_write(std::move(self), 10);
      }
   }

   void session::do_read(session_ptr self)
   {
      socket_.async_read_some(asio::buffer(data_, max_length),
                              [this, s = std::move(self)](const error_code & ec, std::size_t length) mutable
      {
         read_handler(move(s), ec, length);
      });
   }

   void session::do_write(session_ptr self, std::size_t length)
   {
      asio::async_write(socket_, asio::buffer(data_, length),
                        [this, s = std::move(self)](std::error_code ec, std::size_t /*length*/) mutable
      {
         if (!ec)
         {
            do_read(std::move(s));
         }
      });
   }
}

