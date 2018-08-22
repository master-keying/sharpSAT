/*
 * unions.h
 *
 *  Created on: Jun 12, 2018
 *      Author: Radomir Cernoch
 */

#ifndef SHARP_SAT_UNIONS_H_
#define SHARP_SAT_UNIONS_H_

#include <sharpSAT/structures.h>

#include <type_traits>
#include <functional>
#include <limits>
#include <tuple>


namespace sharpSAT {

namespace { // tools for Variant

  //! Finds the index of `Needle` within `Head,Tail` (termination)
  template<size_t Acc, class Needle, class Head, class... Tail>
  static constexpr typename std::enable_if<
    std::is_same<Needle,Head>::value,
  unsigned char>::type variant_find_type() {
    static_assert(Acc <= std::numeric_limits<unsigned char>::max(),
      "variant takes at most numeric_limits<unsigned char>::max() types");
    return static_cast<unsigned char>(Acc);
  }

  //! Finds the index of `Needle` within `Head,Tail` (recursive call)
  template<size_t Acc, class Needle, class Head, class... Tail>
  static constexpr typename std::enable_if<
    ! std::is_same<Needle,Head>::value,
  unsigned char>::type variant_find_type() {
    return variant_find_type<Acc + 1, Needle, Tail...>();
  }



  //! Is one class-list a prefix-list of another class-list
  template <typename ...A>
  struct is_prefix : std::false_type { };

  //! Is one class-list a prefix-list of another class-list
  template <typename A1, typename ...Aother, typename B1, typename ...Bother>
  struct is_prefix<std::tuple<A1, Aother...>, std::tuple<B1, Bother...>> {
    static const bool value = std::is_same<A1, B1>::value
                           && is_prefix<std::tuple<Aother...>,
                                        std::tuple<Bother...>>::value;
  };

  //! Is one class-list a prefix-list of another class-list
  template <typename ...B>
  struct is_prefix<std::tuple<>, std::tuple<B...>> : std::true_type { };
}

template<class... Ts>
struct ReleaseVariant {

  //! Construct the variant by casting to `unsigned`
  template<class T> ReleaseVariant(T object)
  : data(static_cast<unsigned>(object)) {}

  //! Copy constructor of uneven types
  template<class... ProtoTs>
  ReleaseVariant(const ReleaseVariant<ProtoTs...>& prototype)
  : data(prototype.data) {
    static_assert(is_prefix<std::tuple<ProtoTs...>, std::tuple<Ts...>>::value,
      "If the prototype's variants are not a subclass of this variant's,"
      " we can't match the type_id (in Debug mode).");
  }

  //! Raw constructor avoids all means of type-checking
  ReleaseVariant(unsigned data, size_t) : data(data) {}

  //! Construct the typed value and check the type (in debug mode)
	template<class T> T get() const	{
		return T(data);
	}

  //! Raw conversion avoids all means of type-checking
  explicit operator unsigned() const {
    return data;
  }

protected:

  //! Value contained by this variant
	unsigned data;

  //! Let ReleaseVariant access \ref data if template args mismatch.
  template<class... FriendTs>
  friend struct ReleaseVariant;
}; // ReleaseVariant



template<class... Ts>
struct DebugVariant : public ReleaseVariant<Ts...> {

  //! Construct the variant and store the type (in debug mode)
  template<class T>
	DebugVariant(T object)
  : ReleaseVariant<Ts...>(object)
  , type_id(variant_find_type<0,T,Ts...>())
  {}

  //! Copy constructor works only if \ref type_id does match
  template<class... ProtoTs>
  DebugVariant(const DebugVariant<ProtoTs...>& prototype)
  : ReleaseVariant<Ts...>(static_cast<unsigned>(prototype))
  , type_id(prototype.type_id)
  {
    static_assert(is_prefix<std::tuple<ProtoTs...>, std::tuple<Ts...>>::value,
      "If the prototype's variants are not a subclass of this variant's,"
      " we can't match the type_id (in Debug mode).");
  }

  /*!
   * Raw constructor avoids all means of type-checking.
   *
   * Only use, if the `type_id` matches the type of the value.
   */
  DebugVariant(unsigned data, size_t type_id)
  : ReleaseVariant<Ts...>(data, type_id)
  , type_id(type_id)
  {}

  //! Construct the typed value and check the type (in debug mode)
	template<class T> T get() const	{
		assert((type_id == variant_find_type<0,T,Ts...>())
      && "Variant was written a different type than is being read.");
		return T(ReleaseVariant<Ts...>::data);
	}

protected:

  //! Index of the current type within `Ts`
	unsigned char type_id;

  //! Check if the expected type is the one
  void assert_type_matches(unsigned char expected_type) const
  {
    assert(type_id == expected_type
      && "Trying to extract a wrong type from the variant");
	}

  //! Let DebugVariant access \ref type_id if template args mismatch.
  template<class... FriendTs>
  friend struct DebugVariant;
}; // DebugVariant



#if !defined(NDEBUG)
template<class... Ts>
using Variant = DebugVariant<Ts...>;
#else
template<class... Ts>
using Variant = ReleaseVariant<Ts...>;
#endif

} // sharpSAT namespace
#endif // SHARP_SAT_UNIONS_H_