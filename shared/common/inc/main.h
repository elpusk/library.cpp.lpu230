#pragma once

#include <mp_type.h>

/**
 * run the wss server.
*/
int main_wss(const _mp::type_set_wstring &set_parameters);

/**
 * run trace console.
 */
int main_trace(const _mp::type_set_wstring &set_parameters);

/**
 * generate & install certificate 
 */
int main_cert(const _mp::type_set_wstring &set_parameters);

/**
 * remove certificate.
 */
int main_remove_cert(const _mp::type_set_wstring &set_parameters);