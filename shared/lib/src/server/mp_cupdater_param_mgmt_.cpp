#include <websocket/mp_win_nt.h>
#include <server/mp_cupdater_param_mgmt_.h>

_mp::cupdater_param_mgmt& _mp::cupdater_param_mgmt::get_instance()
{
	static _mp::cupdater_param_mgmt instance;
	return instance;
}
_mp::cupdater_param_mgmt::cupdater_param_mgmt()
{
}

_mp::cupdater_param_mgmt::~cupdater_param_mgmt()
{
	m_map.clear();
}

_mp::cupdater_param::type_ptr _mp::cupdater_param_mgmt::get(unsigned long n_session_number) const
{
	_mp::cupdater_param::type_ptr ptr;

	auto it = m_map.find(n_session_number);
	if (it != std::end(m_map)) {
		ptr = it->second;
	}
	return ptr;
}

std::pair<bool, _mp::cupdater_param::type_ptr> _mp::cupdater_param_mgmt::insert(unsigned long n_session_number)
{
	bool b_inserted(false);
	_mp::cupdater_param::type_ptr ptr;
	auto it = m_map.find(n_session_number);
	if (it == std::end(m_map)) {
		// not found, insert
		ptr = std::make_shared<_mp::cupdater_param>(n_session_number);
		m_map.insert(std::make_pair(n_session_number, ptr));
		b_inserted = true;
	}
	else {
		// found
		ptr = it->second;
	}
	return std::make_pair(b_inserted, ptr);
}

bool _mp::cupdater_param_mgmt::erase(unsigned long n_session_number)
{
	auto it = m_map.find(n_session_number);
	if (it != std::end(m_map)) {
		m_map.erase(it);
		return true;
	}
	return false;
}

void _mp::cupdater_param_mgmt::clear()
{
	m_map.clear();
}