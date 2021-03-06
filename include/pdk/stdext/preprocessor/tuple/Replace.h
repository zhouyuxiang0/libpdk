// @copyright 2017-2018 zzu_softboy <zzu_softboy@163.com>
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Created by softboy on 2018/01/15.

// ****************************************************************************
// *                                                                          *
// *     (C) Copyright Paul Mensonides 2002-2011.                             *
// *     (C) Copyright Edward Diener 2011.                                    *
// *     Distributed under the Boost Software License, Version 1.0. (See      *
// *     accompanying file LICENSE_1_0.txt or copy at                         *
// *     http://www.boost.org/LICENSE_1_0.txt)                                *
// *                                                                          *
// ****************************************************************************
// 
// See http://www.boost.org for most recent version.

#ifndef PDK_STDEXT_PREPROCESSOR_TUPLE_REPLACE_H
#define PDK_STDEXT_PREPROCESSOR_TUPLE_REPLACE_H

#include "pdk/stdext/preprocessor/config/Config.h"

# if PDK_PP_VARIADICS

#include "pdk/stdext/preprocessor/array/Replace.h"
#include "pdk/stdext/preprocessor/array/ToTuple.h"
#include "pdk/stdext/preprocessor/tuple/ToArray.h"

// PDK_PP_TUPLE_REPLACE
# define PDK_PP_TUPLE_REPLACE(tuple, i, elem) \
	PDK_PP_ARRAY_TO_TUPLE(PDK_PP_ARRAY_REPLACE(PDK_PP_TUPLE_TO_ARRAY(tuple), i, elem)) \
/**/

// PDK_PP_TUPLE_REPLACE_D
# define PDK_PP_TUPLE_REPLACE_D(d, tuple, i, elem) \
	PDK_PP_ARRAY_TO_TUPLE(PDK_PP_ARRAY_REPLACE_D(d, PDK_PP_TUPLE_TO_ARRAY(tuple), i, elem)) \
/**/

# endif // PDK_PP_VARIADICS

#endif // PDK_STDEXT_PREPROCESSOR_TUPLE_REPLACE_H
