#include <sharpSAT/structures.h>

#include <iostream>

namespace sharpSAT {

static_assert((sizeof(ClauseHeader) / sizeof(LiteralID)) * sizeof(LiteralID) == sizeof(ClauseHeader),
              "Sizes of ClauseHeader and LiteralID must be evenly divisible");


void LiteralID::print() const {
  std::cout << (sign() ? " " : "-") << var() << " ";
}

} // end namespace sharpSAT
