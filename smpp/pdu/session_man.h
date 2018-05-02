#ifndef SESSION_MANAGEMENT_H
#define SESSION_MANAGEMENT_H
#pragma once

#include "smpp/pdu/types.h"

using octet_t = uint8_t;

namespace wsg::smpp::pdu
{
   //namespace body = wsg::smpp::pdu::body;

   struct bind_transmitter : public basic_header<uint8_t, 98>
   {
      using basic_header<uint8_t, 98>::basic_header;

      bind_transmitter() {}

      void set_password(const uint8_t* id, size_t sz)
      {
         assign_n(id, sz,
                  cast_uint8(body::bind_transmitter::password),
                  cast_uint8(body::bind_transmitter::system_type));
      }
      void set_system_id(const uint8_t* id, size_t sz)
      {
         assign_n(id, sz,
                  cast_uint8(body::bind_transmitter::system_id),
                  cast_uint8(body::bind_transmitter::password));
      }
   };

   struct bind_transmitter_resp : public basic_header<uint8_t, 98>
   {
      using basic_header<uint8_t, 98>::basic_header;

      bind_transmitter_resp() = default;

      void set_system_id(const uint8_t* id, size_t sz)
      {
         assign_n(id, sz,
                  cast_uint8(body::bind_transmitter::system_id),
                  cast_uint8(body::bind_transmitter::password));
      }

      void set_tlv(tlv::tag tag, tlv::tlv_t tlv_v = {0x0210, 0x0001, 0})
      {
         assign(tlv_v.tag,    cast_uint8(body::bind_transmitter::password));
         assign(tlv_v.length, cast_uint8(body::bind_transmitter::password) + 2);
         assign(tlv_v.value,  cast_uint8(body::bind_transmitter::password) + 1);
      }

      template <int16_t N>
      constexpr void tlv(uint8_t(&dest)[N])
      {
         //misc::set_null(dest);
         memcpy(dest, &buffer[cast_uint8(body::bind_transmitter::password)], 5);
      }
   };
}

#endif // SESSION_MANAGEMENT_H
