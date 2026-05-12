#pragma once
#include <map>
#include <set>
#include <utility>
#include <xmllite.h>
#include <string>
#include <list>
#include <set>
#include <memory>
#include <algorithm>
#include <fstream>
#include <functional>

#include <windows.h>
#include <tchar.h>
#include <atlbase.h>

#include <tools.h>
#include <ct_type.h>
#include <ct_convert.h>
#include <ct_file.h>


#pragma comment(lib,"Xmllite.lib")

class cxml_list
{
public:

	typedef	std::map<std::wstring, _ns_tools::ct_convert::type_value_type>		type_map_attribute_type;
	typedef	std::pair<std::wstring, _ns_tools::ct_convert::type_value_type>		type_pair_attribute_type;

	class celement_format {
	private:
		std::wstring m_s_name;
		_ns_tools::ct_convert::type_value_type m_type;

		type_map_attribute_type m_map_atttibute_type;
		std::list< std::shared_ptr<celement_format> >		m_list_sub_ptr_element_format;

	public:
		celement_format & set_name(const std::wstring & s_name)
		{
			m_s_name = s_name;
			return *this;
		}

		std::wstring get_name()
		{
			return m_s_name;
		}

		_ns_tools::ct_convert::type_value_type get_type()
		{
			return m_type;
		}
		bool save_to_file(std::ofstream & o_file)
		{
			bool b_result(false);

			do {
				if (!o_file.is_open())
					continue;
				//
				b_result = true;

				size_t n_size(0);

				_ns_tools::ct_file::save_to_file(o_file, m_s_name);
				_ns_tools::ct_file::save_to_file(o_file, m_type);
				_ns_tools::ct_file::save_to_file(o_file, m_map_atttibute_type.size());

				type_map_attribute_type::iterator it_map = begin(m_map_atttibute_type);

				for (; it_map != end(m_map_atttibute_type); ++it_map) {
					_ns_tools::ct_file::save_to_file(o_file, it_map->first);
					_ns_tools::ct_file::save_to_file(o_file, it_map->second);
				}//end for
				
				_ns_tools::ct_file::save_to_file(o_file, m_list_sub_ptr_element_format.size());

				std::list< std::shared_ptr<celement_format> >::iterator it = begin(m_list_sub_ptr_element_format);

				for (;it != end(m_list_sub_ptr_element_format); ++it) {
					b_result = (*it)->save_to_file(o_file);
				}//end for
				
			} while (false);
			return b_result;
		}

		bool load_from_file(std::ifstream & i_file)
		{
			bool b_result(false);

			do {
				if (!i_file.is_open())
					continue;
				//
				b_result = true;
				size_t n_map_atttibute_type(0);

				_ns_tools::ct_file::load_from_file(m_s_name, i_file);
				_ns_tools::ct_file::load_from_file(m_type, i_file);

				_ns_tools::ct_file::load_from_file(n_map_atttibute_type, i_file);

				size_t i(0);
				std::wstring s_attribute_name;
				_ns_tools::ct_convert::type_value_type type;

				for (i = 0; i < n_map_atttibute_type; i++) {
					_ns_tools::ct_file::load_from_file(s_attribute_name, i_file);
					_ns_tools::ct_file::load_from_file(type, i_file);
					add_attribute(s_attribute_name, type);
				}//end for

				size_t n_sub_ptr_element(0);
				_ns_tools::ct_file::load_from_file(n_sub_ptr_element, i_file);

				for (i = 0; i < n_sub_ptr_element; i++) {
					type_ptr_element_format ptr_new_element =
						std::shared_ptr<celement_format>(
							new celement_format()
							);

					m_list_sub_ptr_element_format.push_back(ptr_new_element);

					b_result = ptr_new_element->load_from_file(i_file);
				}//end for
				//
			} while (false);
			return b_result;
		}
		celement_format() : m_type(_ns_tools::ct_convert::value_type_none)
		{
		}
		celement_format(const std::wstring & s_name, _ns_tools::ct_convert::type_value_type type) : m_s_name(s_name), m_type(type)
		{
		}
		celement_format(const std::wstring & s_name) : m_s_name(s_name), m_type(_ns_tools::ct_convert::value_type_none)
		{
		}

		celement_format & operator=(const celement_format & src)
		{
			this->m_s_name = src.m_s_name;
			this->m_type = src.m_type;
			this->m_map_atttibute_type = src.m_map_atttibute_type;
			this->m_list_sub_ptr_element_format = src.m_list_sub_ptr_element_format;
			return *this;
		}

		std::shared_ptr<celement_format> find_in_sub_element_format(const std::wstring &s_name) const
		{
			std::shared_ptr<celement_format> ptr_found;

			do {
				if (m_list_sub_ptr_element_format.empty())
					continue;

				std::list< std::shared_ptr<celement_format> >::const_iterator it = begin(m_list_sub_ptr_element_format);
				for (; it != end(m_list_sub_ptr_element_format); ++it) {
					if ((*it)->m_s_name == s_name) {
						ptr_found = *it;
						break;
					}
				}//end for
			} while (false);
			return ptr_found;
		}

		_ns_tools::ct_convert::type_value_type get_attribute_type(const std::wstring & attribute_name) const
		{
			_ns_tools::ct_convert::type_value_type type(_ns_tools::ct_convert::value_type_none);

			do {
				if (attribute_name.empty())
					continue;
				//
				type_map_attribute_type::const_iterator it = m_map_atttibute_type.find(attribute_name);
				if (it == end(m_map_atttibute_type))
					continue;
				type = it->second;
			} while (false);
			return type;
		}
	
		void add_attribute(const std::wstring & s_attribute_name, _ns_tools::ct_convert::type_value_type type = _ns_tools::ct_convert::value_type_none)
		{
			do {
				if (s_attribute_name.empty())
					continue;
				m_map_atttibute_type[s_attribute_name] = type;
			} while (false);
		}
	
		std::shared_ptr<celement_format> add_sub_element(const std::wstring & s_element_name, _ns_tools::ct_convert::type_value_type type = _ns_tools::ct_convert::value_type_none)
		{
			type_ptr_element_format ptr_new_element =
				std::shared_ptr<celement_format>(
					new celement_format(s_element_name, type)
				);

			m_list_sub_ptr_element_format.push_back( ptr_new_element );
			return ptr_new_element;
		}
	public:
		bool empty()
		{
			bool b_result(true);
			do {
				if (m_s_name.empty())
					continue;
				//
				b_result = false;
			} while (false);
			return b_result;
		}
		void clear()
		{
			m_s_name.clear();
			m_type = _ns_tools::ct_convert::value_type_none;
			m_map_atttibute_type.clear();
			m_list_sub_ptr_element_format.clear();
		}
	};
	typedef	std::shared_ptr<celement_format>			type_ptr_element_format;
	//
	typedef	std::map<std::wstring, _ns_tools::ct_convert::type_pair_type_string> type_map_atttibute;//attribue & value.

	class celement{
	private:
		std::wstring m_s_name;
		_ns_tools::ct_convert::type_value_type m_type;
		std::wstring m_s_data;

		type_map_atttibute m_map_atttibute;
		std::list< std::shared_ptr<celement> >		m_list_sub_ptr_element;

		std::list< std::shared_ptr<celement> >::const_iterator m_iter_found_element;//m_list_sub_ptr_element iterator for multi-found.

	public:
		void print(_ns_tools::type_list_wstring & list_info)
		{
			do {
				if (empty())
					continue;
				std::wstring s_info(m_s_name);
				s_info += L" : ";
				s_info += _ns_tools::ct_convert::get_type_string(m_type);
				s_info += L" : ";
				s_info += m_s_data;
				list_info.push_back(s_info);
				list_info.push_back(L"attribute");
				
				//
				type_map_atttibute::iterator it = begin(m_map_atttibute);
				for (; it != end(m_map_atttibute); ++it) {
					s_info = it->first;
					s_info += L" : ";
					s_info += _ns_tools::ct_convert::get_type_string(it->second.first);
					s_info += L" : ";
					s_info += it->second.second;
					list_info.push_back(s_info);
				}//end for
				//
				std::list< std::shared_ptr<celement> >::iterator it_list = begin(m_list_sub_ptr_element);
				for (; it_list != end(m_list_sub_ptr_element); ++it_list) {
					(*it_list)->print(list_info);
				}//end for
			} while (false);
		}
		bool save_to_file(std::ofstream & o_file)
		{
			bool b_result(false);

			do {
				if (!o_file.is_open())
					continue;
				//
				b_result = true;

				_ns_tools::ct_file::save_to_file(o_file, m_s_name);
				_ns_tools::ct_file::save_to_file(o_file, m_type);
				_ns_tools::ct_file::save_to_file(o_file, m_s_data);

				_ns_tools::ct_file::save_to_file(o_file, m_map_atttibute.size());

				type_map_atttibute::iterator it_map = begin(m_map_atttibute);
				for (; it_map != end(m_map_atttibute); ++it_map) {
					_ns_tools::ct_file::save_to_file(o_file, it_map->first);//name
					_ns_tools::ct_file::save_to_file(o_file, it_map->second.first);//type
					_ns_tools::ct_file::save_to_file(o_file, it_map->second.second);//value
				}//end for

				_ns_tools::ct_file::save_to_file(o_file, m_list_sub_ptr_element.size());

				std::list< std::shared_ptr<celement> >::iterator it = begin(m_list_sub_ptr_element);

				for (;it != end(m_list_sub_ptr_element); ++it) {
					b_result = (*it)->save_to_file(o_file);
				}//end for
				
			} while (false);
			return b_result;
		}

		bool load_from_file(ifstream & i_file)
		{
			bool b_result(false);

			do {
				if (!i_file.is_open())
					continue;

				clear();
				//
				b_result = true;
				size_t n_map_atttibute(0);
				_ns_tools::ct_file::load_from_file(m_s_name, i_file);
				_ns_tools::ct_file::load_from_file(m_type, i_file);
				_ns_tools::ct_file::load_from_file(m_s_data, i_file);
				_ns_tools::ct_file::load_from_file(n_map_atttibute, i_file);

				size_t i(0);
				std::wstring s_attribute_name;
				_ns_tools::ct_convert::type_value_type type;
				std::wstring s_attribute_value;

				for (i = 0; i < n_map_atttibute; i++) {
					_ns_tools::ct_file::load_from_file(s_attribute_name, i_file);
					_ns_tools::ct_file::load_from_file(type, i_file);
					_ns_tools::ct_file::load_from_file(s_attribute_value, i_file);
					m_map_atttibute[s_attribute_name] = make_pair(type, s_attribute_value);
				}//end for

				size_t n_sub_ptr_element(0);
				_ns_tools::ct_file::load_from_file(n_sub_ptr_element, i_file);

				for (i = 0; i < n_sub_ptr_element; i++) {
					type_ptr_element ptr_new_element = type_ptr_element(new celement());
					add_sub_element(ptr_new_element);
					b_result = ptr_new_element->load_from_file(i_file);
				}//end for

				m_iter_found_element = m_list_sub_ptr_element.begin();
				//
			} while (false);
			return b_result;
		}

		celement & add_attribute(const std::wstring & s_attribute, _ns_tools::ct_convert::type_value_type type, const std::wstring & s_value)
		{
			m_map_atttibute[s_attribute] = make_pair(type, s_value);
			return *this;
		}

		celement & add_sub_element(std::shared_ptr<celement> & ptr_new_element)
		{
			m_list_sub_ptr_element.push_back(ptr_new_element);
			return *this;
		}
		celement & set_name(const std::wstring & s_name)
		{
			m_s_name = s_name;
			return *this;
		}
		celement & set_type(_ns_tools::ct_convert::type_value_type type)
		{
			m_type = type;
			return *this;
		}
		celement & set_data(const std::wstring & s_value)
		{
			m_s_data = s_value;
			return *this;
		}

		std::wstring get_name()
		{
			return m_s_name;
		}
		_ns_tools::ct_convert::type_value_type get_type()
		{
			return m_type;
		}

		bool get_attribute(std::wstring & s_data,const std::wstring & attribute_name) const
		{
			bool b_result(false);

			do {
				if (attribute_name.empty())
					continue;
				if (m_map_atttibute.empty())
					continue;

				type_map_atttibute::const_iterator it = m_map_atttibute.find(attribute_name);
				if (it == end(m_map_atttibute))
					continue;
				b_result = _ns_tools::ct_convert::get_data(s_data, it->second.second, it->second.first);
			} while (false);
			return b_result;
		}
		bool get_attribute(int & n_data,const std::wstring & attribute_name) const
		{
			bool b_result(false);

			do {
				if (attribute_name.empty())
					continue;
				if (m_map_atttibute.empty())
					continue;

				type_map_atttibute::const_iterator it = m_map_atttibute.find(attribute_name);
				if (it == end(m_map_atttibute))
					continue;
				b_result = _ns_tools::ct_convert::get_data(n_data, it->second.second, it->second.first);
			} while (false);
			return b_result;
		}
		bool get_attribute(_ns_tools::type_v_buffer & v_data,const std::wstring & attribute_name) const
		{
			bool b_result(false);

			do {
				if (attribute_name.empty())
					continue;
				if (m_map_atttibute.empty())
					continue;

				type_map_atttibute::const_iterator it = m_map_atttibute.find(attribute_name);
				if (it == end(m_map_atttibute))
					continue;
				b_result = _ns_tools::ct_convert::get_data(v_data, it->second.second, it->second.first);
			} while (false);
			return b_result;
		}
		bool get_attribute(bool & b_data,const std::wstring & attribute_name) const
		{
			bool b_result(false);

			do {
				if (attribute_name.empty())
					continue;
				if (m_map_atttibute.empty())
					continue;

				type_map_atttibute::const_iterator it = m_map_atttibute.find(attribute_name);
				if (it == end(m_map_atttibute))
					continue;
				b_result = _ns_tools::ct_convert::get_data(b_data, it->second.second, it->second.first);
			} while (false);
			return b_result;
		}

		std::shared_ptr<celement> find_in_sub_element(const std::wstring &s_name,bool b_first_search = true)
		{
			std::shared_ptr<celement> ptr_found;

			do {
				if (m_list_sub_ptr_element.empty())
					continue;
				if(b_first_search )
					m_iter_found_element = begin(m_list_sub_ptr_element);

				for (; m_iter_found_element != end(m_list_sub_ptr_element); ) {
					if ((*m_iter_found_element)->m_s_name == s_name) {
						ptr_found = *m_iter_found_element;
						++m_iter_found_element;
						break;
					}
					++m_iter_found_element;
				}//end for
			} while (false);
			return ptr_found;
		}

		bool empty() const
		{
			return m_s_name.empty();
		}
		void clear()
		{
			m_s_name.clear();
			m_type = _ns_tools::ct_convert::value_type_none;
			m_s_data.clear();

			m_map_atttibute.clear();
			m_list_sub_ptr_element.clear();
		}

		bool get_data(std::wstring & s_data)
		{
			return _ns_tools::ct_convert::get_data(s_data, m_s_data, m_type);
		}

		bool get_data(int & n_data)
		{
			return _ns_tools::ct_convert::get_data(n_data, m_s_data, m_type);
		}

		bool get_data(_ns_tools::type_v_buffer & v_data)
		{
			return _ns_tools::ct_convert::get_data(v_data, m_s_data, m_type);
		}

		bool get_data(bool & b_data)
		{
			return _ns_tools::ct_convert::get_data(b_data, m_s_data, m_type);
		}
	};

	typedef	std::shared_ptr<celement>		type_ptr_element;
	typedef	std::map<std::wstring, cxml_list::type_ptr_element>		type_map_string_ptr_element;
	typedef	std::pair<std::wstring, cxml_list::type_ptr_element>	type_pair_string_ptr_element;

public:
	void print(_ns_tools::type_list_wstring & list_info)
	{
		do {
			if (empty())
				continue;
			//
			m_root_elements.print(list_info);
		} while (false);
	}

	bool load_dat(const std::wstring & s_file_format_and_data)
	{
		bool b_result(false);

		do {
			if (s_file_format_and_data.empty())
				continue;
			ifstream i_file;
			i_file.open(s_file_format_and_data, std::wifstream::in | std::wifstream::binary);
			if (!i_file.is_open())
				continue;
			//
			clear();

			if (!m_elements_format.load_from_file(i_file)) {
				i_file.close();
				continue;
			}
			if (!m_root_elements.load_from_file(i_file)) {
				i_file.close();
				continue;
			}

			i_file.close();
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	bool save_dat(const std::wstring & s_file_format_and_data)
	{
		bool b_result(false);

		do {
			if (s_file_format_and_data.empty())
				continue;
			std::ofstream o_file;
			o_file.open(s_file_format_and_data, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
			if (!o_file.is_open())
				continue;
			//
			if (!m_elements_format.save_to_file(o_file)) {
				o_file.close();
				continue;
			}
			if (!m_root_elements.save_to_file(o_file)) {
				o_file.close();
				continue;
			}

			o_file.flush();
			o_file.close();
			//
			b_result = true;
		} while (false);
		return b_result;
	}

	cxml_list()
	{
	}
	virtual ~cxml_list()
	{
	}

	cxml_list(const celement_format & elements_format) : m_elements_format(elements_format)
	{
	}

	void clear()
	{
		m_elements_format.clear();
		reset_contents();
	}
	void reset_contents()
	{
		m_root_elements.clear();
	}

	void load_format(const celement_format &format)
	{
		reset_contents();
		m_elements_format.clear();
		m_elements_format = format;
	}

	const celement & get_root_element()
	{
		return m_root_elements;
	}

	bool load_xml(const std::wstring & s_xml_path)
	{
		bool b_result(false);
		ATL::CComPtr<IStream> stream;
		ATL::CComPtr<IXmlReader> pReader;
		HRESULT hr(0);
		//
		XmlNodeType nodeType;

		const wchar_t *pwszPrefix;
		UINT cwchPrefix;
		const wchar_t *pwszLocalName;
		UINT nAttrCount;
		const wchar_t *pwszValue;

		std::wstring sElement;

		celement *p_cur_elements = &m_root_elements;
		celement_format *p_cur_elements_format = &m_elements_format;

		do {
			if (s_xml_path.empty())
				continue;
			if (m_elements_format.empty())
				continue;
			//
			hr = SHCreateStreamOnFile(s_xml_path.c_str(), STGM_READ, &stream);
			if (FAILED(hr))	continue;
			hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL);
			if (FAILED(hr))	continue;
			hr = pReader->SetProperty(XmlReaderProperty_DtdProcessing, DtdProcessing_Prohibit);
			if (FAILED(hr))	continue;
			hr = pReader->SetInput(stream);
			if (FAILED(hr))	continue;
			//
			b_result = true;
			bool b_exit(false);

			// Read through each node until the end
			while (!pReader->IsEOF() && !b_exit) {

				hr = pReader->Read(&nodeType);

				// Check if E_PENDING is returned and perform custom action
				// This is a sample of how one might handle E_PENDING
				if (hr == E_PENDING) {
					// As long as E_PENDING is returned keep trying to read
					while (hr == E_PENDING) {
						Sleep(500);
						hr = pReader->Read(&nodeType);
					}
					continue;
				}

				if (hr != S_OK)
					break;

				switch (nodeType) {
				case XmlNodeType_Element:
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) {
						b_result = false;
						break;
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
						b_result = false;
						break;
					}

					if (cwchPrefix > 0) {
						ATLTRACE(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
					}
					else {
						ATLTRACE(L"Element: %s\n", pwszLocalName);
					}

					sElement = pwszLocalName;
					if (p_cur_elements_format->get_name() != sElement) {
						b_result = false;
						break;
					}

					p_cur_elements->set_name(p_cur_elements_format->get_name()).set_type(p_cur_elements_format->get_type());

					pReader->GetAttributeCount(&nAttrCount);
					if (nAttrCount > 0) {

						if (FAILED(hr = _write_attributes(pReader, p_cur_elements, p_cur_elements_format))) {
							b_result = false;
							break;
						}
						else {
							//
							//conf.addRequest(item);
						}
						if (FAILED(hr = pReader->MoveToElement())) {
							b_result = false;
							break;
						}
					}

					if (pReader->IsEmptyElement()) {
						//ATLTRACE(L" (empty celement)\n");
					}

					if (!_load_xml_file(pReader, p_cur_elements, p_cur_elements_format)) {
						b_exit = true;
					}
					break;
				case XmlNodeType_EndElement:
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) {
						b_result = false;
						break;
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
						b_result = false;
						break;
					}

					if (cwchPrefix > 0)
						ATLTRACE(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
					else
						ATLTRACE(L"End Element: %s\n", pwszLocalName);
					//
					b_exit = true;
					break;
				case XmlNodeType_Text:
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))){
						b_result = false;
						break;
					}
					p_cur_elements->set_data(std::wstring(pwszValue) );
					ATLTRACE(L"Text: %s\n", pwszValue);
					break;
				case XmlNodeType_Whitespace:
				case XmlNodeType_CDATA:
				case XmlNodeType_ProcessingInstruction:
				case XmlNodeType_Comment:
				case XmlNodeType_XmlDeclaration:
				case XmlNodeType_DocumentType:
					break;
				default:
					b_result = false;
					break;
				}//end switch

				if (!b_result)
					break;//exit while
			}//end while
		} while (false);

		return b_result;
	}

	bool empty() const
	{
		return m_root_elements.empty();
	}
protected:
	
	bool _load_xml_file(
		ATL::CComPtr<IXmlReader> pReader
		, celement *p_elements
		, const celement_format *p_elements_format
	)
	{
		bool b_result(false);
		HRESULT hr(0);
		//
		XmlNodeType nodeType;

		const wchar_t *pwszPrefix;
		UINT cwchPrefix;
		const wchar_t *pwszLocalName;
		UINT nAttrCount;
		const wchar_t *pwszValue;

		std::wstring sElement;
		type_ptr_element_format ptr_xml_element_format;
		type_ptr_element ptr_new_element;

		bool b_end_element(false);

		do {
			if (pReader == nullptr || p_elements == nullptr || p_elements_format==nullptr)
				continue;
			//
			b_result = true;

			// Read through each node until the end
			while (!pReader->IsEOF() && !b_end_element ) {

				hr = pReader->Read(&nodeType);

				// Check if E_PENDING is returned and perform custom action
				// This is a sample of how one might handle E_PENDING
				if (hr == E_PENDING) {
					// As long as E_PENDING is returned keep trying to read
					while (hr == E_PENDING) {
						Sleep(500);
						hr = pReader->Read(&nodeType);
					}
					continue;
				}

				if (hr != S_OK)
					break;

				switch (nodeType) {
				case XmlNodeType_Element:
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) {
						b_result = false;
						break;
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
						b_result = false;
						break;
					}

					if (cwchPrefix > 0) {
						ATLTRACE(L"Element: %s:%s\n", pwszPrefix, pwszLocalName);
					}
					else {
						ATLTRACE(L"Element: %s\n", pwszLocalName);
					}

					sElement = pwszLocalName;

					ptr_xml_element_format = p_elements_format->find_in_sub_element_format(sElement);
					if(ptr_xml_element_format==nullptr){
						b_result = false;
						break;
					}

					ptr_new_element = type_ptr_element(new celement());
					p_elements->add_sub_element(ptr_new_element);
					ptr_new_element->set_name(ptr_xml_element_format->get_name()).set_type(ptr_xml_element_format->get_type());

					pReader->GetAttributeCount(&nAttrCount);
					if (nAttrCount > 0) {

						if (FAILED(hr = _write_attributes(pReader, ptr_new_element.get(), ptr_xml_element_format.get()))) {
							b_result = false;
							break;
						}
						else {
							//
							//conf.addRequest(item);
						}
						if (FAILED(hr = pReader->MoveToElement())) {
							b_result = false;
							break;
						}
					}

					if (pReader->IsEmptyElement()) {
						ATLTRACE(L" (empty celement)\n");
					}
					if (!_load_xml_file(pReader, ptr_new_element.get(), ptr_xml_element_format.get())) {
						b_result = false;
					}
					break;
				case XmlNodeType_EndElement:
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) {
						b_result = false;
						break;
					}
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) {
						b_result = false;
						break;
					}

					if (cwchPrefix > 0) {
						ATLTRACE(L"End Element: %s:%s\n", pwszPrefix, pwszLocalName);
					}
					else {
						ATLTRACE(L"End Element: %s\n", pwszLocalName);
					}
					//
					b_end_element = true;
					break;
				case XmlNodeType_Text:
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) {
						b_result = false;
						break;
					}
					p_elements->set_data(std::wstring(pwszValue) );
					ATLTRACE(L"Text: %s\n", pwszValue);
					break;
				case XmlNodeType_Whitespace:
				case XmlNodeType_CDATA:
				case XmlNodeType_ProcessingInstruction:
				case XmlNodeType_Comment:
				case XmlNodeType_XmlDeclaration:
				case XmlNodeType_DocumentType:
					break;
				default:
					b_result = false;
					break;
				}//end switch

				if (!b_result)
					break;//exit while
			}//end while
		} while (false);

		return b_result;
	}

	HRESULT _write_attributes(IXmlReader *pReader, celement *p_elements, const celement_format *p_element_format)
	{
		const wchar_t * pwszPrefix;
		const wchar_t * pwszLocalName;
		const wchar_t * pwszValue;

		std::wstring sAttribute;
		std::wstring sValue;
		_ns_tools::ct_convert::type_value_type type(_ns_tools::ct_convert::value_type_none);

		HRESULT hr = pReader->MoveToFirstAttribute();

		if (S_FALSE == hr)
			return hr;

		if (S_OK != hr) {
			// This is a sample of how one might handle E_PENDING, E_PENDING 
			//if(PENDING(pReader->MoveToNextAttribute())){
			if (pReader->MoveToNextAttribute() == E_PENDING) {
				while (hr == E_PENDING) {
					Sleep(1000);
					hr = pReader->MoveToNextAttribute();
				}// end while
			}
			else { return S_FALSE; }
		}
		else {
			while (TRUE) {
				if (!pReader->IsDefault()) {
					UINT cwchPrefix;
					if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, &cwchPrefix))) { return S_FALSE; }
					if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL))) { return S_FALSE; }
					if (FAILED(hr = pReader->GetValue(&pwszValue, NULL))) { return S_FALSE; }

					if (cwchPrefix > 0) {
						ATLTRACE(L"Attr: %s:%s=\"%s\" \n", pwszPrefix, pwszLocalName, pwszValue);
					}
					else {
						ATLTRACE(L"Attr: %s=\"%s\" \n", pwszLocalName, pwszValue);
					}

					sAttribute = pwszLocalName;		sValue = pwszValue;
					type = p_element_format->get_attribute_type(sAttribute);

					if (type != _ns_tools::ct_convert::value_type_none) {
						p_elements->add_attribute(sAttribute,type, sValue);
					}
				}

				if (S_OK != pReader->MoveToNextAttribute())
					break;
			}//end while
		}
		return hr;
	}

	celement_format m_elements_format;
	celement m_root_elements;	//the parsing attribute will be saved in here.

private:
	// don't call these methods.
	cxml_list(const cxml_list &);
	cxml_list & operator=(const cxml_list &);

};

