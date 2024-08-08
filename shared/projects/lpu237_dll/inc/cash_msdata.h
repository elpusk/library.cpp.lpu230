#pragma once

#include <mp_type.h>
#include <tg_lpu237_dll.h>

class cash_msdata
{
public:
	cash_msdata();
	~cash_msdata();


	bool is_cashed(int n_result_index)
	{
		bool b_result(false);
		do {
			if (n_result_index < 0)
				continue;
			if (m_n_result_index < 0)
				continue;
			if (m_n_result_index != n_result_index)
				continue;
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	void cashing(const int n_result_index, const unsigned long n_result_code, const _mp::type_v_buffer& v_rx);

	unsigned long get_data(unsigned long n_zero_base_iso_track, _mp::type_v_buffer& v_out_iso_data);
	unsigned long get_data(unsigned long n_zero_base_iso_track);

	void reset();
private:
	_mp::type_v_buffer m_v_iso_data[3];
	unsigned long m_dw_result[3];
	int m_n_result_index;
};

