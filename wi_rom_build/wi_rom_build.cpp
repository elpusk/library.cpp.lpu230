// wi_rom_build.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <cstdint>
#include <string>
#include <vector>
#include <wchar.h>
#include <iostream>
#include <iomanip> // for std::setw and std::setfill

#include <mp_cstring.h>
#include <inc/tg_rom.h>


//////////////////////////////////
// local function prototype
static bool SetParameter(int argc, char* argv[]);
static void MessageDisplayParameter();
static void DisplayUsing();
static bool SetModel(const wchar_t* sModel);
static bool SetVersion(const wchar_t* sVersion);
static bool SetCondition(const wchar_t* sCondition);

static void DisplayHeader(const CRom::ROMFILE_HEAD& header);

static std::wstring& GetResultString(const CRom::type_result result);


enum CMD {
	CMD_CREATE,	// c
	CMD_ADD,		// a
	CMD_LOAD,		// r
	CMD_EXTRACT	// e
};

typedef struct TagParameter {
	CMD Cmd;
	std::wstring sRomFileName;
	std::wstring sBinaryFileName;
	uint8_t cMajor;
	uint8_t cMinor;
	uint8_t cFix;
	uint8_t cBuild;
	uint8_t sModel[CRom::MAX_MODEL_NAME_SIZE + 1];
	uint32_t dwCondition;

} Parameter, * PParamter;

Parameter gPara;


int main(int argc, char* argv[])
{
	std::wcout << L" Welcom \"ROM file builder\" tool.\n";
	std::wcout << L" 2016.6.10 yss tools\n";
	std::wcout << L" ================================\n";

	// load command line parameters
	if (!SetParameter(argc, argv)) {
		DisplayUsing();
		return 0;
	}

	// display loaded command line parameters
	MessageDisplayParameter();

	//load rom helper library
	CRom rom(L"tg_rom.dll");
	CRom::type_result result;
	CRom::ROMFILE_HEAD header;
	bool bDisplayResult = true;

	// executes command from user.
	switch (gPara.Cmd) {
	case CMD_CREATE:	//create rom file.
		result = rom.CreateHeader(gPara.sRomFileName.c_str());
		break;
	case CMD_ADD:		//add firmware file to rom file.
		result = rom.LoadHeader(gPara.sRomFileName.c_str(), &header);
		if (result == CRom::result_success) {
			result = rom.AddItem(gPara.sBinaryFileName.c_str(), gPara.cMajor, gPara.cMinor, gPara.cFix, gPara.cBuild, gPara.sModel, gPara.dwCondition);
		}
		break;
	case CMD_LOAD:	//display rom file information
		result = rom.LoadHeader(gPara.sRomFileName.c_str(), &header);
		if (result == CRom::result_success) {
			DisplayHeader(header);
			bDisplayResult = false;
		}
		break;
	case CMD_EXTRACT:
		break;
	default:
		bDisplayResult = false;
		break;
	}//end switch

	if (bDisplayResult) {
		std::wcout << L" * PROCESSING RESULT :: " << GetResultString(result) << std::endl;
	}
	//
	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////////////
//
//
//
bool SetParameter(int argc, char* argv[])
{
	bool bResult = true;

	::memset(&gPara, 0, sizeof(gPara));

	if (argc < 3) {
		bResult = false;
		std::wcout << L" [ERROR] unacceptable Parameter. retry again.\n";
	}
	else {

		std::string sCmd;
		const wchar_t* p_data = nullptr;

		for (int i = 0; i < argc; i++) {
			//std::wcout << argv[i] << std::endl;
			switch (i) {
			case 1:
				sCmd = argv[i];

				if (sCmd == "/c") {
					gPara.Cmd = CMD_CREATE;
				}
				else if (sCmd == "/a") {
					gPara.Cmd = CMD_ADD;
				}
				else if (sCmd == "/r") {
					gPara.Cmd = CMD_LOAD;
				}
				else if (sCmd == "/e") {
					gPara.Cmd = CMD_EXTRACT;
				}
				else {
					bResult = false;
					std::wcout << L" [ERROR] unacceptable command. retry again.\n";
					return bResult;
				}
				break;
			case 2:
				switch (gPara.Cmd) {
				case CMD_CREATE:
					gPara.sRomFileName = _mp::cstring::get_unicode_from_mcsc(argv[i]);		break;
				case CMD_ADD:
					if (argc != 7) {
						bResult = false;
						std::wcout << L" [ERROR] unacceptable parameter of add binary command. retry again.\n";
						return bResult;
					}
					gPara.sRomFileName = _mp::cstring::get_unicode_from_mcsc(argv[i]);
					gPara.sBinaryFileName = _mp::cstring::get_unicode_from_mcsc(argv[i + 1]);

					p_data = _mp::cstring::get_unicode_from_mcsc(argv[i + 2]).c_str();
					if (!SetModel(p_data)) {
						std::wcout << L" [ERROR] unacceptable Model parameter. retry again.\n";
						bResult = false;
						return bResult;
					}

					p_data = _mp::cstring::get_unicode_from_mcsc(argv[i + 3]).c_str();
					if (!SetVersion(p_data)) {
						std::wcout << L" [ERROR] unacceptable version parameter. retry again.\n";
						bResult = false;
						return bResult;
					}

					p_data = _mp::cstring::get_unicode_from_mcsc(argv[i + 4]).c_str();
					if (!SetCondition(p_data)) {
						std::wcout << L" [ERROR] unacceptable condition parameter. retry again.\n";
						bResult = false;
						return bResult;
					}
					break;
				case CMD_LOAD:		gPara.sRomFileName = _mp::cstring::get_unicode_from_mcsc(argv[i]);		break;
				default:
					bResult = false;
					std::wcout << L" [ERROR] unacceptable command. retry again.\n";
					return bResult;
				}//end switch
				break;
			default:
				break;
			}//end switch



		}//end for
	}
	return bResult;
}

void DisplayUsing()
{
	//std::wcout << L"" << std::endl;
	std::wcout << L"======================================" << std::endl;
	std::wcout << L"   using : " << std::endl;
	std::wcout << L" 1. Create romfile." << std::endl;
	std::wcout << L" /c rom_file_name" << std::endl;
	std::wcout << L" ex) create test.rom file." << std::endl;
	std::wcout << L" tg_rom_build /c test.rom" << std::endl;
	std::wcout << std::endl;
	std::wcout << L" 2. add binary file to romfile" << std::endl;
	std::wcout << L" /a rom_file_name binary_file_name model_name version update_conition" << std::endl;
	std::wcout << L" ex) add k1.bin binary file to test.rom file." << std::endl;
	std::wcout << L" the version of k1.bin is 1.2.3.4" << std::endl;
	std::wcout << L" the model name of k1.bin is goodman" << std::endl;
	std::wcout << L" k1.bin will be upated when device firmware' version is less then equal to it." << std::endl;
	std::wcout << L" tg_rom_build /a test.rom k1.bin goodman 1.2.3.4 le" << std::endl;
	std::wcout << L" le : less then equal." << std::endl;
	std::wcout << L" ge : greater then equal." << std::endl;
	std::wcout << L" l : less then." << std::endl;
	std::wcout << L" g : greater then." << std::endl;
	std::wcout << L" e : equal." << std::endl;
	std::wcout << L" ne : not equal " << std::endl;
	std::wcout << std::endl;
	std::wcout << L" 3. Read romfile." << std::endl;
	std::wcout << L" /r rom_file_name" << std::endl;
	std::wcout << L" ex) read test.rom file. and displpay the information of test.rom." << std::endl;
	std::wcout << L" tg_rom_build /r test.rom" << std::endl;
	std::wcout << std::endl;
	std::wcout << L" 4. Extract pure binary firmware from rom file." << std::endl;
	std::wcout << L" /c rom_file_name binary_file_name firmware_zero_base_index_of_rom_file" << std::endl;
	std::wcout << L" ex) Extract the pure firmware from test.rom' index number 2. and It save to k1.bin." << std::endl;
	std::wcout << L" tg_rom_build /e test.rom k1.bin 2" << std::endl;

	std::wcout << L"======================================" << std::endl;
}

bool SetModel(const wchar_t* sModel)
{
	bool bResult = false;

	if (sModel == NULL)
		return bResult;
	//
	std::wstring smodel(sModel);

	if (smodel.length() > CRom::MAX_MODEL_NAME_SIZE)
		return bResult;
	//
	::memset(gPara.sModel, 0, sizeof(gPara.sModel));

	for (int i = 0; i < smodel.length(); i++) {
		gPara.sModel[i] = static_cast<uint8_t>(sModel[i]);
	}//end for

	bResult = true;

	return bResult;
}

bool SetVersion(const wchar_t* sVersion)
{
	bool bResult = false;

	if (sVersion == NULL)
		return bResult;
	//
	std::wstring sversion(sVersion);

	if (sversion.length() < 7)
		return bResult;
	//
	std::vector< int > vVersion;
	std::wstring::size_type pos, len;
	std::wstring::size_type found = sversion.find_first_of(L".");

	pos = len = 0;
	while (found != std::wstring::npos) {
		len = found - pos;
		vVersion.push_back(std::stoi(sversion.substr(pos, len)));
		pos = found + 1;
		found = sversion.find_first_of(L".", pos);

		if (found == std::wstring::npos && vVersion.size() == 3) {
			len = sversion.length() - pos;
			vVersion.push_back(std::stoi(sversion.substr(pos, len)));
		}
	}//end while

	if (vVersion.size() != 4)
		return bResult;
	//
	gPara.cMajor = static_cast<uint8_t>(vVersion[0]);
	gPara.cMinor = static_cast<uint8_t>(vVersion[1]);
	gPara.cFix = static_cast<uint8_t>(vVersion[2]);
	gPara.cBuild = static_cast<uint8_t>(vVersion[3]);
	//
	bResult = true;

	return bResult;
}

bool SetCondition(const wchar_t* sCondition)
{
	bool bResult = false;

	if (sCondition == NULL)
		return bResult;
	//
	gPara.dwCondition = 0;

	std::wstring scondition(sCondition);

	if (scondition == L"le") {
		gPara.dwCondition |= CRom::condition_lt;
		gPara.dwCondition |= CRom::condition_eq;
	}
	else if (scondition == L"ge") {
		gPara.dwCondition |= CRom::condition_gt;
		gPara.dwCondition |= CRom::condition_eq;
	}
	else if (scondition == L"l") {
		gPara.dwCondition |= CRom::condition_lt;
	}
	else if (scondition == L"g") {
		gPara.dwCondition |= CRom::condition_gt;
	}
	else if (scondition == L"e") {
		gPara.dwCondition |= CRom::condition_eq;
	}
	else if (scondition == L"ne") {
		gPara.dwCondition |= CRom::condition_neq;
	}
	else {
		return bResult;
	}
	//
	bResult = true;
	return bResult;
}

void MessageDisplayParameter()
{
	std::wcout << L" : CURRENT COMMAND  :: ";

	switch (gPara.Cmd) {
	case CMD_CREATE:
		std::wcout << L"CREATE ROM FILE";
		break;
	case CMD_ADD:
		std::wcout << L"ADD BINARY FILE TO ROM FILE";
		break;
	case CMD_LOAD:
		std::wcout << L"DISPLAY ROM FILE INFO";
		break;
	default:
		std::wcout << L"UNKNOWN";
		break;
	}//end switch
	std::wcout << std::endl;

	std::wcout << L" : ROM FILE NAME :: " << gPara.sRomFileName << std::endl;
	std::wcout << L" : BINARY FILE NAME :: " << gPara.sBinaryFileName << std::endl;;

	//
	std::wstring smodel;

	for (int i = 0; i < sizeof(gPara.sModel); i++) {
		if (gPara.sModel[i] == 0)
			break;//exit for

		smodel.push_back(static_cast<wchar_t>(gPara.sModel[i]));
	}//end for

	std::wcout << L" : MODEL  :: " << smodel << std::endl;;

	//
	std::wcout << L" : VERSION :: " << gPara.cMajor << L'.' << gPara.cMinor << L'.' << gPara.cFix << L'.' << gPara.cBuild << std::endl;

	//
	std::wcout << L" : UPDATE CONDITION :: ";

	if (gPara.dwCondition & CRom::condition_eq) {
		std::wcout << L"EQUAL,";
	}
	if (gPara.dwCondition & CRom::condition_gt) {
		std::wcout << L"GREATER,";
	}
	if (gPara.dwCondition & CRom::condition_lt) {
		std::wcout << L"LESS,";
	}
	if (gPara.dwCondition & CRom::condition_neq) {
		std::wcout << L"NOT EQUAL,";
	}
	std::wcout << std::endl;

}

void DisplayHeader(const CRom::ROMFILE_HEAD& header)
{
	//std::wcout << L"" <<std::endl;

	std::wcout << L"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;

	std::wcout << L" : header size :: " << header.dwHeaderSize << std::endl;
	std::wcout << L" : header format version :: " << header.sFormatVersion[0] << L'.' << header.sFormatVersion[1] << L'.' << header.sFormatVersion[2] << L'.' << header.sFormatVersion[3] << std::endl;
	std::wcout << L" : the number of binary files : " << header.dwItem << std::endl;
	std::wcout << L"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;

	std::wstring smodel;

	for (int i = 0; i < header.dwItem; i++) {

		smodel.clear();

		for (int j = 0; j < sizeof(header.Item[i].sModel); j++) {
			if (header.Item[i].sModel[j] == 0)
				break;//exit for

			smodel.push_back(static_cast<wchar_t>(header.Item[i].sModel[j]));
		}//end for

		//
		std::wcout << L"--------------------------------------------------------------------------" << std::endl;

		std::wcout << L" index :: " << i << std::endl;
		std::wcout << L" model :: " << smodel << std::endl;
		std::wcout << L" version :: " << header.Item[i].sVersion[0] << L'.' << header.Item[i].sVersion[1] << L'.' << header.Item[i].sVersion[2] << L'.' << header.Item[i].sVersion[3] << std::endl;
		std::wcout << L" offset :: " << header.Item[i].dwOffset << std::endl;
		std::wcout << L" size :: " << header.Item[i].dwSize << L"[byte]" << std::endl;

		std::wcout << L" update condition :: ";
		if (header.Item[i].dwUpdateCondition & CRom::condition_eq)
			std::wcout << L" equal ";
		if (header.Item[i].dwUpdateCondition & CRom::condition_gt)
			std::wcout << L" greater ";
		if (header.Item[i].dwUpdateCondition & CRom::condition_lt)
			std::wcout << L" less ";
		if (header.Item[i].dwUpdateCondition & CRom::condition_neq)
			std::wcout << L" not equal ";
		std::wcout << std::endl;
		//
		unsigned long dwVersion(0);
		unsigned char* pdwVersion = (unsigned char*)&dwVersion;

		pdwVersion[0] = header.sFormatVersion[3];
		pdwVersion[1] = header.sFormatVersion[2];
		pdwVersion[2] = header.sFormatVersion[1];
		pdwVersion[3] = header.sFormatVersion[0];

		if (dwVersion >= 0x01010000) {
			std::wcout << L" hash :: ";
			for (int k = 0; k < 32; k++) {
				std::cout << " " << std::uppercase << std::hex
					<< std::setw(2) << std::setfill('0')
					<< static_cast<int>(header.Item[i].sHash[k]);
			}//end for
			std::wcout << std::endl;
		}

	}//end for


	std::wcout << L"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
}

std::wstring& GetResultString(const CRom::type_result result)
{
	static std::wstring sResult;

	sResult.clear();

	switch (result) {
	case CRom::result_error:
		sResult = L"error : generic error";
		break;
	case CRom::result_error_greater_then_expected:
		sResult = L"error : greater then expected";
		break;
	case CRom::result_error_invalid_parameter:
		sResult = L"error : invalid parameter";
		break;
	case CRom::result_error_not_found:
		sResult = L"error : not found file";
		break;
	case CRom::result_error_not_loaded_dll:
		sResult = L"error : not loaded dll";
		break;
	case CRom::result_error_not_open_file:
		sResult = L"error : not open file";
		break;
	case CRom::result_error_over_capacity:
		sResult = L"error : over capacity";
		break;
	case CRom::result_error_shorter_then_expected:
		sResult = L"error : short then expected";
		break;
	case CRom::result_success:
		sResult = L"success";
		break;
	default:
		break;
	}//end switch

	return  sResult;
}