#pragma once
#include <string>
#include <memory>

#include <mp_coffee_pipe.h>
#include <cupdater_param_.h>

/**
* manager of parameters stroage of lpu230_update 
*/
class cupdater_mgmt {
public:
	typedef std::shared_ptr< cupdater_mgmt > type_ptr;
private:
	typedef std::map< unsigned long, cupdater_param::type_ptr > _type_map;

public:
	static cupdater_mgmt& get_instance();

public:
	virtual ~cupdater_mgmt();
	/**
	* @brief get cupdater_param by session number.
	* @param n_session_number - session number of client.
	* @return cupdater_param::type_ptr - cupdater_param pointer. if not found, return nullptr.
	*/
	cupdater_param::type_ptr get(unsigned long n_session_number) const;
	/**
	* @brief insert cupdater_param by session number.
	* @param n_session_number - session number of client.
	* @param ptr_param - cupdater_param pointer.
	* @return first - true : inserted, false : already exist.
	* @retunr second - the created cupdater_param pointer.
	*/
	std::pair<bool,cupdater_param::type_ptr> insert(unsigned long n_session_number);
	/**
	* @brief erase cupdater_param by session number.
	* @param n_session_number - session number of client.
	* @return true - erased.
	*/
	bool erase(unsigned long n_session_number);

	/**
	* @brief remove all item of map.
	*/
	void clear();

	/**
	* @brief start update process
	*/
	bool run_update(unsigned long n_session);

	/**
	* @brief notify to server about update-progressing
	* @param tuple.first : usb vendor id.
	* @param tuple.second : usb product id.
	* @param tuple.third : client sesion number
	* @param tuple.fourth : the current step of update(0~max step)
	* @param tuple.fifth : the max step of update
	* @param tuple.sixth : the result of current step. , true - success, false - failure.
	* @param tuple.seventh : operation error resaon
	* @return sent result
	*/
	bool notify_to_server(const _mp::ccoffee_pipe::type_tuple_notify_info & nf);

private:
	cupdater_mgmt();
private:
	cupdater_mgmt::_type_map m_map;

private:
	cupdater_mgmt(const cupdater_mgmt&) = delete;
	cupdater_mgmt& operator=(const cupdater_mgmt&) = delete;
};
