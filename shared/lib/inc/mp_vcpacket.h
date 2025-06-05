#pragma once

#include <memory>
#include <mutex>
#include <mp_type.h>

namespace _mp
{
	/**
	* virtual class 
	* need the defintion of
	* is_valid() function.
	*/
	class vcpacket
	{
	public:
		typedef	std::shared_ptr<vcpacket>		type_ptr;

	public:
		virtual bool is_valid()=0;//check packet is valid or not

		bool empty() const 
		{
			return m_v_packet.empty();
		}
		vcpacket() : m_v_packet(0)
		{
			m_n_uid = vcpacket::_get_new_uid();
		}
		
		vcpacket(const vcpacket & src)
		{
			m_n_uid = vcpacket::_get_new_uid();
			*this = src;
		}
		vcpacket & operator=(const vcpacket & src)
		{
			if (this != &src) {
				m_v_packet = src.m_v_packet;
			}
			return *this;
		}

		virtual ~vcpacket(){}
		
		size_t get_packet(type_v_buffer & v_packet) const
		{
			v_packet = m_v_packet;
			return v_packet.size();
		}

		void set_packet(const type_v_buffer & v_packet = type_v_buffer(0))
		{
			m_v_packet = v_packet;
		}

		size_t get_uid() const
		{
			return m_n_uid;
		}
	private:

		/**
		* @brief generate unique id of packet.
		* @return unique id of packet
		*/
		static size_t _get_new_uid()
		{
			static std::mutex _m;
			static size_t n_id(0);
			size_t n(0);
			do {
				std::lock_guard<std::mutex> lock(_m);
				n = n_id;
				++n_id;
			} while (false);
			return n;
		}
	protected:
		type_v_buffer m_v_packet;
		size_t m_n_uid; // this value shall not be copyed copy-constructor or assign operator
	};
}

