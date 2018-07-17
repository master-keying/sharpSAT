/*
 * structures.h
 *
 *  Created on: Jun 25, 2012
 *      Author: Marc Thurley
 */

#ifndef SHARP_SAT_STRUCTURES_H_
#define SHARP_SAT_STRUCTURES_H_

#include <sharpSAT/primitive_types.h>

#include <cstdlib>
#include <vector>
#include <cassert>

namespace sharpSAT {

static const int INVALID_DL = -1;

enum class TriValue : unsigned char {
  F_TRI = 0,
  T_TRI = 1,
  X_TRI = 2
};

class LiteralID {
public:

  LiteralID() {
    value_ = 0;
  }
  LiteralID(int lit) {
    value_ = (std::abs(lit) << 1) + (unsigned) (lit > 0);
  }

  LiteralID(VariableIndex var, bool sign) {
    value_ = (var << 1) + (unsigned) sign;
  }

  VariableIndex var() const {
    return (value_ >> 1);
  }

  int toInt() const {
    return ((int) value_ >> 1) * ((sign()) ? 1 : -1);
  }

  void inc(){++value_;}

  void copyRaw(unsigned int v) {
    value_ = v;
  }

  bool sign() const {
    return (bool) (value_ & 0x01);
  }

  bool operator!=(const LiteralID &rL2) const {
    return value_ != rL2.value_;
  }

  bool operator==(const LiteralID &rL2) const {
    return value_ == rL2.value_;
  }

  const LiteralID neg() const {
    return LiteralID(var(), !sign());
  }

  void print() const;

  unsigned raw() const { return value_;}

private:

  /*!
   * MiniSAT-like encoded literal.
   * 
   * The LSB is the sign (false = 0, true = 1).
   * Remaining bits represent a variable.
   */ 
  unsigned value_;

  template <class _T> friend class LiteralIndexedVector;
};

/*!
 * Not-a-literal is a special literal.
 * 
 * It's represented as value 0, hence its
 * variable is the \ref varsSENTINEL.
 */
static const LiteralID NOT_A_LIT(0, false);
static const auto SENTINEL_LIT = NOT_A_LIT;

class Literal {
public:
  
  /*!
   * The "neighbour" literals in binary clauses.
   * 
   * Invariant: `back()` is \ref SENTINEL_LIT
   */
  std::vector<LiteralID> binary_links_ = std::vector<LiteralID>(1,SENTINEL_LIT);
  
  /*!
   * Subset of clauses, in which the literal appears.
   * 
   * Values represent offsets within \ref Instance::literal_pool_.
   * 
   * _Purpose:_ If a literal is set, the watched clauses will be updated.
   * If set to `true`, clauses are ignored in future search.
   * If set to `false`, the next literal watch for that clause.
   * 
   * _Invariant:_ `front()` is \ref SENTINEL_CL
   */ 
  std::vector<ClauseOfs> watch_list_ = std::vector<ClauseOfs>(1,SENTINEL_CL);
  
  //! Initialized to literal's occurances among all clauses
  float activity_score_ = 0.0f;

  void increaseActivity(unsigned u = 1){
    activity_score_+= u;
  }

  void removeWatchLinkTo(ClauseOfs clause_ofs) {
    for (auto it = watch_list_.begin(); it != watch_list_.end(); it++)
          if (*it == clause_ofs) {
            *it = watch_list_.back();
            watch_list_.pop_back();
            return;
          }
  }

  void replaceWatchLinkTo(ClauseOfs clause_ofs, ClauseOfs replace_ofs) {
    assert(clause_ofs != SENTINEL_CL);
    assert(replace_ofs != SENTINEL_CL);
        for (auto it = watch_list_.begin(); it != watch_list_.end(); it++)
          if (*it == clause_ofs) {
            *it = replace_ofs;
            return;
          }
  }

  void addWatchLinkTo(ClauseOfs clause_ofs) {
    watch_list_.push_back(clause_ofs);
  }

  void addBinLinkTo(LiteralID lit) {
    binary_links_.back() = lit;
    binary_links_.push_back(SENTINEL_LIT);
  }

  void resetWatchList(){
        watch_list_.clear();
        watch_list_.push_back(SENTINEL_CL);
  }

  bool hasBinaryLinkTo(LiteralID lit) {
    for (auto l : binary_links_) {
      if (l == lit)
        return true;
    }
    return false;
  }

  bool hasBinaryLinks() {
    return !binary_links_.empty();
  }
}; // Literal

class Antecedent {

  /*!
   * A flagged integer.
   * 
   * If the LSB is 1, the value represents a \ref ClauseOfs.
   * If the represented clause is \ref NOT_A_CLAUSE,
   * then the value in this field is 1.
   * If `sizeof(unsigned int) = 4` (on 32-bit and 64-bit Linux),
   * this can represent at most ~2*10^9 variables.
   * 
   * If the LSB is 0, the value represents a \ref LiteralID.
   * In that case the 2nd LSB is the sign.
   * Remaining bits represent variable ID. 
   * If `sizeof(unsigned int) = 4` (on 32-bit and 64-bit Linux),
   * this can represent at most ~10^9 variables.
   * Since \ref varsSENTINEL is 0, its `false` \ref LiteralID
   * is also 0 and hence this field is also 0.
   */
  unsigned int val_;

public:
  
  //! Antecendant represents \ref NOT_A_CLAUSE
  Antecedent() {
    val_ = 1;
  }

  //! Antecendant represents a clause
  Antecedent(const ClauseOfs cl_ofs) {
     val_ = (cl_ofs << 1) | 1;
   }

  //! Antecendant represents a literal
  Antecedent(const LiteralID idLit) {
    val_ = (idLit.raw() << 1);
  }

  bool isAClause() const {
    return val_ & 0x01;
  }

  ClauseOfs asCl() const {
      return val_ >> 1;
    }

  LiteralID asLit() {
    LiteralID idLit;
    idLit.copyRaw(val_ >> 1);
    return idLit;
  }
  // A NON-Antecedent will only be A NOT_A_CLAUSE Clause Id
  bool isAnt() {
    return val_ != 1; //i.e. NOT a NOT_A_CLAUSE;
  }
}; // Antecedent


struct Variable {
  Antecedent ante;
  int decision_level = INVALID_DL;
};

namespace overhead {

  //! Number of entries in `std::vector<U>` to fit a `T`.
  template<class T, class U>
  constexpr unsigned calculate() noexcept {
    return (sizeof(T) + sizeof(U) - 1) / sizeof(U);
  }

  /*!
   * Number of entries in `std::vector<U>` to fit a `T`.
   * 
   * This version checks soundness.
   */
  template<
    class T, class U,
    class = typename std::enable_if< (calculate<T,U>() != 0)
      && (calculate<T,U>() * sizeof(U) >= sizeof(T)) >::type>
  constexpr unsigned calculate_and_validate() noexcept {
    return calculate<T,U>();
  }
}; // overhead

/*!
 * Statistics about a clause.
 *
 * For now Clause Header is just a dummy
 * we keep it for possible later changes.
 * 
 * _Warning:_ Due to initialization in \ref Instance::addClause,
 * the constructor is never called. All memory occupied by
 * objects of this will be **zero-initialized**!
 */
class ClauseHeader {
  unsigned creation_time_; // number of conflicts seen at creation time
  unsigned score_;
  unsigned length_;
public:

  void increaseScore() {
    score_++;
  }
  void decayScore() {
      score_ >>= 1;
  }
  unsigned score() {
      return score_;
  }

  unsigned creation_time() {
      return creation_time_;
  }
  unsigned length(){ return length_;}
  void set_length(unsigned length){ length_ = length;}

  void set_creation_time(unsigned time) {
    creation_time_ = time;
  }

  constexpr static unsigned overheadInLits() {
    return overhead::calculate_and_validate<ClauseHeader,LiteralID>();
  }
}; // ClauseHeader
} // sharpSAT namespace
#endif /* STRUCTURES_H_ */
