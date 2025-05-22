#include <websocket/mp_win_nt.h>

#include <functional>
#include <memory>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <cassert>

#include <mp_type.h>
#include <mp_cqueue.h>

#include <server/mp_cserver_.h>
#include <server/mp_cdev_ctl_.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <atltrace.h>
#endif

namespace _mp {
		cdev_ctl::~cdev_ctl()
		{
		}
		cdev_ctl::cdev_ctl(clog* p_log) : cworker_ctl(p_log), m_fun(p_log)
		{
		}

		cio_packet::type_ptr cdev_ctl::_execute(cio_packet::type_ptr& ptr_req_new, cio_packet::type_ptr& ptr_req_cur)
		{
			// true 로 return (transaction 완료)나타내기 위해서는 client 에세 보낼 데이터를 return 하기 전에
			// send_data_to_client_by_ip4() 로 전송해야 함.
			// false 로 return 하면, _continue() 이 transaction 이 완료 될때 까지 반복적으로 호출되서, 처리하고,
			// _continue() 에서  client 에세 보낼 데이터를 send_data_to_client_by_ip4() 로 전송

			if (!ptr_req_new && !ptr_req_cur) {
				m_p_log->log_fmt(L"[I][_EXE] ENTER - new(none), cur(none)\n");
				m_p_log->trace(L"[I][_EXE] ENTER - new(none), cur(none)\n");
			}
			else if (ptr_req_new && !ptr_req_cur) {
				m_p_log->log_fmt(L"[I][_EXE] ENTER - new(%u, %ls), cur(none)\n",
					ptr_req_new->get_session_number(), ptr_req_new->get_action_by_string().c_str()
				);
				m_p_log->trace(L"[I][_EXE] ENTER - new(%u, %ls), cur(none)\n",
					ptr_req_new->get_session_number(), ptr_req_new->get_action_by_string().c_str()
				);
			}
			else if (!ptr_req_new && ptr_req_cur) {
				m_p_log->log_fmt(L"[I][_EXE] ENTER - new(none), cur( %u, %ls )\n",
					ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str()
				);
				m_p_log->trace(L"[I][_EXE] ENTER - new(none), cur( %u, %ls )\n",
					ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str()
				);
			}
			else{
				m_p_log->log_fmt(L"[I][_EXE] ENTER - new(%u, %ls), cur( %u, %ls )\n",
					ptr_req_new->get_session_number(), ptr_req_new->get_action_by_string().c_str(),
					ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str()
				);
				m_p_log->trace(L"[I][_EXE] ENTER - new(%u, %ls), cur( %u, %ls )\n",
					ptr_req_new->get_session_number(), ptr_req_new->get_action_by_string().c_str(),
					ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str()
				);
			}


			bool b_complete(true);
			cio_packet::type_ptr ptr_req_ing;
			type_v_buffer v_rsp;
			cbase_ctl_fn::cresult::type_ptr ptr_result_error, ptr_result;
			cio_packet::type_ptr ptr_response;
			cdev_ctl_fn::cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp ptr_v_req_rsp;

			do {
				if (ptr_req_new->is_response()) {
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_request_type);
					continue;
				}
				size_t n_data(ptr_req_new->get_data_field_size());
				if (n_data > cws_server::const_default_max_rx_size_bytes) {
					if (ptr_req_new->get_data_field_type() == cio_packet::data_field_string_utf8) {
						m_p_log->get_instance().log_fmt(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->get_instance().trace(L"[E] - %ls | overflow rx data(string type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					else {
						m_p_log->log_fmt(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
						m_p_log->trace(L"[E] - %ls | overflow rx data(binary type) : %u(limit : %u).\n", __WFUNCTION__, n_data, cws_server::const_default_max_rx_size_bytes);
					}
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_overflow_buffer);
					continue;
				}

				switch (ptr_req_new->get_action())
				{
				case cio_packet::act_dev_open:
				case cio_packet::act_dev_close:
				case cio_packet::act_dev_transmit:
				case cio_packet::act_dev_cancel:
				case cio_packet::act_dev_write:
				case cio_packet::act_dev_read:
					ptr_result = m_fun.process_event(ptr_req_new, ptr_req_cur);
					assert(ptr_result); // process_event() 의 제약 조건.

					std::tie(std::ignore, b_complete) = ptr_result->process_get_result();
					if (!b_complete) {
						ptr_req_ing = ptr_result->get_req();// 계속 실행 중인 req 를 return 으로 설정.
						break;
					}
					
					// 처리가 끝난 경우.
					ptr_response = ptr_result->get_rsp();
					if (!ptr_response) {
						ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
						ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_device_misformat);
						continue;
					}
					break;
				case cio_packet::act_dev_sub_bootloader:
				case cio_packet::act_dev_independent_bootloader:
				default:// 현재는 지원하지 않으므로 그냥 에러 처리.
					ptr_result_error = std::make_shared<cbase_ctl_fn::cresult>(*ptr_req_new);
					ptr_result_error->after_processing_set_rsp_with_error_complete(cio_packet::error_reason_action_code);
					continue;
				}//end switch

			} while (false);

			// ptr_result_error 가 할당되어 있으면 에러 상태.
			if (ptr_result_error) {
				// 처리 전 결과는 에러
				if (!ptr_result_error->get_req()->is_self()) {
					// 에러를 전송
					ptr_result_error->get_rx(v_rsp);
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp, ptr_req_new->get_session_number());
				}
				if (ptr_req_cur) {
					// 어떤 실행 중에 새로운 것이 온 상태면, 새로운 것 실행 전에 에러 이므로
					// 실행 하던 것 계속 실행
					ptr_req_ing = ptr_req_cur;
				}
			}
			else {
				// 처리 전 에러 없음. 
				if (!ptr_req_ing) {
					// 새롭게 처리 중인 것이 없으면(새로운 req 처리 완료.)
					assert(ptr_response); // ptr_response 것이 없으면 무조건 ptr_result_error 이 있어야.

					// open close complete 경우, q 에 포함되지 않아, 
					// get_all_complete_response() 에 ptr_response 가 포함되지 않는 경우, 포함하도록
					// get_all_complete_response() 의 인자로 ptr_req_new, ptr_response pair 를 준다.
					ptr_v_req_rsp = m_fun.get_all_complete_response(std::make_pair(ptr_req_new, ptr_response));//complete 된 모든 req, rsp pair 를 얻기
					
					// 아직 안됌.
					if (!ptr_v_req_rsp) {
						//complete 된 것 없는 경우
						ptr_req_ing = ptr_req_cur; // 기존 처리 중인 것이 있으면. 그 것을 계속 처리. 없으면 없는 것을 return.
					}
					else {
						// complete 된 것이 존재 하면, 서버로 응답을 보내는데 .......
						// 아직까지 ptr_req_ing 는 null 인 상태.
						for (auto item : *ptr_v_req_rsp) {
							if (item.first.get() == ptr_req_cur.get()) {
								// 전에 실행 중이던 req는 complete 되어서 reference decrease 되고, 
								// 아래에서 server 로 전송.
								ptr_req_cur.reset(); 
							}
							else if (item.first.get() == ptr_req_new.get()) {
								// 새로운 req 처리 완료.
								// get_all_complete_response() 의 return vector item 순서상.
								// 전에 실행 중이던 ptr_req_cur 가 complete 되면 , 이 곳에 오기 전에
								// 위 코드에서 항상 ptr_req_cur.reset() 됨.
								// 이 곳 처리 후, ptr_req_cur.reset() 는 실행되지 않음.
								ptr_req_ing = ptr_req_cur; // 계속 실행 할 req를 설정.
							}

							if (item.first->is_self()) {
								continue;
							}

							type_v_buffer v_rsp;
							item.second->get_packet_by_json_format(v_rsp);
							cserver::get_instance().send_data_to_client_by_ip4(v_rsp, item.second->get_session_number());
						}//end for

						if (!ptr_req_ing) {
							// 만약 계속 실행해야하는 req 가 없으면, read q 에서 complete 되지 않은 req를 찾아서
							// 행해야하는 req로 설정.
							auto v_ptr_req = m_fun.get_front_of_read_queue();
							if (!v_ptr_req.empty()) {
								ptr_req_ing = v_ptr_req[0];
							}

							for (auto item : v_ptr_req) {
								m_p_log->log_fmt(L"[I][_EXE] !ING- (%u, %ls)\n",
									item->get_session_number(), item->get_action_by_string().c_str()
								);
								m_p_log->trace(L"[I][_EXE] !ING- (%u, %ls)\n",
									item->get_session_number(), item->get_action_by_string().c_str()
								);
							}// end for 
						}
						else {
							// 디버깅을 위한 코드
							auto v_ptr_req = m_fun.get_front_of_read_queue();
							for (auto item : v_ptr_req) {
								m_p_log->log_fmt(L"[I][_EXE] ING - (%u, %ls)\n",
									item->get_session_number(), item->get_action_by_string().c_str()
								);
								m_p_log->trace(L"[I][_EXE] ING - (%u, %ls)\n",
									item->get_session_number(), item->get_action_by_string().c_str()
								);
							}// end for 
						}
					}
				}
				else {
					// 새롭게 처리 중인 것이 있으면, 기존 처리 중인 것이 있는 것과 무관하게, 
					// 새로운 것 계속 처리.
					 
					// 디버깅을 위한 코드
					auto v_ptr_req = m_fun.get_front_of_read_queue();
					for (auto item : v_ptr_req) {
						m_p_log->log_fmt(L"[I][_EXE] ING*- (%u, %ls)\n",
							item->get_session_number(), item->get_action_by_string().c_str()
						);
						m_p_log->trace(L"[I][_EXE] ING*- (%u, %ls)\n",
							item->get_session_number(), item->get_action_by_string().c_str()
						);
					}// end for 
				}
			}

			if (!ptr_req_ing) {
				m_p_log->log_fmt(L"[I][_EXE] EXIT - ptr_req_ing(none)\n");
				m_p_log->trace(L"[I][_EXE] EXIT - ptr_req_ing(none)\n");
			}
			else{
				m_p_log->log_fmt(L"[I][_EXE] EXIT - ptr_req_ing(%u, %ls)\n",
					ptr_req_ing->get_session_number(), ptr_req_ing->get_action_by_string().c_str()
				);
				m_p_log->trace(L"[I][_EXE] EXIT - ptr_req_ing(%u, %ls)\n",
					ptr_req_ing->get_session_number(), ptr_req_ing->get_action_by_string().c_str()
				);
			}

			return ptr_req_ing;
		}

		bool cdev_ctl::_continue(cio_packet::type_ptr& ptr_req_cur)
		{
			// _execute() 의 return 이 nullptr 이 아니면, vcworker::_worker() 에 의해 여기 호출됨.
			bool b_complete(false);
			do {
				// 여기는 read request 에 대한 응답만 처리 될 것임.
				cdev_ctl_fn::cdev_ctl_fn::type_ptr_v_pair_ptr_req_ptr_rsp ptr_v_req_rsp = m_fun.get_all_complete_response();
				if (!ptr_v_req_rsp) {
					// 아직 결과가 없음.
					continue;
				}

				for (auto item : *ptr_v_req_rsp) {
					if (item.first.get() == ptr_req_cur.get()) {
						// complete 된 것 중. 현재 실행 중인 request 가 있으면 contine 를 cmplete 로 종료해서,
						// 상위 프로세스에서 현재 request 에 대한 메모리 해제를 요청함.
						b_complete = true; 
					}

					if (item.first->is_self()) {
						continue;
					}

					if (!ptr_req_cur) {
						m_p_log->log_fmt(L"[I][_CON] - cur(none), send(%u, %ls)\n",
							item.first->get_session_number(), item.first->get_action_by_string().c_str()
						);
						m_p_log->trace(L"[I][_CON] - cur(none), send(%u, %ls)\n",
							item.first->get_session_number(), item.first->get_action_by_string().c_str()
						);
					}
					else {
						m_p_log->log_fmt(L"[I][_CON] - cur(%u, %ls), send(%u, %ls)\n",
							ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str(),
							item.first->get_session_number(), item.first->get_action_by_string().c_str()
						);
						m_p_log->trace(L"[I][_CON] - cur(%u, %ls), send(%u, %ls)\n",
							ptr_req_cur->get_session_number(), ptr_req_cur->get_action_by_string().c_str(),
							item.first->get_session_number(), item.first->get_action_by_string().c_str()
						);
					}

					type_v_buffer v_rsp;
					item.second->get_packet_by_json_format(v_rsp);
					cserver::get_instance().send_data_to_client_by_ip4(v_rsp, item.second->get_session_number());
				}//end for
				
			} while (false);
			return b_complete;
		}

}//the end of _mp namespace

