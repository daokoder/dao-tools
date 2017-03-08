/*
// ClangDao: the C/C++ library binding tool for Dao
// http://www.daovm.net
//
// Copyright (c) 2011-2014, Limin Fu
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
// SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
// OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <llvm/ADT/StringExtras.h>
#include <clang/AST/Type.h>
#include <clang/AST/TypeLoc.h>
#include <clang/AST/Expr.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Lex/Preprocessor.h>

#include "cdaoVariable.hpp"
#include "cdaoModule.hpp"

const string daopar_bool = "$(name): bool$(default)";
const string daopar_int = "$(name): int$(default)";
const string daopar_float = "$(name): float$(default)";
const string daopar_double = "$(name): float$(default)";
const string daopar_complex = "$(name): complex$(default)";
const string daopar_string = "$(name): $(dao)string$(default)";
const string daopar_ints = "$(name): $(dao)array<int>";
const string daopar_floats = "$(name): $(dao)array<float>";
const string daopar_doubles = "$(name): $(dao)array<float>";
const string daopar_complexs = "$(name): $(dao)array<complex>$(default)";
const string daopar_buffer = "$(name): cdata$(default)";
const string daopar_stream = "$(name): dao::io::Stream$(default)";
const string daopar_user = "$(name): $(daotype)$(default)";
const string daopar_userdata = "$(name): any$(default)"; // for callback data
const string daopar_callback = "$(name): any"; // for callback, no precise type yet! XXX

const string daopar_daovalue = "$(name): any$(default)";

const string dao2cxx = "  $(cxxtype) $(name) = ($(cxxtype)) ";
const string dao2cxx2 = "  $(cxxtype)* $(name) = ($(cxxtype)*) ";
const string dao2cxx3 = "  $(cxxtype)* $(name) = ($(cxxtype)*) ";
const string dao2cxx4 = "  $(cxxtype) (*$(name))[$(size2)] = ($(cxxtype)(*)[$(size2)]) ";

const string dao2cxx_char = dao2cxx + "DaoValue_TryGetChars( _p[$(index)] )[0];\n";
const string dao2cxx_bool  = dao2cxx + "DaoValue_TryGetBoolean( _p[$(index)] );\n";
const string dao2cxx_int  = dao2cxx + "DaoValue_TryGetInteger( _p[$(index)] );\n";
const string dao2cxx_float = dao2cxx + "DaoValue_TryGetFloat( _p[$(index)] );\n";
const string dao2cxx_double = dao2cxx + "DaoValue_TryGetFloat( _p[$(index)] );\n";
const string dao2cxx_complex = dao2cxx + "DaoValue_TryGetComplex( _p[$(index)] );\n";
const string dao2cxx_mbs = dao2cxx2 + "DaoValue_TryGetChars( _p[$(index)] );\n";
const string dao2cxx_wcs = dao2cxx2 + "DaoValue_TryGetWCString( _p[$(index)] );\n";
const string dao2cxx_bytes = dao2cxx2 + "DaoArray_ToSInt8( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_ubytes = dao2cxx2 + "DaoArray_ToUInt8( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_shorts = dao2cxx2 + "DaoArray_ToSInt16( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_ushorts = dao2cxx2 + "DaoArray_ToUInt16( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_ints = dao2cxx2 + "DaoArray_ToSInt32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_uints = dao2cxx2 + "DaoArray_ToUInt32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_floats = dao2cxx2 + "DaoArray_ToFloat32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_doubles = dao2cxx2 + "DaoArray_ToFloat64( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_complexs8 = dao2cxx2 + "(complex8*) DaoArray_ToFloat32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_complexs16 = dao2cxx2 + "(complex16*) DaoArray_ToFloat64( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_array_buffer = dao2cxx2 + "DaoArray_GetBuffer( (DaoArray*)_p[$(index)] );\n";

const string dao2cxx_bmat = dao2cxx3 + "DaoArray_GetMatrixB( (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_smat = dao2cxx3 + "DaoArray_GetMatrixS( (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_imat = dao2cxx3 + "DaoArray_GetMatrixInt64 (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_fmat = dao2cxx3 + "DaoArray_GetMatrixFloat32( (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_dmat = dao2cxx3 + "DaoArray_GetMatrixFloat64( (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_c8mat = dao2cxx3 + "(complex8*) DaoArray_GetMatrixFloat32( (DaoArray*)_p[$(index)], $(size) );\n";
const string dao2cxx_c16mat = dao2cxx3 + "(complex16*) DaoArray_GetMatrixFloat64( (DaoArray*)_p[$(index)], $(size) );\n";


const string dao2cxx_ubmat = dao2cxx_bmat; // TODO:
const string dao2cxx_usmat = dao2cxx_smat;
const string dao2cxx_uimat = dao2cxx_imat;

const string dao2cxx_bmat2 = dao2cxx4 + "DaoArray_ToSInt8( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_smat2 = dao2cxx4 + "DaoArray_ToSInt16( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_imat2 = dao2cxx4 + "DaoArray_ToSInt32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_fmat2 = dao2cxx4 + "DaoArray_ToFloat32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_dmat2 = dao2cxx4 + "DaoArray_ToFloat64( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_c8mat2 = dao2cxx4 + "(complex8*) DaoArray_ToFloat32( (DaoArray*)_p[$(index)] );\n";
const string dao2cxx_c16mat2 = dao2cxx4 + "(complex16*) DaoArray_ToFloat64( (DaoArray*)_p[$(index)] );\n";

const string dao2cxx_ubmat2 = dao2cxx_bmat2; // TODO:
const string dao2cxx_usmat2 = dao2cxx_smat2;
const string dao2cxx_uimat2 = dao2cxx_imat2;


const string dao2cxx_stream = dao2cxx2 + "DaoStream_GetFile( (DaoStream*)_p[$(index)] );\n";

const string dao2cxx_daovalue = 
"  $(cxxtype)* $(name) = ($(cxxtype)*) _p[$(index)];\n";

const string dao2cxx_void = 
"  $(cxxtype)* $(name) = ($(cxxtype)*) DaoValue_TryGetCdata( _p[$(index)] );\n";

const string dao2cxx_void2 =
"  $(cxxtype) $(name) = ($(cxxtype)) DaoValue_TryGetCdata( _p[$(index)] );\n";

const string dao2cxx_user =
"  $(cxxtype)* $(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _p[$(index)], dao_type_$(typer) );\n";

const string dao2cxx_user2 = 
"  $(cxxtype)* $(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _p[$(index)], dao_type_$(typer) );\n";

const string dao2cxx_user3 =
"  $(cxxtype)** $(name) = ($(cxxtype)**) DaoValue_TryGetCdata2( _p[$(index)] );\n";

const string dao2cxx_user4 = 
"  $(cxxtype)* $(name) = ($(cxxtype)*) DaoValue_TryGetCdata2( _p[$(index)] );\n";

const string dao2cxx_callback =
"  DaoTuple *_$(name) = (DaoTuple*) _p[$(index)];\n\
  $(cxxtype) $(name) = Dao_$(callback);\n";

const string dao2cxx_userdata = "  DaoTuple *$(name) = (DaoTuple*) _p[$(index)];\n";


const string cxx2dao = "  DaoProcess_New";

const string cxx2dao_bool = cxx2dao + "Boolean( _proc, (dao_boolean) $(name) );\n";
const string cxx2dao_int = cxx2dao + "Integer( _proc, (dao_integer) $(name) );\n";
const string cxx2dao_float = cxx2dao + "Float(  _proc,(dao_float) $(name) );\n";
const string cxx2dao_double = cxx2dao + "Float( _proc, (dao_float) $(name) );\n";
const string cxx2dao_int2 = cxx2dao + "Integer( _proc, (dao_integer) *$(name) );\n";
const string cxx2dao_float2 = cxx2dao + "Float( _proc, (dao_float) *$(name) );\n";
const string cxx2dao_double2 = cxx2dao + "Float( _proc, (dao_float) *$(name) );\n";
const string cxx2dao_mbs = cxx2dao+"String( _proc, (char*) $(name), strlen( (char*)$(name) ) );\n"; // XXX for char**
const string cxx2dao_wcs = cxx2dao + "WCString( _proc, (wchar_t*) $(name), wcslen( (wchar_t*)$(name) ) );\n"; // XXX for wchar_t**
const string cxx2dao_bytes = cxx2dao + "VectorSInt8( _proc, (signed char*) $(name), $(size) );\n";
const string cxx2dao_ubytes = cxx2dao + "VectorUInt8( _proc, (unsigned char*) $(name), $(size) );\n";
const string cxx2dao_shorts = cxx2dao + "VectorSInt16( _proc, (signed short*) $(name), $(size) );\n";
const string cxx2dao_ushorts = cxx2dao + "VectorUInt16( _proc, (unsigned short*) $(name), $(size) );\n";
const string cxx2dao_ints = cxx2dao + "VectorSInt32( _proc, (signed int*) $(name), $(size) );\n";
const string cxx2dao_uints = cxx2dao + "VectorUInt32( _proc, (unsigned int*) $(name), $(size) );\n";
const string cxx2dao_floats = cxx2dao + "VectorFloat32( _proc, (float*) $(name), $(size) );\n";
const string cxx2dao_doubles = cxx2dao + "VectorFloat64( _proc, (double*) $(name), $(size) );\n";

const string cxx2dao_bmat = cxx2dao + "MatrixSInt8( _proc, (signed char**) $(name), $(size), $(size2) );\n";
const string cxx2dao_ubmat = cxx2dao + "MatrixUInt8( _proc, (unsigned char**) $(name), $(size), $(size2) );\n";
const string cxx2dao_smat = cxx2dao + "MatrixSInt16( _proc, (signed short**) $(name), $(size), $(size2) );\n";
const string cxx2dao_usmat = cxx2dao + "MatrixUInt16( _proc, (unsigned short**) $(name), $(size), $(size2) );\n";
const string cxx2dao_imat = cxx2dao + "MatrixSInt32( _proc, (signed int**) $(name), $(size), $(size2) );\n";
const string cxx2dao_uimat = cxx2dao + "MatrixUInt32( _proc, (unsigned int**) $(name), $(size), $(size2) );\n";
const string cxx2dao_fmat = cxx2dao + "MatrixFloat32( _proc, (float**) $(name), $(size), $(size2) );\n";
const string cxx2dao_dmat = cxx2dao + "MatrixFloat64( _proc, (double**) $(name), $(size), $(size2) );\n";

const string cxx2dao_bmat2 = cxx2dao_bmat; // XXX
const string cxx2dao_ubmat2 = cxx2dao_ubmat;
const string cxx2dao_smat2 = cxx2dao_smat;
const string cxx2dao_usmat2 = cxx2dao_usmat;
const string cxx2dao_imat2 = cxx2dao_imat;
const string cxx2dao_uimat2 = cxx2dao_uimat;
const string cxx2dao_fmat2 = cxx2dao_fmat;
const string cxx2dao_dmat2 = cxx2dao_dmat;

const string cxx2dao_stream = cxx2dao + "Stream( _proc, (FILE*) $(refer) );\n";
const string cxx2dao_voidp = "  DaoProcess_NewCdata( _proc, NULL, (void*) $(refer), 0 );\n";
const string cxx2dao_user = "  DaoProcess_NewCdata( _proc, dao_type_$(typer), (void*) $(refer), 0 );\n";

const string cxx2dao_daovalue = "  DaoProcess_CacheValue( _proc, (DaoValue*) $(refer) );\n";

const string cxx2dao_userdata = "  DaoProcess_CacheValue( _proc, $(name) );\n";

const string cxx2dao_qchar = cxx2dao+"Integer( _proc, $(name).digitValue() );\n";
const string cxx2dao_qchar2 = cxx2dao+"Integer( _proc, $(name)->digitValue() );\n";
const string cxx2dao_qbytearray = cxx2dao+"String( _proc, (char*) $(name).data(), -1 );\n";
const string cxx2dao_qstring = cxx2dao+"String( _proc, (char*) $(name).toLocal8Bit().data(), -1 );\n";

const string ctxput = "  DaoProcess_Put";

const string ctxput_none = ctxput + "None( _proc );\n";

const string ctxput_bool = ctxput + "Boolean( _proc, (dao_boolean) $(name) );\n";
const string ctxput_int = ctxput + "Integer( _proc, (dao_integer) $(name) );\n";
const string ctxput_float = ctxput + "Float( _proc, (dao_float) $(name) );\n";
const string ctxput_double = ctxput + "Float( _proc, (dao_float) $(name) );\n";
const string ctxput_mbs = ctxput + "Chars( _proc, (char*) $(name) );\n";
const string ctxput_wcs = ctxput + "WCString( _proc, (wchar_t*) $(name) );\n";
const string ctxput_bytes = ctxput + "Bytes( _proc, (char*) $(name), $(size) );\n"; // XXX array?
const string ctxput_shorts = ctxput + "VectorSInt16( _proc, (signed short*) $(name), $(size) );\n";
const string ctxput_ints = ctxput + "VectorSInt32( _proc, (signed int*) $(name), $(size) );\n";
const string ctxput_ushorts = ctxput + "VectorUInt16( _proc, (unsigned short*) $(name), $(size) );\n";
const string ctxput_uints = ctxput + "VectorUInt32( _proc, (unsigned int*) $(name), $(size) );\n";
const string ctxput_daoints = ctxput + "VectorInt64 _proc, (long long*) $(name), $(size) );\n";
const string ctxput_floats = ctxput + "VectorFloat32( _proc, (float*) $(name), $(size) );\n";
const string ctxput_doubles = ctxput + "VectorFloat64( _proc, (double*) $(name), $(size) );\n";

const string ctxput_stream = ctxput + "File( _proc, (FILE*) $(name) );\n"; //XXX PutFile
const string ctxput_voidp = ctxput + "Cdata( _proc, (void*) $(name), dao_type_$(typer) );\n";
const string ctxput_voidp2 = ctxput + "Cdata( _proc, (void*) $(name), NULL );\n";
const string ctxput_user = "  DaoProcess_WrapCdata( _proc, (void*) $(name), dao_type_$(typer) );\n";
const string ctxput_user2 = "  DaoProcess_WrapCdata( _proc, (void*) $(name), NULL );\n";

const string ctxput_nullable_user =
"  if( $(name) ){\n"
"    DaoProcess_WrapCdata( _proc, (void*) $(name), dao_type_$(typer) );\n"
"  }else{\n"
"    DaoProcess_PutNone( _proc );\n"
"  }\n";

const string ctxput_nullable_user2 =
"  if( $(name) ){\n"
"    DaoProcess_WrapCdata( _proc, (void*) $(name), NULL );\n"
"  }else{\n"
"    DaoProcess_PutNone( _proc );\n"
"  }\n";

const string ctxput_daovalue = ctxput + "Value( _proc, (DaoValue*) $(name) );\n";


const string qt_procput = "  Dao_$(typer)_InitSInt16( ($(cxxtype)*) $(name) );\n";
const string qt_put_qobject =
"  DaoValue *dbase = DaoQt_Get_Wrapper( $(name) );\n\
  if( dbase ){\n\
    DaoProcess_PutValue( _proc, dbase );\n\
  }else{\n\
    Dao_$(typer)_InitSInt16( ($(cxxtype)*) $(name) );\n\
    DaoProcess_WrapCdata( _proc, (void*) $(name), dao_type_$(typer) );\n\
  }\n\
";

const string ctxput_copycdata =
"  DaoProcess_CopyCdata( _proc, (void*)&$(name), sizeof($(cxxtype)), dao_type_$(typer) );\n";
const string ctxput_newcdata =
"  DaoProcess_PutCdata( _proc, (void*)new $(cxxtype)( $(name) ), dao_type_$(typer) );\n";
const string ctxput_refcdata =
"  DaoProcess_WrapCdata( _proc, (void*)&$(name), dao_type_$(typer) );\n";

const string cache = "  DaoProcess_New";

const string cache_bool = cache + "Boolean( _proc, (dao_boolean) $(name) );\n";
const string cache_int = cache + "Integer( _proc, (dao_integer) $(name) );\n";
const string cache_float = cache + "Float( _proc, (dao_float) $(name) );\n";
const string cache_double = cache + "Float( _proc, (dao_float) $(name) );\n";
const string cache_mbs = cache + "String( _proc, (char*) $(name), -1 );\n";
const string cache_wcs = cache + "WCString( _proc, (wchar_t*) $(name), -1 );\n";
const string cache_bytes = cache + "String( _proc, (char*) $(name), $(size) );\n"; // XXX array?
const string cache_shorts = cache + "VectorSInt16( _proc, (signed short*) $(name), $(size) );\n";
const string cache_ushorts = cache + "VectorUInt16( _proc, (unsigned short*) $(name), $(size) );\n";
const string cache_ints = cache + "VectorSInt32( _proc, (signed int*) $(name), $(size) );\n";
const string cache_uints = cache + "VectorUInt32( _proc, (unsigned int*) $(name), $(size) );\n";
const string cache_daoints = cache + "VectorInt64 _proc, (long long*) $(name), $(size) );\n";
const string cache_floats = cache + "VectorFloat32( _proc, (float*) $(name), $(size) );\n";
const string cache_doubles = cache + "VectorFloat64( _proc, (double*) $(name), $(size) );\n";

const string cache_stream = cache + "Stream( _proc, (FILE*) $(name) );\n"; //XXX PutFile
const string cache_voidp = cache + "Cdata( _proc, NULL, (void*) $(name), 1 );\n";
const string cache_user = "  DaoProcess_NewCdata( _proc, dao_type_$(typer), (void*) $(name), 0 );\n";

const string cache_daovalue = cache + "Value( _proc, (DaoValue*) $(name) );\n";

const string cache_copycdata =
"  DaoProcess_CopyCdata( _proc, (void*)&$(name), sizeof($(cxxtype)), dao_type_$(typer) );\n";
const string cache_newcdata =
"  DaoProcess_NewCdata( _proc, dao_type_$(typer), (void*)new $(cxxtype)( $(name) ), 1 );\n";
const string cache_refcdata =
"  DaoProcess_NewCdata( _proc, dao_type_$(typer), (void*)&$(name), 0 );\n";



const string daopar_number_plain = "$(name): $(dao)$(number)$(default)";
const string daopar_number_variant = "$(name): $(dao)$(number)|$(daotype)$(default)";

const string dao2cxx_number_plain =
"  $(cxxtype) $(name)( DaoValue_TryGet$(Number)( _p[$(index)] ) );\n";

const string dao2cxx_number_variant =
"  $(cxxtype) *__cdata_$(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _p[$(index)], dao_type_$(typer) );\n"
"  $(cxxtype) __$(name)( __cdata_$(name) ? 0 : DaoValue_TryGet$(Number)( _p[$(index)] ) );\n"
"  $(cxxtype) & $(name) = __cdata_$(name) ? *__cdata_$(name) : __$(name);\n";

const string cxx2dao_number_common = cxx2dao + "$(Number)( _proc, $(name).$(tonumber)() ) );\n";

const string ctxput_number_common = ctxput + "$(Number)( _proc, $(name).$(tonumber)() );\n";

const string cache_number_common = cache + "$(Number)( _proc, $(name).$(tonumber)() );\n";

const string getres_number_common = "  if(DaoValue_Cast$(Number)(_res)) $(name)=$(cxxtype)("
" DaoValue_TryGet$(Number)( _res ) );\n";

const string setter_number_common = "  self->$(name) = $(cxxtype)( DaoValue_TryGet$(Number)( _p[1] ) );\n";



const string daopar_string_plain = "$(name): $(dao)string$(default)";
const string daopar_string_variant = "$(name): $(dao)string|$(daotype)$(default)";

const string dao2cxx_mbs_plain =
"  char *__chars_$(name) = DaoValue_TryGetChars( _p[$(index)] );\n"
"  $(cxxtype) $(name)( __chars_$(name) );\n";

const string dao2cxx_wcs_plain =
"  wchar_t *__chars_$(name) = DaoValue_TryGetWCString( _p[$(index)] );\n"
"  $(cxxtype) $(name)( __chars_$(name) );\n";


const string dao2cxx_mbs_variant =
"  char *__chars_$(name) = DaoValue_TryGetChars( _p[$(index)] );\n"
"  $(cxxtype) *__cdata_$(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _p[$(index)], dao_type_$(typer) );\n"
"  $(cxxtype) __$(name)( __chars_$(name) ? __chars_$(name) : \"\" );\n"
"  $(cxxtype) & $(name) = __cdata_$(name) ? *__cdata_$(name) : __$(name);\n";

const string dao2cxx_wcs_variant =
"  wchar_t *__chars_$(name) = DaoValue_TryGetWCString( _p[$(index)] );\n"
"  $(cxxtype) *__cdata_$(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _p[$(index)], dao_type_$(typer) );\n"
"  $(cxxtype) __$(name)( __chars_$(name) ? __chars_$(name) : L\"\" );\n"
"  $(cxxtype) & $(name) = __cdata_$(name) ? *__cdata_$(name) : __$(name);\n";

const string cxx2dao_mbs_common = cxx2dao 
+ "String( _proc, (char*) $(name).$(tochars)(), strlen( (char*)$(name).$(tochars)() ) );\n";
const string cxx2dao_wcs_common = cxx2dao 
+ "WCString( _proc, (wchar_t*) $(name).$(tochars)(), wcslen( (wchar_t*)$(name).$(tochars)() ) );\n";

const string ctxput_mbs_common = ctxput + "Chars( _proc, (char*) $(name).$(tochars)() );\n";
const string ctxput_wcs_common = ctxput + "WCString( _proc, (wchar_t*) $(name).$(tochars)() );\n";

const string cache_mbs_common = cache + "String( _proc, (char*) $(name).$(tochars)(), -1 );\n";
const string cache_wcs_common = cache + "WCString( _proc, (wchar_t*) $(name).$(tochars)(), -1 );\n";

const string getres_mbs_common = "  if(DaoValue_CastString(_res)) $(name)=$(cxxtype)("
" DaoValue_TryGetChars( _res ) );\n";
const string getres_wcs_common = "  if(DaoValue_CastString(_res)) $(name)=$(cxxtype)("
" DaoValue_TryGetWCString( _res ) );\n";

const string setter_mbs_common = "  self->$(name) = $(cxxtype)( DaoValue_TryGetChars( _p[1] ) );\n";
const string setter_wcs_common = "  self->$(name) = $(cxxtype)( DaoValue_TryGetWCString( _p[1] ) );\n";


#if 0
const string qt_qlist_decl = 
"typedef $(qtype)<$(item)> $(qtype)_$(item);
void Dao_Put$(qtype)_$(item)( DaoProcess *ctx, const $(qtype)_$(item) & qlist );
void Dao_Get$(qtype)_$(item)( DaoList *dlist, $(qtype)_$(item) & qlist );
";
const string qt_qlist_decl2 = 
"typedef $(qtype)<$(item)*> $(qtype)P_$(item);
void Dao_Put$(qtype)P_$(item)( DaoProcess *ctx, const $(qtype)P_$(item) & qlist );
void Dao_Get$(qtype)P_$(item)( DaoList *dlist, $(qtype)P_$(item) & qlist );
";
const string qt_daolist_func =
"void Dao_Put$(qtype)_$(item)( DaoProcess *ctx, const $(qtype)_$(item) & qlist )
{
	DaoList *dlist = DaoProcess_PutList( ctx );
	DaoValue it = { DAO_CDATA, 0, 0, 0, {0} };
	int i, m = qlist.size();
	for(i=0; i<m; i++){
		it.v.cdata = DaoCdata_New( dao_$(item)_Typer, new $(item)( qlist[i] ) );
		DaoList_PushBack( dlist, it );
	}
}
void Dao_Get$(qtype)_$(item)( DaoList *dlist, $(qtype)_$(item) & qlist )
{
	int i, m = DaoList_Size( dlist );
	for(i=0; i<m; i++){
		DaoValue it = DaoList_GetItem( dlist, i );
		if( it.t != DAO_CDATA ) continue;
		if( ! DaoCdata_IsType( it.v.cdata, dao_$(item)_Typer ) ) continue;
		qlist.append( *($(item)*) DaoValue_TryCastCdata( & it, dao_$(item)_Typer ) );
	}
}
";
const string qt_daolist_func_virt =
"void Dao_Put$(qtype)_$(item)( DaoProcess *ctx, const $(qtype)_$(item) & qlist )
{
	DaoList *dlist = DaoProcess_PutList( ctx );
	DaoValue it = { DAO_CDATA, 0, 0, 0, {0} };
	int i, m = qlist.size();
	for(i=0; i<m; i++){
		it.v.cdata = DaoCdata_New( dao_$(item)_Typer, new $(item)( qlist[i] ) );
		DaoList_PushBack( dlist, it );
	}
}
void Dao_Get$(qtype)_$(item)( DaoList *dlist, $(qtype)_$(item) & qlist )
{
	int i, m = DaoList_Size( dlist );
	for(i=0; i<m; i++){
		DaoValue it = DaoList_GetItem( dlist, i );
		if( it.t != DAO_CDATA ) continue;
		if( ! DaoCdata_IsType( it.v.cdata, dao_$(item)_Typer ) ) continue;
		qlist.append( * ($(item)*) DaoValue_TryCastCdata( & it, dao_$(item)_Typer ) );
	}
}
";
const string qt_daolist_func2 =
"void Dao_Put$(qtype)P_$(item)( DaoProcess *ctx, const $(qtype)P_$(item) & qlist )
{
	DaoList *dlist = DaoProcess_PutList( ctx );
	DaoValue it = { DAO_CDATA, 0, 0, 0, {0} };
	int i, m = qlist.size();
	for(i=0; i<m; i++){
		it.v.cdata = DaoCdata_Wrap( dao_$(item)_Typer, qlist[i] );
		DaoList_PushBack( dlist, it );
	}
}
void Dao_Get$(qtype)P_$(item)( DaoList *dlist, $(qtype)P_$(item) & qlist )
{
	int i, m = DaoList_Size( dlist );
	for(i=0; i<m; i++){
		DaoValue it = DaoList_GetItem( dlist, i );
		if( it.t != DAO_CDATA ) continue;
		if( ! DaoCdata_IsType( it.v.cdata, dao_$(item)_Typer ) ) continue;
		qlist.append( ($(item)*) DaoValue_TryCastCdata( & it, dao_$(item)_Typer ) );
	}
}
";
const string qt_daolist_func_virt2 =
"void Dao_Put$(qtype)P_$(item)( DaoProcess *ctx, const $(qtype)P_$(item) & qlist )
{
	DaoList *dlist = DaoProcess_PutList( ctx );
	DaoValue it = { DAO_CDATA, 0, 0, 0, {0} };
	int i, m = qlist.size();
	for(i=0; i<m; i++){
		it.v.cdata = DaoCdata_Wrap( dao_$(item)_Typer, qlist[i] );
		DaoList_PushBack( dlist, it );
	}
}
void Dao_Get$(qtype)P_$(item)( DaoList *dlist, $(qtype)P_$(item) & qlist )
{
	int i, m = DaoList_Size( dlist );
	for(i=0; i<m; i++){
		DaoValue it = DaoList_GetItem( dlist, i );
		if( it.t != DAO_CDATA ) continue;
		if( ! DaoCdata_IsType( it.v.cdata, dao_$(item)_Typer ) ) continue;
		qlist.append( ($(item)*) DaoValue_TryCastCdata( & it, dao_$(item)_Typer ) );
	}
}
";
const string qt_dao2cxx_list =
"  $(cxxtype) $(name);
  Dao_Get$(qtype)_$(item)( _p[$(index)]->v.list, $(name) );
";
const string qt_dao2cxx_list2 =
"  $(cxxtype) $(name);
  Dao_Get$(qtype)P_$(item)( _p[$(index)]->v.list, $(name) );
";
const string qt_daolist_codes = "  Dao_Put$(qtype)_$(item)( _proc, $(name) );\n";
const string qt_daolist_codes2 = "  Dao_Put$(qtype)P_$(item)( _proc, $(name) );\n";
#endif

const string parset_bool = "  DaoProcess_NewBoolean( _proc, (dao_boolean)$(name) );\n";
const string parset_int = "  DaoProcess_NewInteger( _proc, (dao_integer)$(name) );\n";
const string parset_float = "  DaoProcess_NewFloat( _proc, (dao_float)$(name) );\n";
const string parset_double = "  DaoProcess_NewFloat( _proc, (dao_float)$(name) );\n";
const string parset_mbs = "  DaoProcess_NewString( _proc, (char*)$(name), -1 );\n";
const string parset_wcs = "  DaoProcess_NewWCString( _proc, (wchar_t*)$(name), -1 );\n";

const string parset_bytes = "  DaoArray_FromSInt8( (DaoArray*)_p[$(index)] );\n";
const string parset_ubytes = "  DaoArray_FromUInt8( (DaoArray*)_p[$(index)] );\n";
const string parset_shorts = "  DaoArray_FromSInt16( (DaoArray*)_p[$(index)] );\n";
const string parset_ushorts = "  DaoArray_FromUInt16( (DaoArray*)_p[$(index)] );\n";
const string parset_ints = "  DaoArray_FromSInt32( (DaoArray*)_p[$(index)] );\n";
const string parset_uints = "  DaoArray_FromUInt32( (DaoArray*)_p[$(index)] );\n";
const string parset_floats = "  DaoArray_FromFloat32( (DaoArray*)_p[$(index)] );\n";
const string parset_doubles = "  DaoArray_FromFloat64( (DaoArray*)_p[$(index)] );\n";

const string dao2cxx2cst = "  const $(cxxtype)* $(name)= (const $(cxxtype)*) ";
const string dao2cxx_mbs_cst = dao2cxx2cst + "DaoValue_TryGetChars( _p[$(index)] );\n";
const string dao2cxx_wcs_cst = dao2cxx2cst + "DaoValue_TryGetWCString( _p[$(index)] );\n";

const string dao2cxx_mbs2 = 
"  $(cxxtype)* $(name)_old = ($(cxxtype)*) DaoValue_TryGetChars( _p[$(index)] );\n\
  size_t $(name)_len = strlen( $(name)_old );\n\
  $(cxxtype)* $(name) = ($(cxxtype)*) malloc( $(name)_len + 1 );\n\
  void* $(name)_p = strncpy( $(name), $(name)_old, $(name)_len );\n";

const string dao2cxx_wcs2 = 
"  $(cxxtype)* $(name)_old = ($(cxxtype)*) DaoValue_TryGetWCString( _p[$(index)] );\n\
  size_t $(name)_len = wcslen( $(name)_old ) * sizeof(wchar_t);\n\
  $(cxxtype)* $(name) = ($(cxxtype)*) malloc( $(name)_len + sizeof(wchar_t) );\n\
  void* $(name)_p = memcpy( $(name), $(name)_old, $(name)_len );\n";

const string parset_mbs2 = 
"  DaoString_SetChars( (DaoString*)_p[$(index)], (char*)$(name) );\n\
  free( $(name) );\n";

const string parset_wcs2 = 
"  DaoString_SetWCS( (DaoString*)_p[$(index)], (wchar_t*)$(name) );\n\
  free( $(name) );\n";

const string dao2cxx_qchar = "  QChar $(name)( (daoint)DaoValue_TryGetInteger( _p[$(index)] ) );\n";
const string dao2cxx_qchar2 =
"  QChar $(name)( (daoint)DaoValue_TryGetInteger( _p[$(index)] ) )\
  QChar *$(name) = & _$(name);\n";

const string parset_qchar = "  DaoInteger_Set( (DaoInteger*)_p[$(index)], $(name).digitValue() );\n";
const string parset_qchar2 = "  DaoInteger_Set( (DaoInteger*)_p[$(index)], $(name)->digitValue() );\n";
const string ctxput_qchar = "  DaoProcess_PutInteger( _proc, $(name).digitValue() );\n";
const string ctxput_qchar2 = "  DaoProcess_PutInteger( _proc, $(name)->digitValue() );\n";

const string dao2cxx_qbytearray =
"  char *_mbs$(index) = DaoValue_TryGetChars( _p[$(index)] );\n\
  QByteArray $(name)( _mbs$(index) );\n";

const string dao2cxx_qbytearray2 =
"  char *_mbs$(index) = DaoValue_TryGetChars( _p[$(index)] );\n\
  QByteArray _$(name)( _mbs$(index) );\n\
  QByteArray *$(name) = & _$(name);\n";

const string parset_qbytearray =
"  DaoString_SetChars( (DaoString*)_p[$(index)], (char*) $(name).data() );\n";
const string parset_qbytearray2 =
"  DaoString_SetChars( (DaoString*)_p[$(index)], (char*) $(name)->data() );\n";
const string ctxput_qbytearray = 
"  DaoProcess_PutString( _proc, $(name).data(), -1 );\n";

const string dao2cxx_qstring =
"  char *_mbs$(index) = DaoValue_TryGetChars( _p[$(index)] );\n\
  QString $(name)( _mbs$(index) );\n";

const string dao2cxx_qstring2 =
"  char *_mbs$(index) = DaoValue_TryGetChars( _p[$(index)] );\n\
  QString _$(name)( _mbs$(index) );\n\
  QString *$(name) = & _$(name);\n";

const string parset_qstring =
"  DaoString_SetChars( (DaoString*)_p[$(index)], (char*)$(name).toLocal8Bit().data() );\n";
const string parset_qstring2 =
"  DaoString_SetChars( (DaoString*)_p[$(index)], (char*)$(name)->toLocal8Bit().data() );\n";
const string ctxput_qstring = 
"  DaoProcess_PutString( _proc, $(name).toLocal8Bit().data(), 1 );\n";

const string getres_i = "  if(DaoValue_CastInteger(_res)) $(name)=($(cxxtype))";
const string getres_f = "  if(DaoValue_CastFloat(_res)) $(name)=($(cxxtype))";
const string getres_d = "  if(DaoValue_CastFloat(_res)) $(name)=($(cxxtype))";
const string getres_s = "  if(DaoValue_CastString(_res)) $(name)=($(cxxtype)*)";
const string getres_a = "  if(DaoValue_CastArray(_res))\n    $(name)=($(cxxtype)*)";
const string getres_p = "  if(DaoValue_CastCdata(_res,NULL)) $(name)=($(cxxtype)) ";
const string getres_io = "  if(DaoValue_CastStream(_res)) $(name)=($(cxxtype))";

const string getres_bool  = getres_i + "DaoValue_TryGetBoolean(_res);\n";
const string getres_int  = getres_i + "DaoValue_TryGetInteger(_res);\n";
const string getres_float = getres_f + "DaoValue_TryGetFloat(_res);\n";
const string getres_double = getres_d + "DaoValue_TryGetFloat(_res);\n";
const string getres_mbs = getres_s + "DaoValue_TryGetChars( _res );\n";
const string getres_wcs = getres_s + "DaoValue_TryGetWCString( _res );\n";
const string getres_bytes = getres_a + "DaoArray_ToInt8( (DaoArray*)_res );\n";
const string getres_ubytes = getres_a + "DaoArray_ToUInt8( (DaoArray*)_res );\n";
const string getres_shorts = getres_a + "DaoArray_ToInt16( (DaoArray*)_res );\n";
const string getres_ushorts = getres_a + "DaoArray_ToUInt16( (DaoArray*)_res );\n";
const string getres_ints = getres_a + "DaoArray_ToSInt32( (DaoArray*)_res );\n";
const string getres_uints = getres_a + "DaoArray_ToUInt32( (DaoArray*)_res );\n";
const string getres_floats = getres_a + "DaoArray_ToFloat32( (DaoArray*)_res );\n";
const string getres_doubles = getres_a + "DaoArray_ToFloat64( (DaoArray*)_res );\n";
const string getres_stream = getres_io + "DaoStream_GetFile( (DaoArray*)_res );\n";
const string getres_buffer = getres_p + "DaoValue_TryGetCdata( _res );\n";


const string getres_qchar =
"  if(DaoValue_CastInteger(_res)) $(name)= QChar(DaoValue_TryGetInteger(_res));\n";
const string getres_qbytearray =
"  if(DaoValue_CastString(_res)) $(name)= DaoValue_TryGetChars( _res );\n";
const string getres_qstring =
"  if(DaoValue_CastString(_res)) $(name)= DaoValue_TryGetChars( _res );\n";

const string getres_cdata = 
"  if( DaoValue_CastObject(_res) ) _res = (DaoValue*)DaoObject_CastCdata( (DaoObject*)_res, dao_type_$(typer) );\n\
  if( DaoValue_CastCdata( _res, dao_type_$(typer) ) ){\n";

const string getres_user = getres_cdata +
"    $(name) = ($(cxxtype)*) DaoValue_TryCastCdata( _res, dao_type_$(typer) );\n  }\n";

const string getres_user2 = getres_cdata +
"    $(name) = *($(cxxtype)*) DaoValue_TryCastCdata( _res, dao_type_$(typer) );\n  }\n";


const string getitem_int = ctxput + "Integer( _proc, (dao_integer) self->$(name)[DaoValue_TryGetInteger(_p[1])] );\n";
const string getitem_float = ctxput + "Float( _proc, (dao_float) self->$(name)[DaoValue_TryGetInteger(_p[1])] );\n";
const string getitem_double = ctxput + "Float( _proc, (dao_float) self->$(name)[DaoValue_TryGetInteger(_p[1])] );\n";

const string setitem_int = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  self->$(name)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetInteger(_p[2]);\n";
const string setitem_float = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  self->$(name)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetFloat(_p[2]);\n";
const string setitem_double = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  self->$(name)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetFloat(_p[2]);\n";

const string getitem_int2 = ctxput + "Integer( _proc, (dao_integer) (*self)[DaoValue_TryGetInteger(_p[1])] );\n";
const string getitem_float2 = ctxput + "Float( _proc, (dao_float) (*self)[DaoValue_TryGetInteger(_p[1])] );\n";
const string getitem_double2 = ctxput + "Float( _proc, (dao_float) (*self)[DaoValue_TryGetInteger(_p[1])] );\n";

const string setitem_int2 = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  (*self)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetInteger(_p[2]);\n";

const string setitem_float2 = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  (*self)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetFloat(_p[2]);\n";

const string setitem_double2 = 
"  if( DaoValue_TryGetInteger(_p[1]) < 0 || DaoValue_TryGetInteger(_p[1]) >= $(size) ) return;\n\
  (*self)[DaoValue_TryGetInteger(_p[1])] = DaoValue_TryGetFloat(_p[2]);\n";

const string setter_bool = "  self->$(name) = ($(cxxtype)) DaoValue_TryGetBoolean(_p[1]);\n";
const string setter_int = "  self->$(name) = ($(cxxtype)) DaoValue_TryGetInteger(_p[1]);\n";
const string setter_float = "  self->$(name) = ($(cxxtype)) DaoValue_TryGetFloat(_p[1]);\n";
const string setter_double = "  self->$(name) = ($(cxxtype)) DaoValue_TryGetFloat(_p[1]);\n";
const string setter_string = // XXX array?
"  int size = DaoString_Size( (DaoString*)_p[1] );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoValue_TryGetChars( _p[1] ), size );\n";

const string setter_shorts =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToSInt16( array ), size*sizeof(signed short) );\n\
  DaoArray_FromSInt16( array );\n";

const string setter_ushorts =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToUInt16( array ), size*sizeof(unsigned short) );\n\
  DaoArray_FromUInt16( array );\n";

const string setter_ints =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToSInt32( array ), size*sizeof(signed int) );\n\
  DaoArray_FromSInt32( array );\n";

const string setter_uints =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToUInt32( array ), size*sizeof(unsigned int) );\n\
  DaoArray_FromUInt32( array );\n";

const string setter_floats =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToFloat32( array ), size*sizeof(float) );\n\
  DaoArray_FromFloat32( array );\n";

const string setter_doubles =
"  DaoArray *array = (DaoArray*) _p[1];\n\
  int size = DaoArray_Size( array );\n\
  if( size > $(size) ) size = $(size);\n\
  memmove( self->$(name), DaoArray_ToFloat64( array ), size*sizeof(double) );\n\
  DaoArray_FromFloat64( array );\n";

extern string cdao_string_fill( const string & tpl, const map<string,string> & subs );
extern string cdao_qname_to_idname( const string & qname );
extern string normalize_type_name( const string & name );
extern string cdao_make_dao_template_type_name( const string & name );
extern string cdao_substitute_typenames( const string & qname );

struct CDaoVarTemplates
{
	string daopar;
	string dao2cxx;
	string cxx2dao;
	string ctxput;
	string cache;
	string parset;
	string getres;
	string setter;
	string get_item;
	string set_item;

	void Generate( CDaoVariable *var, map<string,string> & kvmap, int daopid, int cxxpid );
	void SetupBoolScalar(){
		daopar = daopar_bool;
		dao2cxx = dao2cxx_bool;
		cxx2dao = cxx2dao_bool;
		ctxput = ctxput_bool;
		cache = cache_bool;
		getres = getres_bool;
		setter = setter_bool;
	}
	void SetupIntScalar(){
		daopar = daopar_int;
		dao2cxx = dao2cxx_int;
		cxx2dao = cxx2dao_int;
		ctxput = ctxput_int;
		cache = cache_int;
		getres = getres_int;
		setter = setter_int;
	}
	void SetupFloatScalar(){
		daopar = daopar_float;
		dao2cxx = dao2cxx_float;
		cxx2dao = cxx2dao_float;
		ctxput = ctxput_float;
		cache = cache_float;
		getres = getres_float;
		setter = setter_float;
	}
	void SetupDoubleScalar(){
		daopar = daopar_double;
		dao2cxx = dao2cxx_double;
		cxx2dao = cxx2dao_double;
		ctxput = ctxput_double;
		cache = cache_double;
		getres = getres_double;
		setter = setter_double;
	}
	void SetupMBString(){
		daopar = daopar_string;
		dao2cxx = dao2cxx_mbs;
		cxx2dao = cxx2dao_mbs;
		ctxput = ctxput_mbs;
		cache = cache_mbs;
		//parset = parset_mbs;
		getres = getres_mbs;
	}
	void SetupWCString(){
		daopar = daopar_string;
		dao2cxx = dao2cxx_wcs;
		cxx2dao = cxx2dao_wcs;
		ctxput = ctxput_wcs;
		cache = cache_wcs;
		//parset = parset_wcs;
		getres = getres_wcs;
	}
	void SetupNumber( const string & Number, const string & number, const string & tonumber, bool variant = false ){
		map<string,string> kvmap;
		kvmap["Number"] = Number;
		kvmap["number"] = number;
		kvmap["tonumber"] = tonumber;
		if( variant ){
			daopar = cdao_string_fill( daopar_number_variant, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_number_variant, kvmap );
		}else{
			daopar = cdao_string_fill( daopar_number_plain, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_number_plain, kvmap );
		}
		cxx2dao = cdao_string_fill( cxx2dao_number_common, kvmap );
		ctxput = cdao_string_fill( ctxput_number_common, kvmap );
		cache = cdao_string_fill( cache_number_common, kvmap );
		//parset = cdao_string_fill( parset_number, kvmap );
		getres = cdao_string_fill( getres_number_common, kvmap );
		setter = setter_number_common;
	}
	void SetupMBString( const string & tochars, bool variant = false ){
		map<string,string> kvmap;
		kvmap["tochars"] = tochars;
		if( variant ){
			daopar = cdao_string_fill( daopar_string_variant, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_mbs_variant, kvmap );
		}else{
			daopar = cdao_string_fill( daopar_string_plain, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_mbs_plain, kvmap );
		}
		cxx2dao = cdao_string_fill( cxx2dao_mbs_common, kvmap );
		ctxput = cdao_string_fill( ctxput_mbs_common, kvmap );
		cache = cdao_string_fill( cache_mbs_common, kvmap );
		//parset = cdao_string_fill( parset_mbs, kvmap );
		getres = cdao_string_fill( getres_mbs_common, kvmap );
		setter = setter_mbs_common;
	}
	void SetupWCString( const string & tochars, bool variant = false ){
		map<string,string> kvmap;
		kvmap["tochars"] = tochars;
		if( variant ){
			daopar = cdao_string_fill( daopar_string_variant, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_wcs_variant, kvmap );
		}else{
			daopar = cdao_string_fill( daopar_string_plain, kvmap );
			dao2cxx = cdao_string_fill( dao2cxx_wcs_plain, kvmap );
		}
		cxx2dao = cdao_string_fill( cxx2dao_wcs_common, kvmap );
		ctxput = cdao_string_fill( ctxput_wcs_common, kvmap );
		cache = cdao_string_fill( cache_wcs_common, kvmap );
		//parset = cdao_string_fill( parset_wcs, kvmap );
		getres = cdao_string_fill( getres_wcs_common, kvmap );
		setter = setter_wcs_common;
	}
};
void CDaoVarTemplates::Generate( CDaoVariable *var, map<string,string> & kvmap, int daopid, int cxxpid )
{
	string dft, typer = cdao_qname_to_idname( var->daotype );
	char sindex[50];
	sprintf( sindex, "%i", daopid );
	if( var->isNullable ) dft = "|none";
	if( var->daodefault.size() ) dft += " =" + var->daodefault;
	if( var->cxxtyper.size() ) typer = var->cxxtyper;
	if( var->hintCxxType.size() ) typer = var->hintCxxType;

	if( var->daotype.find( "std::" ) == 0 ) var->daotype.replace( 0, 5, "_std::" );
	if( var->daotype.find( "io::" ) == 0 ) var->daotype.replace( 0, 4, "_io::" );

	if( var->hintImplicit.size() ){
		daopar = "";
		dao2cxx = "  $(cxxtype)* $(name) = " + var->hintImplicit + "( _proc );\n";
		var->ignore = true;
	}

	kvmap[ "dao" ] = "";
	kvmap[ "daotype" ] = var->daotype;
	kvmap[ "cxxtype" ] = var->cxxtype2;
	kvmap[ "typer" ] = typer;
	kvmap[ "name" ] = var->name;
	kvmap[ "namespace" ] = "";
	kvmap[ "namespace2" ] = "";
	kvmap[ "index" ] = sindex;
	kvmap[ "default" ] = dft;
	kvmap[ "refer" ] = var->qualtype->isPointerType() ? var->name : "&" + var->name;
	if( var->hostype && var->hostype->name == "string" && var->daotype == "string" )
		kvmap[ "dao" ] = "dao::";
	if( var->isCallback ){
		var->cxxtype = normalize_type_name( var->qualtype.getAsString() );
		kvmap[ "cxxtype" ] = var->cxxtype;
	}
	var->daopar = cdao_string_fill( daopar, kvmap );
	var->dao2cxx = cdao_string_fill( dao2cxx, kvmap );
	var->parset = cdao_string_fill( parset, kvmap );
	sprintf( sindex, "%i", cxxpid );
	kvmap[ "index" ] = sindex;
	var->cxx2dao = cdao_string_fill( cxx2dao, kvmap );
	var->cacheReturn = cdao_string_fill( cache, kvmap );
	if( var->extraReturn ) var->ctxput = cdao_string_fill( ctxput, kvmap );
	if( daopid == VAR_INDEX_RETURN ){
		var->ctxput = cdao_string_fill( ctxput, kvmap );
		var->getres = cdao_string_fill( getres, kvmap );
	}else if( daopid == VAR_INDEX_FIELD ){
		if( not var->qualtype.isConstQualified() ){
			var->setter = cdao_string_fill( setter, kvmap );
			var->set_item = cdao_string_fill( set_item, kvmap );
		}
		var->get_item = cdao_string_fill( get_item, kvmap );
		kvmap[ "name" ] = "self->" + var->name;
		var->getter = cdao_string_fill( ctxput, kvmap );
	}
	//outs() << var->daopar << "\n";
	//outs() << var->dao2cxx << "\n";
	//outs() << var->cxx2dao << "\n";
	//outs() << var->parset << "\n";
}

CDaoVariable::CDaoVariable( CDaoModule *mod, const VarDecl *decl )
{
	module = mod;
	hostype = NULL;
	initor = NULL;
	ignore = false;
	extraReturn = false;
	isNullable = false;
	isCallback = false;
	isUserData = false;
	hasArrayHint = false;
	unsupported = false;
	wrapNone = false;
	wrapImport = false;
	wrapOpaque = false;
	wrapDirect = false;
	useTypeTag = false;
	useDefault = true;
	hasBaseHint = false;
	hasDeleteHint = false;
	hasUniThreadHint = false;
	hasDaoTypeHint = false;
	hasCodeBlockHint = false;
	hasMacroHint = false;
	hasMacro2Hint = false;
	hasRefCountHint = false;
	hasExternalUseHint = false;
	isArithmeticType = false;
	isObjectType = false;
	isPointerType = false;
	useDaoString = false;
	useUserWrapper = false;
	userFieldCB = false;
	argvLike = false;
	readonly = false;
	ispixels = false;
	isbuffer = false;
	isMBS = false;
	isWCS = false;
	isNew = false;
	isNumber = 0;
	SetDeclaration( decl );
}
void CDaoVariable::SetQualType( QualType qtype, SourceLocation loc )
{
	qualtype = qtype;
	location = loc;
}
void CDaoVariable::SetDeclaration( const VarDecl *decl )
{
	if( decl == NULL ) return;
	name = decl->getName().str();
	//outs() << ">>> variable: " << name << " " << decl->getType().getAsString() << "\n";
	//outs() << ">>> variable: " << name << " " << decl->getTypeSourceInfo()->getType().getAsString() << "\n";
	SetQualType( decl->getTypeSourceInfo()->getType(), decl->getLocation() );
	initor = decl->getAnyInitializer();
}
void CDaoVariable::SetHints( const string & hints )
{
	size_t pos = hints.find( "_dao_hint_" );
	string hints2, hint;
	name = hints.substr( 0, pos );
	if( pos == string::npos ) return;
	hints2 = hints.substr( pos+4 );
	while( hints2.find( "_hint_" ) == 0 ){
		pos = hints2.find( '_', 6 );
		if( pos == string::npos ){
			hint = hints2.substr( 6 );
			pos = hints2.size();
		}else{
			hint = hints2.substr( 6, pos - 6 );
		}
		if( hint == "nullable" ){
			isNullable = true;
		}else if( hint == "unsupported" ){
			unsupported = true;
		}else if( hint == "usetag" ){
			useTypeTag = true;
		}else if( hint == "string" ){
			useDaoString = true;
		}else if( hint == "argv" ){
			argvLike = true;
		}else if( hint == "readonly" ){
			readonly = true;
		}else if( hint == "ignore" ){
			ignore = true;
		}else if( hint == "callbackdata" ){
			isUserData = true;
			size_t pos2 = hints2.find( "_hint_", pos );
			if( pos2 == string::npos ) pos2 = hints2.size();
			if( pos2 > pos ) callback = hints2.substr( pos+1, pos2 - pos - 1 );
			pos = pos2;
			if( callback == "" ) errs() << "Warning: need callback name for \"callbackdata\" hint!\n";
		}else if( hint == "userwrapper" ){
			useUserWrapper = true;
			size_t pos2 = hints2.find( "_hint_", pos );
			if( pos2 == string::npos ) pos2 = hints2.size();
			if( pos2 > pos ) userWrapper = hints2.substr( pos+1, pos2 - pos - 1 );
			pos = pos2;
			if( userWrapper == "" ) errs() << "Warning: need function name for \"userwrapper\" hint!\n";
		}else if( hint == "array" || hint == "qname" || hint == "pixels" || hint == "daotype" || hint == "buffer" || hint == "int" || hint == "float" || hint == "double" || hint == "mbstring" || hint == "wcstring" || hint == "base" || hint == "wraptype" || hint == "codeblock" || hint == "delete" || hint == "new" || hint == "cxxtype" || hint == "macro" || hint == "macro2" || hint == "refcount" || hint == "extuse" || hint == "cxxbase" || hint == "implicit" || hint == "fieldcb" || hint == "unithread" ){
			bool hasMacro = false;
			size_t pos2 = hints2.find( "_hint_", pos );
			vector<string> *parts = & names;
			string hintype = hint;
			if( hint == "qname" ){
				parts = & scopes;
			}else if( hint == "array" ){
				parts = & sizes;
			}else if( hint == "pixels" ){
				ispixels = true;
			}else if( hint == "daotype" ){
				hasDaoTypeHint = true;
			}else if( hint == "codeblock" ){
				hasCodeBlockHint = true;
			}else if( hint == "buffer" ){
				isbuffer = true;
			}else if( hint == "int" ){
				isNumber = 1;
			}else if( hint == "float" ){
				isNumber = 2;
			}else if( hint == "double" ){
				isNumber = 3;
			}else if( hint == "mbstring" ){
				isMBS = true;
			}else if( hint == "wcstring" ){
				isWCS = true;
			}else if( hint == "new" ){
				isNew = true;
			}else if( hint == "base" ){
				hasBaseHint = true;
			}else if( hint == "cxxbase" ){
			}else if( hint == "implicit" ){
			}else if( hint == "macro" ){
				hasMacroHint = true;
			}else if( hint == "macro2" ){
				hasMacro2Hint = true;
			}else if( hint == "refcount" ){
				hasRefCountHint = true;
			}else if( hint == "extuse" ){
				hasExternalUseHint = true;
			}else if( hint == "fieldcb" ){
				userFieldCB = true;
			}else if( hint == "delete" ){
				hasDeleteHint = true;
			}else if( hint == "unithread" ){
				hasUniThreadHint = true;
			}

			hint = "";
			if( pos2 == string::npos ) pos2 = hints2.size();
			if( pos2 != pos ) hint = hints2.substr( pos+1, pos2 - pos - 1 );
			size_t concat = 0, from = 0;
			while( from < hint.size() ){
				pos = hint.find( '_', from );
				if( pos > hint.size() ) pos = hint.size();
				string s = hint.substr( from, pos - from );
				string converted;
				if( s == "UNDERSCORE" || s == "" ){
					converted = "_";
					concat = 1;
				}else if( s == "TIMES" ){
					converted = "*";
					concat = 1;
				}else if( s == "DOT" ){
					converted = ".";
					concat = 1;
				}else if( s == "LB" ){
					converted = "(";
					concat = 1;
				}else if( s == "RB" ){
					converted = ")";
					concat = 1;
				}else if( s == "LT" ){
					converted = "<";
					concat = 1;
				}else if( s == "GT" ){
					converted = ">";
					concat = 1;
				}else if( s == "OR" ){
					converted = "|";
					concat = 1;
				}else if( s == "COMMA" ){
					converted = ",";
					concat = 1;
				}else if( s == "AT" ){
					converted = "@";
					concat = 1;
				}else if( s == "COLON" ){
					converted = ":";
					concat = 1;
				}else if( s == "COLON2" ){
					converted = "::";
					concat = 1;
				}else if( s == "FIELD" ){
					converted = "=>";
					concat = 1;
				}else if( concat ){
					parts->back() += s;
					concat = 0;
				}else{
					parts->push_back( s );
					concat = 0;
				}
				if( converted.size() ){
					if( parts->size() ){
						parts->back() += converted;
					}else{
						parts->push_back( converted );
					}
				}
				from = pos + 1;
			}
			if( hintype == "daotype" ){
				hintDaoType = names[0];
				names.clear();
			}else if( hintype == "codeblock" && names.size() ){
				hintCodeBlock = names[0];
				names.clear();
			}else if( (hintype == "new" || hintype == "cxxtype") && names.size() ){
				hintCxxType = names[0];
				hintDaoType = names[0];
				hasDaoTypeHint = true;
				names.clear();
			}else if( hintype == "cxxbase" && names.size() ){
				hintCxxBase = names[0];
				names.clear();
			}else if( hintype == "macro" && names.size() ){
				hintMacro = names[0];
				names.clear();
			}else if( hintype == "macro2" && names.size() ){
				hintMacro2 = names[0];
				names.clear();
			}else if( hintype == "refcount" && names.size() ){
				hintRefCount = names[0];
				names.clear();
			}else if( hintype == "extuse" && names.size() ){
				hintExternalUse = names[0];
				names.clear();
			}else if( hintype == "delete" && names.size() ){
				hintDelete = names[0];
				names.clear();
			}else if( hintype == "implicit" && names.size() ){
				hintImplicit = names[0];
				names.clear();
			}else if( hintype == "wraptype" ){
				if( names[0] == "opaque" ){
					wrapOpaque = true;
				}else if( names[0] == "direct" ){
					wrapDirect = true;
				}else if( names[0] == "import" ){
					wrapImport = true;
				}else if( names[0] == "none" ){
					wrapNone = true;
				}
				names.clear();
			}
			//outs() << "array hint: " << hint << " " << sizes.size() << "\n";
			pos = pos2;
		}
		hints2.erase( 0, pos );
	}
}
int CDaoVariable::Generate( int daopar_index, int cxxpar_index )
{
	string prefix, suffix;
	int retcode;
	if( name == "" ) name = "_p" + utostr( daopar_index );
	if( module->nullPointers && name != "self" ) isNullable = true;
	retcode = Generate2( daopar_index, cxxpar_index );
	unsupported = unsupported or (retcode != 0);
	if( unsupported == false ){
		MakeCxxParameter( prefix, suffix );
		prefix = cdao_substitute_typenames( prefix );
		suffix = cdao_substitute_typenames( suffix );
		cxxpar = prefix + " " + name + suffix;
		cxxtype = prefix + suffix;
	}
	if( unsupported ) outs()<<"unsupported: "<<cxxtype<<" "<<name<<"\n";
	if( hasDaoTypeHint ){
		daotype = hintDaoType;
		daopar = name + " :" + daotype;
	}
	if( extraReturn ){
		cacheParam = parset;
		parset = "";
	}
	return retcode || unsupported;
}
int CDaoVariable::Generate2( int daopar_index, int cxxpar_index )
{
	if( initor ){
		SourceRange range = initor->getSourceRange();
		daodefault = module->ExtractSource( range, true );
		if( daodefault.find( '=' ) == 0 ) daodefault.erase(0,1);

		for(int i=scopes.size()-1; i>=0; i--){
			daodefault = scopes[i] + "::" + daodefault;
			cxxdefault = scopes[i] + "::" + cxxdefault;
		}
		if( daodefault.size() ) cxxdefault = "=" + daodefault;
		if( daodefault == "0L" ) daodefault = "0";

		std::replace( daodefault.begin(), daodefault.end(), '\"', '\'');

		Preprocessor & pp = module->compiler->getPreprocessor();
		SourceManager & sm = module->compiler->getSourceManager();
		SourceLocation start = sm.getExpansionLoc( range.getBegin() );
		SourceLocation end = sm.getExpansionLoc( range.getEnd() );
		const char *p = sm.getCharacterData( start );
		const char *q = sm.getCharacterData( pp.getLocForEndOfToken( end ) );
		std::string codes;

		while( p != q ) codes += *(p++);
		p = codes.c_str();
		q = p + codes.size();

		Lexer lexer( start, module->compiler->getLangOpts(), p, p, q );
		Token token;
		vector<Token> tokens;
		while( lexer.getBufferLocation() < q ){
			lexer.LexFromRawLexer( token );
			tokens.push_back( token );
		}
		if( tokens.size() > 1 ){
			for(int i=0,n=tokens.size(); i<n; i++){
				tok::TokenKind kind = tokens[i].getKind();
				if( kind != tok::raw_identifier && kind != tok::coloncolon ){
					daodefault = "0";
					isNullable = true;
					useDefault = false;
					break;
				}
			}
		}
	}
#if 0
	if( ParmVarDecl *par = dyn_cast<ParmVarDecl>( decl ) ){
		SourceRange range = par->getDefaultArgRange();
		outs() << (range.getBegin() == range.getEnd()) << "\n";
	}
#endif
	QualType canotype = qualtype.getCanonicalType();
	string ctypename = cdao_substitute_typenames( canotype.getAsString() );
	cxxtype2 = cdao_substitute_typenames( GetStrippedTypeName( canotype ) );
	//XXX
	if( cxxtype2.find( "class " ) == 0 ) cxxtype2.erase( 0, 6 );
	cxxtype = ctypename;
	cxxcall = name;
	//outs() << cxxtype << " " << cxxdefault << "  " << type->getTypeClassName() << "\n";
	//outs() << qualType.getAsString() << " " << qualType->getTypeClassName() << "\n";
	//outs() << type->isPointerType() << " is pointer type\n";
	//outs() << type->isArrayType() << " is array type\n";
	//outs() << type->isConstantArrayType() << " is constant array type\n";

	CDaoVarTemplates tpl;
	tpl.SetupIntScalar();
	map<string,string> kvmap;
	int m = daodefault.size();
	if( m > 1 && daodefault[m-1] == 'f' ){
		if( daodefault.find( '.' ) != string::npos ) daodefault[m-1] = '0';
	}
	if( canotype->isBuiltinType() ){
		return GenerateForBuiltin( daopar_index, cxxpar_index );
	}else if( canotype->isPointerType() and not hasArrayHint ){
		isPointerType = true;
		return GenerateForPointer( daopar_index, cxxpar_index );
	}else if( canotype->isReferenceType() ){
		return GenerateForReference( daopar_index, cxxpar_index );
	}else if( canotype->isArrayType() ){
		return GenerateForArray( daopar_index, cxxpar_index );
	}else if( canotype->isEnumeralType() ){
		daotype = "int";
		isArithmeticType = true;
		tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
		return 0;
	}else if( CDaoUserType *UT = module->HandleUserType( canotype, location ) ){
		if( UT->unsupported ) return 1;
		UT->used = true;
		daotype = cdao_make_dao_template_type_name( UT->qname );
		cxxtype2 = UT->qname;
		cxxtyper = UT->idname;
		cxxcall = name;
//		if( daopar_index >= 0 ){
			if( hostype != UT and UT->isMBString ){
				tpl.SetupMBString( UT->toValue, module->UseVariantString() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = "string";
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}else if( hostype != UT and UT->isWCString ){
				tpl.SetupWCString( UT->toValue, module->UseVariantString() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = "string";
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}else if( hostype != UT and UT->isNumber ){
				string numbers[4] = {"","int","float","float"};
				string Numbers[4] = {"","Integer","Float","Float"};
				tpl.SetupNumber( Numbers[UT->isNumber], numbers[UT->isNumber], UT->toValue, module->UseVariantNumber() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = numbers[UT->isNumber];
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}
//		}
		cxxcall = "*" + name;
		isObjectType = true;
		if( daodefault.size() ){
			daodefault = "0";
			useDefault = false;
		}
		tpl.daopar = daopar_user;
		tpl.getres = getres_user2;
		tpl.dao2cxx = dao2cxx_user2;
		tpl.cxx2dao = cxx2dao_user;
		tpl.setter = "";
		if( daopar_index == VAR_INDEX_RETURN ){
			if( dyn_cast<CXXRecordDecl>( UT->decl ) ){
				tpl.ctxput = ctxput_newcdata;
				tpl.cache = cache_newcdata;
			}else{
				tpl.ctxput = ctxput_copycdata;
				tpl.cache = cache_copycdata;
			}
		}else{
			tpl.ctxput = ctxput_refcdata;
			tpl.cache = cache_refcdata;
		}
		if( daodefault == "0" || daodefault == "NULL" ){
			daodefault = "none";
			isNullable = true;
		}
		tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
		return 0;
	}
	//TryHandleTemplateClass( qualtype );
	return 1;
}
int CDaoVariable::GenerateForBuiltin( int daopar_index, int cxxpar_index )
{
	QualType canotype = qualtype.getCanonicalType();
	CDaoVarTemplates tpl;
	if( canotype->isArithmeticType() ){
		daotype = "int";
		isNullable = false;
		isArithmeticType = true;
		tpl.SetupIntScalar();
		switch( canotype->getAs<BuiltinType>()->getKind() ){
		case BuiltinType::Bool :
			daotype = "bool";
			tpl.SetupBoolScalar();
			break;
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
		case BuiltinType::WChar_U :
		case BuiltinType::Char16 :
		case BuiltinType::Char32 :
		case BuiltinType::UShort :
		case BuiltinType::UInt :
		case BuiltinType::ULong :
		case BuiltinType::ULongLong : // FIXME
		case BuiltinType::UInt128 : // FIXME
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
		case BuiltinType::WChar_S :
		case BuiltinType::Short :
		case BuiltinType::Int :
		case BuiltinType::Long :
		case BuiltinType::LongLong : // FIXME
		case BuiltinType::Int128 : // FIXME
			break;
		case BuiltinType::Float :
			daotype = "float";
			tpl.SetupFloatScalar();
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "float";
			tpl.SetupDoubleScalar();
			break;
		default : break;
		}
		if( cxxtype != daotype ) module->daoTypedefs[ cxxtype ] = daotype;
	}
	map<string,string> kvmap;
	tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
	return 0;
}
int CDaoVariable::GenerateForPointer( int daopar_index, int cxxpar_index )
{
	bool isconst = qualtype.getCVRQualifiers() & Qualifiers::Const;
	bool isparam = daopar_index >= 0;
	char sindex[50];
	sprintf( sindex, "_p[%i]", daopar_index );
	QualType canotype = qualtype.getCanonicalType();
	QualType qtype1 = qualtype->getPointeeType();
	QualType qtype2 = canotype->getPointeeType();
	CDaoVarTemplates tpl;
	map<string,string> kvmap;
	kvmap[ "size" ] = "0";
	kvmap[ "dao" ] = "";

	if( argvLike ){
		string lpar = string("(DaoList*)") + sindex;
		daotype = "list<string>";
		cxxcall = name;
		cxxtype = "char**";
		cxxpar = "char **" + name;
		daopar = name + ": list<string>";
		dao2cxx = "  static char **__" + name + " = NULL;\n";
		dao2cxx += "  " + cxxpar + " = __" + name + " ? __" + name + " : (__" + name;
		dao2cxx = dao2cxx + " = DaoStringList_ToStaticCStringArray( " + lpar + " ));\n";
		//XXX cxx2dao = 
		module->writeStringListConversion = true;
		return 0;
	}

	if( qtype2->isBuiltinType() and qtype2->isArithmeticType() ){
		const BuiltinType *type = qtype2->getAs<BuiltinType>();
		if( not qtype2->isAnyCharacterType() ){
			if( qtype1.isConstQualified() && sizes.size() == 0 ) sizes.push_back( "0" );
		}
	}

	if( sizes.size() == 1 ) return GenerateForArray( qtype2, sizes[0], daopar_index, cxxpar_index );
	if( sizes.size() == 2 && qtype2->isPointerType() ){
		QualType qtype3 = qtype2->getAs<PointerType>()->getPointeeType();
		return GenerateForArray( qtype3, sizes[0], sizes[1], daopar_index, cxxpar_index );
	}else if( isbuffer && qtype2->isPointerType() ){
		QualType qtype3 = qtype2->getPointeeType();
		CDaoVariable var( module );
		string lpar = string("(DaoList*)") + sindex;

		var.SetQualType( qtype2 );
		var.Generate();
		if( var.unsupported ) return 1;
		if( CDaoUserType *UT = module->HandleUserType( qtype3, location ) ){
			string typer = "dao_type_" + UT->idname;
			if( UT->unsupported ) return 1;
			pre_call += "    name[_i] = DaoValue_TryCastCdata( value );\n";
			daotype = "list<" + var.daotype + ">";
			cxxcall = name;
			cxxtype = var.cxxtype + "*";
			cxxpar = cxxtype + name;
			daopar = name + " :" + daotype;
			dao2cxx = "  " + cxxpar + " = (" + cxxtype + ") calloc( ";
			dao2cxx += names[0] + ", sizeof(" + var.cxxtype + ") );\n";
			pre_call = string("  int _i, _num = DaoList_Size( ") + lpar + " );\n";
			pre_call += "  for(_i=0; _i<_num; _i++){\n";
			pre_call += string("    DaoValue *value = DaoList_GetItem( ") + lpar + ", _i );\n";
			pre_call += "    " + name + "[_i] = (" + var.cxxtype;
			pre_call += ")DaoValue_TryCastCdata( value, " + typer + " );\n  }\n";

			if( names.size() > 1 && names[1] != "" ){
				post_call = string("  DaoList_Clear( ") + lpar + " );\n";
				post_call += "  for(_i=0; _i<" + names[1] + "; _i++){\n";
				post_call += "    DaoCdata *cdata = DaoCdata_Wrap( " + typer + ", " + name + "[_i] );\n";
				post_call += string("    DaoList_Append( ") + lpar + ", (DaoValue*) cdata );\n  }\n";
			}
			post_call += "  free( " + name + " );\n";
		}else if( qtype2->isVoidPointerType() ){
			daotype = "list<" + var.daotype + ">";
			cxxcall = name;
			cxxtype = "void**";
			cxxpar = "void **" + name;
			daopar = name + " :" + daotype;
			dao2cxx = "  " + cxxpar + " = (void**) calloc( " + names[0] + ", sizeof(void*) );\n";
			pre_call = string("  int _i, _num = DaoList_Size( ") + lpar + " );\n";
			pre_call += "  for(_i=0; _i<_num; _i++){\n";
			pre_call += string("    DaoValue *value = DaoList_GetItem( ") + lpar + ", _i );\n";
			pre_call += "    " + name + "[_i] = DaoValue_TryCastCdata( value, NULL );\n  }\n";
			if( names.size() > 1 && names[1] != "" ){
				post_call = string("  DaoList_Clear( ") + lpar + " );\n";
				post_call += "  for(_i=0; _i<" + names[1] + "; _i++){\n";
				post_call += "    DaoCdata *cdata = DaoCdata_Wrap( NULL, " + name + "[_i] );\n";
				post_call += string("    DaoList_Append( ") + lpar + ", (DaoValue*) cdata );\n  }\n";
			}
			post_call += "  free( " + name + " );\n";
		}else{
			return 1;
		}
		return 0;
	}else if( isbuffer && canotype->isVoidPointerType() ){
#warning "=========================== byte array?"
		daotype = "string";
		cxxcall = name;
		cxxtype = "void*";
		cxxpar = "void *" + name;
		daopar = "&" + name + " :" + daotype;
		dao2cxx = "  " + cxxpar + " = (void*) DaoValue_TryGetChars( " + sindex + " );\n";
		pre_call = string("  DString *_str = DaoValue_TryGetString( ") + sindex + " );\n";
		pre_call += "  if( DString_Size( _str ) < " + names[0] + " ){\n";
		pre_call += string("    DString_Resize( _str, ") + names[0] + " );\n";
		pre_call += "    " + name + " = DString_GetData( _str );\n  }\n";
		if( names.size() > 1 && names[1] != "" )
			post_call = string("    DString_Resize( _str, ") + names[1] + " );\n";
		return 0;
	}else if( qtype2->isPointerType() && daopar_index >= 0 && isconst == false ){
		QualType qtype3 = qtype2->getPointeeType();
		CDaoUserType *UT = module->HandleUserType( qtype3, location );
		bool isconst2 = qtype2.getCVRQualifiers() & Qualifiers::Const;
		isconst2 |= qtype3.getCVRQualifiers() & Qualifiers::Const;
		if( isconst2 == false && UT != NULL ){
			if( UT->unsupported ) return 1;
			UT->used = true;
			if( qtype3.getAsString() == "FILE" ){
#warning"====================FILE**"
				daotype = "dao::io::Stream";
				cxxtype = "FILE";
				extraReturn = true;
				tpl.daopar = daopar_stream;
				tpl.dao2cxx = dao2cxx_stream;
				tpl.getres = getres_stream;
				tpl.ctxput = isNew ? ctxput_voidp : ctxput_user;
				tpl.cache = cache_stream;
				tpl.cxx2dao = cxx2dao_stream;
				if( daodefault == "0" || daodefault == "NULL" ) daodefault = "io";
			}else{
				daotype = cdao_make_dao_template_type_name( UT->qname );
				extraReturn = true;
				cxxtype2 = UT->qname;
				cxxtyper = UT->idname;
				tpl.daopar = daopar_user;
				tpl.ctxput = ctxput_user;
				tpl.cache = cache_user;
				tpl.getres = getres_user;
				tpl.dao2cxx = dao2cxx_user2;
				tpl.cxx2dao = cxx2dao_user;
				if( daodefault == "0" || daodefault == "NULL" ){
					daodefault = "none";
					isNullable = true;
				}
			}
			cxxcall = "&" + name;
			tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
			return 0;
		}
		return 1;
	}

	if( qtype2->isBuiltinType() and qtype2->isArithmeticType() ){
		const BuiltinType *type = qtype2->getAs<BuiltinType>();
		daotype = "int";
		cxxcall = "&" + name;
		isNullable = false;
		tpl.SetupIntScalar();
		switch( type->getKind() ){
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
			daotype = "string";
			cxxcall = name;
			tpl.SetupMBString();
			//tpl.parset = parset_mbs;
#warning"=======================const char*"
			if( daodefault == "0" || daodefault == "NULL" ){
				daodefault = "none";
				isNullable = true;
			}
			break;
		case BuiltinType::WChar_U :
		case BuiltinType::WChar_S :
		case BuiltinType::Char16 :
		case BuiltinType::Char32 :
			daotype = "string";
			cxxcall = name;
			tpl.SetupWCString();
			//tpl.parset = parset_wcs;
			if( daodefault == "0" || daodefault == "NULL" ){
				daodefault = "none";
				isNullable = true;
			}
			break;
		case BuiltinType::UShort :
		case BuiltinType::Bool : // TODO;
		case BuiltinType::UInt :
		case BuiltinType::ULong :
		case BuiltinType::ULongLong :
		case BuiltinType::UInt128 :
		case BuiltinType::Short :
		case BuiltinType::Int :
		case BuiltinType::Long :
		case BuiltinType::LongLong :  // FIXME
		case BuiltinType::Int128 :  // FIXME
			daotype = "int";
#warning"=======================const int*"
			extraReturn = true;
			tpl.SetupIntScalar();
			tpl.parset = parset_int;
			tpl.ctxput = isparam ? ctxput_int : ctxput_ints;
			tpl.cache = cache_ints;
			tpl.getres = getres_ints;
			tpl.cxx2dao = cxx2dao_int2;
			break;
		case BuiltinType::Float :
			daotype = "float";
			extraReturn = true;
			tpl.SetupFloatScalar();
			tpl.parset = parset_float;
			tpl.ctxput = isparam ? ctxput_float : ctxput_floats;
			tpl.cache = cache_floats;
			tpl.getres = getres_floats;
			tpl.cxx2dao = cxx2dao_float2;
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "float";
			extraReturn = true;
			tpl.SetupDoubleScalar();
			tpl.parset = parset_double;
			tpl.ctxput = isparam ? ctxput_double : ctxput_doubles;
			tpl.cache = cache_doubles;
			tpl.getres = getres_doubles;
			tpl.cxx2dao = cxx2dao_double2;
			break;
		default : break;
		}
		tpl.setter = "";
	}else if( qtype2->isEnumeralType() ){
		daotype = "int";
		cxxcall = "&" + name;
		isNullable = false;

		tpl.SetupIntScalar();
		tpl.parset = parset_int;
		tpl.ctxput = isparam ? ctxput_int : ctxput_ints;
		tpl.cache = cache_ints;
		tpl.getres = getres_ints;
		tpl.cxx2dao = cxx2dao_int2;
		tpl.setter = "";
	}else if( CDaoUserType *UT = module->HandleUserType( qtype1, location ) ){
		if( UT->unsupported ) return 1;
		UT->used = true;
		if( qtype1.getAsString() == "FILE" ){
			daotype = "dao::io::Stream";
			cxxtype = "FILE";
			tpl.daopar = daopar_stream;
			tpl.dao2cxx = dao2cxx_stream;
			tpl.getres = getres_stream;
			tpl.ctxput = ctxput_stream;
			tpl.cache = cache_stream;
			tpl.cxx2dao = cxx2dao_stream;
			if( daodefault == "0" || daodefault == "NULL" ) daodefault = "io";
		}else{
			daotype = cdao_make_dao_template_type_name( UT->qname );
			cxxtype2 = UT->qname;
			cxxtyper = UT->idname;
			tpl.daopar = daopar_user;
			tpl.ctxput = isNew ? ctxput_voidp : ctxput_user;
			tpl.cache = cache_user;
			tpl.getres = getres_user;
			tpl.dao2cxx = dao2cxx_user2;
			tpl.cxx2dao = cxx2dao_user;
			if( daopar_index < 0 and isNullable ){
				daotype = daotype + "|none";
				tpl.ctxput = ctxput_nullable_user;
			}
			if( daodefault == "0" || daodefault == "NULL" ){
				daodefault = "none";
				isNullable = true;
			}
		}
	}else if( canotype->isVoidPointerType() ){
		if( isUserData ){
			daotype = "any";
			cxxtype = "DaoValue*";
			cxxpar = "DaoValue *" + name;
			cxxcall = name;
			tpl.daopar = daopar_userdata;
			tpl.dao2cxx = dao2cxx_userdata;
			tpl.cxx2dao = cxx2dao_userdata;
			kvmap[ "callback" ] = callback;
		}else if( hasDaoTypeHint && hintDaoType == "any" ){
			daotype = "any";
			tpl.daopar = daopar_daovalue;
			tpl.dao2cxx = dao2cxx_daovalue;
			tpl.cxx2dao = cxx2dao_daovalue;
			tpl.ctxput = ctxput_daovalue;
			tpl.cache = cache_daovalue;
		}else{
			daotype = "cdata";
			tpl.daopar = daopar_buffer;
			tpl.dao2cxx = dao2cxx_void;
			tpl.cxx2dao = cxx2dao_voidp;
			tpl.ctxput = isNew ? ctxput_voidp2 : ctxput_user2;
			tpl.cache = cache_voidp;
			if( daopar_index < 0 and isNullable ){
				daotype = daotype + "|none";
				tpl.ctxput = ctxput_nullable_user2;
			}
			if( hasDaoTypeHint && hintDaoType.find( "array" ) == 0 ){
				daotype = hintDaoType;
				tpl.daopar = "$(name):" + daotype + "$(default)";
				tpl.dao2cxx = dao2cxx_array_buffer;
			}
		}
		if( daodefault == "0" || daodefault == "NULL" ){
			daodefault = "none";
			isNullable = true;
		}
	}else if( const FunctionProtoType *ft = qtype2->getAs<FunctionProtoType>() ){
		//outs() << "callback: " << qualType.getAsString() << " " << qtype2.getAsString() << "\n";
		if( module->allCallbacks.find( ft ) == module->allCallbacks.end() ){
			module->allCallbacks[ ft ] = new CDaoFunction( module );
			CDaoFunction *func = module->allCallbacks[ ft ];
			string qname = GetStrippedTypeName( qtype1 );
			func->SetCallback( (FunctionProtoType*)ft, NULL, qname );
			func->cxxName = cdao_qname_to_idname( qname );
			func->location = location;
			if( func->retype.callback == "" ){
				errs() << "Warning: callback \"" << qtype1.getAsString() << "\" is not supported!\n";
				func->excluded = true;
			}
		}
		CDaoFunction *func = module->allCallbacks[ ft ];
		func->Generate();
		if( func->excluded ) return 1;
		daotype = "any";
		isCallback = true;
		tpl.daopar = daopar_callback;
		tpl.dao2cxx = dao2cxx_callback;
		kvmap[ "callback" ] = func->cxxName; // XXX
	}else{
		return 1;
	}
	tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
	if( qtype2.getCVRQualifiers() & Qualifiers::Const ){
		extraReturn = false;
		parset = "";
		setter = "";
	}
	return 0;
}
int CDaoVariable::GenerateForReference( int daopar_index, int cxxpar_index )
{
	QualType canotype = qualtype.getCanonicalType();
	QualType qtype1 = qualtype->getPointeeType();
	QualType qtype2 = canotype->getPointeeType();

	CDaoVarTemplates tpl;
	map<string,string> kvmap;
	if( qtype2->isBuiltinType() and qtype2->isArithmeticType() ){
		const BuiltinType *type = qtype2->getAs<BuiltinType>();
		daotype = "int";
		cxxcall = name;
		isNullable = false;
		tpl.SetupIntScalar();
		switch( type->getKind() ){
		case BuiltinType::Bool :
			daotype = "bool";
			extraReturn = true;
			tpl.SetupBoolScalar();
			tpl.parset = parset_bool;
			break;
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
		case BuiltinType::WChar_U :
		case BuiltinType::WChar_S :
		case BuiltinType::Char16 :
		case BuiltinType::Char32 :
		case BuiltinType::UShort :
		case BuiltinType::UInt :
		case BuiltinType::ULong :
		case BuiltinType::ULongLong :
		case BuiltinType::UInt128 :
		case BuiltinType::Short :
		case BuiltinType::Int :
		case BuiltinType::Long :
		case BuiltinType::LongLong :  // FIXME
		case BuiltinType::Int128 :  // FIXME
			daotype = "int";
#warning"=======================const int*"
			extraReturn = true;
			tpl.SetupIntScalar();
			tpl.parset = parset_int;
			break;
		case BuiltinType::Float :
			daotype = "float";
			extraReturn = true;
			tpl.SetupFloatScalar();
			tpl.parset = parset_float;
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "float";
			extraReturn = true;
			tpl.SetupDoubleScalar();
			tpl.parset = parset_double;
			break;
		default : break;
		}
	}else if( qtype2->isEnumeralType() ){
		daotype = "int";
		cxxcall = name;
		isNullable = false;
		extraReturn = true;

		tpl.SetupIntScalar();
		tpl.parset = parset_int;
	}else if( CDaoUserType *UT = module->HandleUserType( qtype1, location ) ){
		if( UT->unsupported ) return 1;
		UT->used = true;
		daotype = cdao_make_dao_template_type_name( UT->qname );
		cxxtype2 = UT->qname;
		cxxtyper = UT->idname;
		cxxcall = name;
//		if( daopar_index >= 0 ){
			if( hostype != UT and UT->isMBString ){
				tpl.SetupMBString( UT->toValue, module->UseVariantString() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = "string";
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}else if( hostype != UT and UT->isWCString ){
				tpl.SetupWCString( UT->toValue, module->UseVariantString() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = "string";
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}else if( hostype != UT and UT->isNumber ){
				string numbers[4] = {"","int","float","float"};
				string Numbers[4] = {"","Integer","Float","Float"};
				tpl.SetupNumber( Numbers[UT->isNumber], numbers[UT->isNumber], UT->toValue, module->UseVariantNumber() );
				tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
				daotype = numbers[UT->isNumber];
				if( qualtype.getCVRQualifiers() & Qualifiers::Const ){
					extraReturn = false;
					parset = "";
				}
				return 0;
			}
//		}
		cxxcall = "*" + name;
		tpl.daopar = daopar_user;
		tpl.ctxput = ctxput_refcdata;
		tpl.cache = cache_refcdata;
		tpl.getres = getres_user;
		tpl.dao2cxx = dao2cxx_user2;
		tpl.cxx2dao = cxx2dao_user;
		if( daodefault.size() ){
			daodefault = "0";
			useDefault = false;
		}
		if( daodefault == "0" || daodefault == "NULL" ){
			daodefault = "none";
			isNullable = true;
		}
	}else{
		return 1;
	}
	tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
	if( qtype2.getCVRQualifiers() & Qualifiers::Const ){
		extraReturn = false;
		parset = "";
		setter = "";
	}
	return 0;
}
int CDaoVariable::GenerateForArray( int daopar_index, int cxxpar_index )
{
	bool constsize = false;
	string size;
	QualType canotype = qualtype.getCanonicalType();
	const ArrayType *type = (ArrayType*)canotype.getTypePtr();
	if( canotype->isConstantArrayType() ){
		const ConstantArrayType *at = (ConstantArrayType*)canotype.getTypePtr();
		size = at->getSize().toString( 10, false );
		constsize = true;
	}
	return GenerateForArray( type->getElementType(), size, daopar_index, cxxpar_index, constsize );
}
int CDaoVariable::GenerateForArray( QualType elemtype, string size, int daopar_index, int cxxpar_index, bool constsize )
{
	const Type *type2 = elemtype.getTypePtr();
	string ctypename2 = elemtype.getAsString();
	if( type2->isArrayType() ){
		const ArrayType *type3 = (ArrayType*) type2;
		string size2;
		if( type2->isConstantArrayType() ){
			ConstantArrayType *at = (ConstantArrayType*) type2;
			size2 = at->getSize().toString( 10, false );
		}
		return GenerateForArray2( type3->getElementType(), size, size2, daopar_index, cxxpar_index );
	}
	CDaoVarTemplates tpl;
	map<string,string> kvmap;
	kvmap[ "dao" ] = "";
	if( type2->isBuiltinType() and type2->isArithmeticType() ){
		const BuiltinType *type3 = (const BuiltinType*) type2;
		daotype = "array<int>";
		dao_itemtype = "int";
		cxxcall = name;
		tpl.daopar = daopar_ints;
		tpl.ctxput = ctxput_ints;
		tpl.cache = cache_ints;
		tpl.parset = parset_ints;
		tpl.getres = getres_ints;
		tpl.setter = setter_ints;
		tpl.get_item = name == "this" ? getitem_int2 : getitem_int;
		tpl.set_item = name == "this" ? setitem_int2 : setitem_int;
		if( daodefault == "0" || daodefault == "NULL" ) daodefault = "[]";
		switch( type3->getKind() ){
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
			tpl.dao2cxx = dao2cxx_bytes;
			tpl.cxx2dao = cxx2dao_bytes;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_bytes;
			tpl.getres = getres_bytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::Bool :
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
			tpl.dao2cxx = dao2cxx_ubytes;
			tpl.cxx2dao = cxx2dao_ubytes;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_ubytes;
			tpl.getres = getres_ubytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::UShort :
			tpl.dao2cxx = dao2cxx_ushorts;
			tpl.cxx2dao = cxx2dao_ushorts;
			tpl.ctxput = ctxput_ushorts;
			tpl.cache = cache_ushorts;
			tpl.parset = parset_ushorts;
			tpl.getres = getres_ushorts;
			tpl.setter = setter_ushorts;
			break;
		case BuiltinType::Char16 :
		case BuiltinType::Short :
			tpl.dao2cxx = dao2cxx_shorts;
			tpl.cxx2dao = cxx2dao_shorts;
			tpl.ctxput = ctxput_shorts;
			tpl.cache = cache_shorts;
			tpl.parset = parset_shorts;
			tpl.getres = getres_shorts;
			tpl.setter = setter_shorts;
			break;
		case BuiltinType::UInt :
			tpl.dao2cxx = dao2cxx_uints;
			tpl.cxx2dao = cxx2dao_uints;
			tpl.ctxput = ctxput_uints;
			tpl.cache = cache_uints;
			tpl.parset = parset_uints;
			tpl.getres = getres_uints;
			tpl.setter = setter_uints;
			break;
		case BuiltinType::Int :
		case BuiltinType::Char32 :
			tpl.dao2cxx = dao2cxx_ints;
			tpl.cxx2dao = cxx2dao_ints;
			tpl.ctxput = ctxput_ints;
			tpl.cache = cache_ints;
			tpl.parset = parset_ints;
			tpl.getres = getres_ints;
			tpl.setter = setter_ints;
			break;
		case BuiltinType::WChar_U : // FIXME: check size
		case BuiltinType::ULong :
		case BuiltinType::ULongLong : // FIXME
		case BuiltinType::UInt128 : // FIXME
			tpl.dao2cxx = dao2cxx_uints;
			tpl.cxx2dao = cxx2dao_uints;
			break;
		case BuiltinType::WChar_S :
		case BuiltinType::Long : // FIXME: check size
		case BuiltinType::LongLong : // FIXME
		case BuiltinType::Int128 : // FIXME
			tpl.dao2cxx = dao2cxx_ints;
			tpl.cxx2dao = cxx2dao_ints;
			break;
		case BuiltinType::Float :
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_floats;
			tpl.dao2cxx = dao2cxx_floats;
			tpl.cxx2dao = cxx2dao_floats;
			tpl.ctxput = ctxput_floats;
			tpl.cache = cache_floats;
			tpl.parset = parset_floats;
			tpl.getres = getres_floats;
			tpl.setter = setter_floats;
			tpl.get_item = name == "this" ? getitem_float2 : getitem_float;
			tpl.set_item = name == "this" ? setitem_float2 : setitem_float;
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_doubles;
			tpl.dao2cxx = dao2cxx_doubles;
			tpl.cxx2dao = cxx2dao_doubles;
			tpl.ctxput = ctxput_doubles;
			tpl.cache = cache_doubles;
			tpl.parset = parset_doubles;
			tpl.getres = getres_doubles;
			tpl.setter = setter_doubles;
			tpl.get_item = name == "this" ? getitem_double2 : getitem_double;
			tpl.set_item = name == "this" ? setitem_double2 : setitem_double;
			break;
		default : break;
		}
	}else if( elemtype->isPointerType() ){
		QualType ptype = elemtype->getPointeeType();
		if( CDaoUserType *UT = module->HandleUserType( ptype, location ) ){
			if( UT->unsupported ) return 1;
			if( daopar_index != VAR_INDEX_FIELD ) return 1;
			UT->used = true;
			daotype = cdao_make_dao_template_type_name( UT->qname );
			cxxtype = UT->qname + "**";
			cxxtype2 = UT->qname + "**";
			cxxtyper = UT->idname;
			daotype = "list<" + daotype + ">";
			daopar = name + " :" + daotype;
			getter = "  DaoList *list = DaoProcess_PutList( _proc );\n";
			if( constsize ){
				getter += "  daoint i, n = " + size + ";\n";
			}else{
				getter += "  daoint i, n = self->" + size + ";\n";
			}
			getter += "  for(i=0; i<n; i++){\n";
			getter += "    DaoCdata *it = DaoProcess_NewCdata( _proc, dao_type_" + UT->idname;
			getter += ", self->" + name + "[i], 0 );\n";
			getter += "    DaoList_PushBack( list, (DaoValue*) it );\n";
			getter += "  }\n";
			return 0;
		}
		return 1;
	}else{
		return 1;
	}
	if( size.size() == 0 ) size = "0";
	kvmap[ "size" ] = size;
	tpl.Generate( this, kvmap, daopar_index, cxxpar_index );
	if( elemtype.getCVRQualifiers() & Qualifiers::Const ){
		/* DaoArray_FromXXX() is needed regardless of the "const" qualifier: */
		//parset = "";
		setter = "";
	}
	return 0;
}
int CDaoVariable::GenerateForArray( QualType elemtype, string size, string size2, int dpid, int cpid )
{
	const Type *type2 = elemtype.getTypePtr();
	string ctypename2 = elemtype.getAsString();
	CDaoVarTemplates tpl;
	map<string,string> kvmap;
	kvmap[ "dao" ] = "";
	if( type2->isBuiltinType() and type2->isArithmeticType() ){
		const BuiltinType *type3 = (const BuiltinType*) type2;
		daotype = "array<int>";
		dao_itemtype = "int";
		cxxcall = name;
		tpl.daopar = daopar_ints;
		tpl.ctxput = ctxput_ints;
		tpl.cache = cache_ints;
		tpl.parset = parset_ints;
		tpl.getres = getres_ints;
		tpl.setter = setter_ints;
		tpl.get_item = name == "this" ? getitem_int2 : getitem_int;
		tpl.set_item = name == "this" ? setitem_int2 : setitem_int;
		if( daodefault == "0" || daodefault == "NULL" ) daodefault = "[]";
		switch( type3->getKind() ){
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
			tpl.dao2cxx = dao2cxx_bmat;
			tpl.cxx2dao = cxx2dao_bmat;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_bytes;
			tpl.getres = getres_bytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::Bool :
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
			tpl.dao2cxx = dao2cxx_ubmat;
			tpl.cxx2dao = cxx2dao_ubmat;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_ubytes;
			tpl.getres = getres_ubytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::UShort :
			tpl.dao2cxx = dao2cxx_usmat;
			tpl.cxx2dao = cxx2dao_usmat;
			tpl.ctxput = ctxput_shorts;
			tpl.cache = cache_shorts;
			tpl.parset = parset_ushorts;
			tpl.getres = getres_ushorts;
			tpl.setter = setter_shorts;
			break;
		case BuiltinType::Char16 :
		case BuiltinType::Short :
			tpl.dao2cxx = dao2cxx_smat;
			tpl.cxx2dao = cxx2dao_smat;
			tpl.ctxput = ctxput_shorts;
			tpl.cache = cache_shorts;
			tpl.parset = parset_shorts;
			tpl.getres = getres_shorts;
			tpl.setter = setter_shorts;
			break;
		case BuiltinType::WChar_U : // FIXME: check size
		case BuiltinType::UInt :
		case BuiltinType::ULong :
		case BuiltinType::ULongLong : // FIXME
		case BuiltinType::UInt128 : // FIXME
			tpl.dao2cxx = dao2cxx_uimat;
			tpl.cxx2dao = cxx2dao_uimat;
			break;
		case BuiltinType::WChar_S :
		case BuiltinType::Char32 :
		case BuiltinType::Int :
		case BuiltinType::Long : // FIXME: check size
		case BuiltinType::LongLong : // FIXME
		case BuiltinType::Int128 : // FIXME
			tpl.dao2cxx = dao2cxx_imat;
			tpl.cxx2dao = cxx2dao_imat;
			break;
		case BuiltinType::Float :
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_floats;
			tpl.dao2cxx = dao2cxx_fmat;
			tpl.cxx2dao = cxx2dao_fmat;
			tpl.ctxput = ctxput_floats;
			tpl.cache = cache_floats;
			tpl.parset = parset_floats;
			tpl.getres = getres_floats;
			tpl.setter = setter_floats;
			tpl.get_item = name == "this" ? getitem_float2 : getitem_float;
			tpl.set_item = name == "this" ? setitem_float2 : setitem_float;
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_doubles;
			tpl.dao2cxx = dao2cxx_dmat;
			tpl.cxx2dao = cxx2dao_dmat;
			tpl.ctxput = ctxput_doubles;
			tpl.cache = cache_doubles;
			tpl.parset = parset_doubles;
			tpl.getres = getres_doubles;
			tpl.setter = setter_doubles;
			tpl.get_item = name == "this" ? getitem_double2 : getitem_double;
			tpl.set_item = name == "this" ? setitem_double2 : setitem_double;
			break;
		default : break;
		}
	}else{
		return 1;
	}
	if( size.size() == 0 ) size = "0";
	if( size2.size() == 0 ) size2 = "0";
	kvmap[ "size" ] = size;
	kvmap[ "size2" ] = size2;
	tpl.Generate( this, kvmap, dpid, cpid );
	if( elemtype.getCVRQualifiers() & Qualifiers::Const ){
		/* DaoArray_FromXXX() is needed regardless of the "const" qualifier: */
		//parset = "";
		setter = "";
	}
	return 0;
}
int CDaoVariable::GenerateForArray2( QualType elemtype, string size, string size2, int dpid, int cpid )
{
	const Type *type2 = elemtype.getTypePtr();
	string ctypename2 = elemtype.getAsString();
	CDaoVarTemplates tpl;
	map<string,string> kvmap;
	kvmap[ "dao" ] = "";
	if( type2->isBuiltinType() and type2->isArithmeticType() ){
		const BuiltinType *type3 = (const BuiltinType*) type2;
		daotype = "array<int>";
		dao_itemtype = "int";
		cxxcall = name;
		tpl.daopar = daopar_ints;
		tpl.ctxput = ctxput_ints;
		tpl.cache = cache_ints;
		tpl.parset = parset_ints;
		tpl.getres = getres_ints;
		tpl.setter = setter_ints;
		tpl.get_item = name == "this" ? getitem_int2 : getitem_int;
		tpl.set_item = name == "this" ? setitem_int2 : setitem_int;
		if( daodefault == "0" || daodefault == "NULL" ) daodefault = "[]";
		switch( type3->getKind() ){
		case BuiltinType::Char_S :
		case BuiltinType::SChar :
			tpl.dao2cxx = dao2cxx_bmat2;
			tpl.cxx2dao = cxx2dao_bmat2;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_bytes;
			tpl.getres = getres_bytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::Bool :
		case BuiltinType::Char_U :
		case BuiltinType::UChar :
			tpl.dao2cxx = dao2cxx_ubmat2;
			tpl.cxx2dao = cxx2dao_ubmat2;
			tpl.ctxput = ctxput_bytes;
			tpl.cache = cache_bytes;
			tpl.parset = parset_ubytes;
			tpl.getres = getres_ubytes;
			tpl.setter = setter_string;
			break;
		case BuiltinType::UShort :
			tpl.dao2cxx = dao2cxx_usmat2;
			tpl.cxx2dao = cxx2dao_usmat2;
			tpl.ctxput = ctxput_shorts;
			tpl.cache = cache_shorts;
			tpl.parset = parset_ushorts;
			tpl.getres = getres_ushorts;
			tpl.setter = setter_shorts;
			break;
		case BuiltinType::Char16 :
		case BuiltinType::Short :
			tpl.dao2cxx = dao2cxx_smat2;
			tpl.cxx2dao = cxx2dao_smat2;
			tpl.ctxput = ctxput_shorts;
			tpl.cache = cache_shorts;
			tpl.parset = parset_shorts;
			tpl.getres = getres_shorts;
			tpl.setter = setter_shorts;
			break;
		case BuiltinType::WChar_U : // FIXME: check size
		case BuiltinType::UInt :
		case BuiltinType::ULong :
		case BuiltinType::ULongLong : // FIXME
		case BuiltinType::UInt128 : // FIXME
			tpl.dao2cxx = dao2cxx_uimat2;
			tpl.cxx2dao = cxx2dao_uimat2;
			break;
		case BuiltinType::WChar_S :
		case BuiltinType::Char32 :
		case BuiltinType::Int :
		case BuiltinType::Long : // FIXME: check size
		case BuiltinType::LongLong : // FIXME
		case BuiltinType::Int128 : // FIXME
			tpl.dao2cxx = dao2cxx_imat2;
			tpl.cxx2dao = cxx2dao_imat2;
			break;
		case BuiltinType::Float :
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_floats;
			tpl.dao2cxx = dao2cxx_fmat2;
			tpl.cxx2dao = cxx2dao_fmat2;
			tpl.ctxput = ctxput_floats;
			tpl.cache = cache_floats;
			tpl.parset = parset_floats;
			tpl.getres = getres_floats;
			tpl.setter = setter_floats;
			tpl.get_item = name == "this" ? getitem_float2 : getitem_float;
			tpl.set_item = name == "this" ? setitem_float2 : setitem_float;
			break;
		case BuiltinType::Double :
		case BuiltinType::LongDouble : // FIXME
			daotype = "array<float>";
			dao_itemtype = "float";
			tpl.daopar = daopar_doubles;
			tpl.dao2cxx = dao2cxx_dmat2;
			tpl.cxx2dao = cxx2dao_dmat2;
			tpl.ctxput = ctxput_doubles;
			tpl.cache = cache_doubles;
			tpl.parset = parset_doubles;
			tpl.getres = getres_doubles;
			tpl.setter = setter_doubles;
			tpl.get_item = name == "this" ? getitem_double2 : getitem_double;
			tpl.set_item = name == "this" ? setitem_double2 : setitem_double;
			break;
		default : break;
		}
	}else{
		return 1;
	}
	if( size.size() == 0 ) size = "0";
	if( size2.size() == 0 ) size2 = "0";
	kvmap[ "size" ] = size;
	kvmap[ "size2" ] = size2;
	tpl.Generate( this, kvmap, dpid, cpid );
	if( elemtype.getCVRQualifiers() & Qualifiers::Const ){
		/* DaoArray_FromXXX() is needed regardless of the "const" qualifier: */
		//parset = "";
		setter = "";
	}
	return 0;
}
void CDaoVariable::MakeCxxParameter( string & prefix, string & suffix )
{
	const Type *type = qualtype.getTypePtr();
	prefix = "";
	if( const TypedefType *TDT = dyn_cast<TypedefType>( type ) ){
		DeclContext *DC = TDT->getDecl()->getDeclContext();
		if( DC->isNamespace() ){
			NamespaceDecl *ND = dyn_cast<NamespaceDecl>( DC );
			prefix = ND->getQualifiedNameAsString() + "::";
		}else if( DC->isRecord() ){
			RecordDecl *RD = dyn_cast<RecordDecl>( DC )->getDefinition();
			CDaoUserType *UT = module->GetUserType( RD );
			//outs() << RD->getQualifiedNameAsString() << " " << UT << " " << (void*)RD << "\n";
			if( hostype && hostype->decl ){
				// For vector( size_type __n, ... ), the canonical type name of size_type is
				// std::vector::size_type, even for vector<bool>. And RD will be a specialization
				// different from vector<bool>!
				RecordDecl *RD2 = hostype->decl;
				ClassTemplateSpecializationDecl *CTS = dyn_cast<ClassTemplateSpecializationDecl>( RD2 );
				ClassTemplateSpecializationDecl *CTS2 = dyn_cast<ClassTemplateSpecializationDecl>( RD );
				CXXRecordDecl *CRD1 = CTS ? CTS->getSpecializedTemplate()->getTemplatedDecl() : NULL;
				CXXRecordDecl *CRD2 = CTS2 ? CTS2->getSpecializedTemplate()->getTemplatedDecl() : NULL;
				if( CRD1 && CRD2 && CRD1 == CRD2 ) UT = hostype;
				if( CTS && RD == CTS->getSpecializedTemplate()->getTemplatedDecl() ) UT = hostype;
			}
			if( UT ){
				prefix = UT->qname + "::";
			}else{
				prefix = RD->getQualifiedNameAsString() + "::";
			}
			//outs() << ">>>>>>>>>>>>>>> " << name <<  " " << prefix << " " << (void*)this << "\n";
		}
		prefix += qualtype.getUnqualifiedType().getAsString();
	}else{
		QualType canotype = qualtype.getCanonicalType();
		MakeCxxParameter( canotype, prefix, suffix );
	}
}
void CDaoVariable::MakeCxxParameter( QualType qtype, string & prefix, string & suffix )
{
	const Type *type = qtype.getTypePtr();
	const RecordDecl *decl = type->getAsCXXRecordDecl();
	if( decl == NULL ){
		const RecordType *t = type->getAsStructureType();
		if( t == NULL ) t = type->getAsUnionType();
		if( t ) decl = t->getDecl();
	}
	if( type->isBooleanType() ){
		prefix = "bool" + prefix;
	}else if( type->isBuiltinType() ){
		prefix = qtype.getUnqualifiedType().getAsString() + prefix;
	}else if( type->isPointerType() ){
		const PointerType *type2 = (const PointerType*) type;
		prefix += "*";
		MakeCxxParameter( type2->getPointeeType(), prefix, suffix );
	}else if( type->isReferenceType() ){
		const ReferenceType *type2 = (const ReferenceType*) type;
		prefix += "&";
		MakeCxxParameter( type2->getPointeeType(), prefix, suffix );
	}else if( type->isArrayType() ){
		const ArrayType *type2 = (const ArrayType*) type;
		string size = "[]";
		if( type->isConstantArrayType() ){
			ConstantArrayType *at = (ConstantArrayType*) type;
			size = "[" + at->getSize().toString( 10, false ) + "]";
		}
		suffix += size;
		MakeCxxParameter( type2->getElementType(), prefix, suffix );
	}else if( type->isEnumeralType() ){
		const EnumType *type2 = (const EnumType*) type;
		const EnumDecl *edec = type2->getDecl();
		const DeclContext *parent = edec ? edec->getParent() : NULL;
		if( parent && parent->isRecord() && edec->getAccess() != AS_public ) unsupported = true;
		prefix = cdao_substitute_typenames( qtype.getUnqualifiedType().getAsString() ) + prefix;
	}else if( decl ){
		// const C & other: const is part of the name, not a qualifier.
		prefix = cdao_substitute_typenames(qtype.getUnqualifiedType().getAsString()) + prefix;
	}
	if( qtype.getCVRQualifiers() & Qualifiers::Const ){
		if( type->isPointerType() ){
			prefix = prefix + "const ";
		}else{
			prefix = "const " + prefix;
		}
	}
}
QualType CDaoVariable::GetStrippedType( QualType qtype )
{
	const Type *type = qtype.getTypePtr();
	if( type->isPointerType() ){
		const PointerType *type2 = (const PointerType*) type;
		return type2->getPointeeType();
		return GetStrippedType( type2->getPointeeType() );
	}else if( type->isReferenceType() ){
		const ReferenceType *type2 = (const ReferenceType*) type;
		return GetStrippedType( type2->getPointeeType() );
	}else if( type->isArrayType() ){
		const ArrayType *type2 = (const ArrayType*) type;
		return GetStrippedType( type2->getElementType() );
	}
	return qtype;
}
string CDaoVariable::GetStrippedTypeName( QualType qtype )
{
	QualType QT = GetStrippedType( qtype );
	if( QT->isBooleanType() ) return "bool"; // clang always produces "_Bool"
	return QT.getAsString();
}
