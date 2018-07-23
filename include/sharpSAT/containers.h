/*
 * containers.h
 *
 *  Created on: Jun 27, 2012
 *      Author: Marc Thurley
 */

#ifndef SHARP_SAT_CONTAINERS_H_
#define SHARP_SAT_CONTAINERS_H_

#include <sharpSAT/structures.h>

namespace sharpSAT {

template<class _T>
class LiteralIndexedVector: protected std::vector<_T> {

public:
	LiteralIndexedVector(unsigned size = 0) :
			std::vector<_T>(size * 2) {
	}
	LiteralIndexedVector(unsigned size,
			const typename std::vector<_T>::value_type& __value) :
			std::vector<_T>(size * 2, __value) {
	}
	inline _T &operator[](const LiteralID lit) {
		return *(std::vector<_T>::begin() + static_cast<unsigned>(lit));
	}

	inline const _T &operator[](const LiteralID &lit) const {
		return *(std::vector<_T>::begin() + static_cast<unsigned>(lit));
	}

	inline typename std::vector<_T>::iterator begin() {
		return std::vector<_T>::begin() + 2;
	}

	void resize(unsigned _size) {
		std::vector<_T>::resize(_size * 2);
	}
	void resize(unsigned _size, const typename std::vector<_T>::value_type& _value) {
		std::vector<_T>::resize(_size * 2, _value);
	}

	void reserve(unsigned _size) {
		std::vector<_T>::reserve(_size * 2);
	}

	LiteralID end_lit() {
		return LiteralID(VariableIndex(size() / 2), false);
	}

	using std::vector<_T>::end;
	using std::vector<_T>::size;
	using std::vector<_T>::clear;
	using std::vector<_T>::push_back;
}; // LiteralIndexedVector



//! Vector indexed by \ref VariableIndex
template<class T>
struct VariableIndexedVector : public std::vector<T> {

  VariableIndexedVector()
  : std::vector<T>()
  {}

  VariableIndexedVector(size_t n)
  : std::vector<T>(n)
  {}

  VariableIndexedVector(size_t n, const T& value)
  : std::vector<T>(n, value)
  {}

  typename std::vector<T>::const_reference operator [](const VariableIndex& var) const {
    return std::vector<T>::operator[](static_cast<unsigned>(var));
  }

  typename std::vector<T>::reference operator [](const VariableIndex& var) {
    return std::vector<T>::operator[](static_cast<unsigned>(var));
  }
}; // var_vactor<T>
} // sharpSAT namespace
#endif /* CONTAINERS_H_ */
