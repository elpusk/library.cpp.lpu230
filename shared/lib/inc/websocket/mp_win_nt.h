#pragma once

/**
* windows boost::beast 를 사용하면서 이 파일을 모든 소스 파일 맨 앞에 include 하지 않으면, winsocket 이 already defined 되었다는 에러 남. 
*/
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN
#endif
