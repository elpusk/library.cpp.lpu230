#pragma once
//base lib
#include <algorithm>
#include <cstdlib>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#ifdef max
#error	max macro has been defined already.(may be windows.h). max will be defined in boost::asio library.\
	insert #undef max to the last line of stdafx.h
#endif // max

#ifdef min
#error	min macro has been defined already.(may be windows.h). min will be defined in boost::asio library.\
	insert #undef min to the last line of stdafx.h
#endif // min
#endif

//3'th part lib
#include <boost/asio.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <openssl/x509_vfy.h>


//my lib
#include <mp_type.h>
#include <mp_cstring.h>

#ifdef _WIN32
#pragma comment (lib, "crypt32")

// two static library
#pragma comment(lib, "libcrypto.lib")
#pragma comment(lib, "libssl.lib")
#endif

#define	_WS_TOOLS_SERVER_NAME		"elpusk.server.websocket.coffee.manager"
#define	_WS_TOOLS_SERVER_PROTOCOL	"elpusk.protocol.coffee.manager"

namespace _mp {
	namespace _ws_tools
	{
		enum {
			SERVER_PORT_VIRTUAL_ICC = 7050,
			SERVER_PORT_COFFEE_MANAGER = 7100,
			WEBSOCKET_SERVER_PORT_COFFEE_MANAGER = 80,
			WEBSOCKET_SECURITY_SERVER_PORT_COFFEE_MANAGER = 443,
			SIZE_BUFFER_IO_SERVER = 2048
		};

		static boost::asio::ip::address get_local_ip4()
		{
			boost::asio::ip::address ip4;
			boost::asio::io_service io_service;

			boost::asio::ip::tcp::resolver resolver(io_service);

			boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), "");
			boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

			while (it != boost::asio::ip::tcp::resolver::iterator()) {
				boost::asio::ip::address addr = (it++)->endpoint().address();
				if (!addr.is_v4())
					continue;
				if (addr.to_v4().is_loopback())
					continue;
				if (addr.to_v4().is_multicast())
					continue;
				if (addr.to_v4().is_unspecified())
					continue;

				ip4 = addr;
				break;
			}//end while
			return ip4;
		}
		static boost::asio::ip::address get_local_ip6()
		{
			boost::asio::ip::address ip6;
			boost::asio::io_service io_service;

			boost::asio::ip::tcp::resolver resolver(io_service);

			boost::asio::ip::tcp::resolver::query query(boost::asio::ip::host_name(), "");
			boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);

			while (it != boost::asio::ip::tcp::resolver::iterator()) {
				boost::asio::ip::address addr = (it++)->endpoint().address();

				if (!addr.is_v6())
					continue;
				if (addr.to_v6().is_link_local())
					continue;
				if (addr.to_v6().is_loopback())
					continue;
				if (addr.to_v6().is_multicast())
					continue;
				//if (addr.to_v6().is_site_local())
				//	continue;
				if (addr.to_v6().is_unspecified())
					continue;
				ip6 = addr;
				break;
			}//end while
			return ip6;
		}
	}
}
