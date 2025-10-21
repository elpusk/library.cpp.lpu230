#pragma once

#include <memory>
#include <vector>
#include <fstream>
#include <tg_rom.h>
#include "protocol.h"

class CHidBootBuffer
{
public:
	typedef	std::shared_ptr< std::vector<char> >	type_sector;
	typedef	std::vector< CHidBootBuffer::type_sector >	type_v_read_buffer;

	enum {
		C_NUM_HID_DL_ADD_OPERATION = 5,
		C_LPC1343_NUM_TOTAL_SECTOR = 7,
		C_NUM_PACKET_PER_SECTOR = 76, //(C_PACKET_SIZE-10)/C_SECTOR_SIZE
		C_LPC1343_MSD_FILE_MAX_SIZE = 32 * 1024,
		C_LPC1343_HID_FILE_MAX_SIZE = 24 * 1024
	};

	enum {
		C_SECTOR_SIZE = 4096,
		C_SECTOR_BUFFER_SIZE = 4104
	};

	enum {
		C_MAX_SIZE_FIRMWARE_BYTE = 90 * 1024 * 1024 //the maximum firmware size 90MBytes
	};

	enum {
		C_PACKET_SIZE = 64,
		C_DATA_SIZE_OF_ONE_PACKET = (CHidBootBuffer::C_PACKET_SIZE - (sizeof(HidBLRequest) - 1))
	};

public:
	CHidBootBuffer()
	{
		reset();
	}

	/**
	* reset by default setting.(default device is LPC1343.)
	*
	*/
	void reset()
	{
		reset(1, CHidBootBuffer::C_LPC1343_NUM_TOTAL_SECTOR, false, false);
	}

	void reset(int n_app_start_sector, int n_number_of_app_sector, bool b_sector_info, bool b_adjust_by_file_size)
	{
		m_v_buffer.clear();
		//
		m_v_send_orders.resize(n_number_of_app_sector);

		for (int i = 0; i < n_number_of_app_sector; i++) {
			m_v_send_orders[i] = (n_app_start_sector + 1) + i;
			m_v_buffer.push_back(CHidBootBuffer::type_sector(new std::vector<char>(CHidBootBuffer::C_SECTOR_BUFFER_SIZE, (char)0xFF)));
		}//end for

		*(m_v_send_orders.end() - 1) = n_app_start_sector;

		m_n_app_area_size = CHidBootBuffer::C_SECTOR_SIZE * n_number_of_app_sector;

		m_n_cur_send_index = 0;
		m_n_offset = 0;
		m_n_packet_chain = 0;

		m_n_file_size = 0;
		//
		m_n_erasing_sec_index = 0;
		m_n_erase_start_sec = m_n_app_start_sector = n_app_start_sector;
		m_n_number_of_app_sector = n_number_of_app_sector;
		m_n_number_of_erase_sector = n_number_of_app_sector;
		//
		int n_sec_system_var(-1);

		if (!b_adjust_by_file_size) {
			m_b_sector_info = b_sector_info;
		}
		else {//In this case, system variables are included to erase area.(the last secor)
			n_sec_system_var = m_v_erase_sector.back();
		}

		//setup eraseing sector vector
		m_v_erase_sector.resize(m_n_number_of_erase_sector);
		for (int i = 0; i < m_n_number_of_erase_sector; i++) {
			m_v_erase_sector[i] = m_n_erase_start_sec + i;
		}//end for

		if (n_sec_system_var >= 0) {
			//system variable will be erased.
			m_v_erase_sector.push_back(n_sec_system_var);
		}
	}

	bool set_file_size_and_adjust_erase_write_area(int n_file_size)
	{
		bool b_result(false);

		do {
			if (n_file_size < 0)
				continue;
			if (n_file_size > CHidBootBuffer::C_MAX_SIZE_FIRMWARE_BYTE)
				continue;
			if (n_file_size > m_n_app_area_size)
				continue;
			//
			int n_sec_num = n_file_size / CHidBootBuffer::C_SECTOR_SIZE;
			if (n_file_size % CHidBootBuffer::C_SECTOR_SIZE != 0) {
				++n_sec_num;
			}

			//adjust app sector area.
			// the m_b_sector_info variable is maintained.
			reset(m_n_app_start_sector, n_sec_num, m_b_sector_info, true);

			m_n_file_size = n_file_size;
			b_result = true;
		} while (false);
		return b_result;
	}

	int get_next_send_sector_number()
	{
		int n_sec(-1);

		do {
			if (m_n_cur_send_index < 0)
				continue;
			if ((m_v_send_orders.size() - 1) < m_n_cur_send_index) {
				continue;
			}
			n_sec = m_v_send_orders[m_n_cur_send_index];
		} while (false);
		return n_sec;
	}
	int get_sending_sector_number() const
	{
		return m_n_cur_send_index;
	}

	bool is_complete_send() const
	{
		if (m_n_cur_send_index < m_n_number_of_app_sector) {
			return false;
		}
		return true;
	}

	int get_erasing_sector_number() const
	{
		if (m_n_erasing_sec_index < m_v_erase_sector.size())
			return m_v_erase_sector[m_n_erasing_sec_index];
		return -1;//error
	}

	/**
	* return the current erasing sector number before increasing.
	*/
	int increase_erasing_sector_number()
	{
		int n_cur(-1);

		if (m_n_erasing_sec_index < m_v_erase_sector.size()) {
			n_cur = m_v_erase_sector[m_n_erasing_sec_index];
			++m_n_erasing_sec_index;
		}
		return n_cur;
	}

	bool is_complete_erase() const
	{
		if (!m_b_sector_info)
			return true;
		//
		if (m_n_erasing_sec_index < m_v_erase_sector.size() - 1)
			return false;
		return true;
	}
	void load_to_buffer_from_file_current_pos(std::ifstream& f)
	{
		int i = 0;
		while (!f.eof() && i < m_n_number_of_app_sector) {
			f.read(&((*(m_v_buffer[i]))[0]), CHidBootBuffer::C_SECTOR_SIZE);
			i++;
		}
	}

	/**
	* return changed read_offset_of_rom_item
	*/
	int load_to_buffer_from_file_current_pos
	(
		CRom& rom,
		CRom::ROMFILE_HEAD& rom_header,
		int n_rom_item_index,
		int n_read_offset_of_rom_item
	)
	{
		int  i = 0;

		do {
			if (n_rom_item_index < 0)
				continue;
			while (n_read_offset_of_rom_item < rom_header.Item[n_rom_item_index].dwSize && i < m_n_number_of_app_sector) {
				rom.ReadBinaryOfItem(reinterpret_cast<unsigned char*>(&((*(m_v_buffer[i]))[0])), CHidBootBuffer::C_SECTOR_SIZE, n_read_offset_of_rom_item, &rom_header.Item[n_rom_item_index]);
				n_read_offset_of_rom_item += CHidBootBuffer::C_SECTOR_SIZE;
				i++;
			}

		} while (false);
		return n_read_offset_of_rom_item;
	}

	unsigned long get_app_area_size() const
	{
		unsigned long n(0);

		do {
			if (!m_b_sector_info)
				continue;
			//
			n = m_n_number_of_app_sector * CHidBootBuffer::C_SECTOR_SIZE;
		} while (false);
		return n;
	}

	void get_one_packet_data_from_buffer(int n_index, unsigned char* ps_packet_data_field)
	{
		memcpy(ps_packet_data_field, &((*m_v_buffer[n_index])[m_n_offset]), CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET);
	}

	void move_read_buffer_position_to_next()
	{
		if (m_n_offset >= C_SECTOR_BUFFER_SIZE) {
			m_n_cur_send_index++;
			m_n_offset = 0;
			m_n_packet_chain = 0;
		}
		else {
			m_n_packet_chain++;
		}
	}

	bool is_complete_get_one_sector_from_buffer() const
	{
		if ((m_n_offset + CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET) >= CHidBootBuffer::C_SECTOR_SIZE)
			return true;
		else
			return false;
	}

	void increase_offset_in_one_sector()
	{
		m_n_offset += CHidBootBuffer::C_DATA_SIZE_OF_ONE_PACKET;
	}
	int get_packet_chain() const
	{
		return m_n_packet_chain;
	}
	int get_zero_base_index_of_sector(int n_sector) const
	{
		return n_sector - m_n_app_start_sector;
	}
	int get_the_number_of_packet_for_app() const
	{
		return m_n_number_of_app_sector * CHidBootBuffer::C_NUM_PACKET_PER_SECTOR;
	}
	int get_the_number_of_app_sector() const
	{
		return m_n_number_of_app_sector;
	}
	bool is_sector_info_exist() const
	{
		return m_b_sector_info;
	}
private:
	CHidBootBuffer::type_v_read_buffer m_v_buffer;
	int m_n_cur_send_index;
	int m_n_offset;
	int m_n_app_area_size;
	int m_n_file_size;
	int m_n_packet_chain;
	int m_n_app_start_sector;
	int m_n_number_of_app_sector;

	int m_n_erasing_sec_index;
	int m_n_erase_start_sec;
	int m_n_number_of_erase_sector;
	std::vector<int> m_v_erase_sector;

	bool m_b_sector_info;

	std::vector<int> m_v_send_orders;
};