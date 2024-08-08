#pragma once

#include <mp_type.h>

namespace _mp
{
	class icprotocol
	{
	public:
		typedef	enum {
			gtp_result_no_more_transaction, //no more data in tx queue
			gtp_result_more_packet_in_this_transaction,
			gtp_result_no_more_packet_in_this_transaction
		}type_get_tx_packet_result;

		typedef	enum {
			grp_result_unknown_packet,
			grp_result_need_more_packet_for_this_transaction,
			grp_result_complete_packet_for_this_transaction
		}type_get_rx_packet_result;

	public:
		virtual ~icprotocol() {};
		
		virtual icprotocol::type_get_tx_packet_result get_tx_packet(type_v_buffer& v_out_packet) = 0;
		
		//save rx packet.
		virtual icprotocol::type_get_rx_packet_result set_rx_packet(const type_v_buffer& v_in_packet) = 0;

		//return the numer of remainder tx transaction. and one transaction tx data
		virtual size_t get_tx_transaction(type_v_buffer& v_out_packet) = 0;

		//set one transaction rx data
		virtual bool set_rx_transaction(const type_v_buffer& v_in_packet) = 0;

	};
}
