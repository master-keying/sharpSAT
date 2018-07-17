#include <sharpSAT/structures.h>

#include <iostream>

namespace sharpSAT {

void LiteralID::print() const {
  std::cout << (sign() ? " " : "-") << var() << " ";
}

} // end namespace sharpSAT
