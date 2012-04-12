// Module name:
#define module_name Second
#undef module_name

// The "Second" module will require the "First" module:
#include "mod_first.cpp"

#define dao_get_some_values( isize, buf_dao_hint_buffer_isize_osize, osize ) get_some_values(int,void**,int*)
#define x_dao_hint_userwrapper_dao_get_some_values( isize, buf_dao_hint_buffer_isize_osize, osize ) get_some_values(int,SecondClass**,int*)

// Headers to be wrapped:
#include "second.h"

