#ifndef smpp_pdu_types_h
#define smpp_pdu_types_h

#include "cuap/buffer.h"
//#include "cuap/typedefs.h"

#if (CUAP_IS_BIG_ENDIAN == 1)
   #define htobe16_mod htobe16
   #define htobe32_mod htobe32
#else
   #define htobe16_mod
   #define htobe32_mod
#endif // USE_BIG_ENDIAN

namespace wsg::smpp::pdu
{
   constexpr static uint32_t OxFFFFFFFF = 0xFFFFFFFF;
   constexpr static uint32_t Ox00000000 = 0x00000000;

   enum class command_ids : uint32_t
   {
      /** Session Management Operations*/
      bind_transmitter = 0x00000002, bind_transmitter_resp = 0x80000002, bind_receiver = 0x00000001,
      bind_receiver_resp = 0x80000001, bind_transceiver = 0x00000009, bind_transceiver_resp = 0x80000009,
      outbind = 0x0000000B, unbind = 0x00000006, unbind_resp= 0x80000006,
      enquire_link = 0x00000015, enquire_link_resp = 0x80000015,
      alert_notification = 0x00000102,
      generic_nack = 0x80000000,

      /** Message Submission Operations*/
      submit_sm = 0x00000004, submit_sm_resp = 0x80000004, data_sm = 0x00000103, data_sm_resp = 0x80000103,
      submit_multi = 0x00000021, submit_multi_resp = 0x80000021,

      /** Message Delivery Operations*/
      deliver_sm = 0x00000005, deliver_sm_resp = 0x8000005,

      /** Message Broadcast Operation*/
      broadcast_sm = 0x00000111, broadcast_sm_resp = 0x80000111,

      /** Ancillary Submission Operations*/
      cancel_sm = 0x00000008, cancel_sm_resp = 0x80000008, query_sm = 0x00000003, query_sm_resp = 0x80000003,
      replace_sm = 0x00000007, replace_sm_resp = 0x80000007,

      /** Ancillary Broadcast Operations*/
      query_broadcast_sm = 0x00000112, query_broadcast_sm_resp = 0x80000112, cancel_broadcast_sm = 0x00000113,
      cancel_broadcast_sm_resp = 0x80000113,
   };

   /*
    * Reserved for MC Vendor  0x00010200 - 0x000102FF
                              0x80010200 - 0x800102FF
   */
   enum class header : uint8_t
   { command_length = 0, command_id = 4, command_status = 8, sequence_number = 12};

   /* addr_ton, source_addr_ton, dest_addr_ton, esme_addr_ton
      PDU Field Definitions: These fields define the Type of Number (TON) to be used in the SME address parameters.
      The following TON values are defined:
   */
   enum class pdu_fields_ton : uint8_t
   {
      unknown = 0b00000000, international = 0b00000001, national = 0b00000010, network_specific = 0b00000011,
      subscriber_num = 0b00000100, alpanumeric = 0b00000101, abbreviated = 0b00000110
      /* All other values reserved */
   };

   /* addr_npi, source_addr_npi, dest_addr_npi, esme_addr_npi
      These fields define the Numeric Plan Indicator (NPI) to be used in the SME address
      parameters. The following NPI values are defined:
   */
   enum class pdu_fields_npi : uint8_t
   {
      unknown = 0b00000000, isdn_E163_E164 = 0b00000001, data_X_dot_121 = 0b00000011, telex_F_dot_69 = 0b00000100,
      land_mobile_E_dot_212 = 0b00000110, national = 0b00001000, private_ = 0b00001001, ERMES= 0b00001010,
      internet /* IP */ = 0b00001110, wap_client /* to be defined by WAP Forum */ = 0b00010010
      /* All other values reserved */
   };

   /**
      command_status, error_status_code
      The command_status field of a SMPP message response indicates the success or failure of
      a SMPP request. It is relevant only in the SMPP response message and should be set to
      NULL in SMPP request messages.
      The SMPP Error status codes are returned by the MC in the command_status field of the
      SMPP message header and in the error_status_code field of a submit_multi_resp message.
      The complete set of SMPP Error Codes and their associated values are defined in the
      following table.
   */

   enum class command_status : int16_t
   {
      ESME_ROK = 0x00000001, ESME_RINVMSGLEN, ESME_RINVCMDLEN, ESME_RINVCMDID, ESME_RINVBNDSTS, ESME_RALYBND,
      ESME_RINVPRTFLG, ESME_RINVREGDLVFLG, ESME_RSYSERR, ESME_RINVSRCADR, ESME_RINVDSTADR, ESME_RINVMSGID,
      ESME_RBINDFAIL, ESME_RINVPASWD, ESME_RINVSYSID, ESME_RCANCELFAIL = 0x00000011,ESME_RREPLACEFAIL = 0x00000013,
      ESME_RMSGQFUL, ESME_RINVSERTYP, ESME_RINVNUMDESTS = 0x00000033, ESME_RINVDLNAME = 0x00000034,
      ESME_RINVDESTFLAG = 0x00000040, ESME_RINVSUBREP = 0x00000042, ESME_RINVESMCLASS, ESME_RCNTSUBDL,
      ESME_RSUBMITFAIL = 0x00000045, ESME_RINVSRCTON = 0x00000048, ESME_RINVSRCNPI, ESME_RINVDSTTON,
      ESME_RINVDSTNPI, ESME_RINVSYSTYP = 0x00000053, ESME_RINVREPFLAG = 0x00000054, ESME_RINVNUMMSGS,
      ESME_RTHROTTLED = 0x00000058, ESME_RINVSCHED = 0x00000061, ESME_RINVEXPIRY, ESME_RINVDFTMSGID,
      ESME_RX_T_APPN, ESME_RX_P_APPN, ESME_RX_R_APPN, ESME_RQUERYFAIL, ESME_RINVTLVSTREAM = 0x000000C0,
      ESME_RTLVNOTALLWD, ESME_RINVTLVLEN, ESME_RMISSINGTLV, ESME_RINVTLVVAL, ESME_RDELIVERYFAILURE = 0x000000FE,
      ESME_RUNKNOWNERR = 0x000000FF , ESME_RSERTYPUNAUTH = 0x00000100, ESME_RPROHIBITED, ESME_RSERTYPUNAVAIL,
      ESME_RSERTYPDENIED, ESME_RINVDCS, ESME_RINVSRCADDRSUBUNIT, ESME_RINVDSTADDRSUBUNIT, ESME_RINVBCASTFREQINT,
      ESME_RINVBCASTALIAS_NAME, ESME_RINVBCASTAREAFMT, ESME_RINVNUMBCAST_AREAS = 0x0000010A, ESME_RINVBCASTCNTTYPE,
      ESME_RINVBCASTMSGCLASS, ESME_RBCASTFAIL, ESME_RBCASTQUERYFAIL, ESME_RBCASTCANCELFAIL,
      SME_RINVBCAST_REP = 0x00000110, ESME_RINVBCASTSRVGRP, ESME_RINVBCASTCHANIND
      /** Reserved for MC vendorspecific errors: 0x00000400-0x000004FF */
      /** All other values reserved */
   };

   enum class data_coding : uint8_t
   {
      MC_specific = 0b00000000, IA5_CCITT_T50_ASCII_ANSI_X3_4 = 0b00000001, octet_unspecified_8bit,
      Latin_1_ISO_8859_1, octet_unspecified_8bit_II, JIS_X0208_1990, Cyrillic_ISO_8859_5,
      Latin_or_Hebrew_ISO_8859_8, UCS2_ISO_IEC_10646, Pictogram_Encoding, ISO_2022_JP_Music_Codes, reservedI,
      reservedII, Extended_Kanji_JIS_X_0212_1990, KS_C_5601, reservedIII /* 0 0 0 0 1 1 1 1 -> 1 0 1 1 1 1 1 1*/,
      /* 1100xxxx  GSM MWI control - see [GSM 03.38] */
      /* 1101xxxx  GSM MWI control - see [GSM 03.38] */
      /* 1110xxxx  GSM MWI control - see [GSM 03.38] */
      /* 1111xxxx  GSM Message class control - see [GSM 03.38] */
   };

   enum class dest_flag : uint8_t
   {
      Ox01 = 0x01 /** SME Address */, Ox02 = 0x02 /** Distribution List Name */
   };

   /*
    esm_class
      The esm_class parameter is used to indicate special message attributes associated with the
   short message.
   The esm_class parameter is encoded as follows in the submit_sm, submit_multi and
   data_sm (ESME -> MC) PDUs:*/

   enum class esm_class : uint8_t
   {
      MC_mode = 0b00 /* TODO: complete other members */
   };

   /**
      interface_version
         This parameter is used to indicate the version of the SMPP protocol. The following interface
      version values are defined:
   */

   union Ox00_33
   {
      int8_t Ox00 = 0x00;
   };

   enum class interface_version : uint8_t
   {
      Ox00 = 0x00, version, Ox03 = 0x33 /* Indicates that the ESME supports version 3.3 or earlier of the SMPP protocol.*/,
      Ox34 = 0x34, Ox50 = 0x50 /* Version 3.4 and 5.0*/
   };

   enum class message_state : uint8_t
   {
      SCHEDULED = 0, ENROUTE, DELIVERED, EXPIRED, DELETED, UNDELIVERABLE, ACCEPTED, UNKNOWN, REJECTED, SKIPPED
   };

   /* The priority_flag parameter allows the originating SME to assign a priority level to the short
      message:
   */
   enum class priority_flag : uint8_t
   {
      OxO0 = 0, Ox01, OxO2, OxO3, OxO4, reserved /* All other values reserved*/
   };

   /* Set according to GSM 03.40 [GSM 03.40] */
   enum class protocol_id: uint8_t { /* TODO */ };

   /* The registered_delivery parameter is used to request a MC delivery receipt and/or SME
      originated acknowledgements. The following values are defined
   */
   enum class registered_delivery : uint8_t {/* TODO */};

   enum class replace_if_present_flag : uint8_t
   {
      dont_replace = 0, replace = 1, reserved /* 2 - 255 is Reserved */
   };

   struct service_type
   {
      char default_[1] = "", cellular_messaging[4] = "CMT", cellular_pining[4] = "CPT", voice_mail_noti[4] = "VMT",
           voice_mail_alert[4] = "VMA", wap[4] = "WAP", ussd[5] = "USSD", cell_broadcast_svc[4] = "CBS",
           generic_udp_transport_svc[5] = "GUTS";
   };

   namespace tlv
   {
      struct tlv_t
      {
        uint16_t tag, length; uint8_t value;
      };
      /** ind = indicator, scv, service, msg = message
      */
      enum class tag : uint16_t
      {
         dest_addr_subunit = 0x0005, dest_network_type, dest_bearer_type, dest_telematics_id,
         source_addr_subunit = 0x000D, source_network_type, source_bearer_type, source_telematics_id = 0x0010,
         qos_time_to_live = 0x0017, payload_type = 0x0019, additional_status_info_text = 0x001D,
         receipted_message_id = 0x001E, ms_message_wait_facilities = 0x0030, privacy_indicator = 0x0201,
         source_subaddr, dest_subaddr, user_message_ref, user_resp_code, source_port = 0x020A, dest_port,
         sar_message_ref_num, lang_indicator, sar_total_segments, sar_segment_seqnum, sc_interface_ver = 0x0210,
         callback_num_pres_ind = 0x0302, callback_num_atag, number_of_messages, callback_num = 0x0381,
         dpf_result = 0x0420, set_dpf, ms_availability_status, network_err_code, message_payload,
         delivery_failure_reason, more_messages_to_send, message_state, congestion_state, ussd_service_op = 0x0501,
         broadcast_channel_indicator = 0x0600, broadcast_content_type, broadcast_content_type_info,
         broadcast_message_class, broadcast_rep_num, broadcast_frequency_interval, broadcast_area_identifier,
         broadcast_error_status, broadcast_area_success, broadcast_end_time, broadcast_service_group = 0x060A,
         billing_identification, source_network_id, dest_network_id, source_node_id = 0x0610, dest_node_id,
         dest_addr_np_resolution, dest_addr_np_information, dest_addr_np_country, display_time = 0x1201,
         sms_signal = 0x1203, ms_validity = 0x1204, alert_on_message_delivery = 0x130C, its_reply_type = 0x1380,
         its_session_info = 0x1383
         /* TODO: Add remamining members */
      };

      namespace tags {
         enum class generic {
            dest_network_type = 0x01
         };

         enum class gsm {
            dest_addr_subunit = 0x05, dest_telematics_id, source_addr_subunit, source_telematics_id,
         };

         enum class cdma {

         };

         enum class cdma_tdma {

         };
      }
   }

}

namespace wsg::smpp::pdu {

   template <class T = uint8_t, uint16_t SZ = 16>
   struct basic_header : public wsg::misc::static_buffer<T, SZ>
   {
      using misc::static_buffer<T, SZ>::static_buffer;

      void set_command_len(uint32_t val)    { this->assign(val, header::command_length); }
      void set_command_id(uint32_t val)     { this->assign(val, header::command_id); }
      void set_command_status(uint32_t val) { this->assign(val, header::command_status); }
      void set_sequence_num(uint32_t val)   { this->assign(val, header::sequence_number); }

      constexpr uint32_t command_len()    { return this->get_u32(header::command_length); }
      constexpr uint32_t command_id()     { return this->get_u32(header::command_id); }
      constexpr uint32_t command_status() { return this->get_u32(header::command_status); }
      constexpr uint32_t sequence_num()   { return this->get_u32(header::sequence_number); }

      // Methods to test fot the command type/id

      constexpr bool is_bind_transmitter()       { return command_id() == command_ids::bind_transmitter; }
      constexpr bool is_bind_transmitter_resp()  { return command_id() == command_ids::bind_transmitter_resp; }

      constexpr bool is_bind_receiver()      { return command_id() == command_ids::bind_receiver; }
      constexpr bool is_bind_receiver_resp() { return command_id() == command_ids::bind_receiver_resp; }

      constexpr bool is_bind_transceiver()      { return command_id() == command_ids::bind_transceiver; }
      constexpr bool is_bind_transceiver_resp() { return command_id() == command_ids::bind_transceiver_resp; }
   };

   using header_msg = basic_header<uint8_t, 16>;
}

namespace wsg::smpp::pdu::body
{
   enum class bind_transmitter : uint8_t {
      system_id = 16, password = 32, system_type = 41, interface_version = 54, addr_ton = 55, addr_npi = 56,
      address_range = 97
   };

   enum class bind_transmitter_resp : uint8_t {
      system_id = 16, sc_interface_version = 32
   };

   using bind_receiver      = bind_transmitter;
   using bind_receiver_resp = bind_transmitter_resp;

}

template <typename T, typename T2>
constexpr T cast(T2&& src)
{
  return static_cast<T>(src);
}

template <typename T>
constexpr auto cast_int8(T&& src) { return cast<int8_t>(src); }
template <typename T>
constexpr uint8_t cast_uint8(T&& src) { return cast<uint8_t>(src); }
template <typename T>
constexpr auto cast_int16(T&& src) { return cast<int16_t>(src); }

namespace wsg::smpp::pdu
{
   void tests();
   void main();
}

#endif//smpp_pdu_types_h
