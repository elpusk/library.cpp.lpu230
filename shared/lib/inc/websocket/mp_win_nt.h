#pragma once

/**
* windows boost::beast �� ����ϸ鼭 �� ������ ��� �ҽ� ���� �� �տ� include ���� ������, winsocket �� already defined �Ǿ��ٴ� ���� ��. 
*/
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN
#endif
