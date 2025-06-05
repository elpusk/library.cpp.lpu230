#pragma once

#include <mutex>
#include <vector>

#include <mp_type.h>
#include <mp_cconvert.h>

namespace _mp{

    /**
    * the item mp_cqueue_ptr class
    */
    class cqitem_dev {
    public:
        typedef enum : int {
            result_not_yet,
            result_success,
            result_error,
            result_cancel
        } type_result;

        typedef enum : int {
            req_only_tx,
            req_only_rx,
            req_tx_rx,
            req_cancel
        }type_request;
    public:
        typedef std::shared_ptr< cqitem_dev > type_ptr;

        /**
        * @param first this reference
        * @param second user data
        * @return : true -> complete, false -> read more
        */
        typedef	bool(* type_cb)(cqitem_dev &,void*);


        /**
        * for cio_packet,callback type 
        * @param first this reference
        * @param second user data
        * @return
        *   first : true -> complete, false -> read more
        * 
        *   second : next reading request uid vector.
        */
        typedef	std::pair<bool, std::vector<size_t>>(*type_cb_for_packet)(cqitem_dev&, void*);

    public:
        cqitem_dev( const cqitem_dev& src )
        {
            _ini();
            *this = src;
        }
        cqitem_dev& operator=(const cqitem_dev& src)
        {
            if (this != &src) {
                m_v_tx = src.m_v_tx;
                m_v_rx = src.m_v_rx;

                m_cb = src.m_cb;
                m_cb_for_packet = src.m_cb_for_packet;

                m_p_user_for_cb = src.m_p_user_for_cb;
                m_result = src.m_result;
                m_s_info = src.m_s_info;
                m_n_session_number = src.m_n_session_number;

                m_n_uid = src.m_n_uid;
            }

            return *this;
        }

        cqitem_dev(size_t n_uid,const type_v_buffer & v_tx, bool b_need_read, type_cb cb, void* p_user_for_cb, unsigned long n_session_number)
        {
            _ini();
            m_v_tx = v_tx;
            m_b_need_read = b_need_read;
            m_cb = cb;
            m_cb_for_packet = nullptr;
            m_p_user_for_cb = p_user_for_cb;
            m_n_session_number = n_session_number;

            m_n_uid = n_uid;
        }

        cqitem_dev(size_t n_uid, const type_v_buffer& v_tx, bool b_need_read, type_cb_for_packet cb_for_packet, void* p_user_for_cb, unsigned long n_session_number)
        {
            _ini();
            m_v_tx = v_tx;
            m_b_need_read = b_need_read;
            m_cb = nullptr;
            m_cb_for_packet = cb_for_packet;
            m_p_user_for_cb = p_user_for_cb;
            m_n_session_number = n_session_number;

            m_n_uid = n_uid;
        }

        virtual ~cqitem_dev()
        {

        }

        cqitem_dev::type_request get_request_type()
        {
            cqitem_dev::type_request t(cqitem_dev::req_cancel);

            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_v_tx.empty() && m_b_need_read) {
                t = cqitem_dev::req_only_rx;
            }
            else if (!m_v_tx.empty() && m_b_need_read) {
                t = cqitem_dev::req_tx_rx;
            }
            else if (!m_v_tx.empty() && !m_b_need_read) {
                t = cqitem_dev::req_only_tx;
            }

            return t;
        }

        std::wstring get_request_type_by_string()
        {
            std::wstring r;

            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_v_tx.empty() && m_b_need_read) {
                r = L"req_only_rx";
            }
            else if (!m_v_tx.empty() && m_b_need_read) {
                r = L"req_tx_rx";
            }
            else if (!m_v_tx.empty() && !m_b_need_read) {
                r = L"req_only_tx";
            }
            else {
                r = L"req_cancel";
            }

            return r;
        }

        bool is_empty_tx() const
        {
            if (m_v_tx.size() == 0)
                return true;
            else
                return false;
        }
        bool is_need_rx() const
        {
            return m_b_need_read;
        }

        type_v_buffer get_tx() const
        {
            return m_v_tx;
        }
        type_v_buffer get_rx()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_v_rx;
        }

        unsigned long get_session_number() const
        {
            return m_n_session_number;
        }

        bool is_complete()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_result == cqitem_dev::result_not_yet) {
                return false;
            }
            return true;
        }
        cqitem_dev::type_result get_result()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_result;
        }

        std::wstring get_info()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_s_info;
        }

        /**
        * the sum of get_result(), get_rx() and  get_info()
        * @return first->get_result(), second -> get_rx(), third -> get_info()
        */
        std::tuple<cqitem_dev::type_result, type_v_buffer, std::wstring> get_result_all()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return std::make_tuple(m_result, m_v_rx, m_s_info);
        }

        /**
        * string version of get_result_all().
        * @return first-> strig of get_result(), second -> hex string of get_rx(), third -> get_info()
        */
        std::tuple<std::wstring, std::wstring, std::wstring> get_result_all_by_string()
        {
            auto result = cqitem_dev::get_result_all();
            std::wstring s[3];
            s[0] = cqitem_dev::get_result_string(std::get<0>(result));
            cconvert::hex_string_from_binary(s[1], std::get<1>(result));
            s[2] = std::get<2>(result);
            return std::make_tuple(s[0], s[1], s[2]);
        }

        cqitem_dev& set_result(cqitem_dev::type_result result, const type_v_buffer& v_rx, const std::wstring& s_info = L"")
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_result = result;
            m_v_rx = v_rx;
            m_s_info = s_info;
            return *this;
        }

        void reset_result()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_result = cqitem_dev::result_not_yet;
            m_v_rx.resize(0);
            m_s_info.clear();
        }

        /**
        * @brief execute callback
        * @return : true -> complete, false -> read more
        */

        bool run_callback()
        {
            if (!m_cb) {
                return true;//No need repeat reading
            }
            return m_cb(*this, m_p_user_for_cb);
        }

        /**
        * @brief execute callback for packet
        * @return
        *   first : true -> complete, false -> read more
        * 
        *   second : request which need more reading.
        * 
        *   if first is false, second is always std::vector<size_t>().
        * 
        *   else second is std::vector<size_t>()(stop reading, all complete) 
        *
        *   or the next reading request uid vector( the current complete, need reading for the next req).
        */

        std::pair<bool, std::vector<size_t>> run_callback_for_packet()
        {
            if (!m_cb_for_packet) {
                return std::make_pair(true, std::vector<size_t>());//No need repeat reading
            }
            return m_cb_for_packet(*this, m_p_user_for_cb);
        }

        /**
        * @brief get unique id
        * @return unique id of this Q item
        */
        size_t get_uid() const
        {
            return m_n_uid;
        }
    protected:
        void _ini()
        {
            m_v_tx.resize(0);
            m_v_rx.resize(0);
            m_b_need_read = false;
            m_cb = nullptr;
            m_cb_for_packet = nullptr;
            m_p_user_for_cb = nullptr;
            m_result = cqitem_dev::result_not_yet;
            m_s_info.clear();
            m_n_session_number = 0;

            m_n_uid = 0;
        }

    protected:
        //NO need muetx
        type_v_buffer m_v_tx;//set by constrcuture
        bool m_b_need_read;
        cqitem_dev::type_cb m_cb;//set by constrcuture
        cqitem_dev::type_cb_for_packet m_cb_for_packet;//set by constrcuture

        void* m_p_user_for_cb;//set by constrcuture
        unsigned long m_n_session_number;//set by constrcuture

        std::mutex m_mutex;
        type_v_buffer m_v_rx;
        cqitem_dev::type_result m_result;
        std::wstring m_s_info;

        size_t m_n_uid; //unique id 

    private:
        cqitem_dev() = delete;

    public://static helper fuctions
        static std::wstring get_result_string(cqitem_dev::type_result result)
        {
            std::wstring s = L"";
            switch (result) {
            case cqitem_dev::result_not_yet:
                s = L"result is not yet"; break;
            case cqitem_dev::result_success:
                s = L"result is success"; break;
            case cqitem_dev::result_error:
                s = L"result is error"; break;
            case cqitem_dev::result_cancel:
                s = L"result is cancel"; break;
            default:
                break;
            }//end switch
            return s;
        }


    };

    
}