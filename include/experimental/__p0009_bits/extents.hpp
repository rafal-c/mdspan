/*
//@HEADER
// ************************************************************************
//
//                        Kokkos v. 2.0
//              Copyright (2019) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Christian R. Trott (crtrott@sandia.gov)
//
// ************************************************************************
//@HEADER
*/

#pragma once

#include "macros.hpp"
#include "static_array.hpp"
#include "standard_layout_static_array.hpp"
#include "trait_backports.hpp" // integer_sequence, etc.

#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
#  include "no_unique_address.hpp"
#endif

#include <array>
#include <cstddef>

namespace std {
namespace experimental {

namespace detail {

#ifndef _MDSPAN_OVERWRITE_EXTENTS_SIZE_TYPE
using extents_size_type = size_t;
#else
using extents_size_type = _MDSPAN_OVERWRITE_EXTENTS_SIZE_TYPE;
#endif

template<size_t ... Extents>
struct _count_dynamic_extents;

template<size_t E, size_t ... Extents>
struct _count_dynamic_extents<E,Extents...> {
  static constexpr size_t val = (E==dynamic_extent?1:0) + _count_dynamic_extents<Extents...>::val;
};

template<>
struct _count_dynamic_extents<> {
  static constexpr size_t val = 0;
};

template <size_t... Extents, size_t... OtherExtents>
static constexpr std::false_type _check_compatible_extents(
  std::false_type, std::integer_sequence<size_t, Extents...>, std::integer_sequence<size_t, OtherExtents...>
) noexcept { return { }; }

template <size_t... Extents, size_t... OtherExtents>
static std::integral_constant<
  bool,
  _MDSPAN_FOLD_AND(
    (
      Extents == dynamic_extent
        || OtherExtents == dynamic_extent
        || Extents == OtherExtents
    ) /* && ... */
  )
>
_check_compatible_extents(
  std::true_type, std::integer_sequence<size_t, Extents...>, std::integer_sequence<size_t, OtherExtents...>
) noexcept { return { }; }

struct __extents_tag { };

} // end namespace detail

template <size_t... Extents>
class extents
#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
  : private detail::__no_unique_address_emulation<
      detail::__partially_static_sizes_tagged<detail::__extents_tag, detail::extents_size_type /*size_type*/, static_cast<detail::extents_size_type>(Extents)...>>
#endif
{
public:

  using size_type = detail::extents_size_type;

  using __storage_t = detail::__partially_static_sizes_tagged<detail::__extents_tag, size_type, static_cast<size_type>(Extents)...>;

#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
  _MDSPAN_NO_UNIQUE_ADDRESS __storage_t __storage_;
#else
  using __base_t = detail::__no_unique_address_emulation<__storage_t>;
#endif

 private:

  MDSPAN_FORCE_INLINE_FUNCTION _MDSPAN_CONSTEXPR_14
  __storage_t& __storage() noexcept {
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    return __storage_;
#else
    return this->__base_t::__ref();
#endif
  }
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr __storage_t const& __storage() const noexcept {
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    return __storage_;
#else
    return this->__base_t::__ref();
#endif
  }

  template <size_t... Idxs>
  MDSPAN_FORCE_INLINE_FUNCTION
  static constexpr
  size_type _static_extent_impl(size_t n, std::integer_sequence<size_t, Idxs...>) noexcept {
    return _MDSPAN_FOLD_PLUS_RIGHT(((Idxs == n) ? Extents : 0), /* + ... + */ 0);
  }

  template <size_t...>
  friend class extents;

  template <size_t... OtherExtents, size_t... Idxs>
  MDSPAN_INLINE_FUNCTION
  constexpr bool _eq_impl(std::experimental::extents<OtherExtents...>, false_type, index_sequence<Idxs...>) const noexcept { return false; }
  template <size_t... OtherExtents, size_t... Idxs>
  MDSPAN_INLINE_FUNCTION
  constexpr bool _eq_impl(
    std::experimental::extents<OtherExtents...> other,
    true_type, index_sequence<Idxs...>
  ) const noexcept {
    return _MDSPAN_FOLD_AND(
      (__storage().template __get_n<Idxs>() == other.__storage().template __get_n<Idxs>()) /* && ... */
    );
  }

  template <size_t... OtherExtents, size_t... Idxs>
  MDSPAN_INLINE_FUNCTION
  constexpr bool _not_eq_impl(std::experimental::extents<OtherExtents...>, false_type, index_sequence<Idxs...>) const noexcept { return true; }
  template <size_t... OtherExtents, size_t... Idxs>
  MDSPAN_INLINE_FUNCTION
  constexpr bool _not_eq_impl(
    std::experimental::extents<OtherExtents...> other,
    true_type, index_sequence<Idxs...>
  ) const noexcept {
    return _MDSPAN_FOLD_OR(
      (__storage().template __get_n<Idxs>() != other.__storage().template __get_n<Idxs>()) /* || ... */
    );
  }

#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
  MDSPAN_INLINE_FUNCTION constexpr explicit
  extents(__base_t&& __b) noexcept
    : __base_t(::std::move(__b))
  { }
#endif

public:

  MDSPAN_INLINE_FUNCTION
  static constexpr size_t rank() noexcept { return sizeof...(Extents); }
  MDSPAN_INLINE_FUNCTION
  static constexpr size_t rank_dynamic() noexcept { return _MDSPAN_FOLD_PLUS_RIGHT((int(Extents == dynamic_extent)), /* + ... + */ 0); }

  //--------------------------------------------------------------------------------
  // Constructors, Destructors, and Assignment

  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr extents() noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr extents(extents const&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED constexpr extents(extents&&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED extents& operator=(extents const&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED _MDSPAN_CONSTEXPR_14_DEFAULTED extents& operator=(extents&&) noexcept = default;
  MDSPAN_INLINE_FUNCTION_DEFAULTED ~extents() noexcept = default;

  MDSPAN_TEMPLATE_REQUIRES(
    size_t... OtherExtents,
    /* requires */ (
      /* multi-stage check to protect from invalid pack expansion when sizes don't match? */
      decltype(detail::_check_compatible_extents(
        std::integral_constant<bool, sizeof...(Extents) == sizeof...(OtherExtents)>{},
        std::integer_sequence<size_t, Extents...>{},
        std::integer_sequence<size_t, OtherExtents...>{}
      ))::value
    )
  )
  MDSPAN_INLINE_FUNCTION
  MDSPAN_CONDITIONAL_EXPLICIT((((Extents != dynamic_extent) && (OtherExtents == dynamic_extent)) || ...))
  constexpr extents(const extents<OtherExtents...>& __other)
    noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    : __storage_{
#else
    : __base_t(__base_t{__storage_t{
#endif
        __other.__storage().__enable_psa_conversion()
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      }
#else
      }})
#endif
  { }

#ifdef __NVCC__
    MDSPAN_TEMPLATE_REQUIRES(
    class... Integral,
    /* requires */ (
      // TODO: check whether the other version works with newest NVCC, doesn't with 11.4
      // NVCC seems to pick up rank_dynamic from the wrong extents type???
      _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(is_convertible, Integral, size_type) /* && ... */)
      // NVCC chokes on the fold thingy here so wrote the workaround
      && ((sizeof...(Integral) == detail::_count_dynamic_extents<Extents...>::val) ||
          (sizeof...(Integral) == sizeof...(Extents)))
      )
    )
#else
    MDSPAN_TEMPLATE_REQUIRES(
    class... Integral,
    /* requires */ (
       _MDSPAN_FOLD_AND(_MDSPAN_TRAIT(is_convertible, Integral, size_type) /* && ... */)
        && ((sizeof...(Integral) == rank_dynamic()) ||
            (sizeof...(Integral) == rank()))
      )
    )
#endif
  MDSPAN_INLINE_FUNCTION
  explicit constexpr extents(Integral... exts) noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    : __storage_{
#else
    : __base_t(__base_t{typename __base_t::__stored_type{
#endif
      std::conditional_t<sizeof...(Integral)==rank_dynamic(),
        detail::__construct_psa_from_dynamic_exts_values_tag_t,
        detail::__construct_psa_from_all_exts_values_tag_t>(),
        static_cast<size_t>(exts)...
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      }
#else
      }})
#endif
  { }

    // TODO: check whether this works with newest NVCC, doesn't with 11.4
#ifdef __NVCC__
  // NVCC seems to pick up rank_dynamic from the wrong extents type???
  // NVCC chokes on the fold thingy here so wrote the workaround
  MDSPAN_TEMPLATE_REQUIRES(
    class SizeType, size_t N,
    /* requires */ (
      _MDSPAN_TRAIT(is_convertible, SizeType, size_type)
      && ((N == detail::_count_dynamic_extents<Extents...>::val) ||
          (N == sizeof...(Extents)))
    )
  )
#else
    MDSPAN_TEMPLATE_REQUIRES(
        class SizeType, size_t N,
        /* requires */ (
            _MDSPAN_TRAIT(is_convertible, SizeType, size_type)
      && (N == rank() || N == rank_dynamic())
    )
  )
#endif
  MDSPAN_CONDITIONAL_EXPLICIT(N != rank_dynamic())
  MDSPAN_INLINE_FUNCTION
  constexpr
  extents(std::array<SizeType, N> const& exts) noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    : __storage_{
#else
    : __base_t(__base_t{typename __base_t::__stored_type{
#endif
      std::conditional_t<N==rank_dynamic(),
        detail::__construct_psa_from_dynamic_exts_array_tag_t<0>,
        detail::__construct_psa_from_all_exts_array_tag_t>(),
        std::array<SizeType,N>{exts}
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      }
#else
      }})
#endif
  { }

  // Need this constructor for some submdspan implementation stuff
  // for the layout_stride case where I use an extents object for strides
  MDSPAN_INLINE_FUNCTION
  constexpr explicit
  extents(__storage_t const& sto ) noexcept
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
    : __storage_{
#else
    : __base_t(__base_t{
#endif
        sto
#if defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      }
#else
      })
#endif
  { }

  //--------------------------------------------------------------------------------

  MDSPAN_INLINE_FUNCTION
  static constexpr
  size_type static_extent(size_t n) noexcept {
    return _static_extent_impl(n, std::make_integer_sequence<size_t, sizeof...(Extents)>{});
  }

  MDSPAN_INLINE_FUNCTION
  constexpr
  size_type extent(size_t n) const noexcept {
    return __storage().__get(n);
  }

  //--------------------------------------------------------------------------------

  template<size_t... RHS>
  MDSPAN_INLINE_FUNCTION
  friend constexpr bool operator==(extents const& lhs, extents<RHS...> const& rhs) noexcept {
    return lhs._eq_impl(
      rhs, std::integral_constant<bool, (sizeof...(RHS) == rank())>{},
      make_index_sequence<sizeof...(RHS)>{}
    );
  }

#ifndef MDSPAN_HAS_CXX_20
  template<size_t... RHS>
  MDSPAN_INLINE_FUNCTION
  friend constexpr bool operator!=(extents const& lhs, extents<RHS...> const& rhs) noexcept {
    return lhs._not_eq_impl(
      rhs, std::integral_constant<bool, (sizeof...(RHS) == rank())>{},
      make_index_sequence<sizeof...(RHS)>{}
    );
  }
#endif

public:  // (but not really)

  MDSPAN_INLINE_FUNCTION static constexpr
  extents __make_extents_impl(detail::__partially_static_sizes<size_type, static_cast<size_type>(Extents)...>&& __bs) noexcept {
    // This effectively amounts to a sideways cast that can be done in a constexpr
    // context, but we have to do it to handle the case where the extents and the
    // strides could accidentally end up with the same types in their hierarchies
    // somehow (which would cause layout_stride::mapping to not be standard_layout)
    return extents(
#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      __base_t{
#endif
        ::std::move(__bs.template __with_tag<detail::__extents_tag>())
#if !defined(_MDSPAN_USE_ATTRIBUTE_NO_UNIQUE_ADDRESS)
      }
#endif
    );
  }

  template <size_t N>
  MDSPAN_FORCE_INLINE_FUNCTION
  constexpr
  size_type __extent() const noexcept {
    return __storage().template __get_n<N>();
  }

  template <size_t N, size_t Default=dynamic_extent>
  MDSPAN_INLINE_FUNCTION
  static constexpr
  size_type __static_extent() noexcept {
    return __storage_t::template __get_static_n<N, Default>();
  }

};

namespace detail {

template <size_t Rank, class Extents = ::std::experimental::extents<>>
struct __make_dextents;

template <size_t Rank, size_t... ExtentsPack>
struct __make_dextents<Rank, ::std::experimental::extents<ExtentsPack...>> {
  using type = typename __make_dextents<Rank - 1,
    ::std::experimental::extents<::std::experimental::dynamic_extent, ExtentsPack...>>::type;
};

template <size_t... ExtentsPack>
struct __make_dextents<0, ::std::experimental::extents<ExtentsPack...>> {
  using type = ::std::experimental::extents<ExtentsPack...>;
};

} // end namespace detail

template <size_t Rank>
using dextents = typename detail::__make_dextents<Rank>::type;

#if defined(_MDSPAN_USE_CLASS_TEMPLATE_ARGUMENT_DEDUCTION)
template <class... SizeTypes>
extents(SizeTypes...)
  -> extents<detail::__make_dynamic_extent<SizeTypes>()...>;
#endif

namespace detail {

template <class T>
struct __is_extents : ::std::false_type {};

template <size_t... ExtentsPack>
struct __is_extents<::std::experimental::extents<ExtentsPack...>> : ::std::true_type {};

template <class T>
static constexpr bool __is_extents_v = __is_extents<T>::value;


template <typename Extents>
struct __extents_to_partially_static_sizes;

template <size_t... ExtentsPack>
struct __extents_to_partially_static_sizes<::std::experimental::extents<ExtentsPack...>> {
  using type = detail::__partially_static_sizes<
          typename ::std::experimental::extents<ExtentsPack...>::size_type,
          static_cast<typename ::std::experimental::extents<ExtentsPack...>::size_type>(ExtentsPack)...>;
};

template <typename Extents>
using __extents_to_partially_static_sizes_t = typename __extents_to_partially_static_sizes<Extents>::type;

} // end namespace detail
} // end namespace experimental
} // end namespace std
