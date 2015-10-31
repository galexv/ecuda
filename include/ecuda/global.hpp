/*
Copyright (c) 2014-2015, Scott Zuyderduyn
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those
of the authors and should not be interpreted as representing official policies,
either expressed or implied, of the FreeBSD Project.
*/

//----------------------------------------------------------------------------
// global.hpp
//
// Global defines and macros.
//
// Author: Scott D. Zuyderduyn, Ph.D. (scott.zuyderduyn@utoronto.ca)
//----------------------------------------------------------------------------

#pragma once
#ifndef ECUDA_GLOBAL_HPP
#define ECUDA_GLOBAL_HPP

#include <stdexcept>
#include <sstream>

#include "cuda_error.hpp"

// Alias for detecting C++11 support because GCC 4.6 screws up the __cplusplus flag
#if __cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__)
#define __CPP11_SUPPORTED__
#endif

///
/// Function wrapper that capture and throw an exception on error.  All calls
/// to functions in the CUDA API that return an error code should use this.
///
#define CUDA_CALL(x) do { if((x)!=cudaSuccess) { std::ostringstream oss; oss << __FILE__; oss << ":"; oss << __LINE__; oss << " "; oss << cudaGetErrorString(cudaGetLastError()); throw ::ecuda::cuda_error(x,oss.str()); /*std::runtime_error(oss.str());*/ }} while(0);

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)
///
/// String wrapper that adds the source file and line to a given error message.
///
#define EXCEPTION_MSG(x) "" __FILE__ ":" S__LINE__ " " x

///
/// Macro that performs a check for any outstanding CUDA errors.  This macro
/// should be declared after any CUDA API calls that do not return an error code
/// (e.g. after calling kernel functions).
///
#define CUDA_CHECK_ERRORS() do { cudaError_t error = cudaGetLastError(); if( error != cudaSuccess ) throw ::ecuda::cuda_error(error,std::string(cudaGetErrorString(error))); } while(0);

///
/// Macro that calls a CUDA kernel function, waits for completion, and throws
/// an ecuda::cuda_error exception if any errors are reported by cudaGetLastError().
///
#define CUDA_CALL_KERNEL_AND_WAIT(...) do {\
		__VA_ARGS__;\
		{ cudaError_t error = cudaGetLastError(); if( error != cudaSuccess ) throw ::ecuda::cuda_error(error,std::string(cudaGetErrorString(error))); }\
		cudaDeviceSynchronize();\
		{ cudaError_t error = cudaGetLastError(); if( error != cudaSuccess ) throw ::ecuda::cuda_error(error,std::string(cudaGetErrorString(error))); }\
	} while(0);

/** Replace nullptr with NULL if nvcc still doesn't support C++11. */
#ifndef __CPP11_SUPPORTED__
#define nullptr NULL
#endif

/** Allow noexcept and constexpr if C++11 supported. */
#ifdef __CPP11_SUPPORTED__
#define __NOEXCEPT__ noexcept
#define __CONSTEXPR__ constexpr
#else
#define __NOEXCEPT__
#define __CONSTEXPR__
#endif

#ifdef _WIN32
#define __HOST__
#define __DEVICE__
#else
#define __HOST__ __host__
#define __DEVICE__ __device__
#endif

// Used throughout the API for compile-time conditions.
/*
 * phase this out in favour of std::true_type,std::false_type
namespace ecuda {
namespace detail {

struct __true_type { __CONSTEXPR__ operator bool() const __NOEXCEPT__ { return true; } };
struct __false_type { __CONSTEXPR__ operator bool() const __NOEXCEPT__ { return false; } };

} // namespace detail
} // namespace ecuda
*/

//
// Quick implementation of compile-time assertions. If C++11 is available, then
// just use the new static_assert keyword.
//
// This approach was borrowed from the Eigen linear algebra template library
// (http://eigen.tuxfamily.org).
//
#ifdef __CPP11_SUPPORTED__
#define ECUDA_STATIC_ASSERT(x,msg) static_assert(x,#msg)
#else

namespace ecuda {

namespace impl {

template<bool condition> struct static_assertion {};
template<> struct static_assertion<true>
{
	enum {
		CANNOT_USE_NONCONTIGUOUS_DEVICE_ITERATOR_AS_DESTINATION_FOR_COPY,
		CANNOT_USE_NONCONTIGUOUS_DEVICE_ITERATOR_AS_SOURCE_FOR_COPY,
		CANNOT_CALL_COPY_ON_HOST_MEMORY_INSIDE_DEVICE_CODE,
		CANNOT_USE_NONCONTIGUOUS_DEVICE_ITERATOR_WITH_MAX_ELEMENT,
		COPY_IS_UNEXPECTEDLY_BETWEEN_IDENTICAL_TYPES,
		CANNOT_CALL_LEXICOGRAPHICAL_COMPARE_ON_HOST_MEMORY_INSIDE_DEVICE_CODE,
		CANNOT_FILL_WITH_NONCONTIGUOUS_DEVICE_ITERATOR,
		CANNOT_ACCESS_HOST_MEMORY_FROM_DEVICE_CODE,
		CANNOT_CALCULATE_DISTANCE_OF_NONCONTIGUOUS_DEVICE_ITERATOR_FROM_HOST_CODE,
		CANNOT_ADVANCE_NONCONTIGUOUS_DEVICE_ITERATOR_FROM_HOST_CODE,
		COPY_ITERATOR_VALUE_TYPES_DO_NOT_MATCH_ARGUMENT_TYPES
	};
};

} // namespace impl
} // namespace ecuda

#define ECUDA_STATIC_ASSERT(x,msg) if(ecuda::impl::static_assertion<static_cast<bool>(x)>::msg) {}
#endif

#endif
