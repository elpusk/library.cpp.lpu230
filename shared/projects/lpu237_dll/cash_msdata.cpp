#include <websocket/mp_win_nt.h>
#include "cash_msdata.h"
#include <cprotocol_lpu237.h>

cash_msdata::cash_msdata() : 
	m_v_iso_data{ _mp::type_v_buffer(0),_mp::type_v_buffer(0), _mp::type_v_buffer(0)}
	, m_dw_result{ LPU237_DLL_RESULT_ERROR ,LPU237_DLL_RESULT_ERROR ,LPU237_DLL_RESULT_ERROR }
	, m_n_result_index(-1)
{
}

cash_msdata::~cash_msdata()
{
}

void cash_msdata::cashing(const int n_result_index,const unsigned long n_result_code, const _mp::type_v_buffer& v_rx)
{

	std::wstring s_track;

	if (n_result_index < 0)
		return;
	m_n_result_index = n_result_index;

	for (int i = 0; i < 3; i++) {
		m_dw_result[i] = n_result_code;
		m_v_iso_data[i].resize(0);
		//
		if (n_result_code != LPU237_DLL_RESULT_SUCCESS)
			continue;
		//
		s_track.clear();

		if (!cprotocol_lpu237::get_track_data_from_rx(i + 1, s_track, v_rx)) {
			m_dw_result[i] = LPU237_DLL_RESULT_ERROR_MSR;
			continue;
		}
		std::for_each(std::begin(s_track), std::end(s_track), [&](const wchar_t data) {
			m_v_iso_data[i].push_back(static_cast<unsigned char>(data));
			});

		m_dw_result[i] = static_cast<unsigned long>(s_track.length());
	}//end for
}

unsigned long cash_msdata::get_data(unsigned long n_zero_base_iso_track, _mp::type_v_buffer& v_out_iso_data)
{
	unsigned long n_result_code(LPU237_DLL_RESULT_ERROR);

	do {
		v_out_iso_data.resize(0);

		if (n_zero_base_iso_track > 2)
			continue;
		//
		if (m_n_result_index < 0)
			continue;
		//
		n_result_code = m_dw_result[n_zero_base_iso_track];
		v_out_iso_data = m_v_iso_data[n_zero_base_iso_track];

		m_dw_result[n_zero_base_iso_track] = LPU237_DLL_RESULT_ERROR;
		m_v_iso_data[n_zero_base_iso_track].resize(0);

	} while (false);
	return n_result_code;
}

unsigned long cash_msdata::get_data(unsigned long n_zero_base_iso_track)
{
	unsigned long n_result_code(LPU237_DLL_RESULT_ERROR);
	bool b_reset(false);

	do {
		if (n_zero_base_iso_track > 2)
			continue;
		//
		if (m_n_result_index < 0)
			continue;
		//
		n_result_code = m_dw_result[n_zero_base_iso_track];
		
		switch (n_result_code) {
		case LPU237_DLL_RESULT_SUCCESS:
		case LPU237_DLL_RESULT_ICC_INSERTED:
		case LPU237_DLL_RESULT_ICC_REMOVED:
		case LPU237_DLL_RESULT_CANCEL:
		case LPU237_DLL_RESULT_ERROR:
		case LPU237_DLL_RESULT_ERROR_MSR:
			b_reset = true;
			break;
		default://data size
			break;
		}//end switch

	} while (false);

	if (b_reset) {
		reset();
	}
	return n_result_code;
}

void cash_msdata::reset()
{
	m_n_result_index = -1;
	for (int i = 0; i < 3; i++) {
		m_dw_result[i] = LPU237_DLL_RESULT_ERROR;
		m_v_iso_data[i].resize(0);
	}//end for
}
