#pragma once

#include <memory>
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
		vcpacket() : m_v_packet(0){}
		
		vcpacket(const vcpacket & src)
		{
			*this = src;
		}
		vcpacket & operator=(const vcpacket & src)
		{
			m_v_packet = src.m_v_packet;
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

	protected:
		type_v_buffer m_v_packet;
	};
}

