/*
 * primitive_types.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef SHARP_SAT_PRIMITIVE_TYPES_H_
#define SHARP_SAT_PRIMITIVE_TYPES_H_

#include <limits>
#include <cassert>

namespace sharpSAT {

struct VariableIndex {
  unsigned var_id_;

public:
  VariableIndex() : var_id_(0) {}
  explicit VariableIndex(unsigned var_id) : var_id_(var_id) {}

  explicit operator unsigned() const { return var_id_; }

  bool operator ==(const VariableIndex& rhs) const
  { return var_id_ == rhs.var_id_; }

  bool operator !=(const VariableIndex& rhs) const
  { return var_id_ != rhs.var_id_; }

  bool operator <(const VariableIndex& rhs) const
  { return var_id_ < rhs.var_id_; }

  bool operator >(const VariableIndex& rhs) const
  { return var_id_ > rhs.var_id_; }

  bool operator <=(const VariableIndex& rhs) const
  { return var_id_ <= rhs.var_id_; }

  bool operator >=(const VariableIndex& rhs) const
  { return var_id_ >= rhs.var_id_; }

  VariableIndex& operator ++() {
    ++var_id_;
    return *this;
  }

  VariableIndex operator ++(int) {
    auto before = *this;
    ++var_id_;
    return before;
  }

  VariableIndex& operator --() {
    --var_id_;
    return *this;
  }

  VariableIndex operator --(int) {
    auto before = *this;
    --var_id_;
    return before;
  }
};

static const VariableIndex varsSENTINEL(0);



struct ClauseIndex {
  unsigned clause_index_;

public:
  ClauseIndex() : clause_index_(0) {}
  explicit ClauseIndex(unsigned var_id) : clause_index_(var_id) {}

  explicit operator unsigned() const { return clause_index_; }

  bool operator ==(const ClauseIndex& rhs) const
  { return clause_index_ == rhs.clause_index_; }

  bool operator !=(const ClauseIndex& rhs) const
  { return clause_index_ != rhs.clause_index_; }

  bool operator <(const ClauseIndex& rhs) const
  { return clause_index_ < rhs.clause_index_; }

  bool operator >(const ClauseIndex& rhs) const
  { return clause_index_ > rhs.clause_index_; }

  bool operator <=(const ClauseIndex& rhs) const
  { return clause_index_ <= rhs.clause_index_; }

  bool operator >=(const ClauseIndex& rhs) const
  { return clause_index_ >= rhs.clause_index_; }

  ClauseIndex& operator ++() {
    ++clause_index_;
    return *this;
  }

  ClauseIndex operator ++(int) {
    auto before = *this;
    ++clause_index_;
    return before;
  }

  ClauseIndex& operator --() {
    --clause_index_;
    return *this;
  }

  ClauseIndex operator --(int) {
    auto before = *this;
    --clause_index_;
    return before;
  }
};

static const ClauseIndex clsSENTINEL(0);



//!< Offset of a clause
struct ClauseOfs {
  unsigned clause_ofs_;

public:
  ClauseOfs() : clause_ofs_(0) {}
  explicit ClauseOfs(unsigned var_id) : clause_ofs_(var_id) {}

  explicit operator unsigned() const { return clause_ofs_; }

  bool operator ==(const ClauseOfs& rhs) const
  { return clause_ofs_ == rhs.clause_ofs_; }

  bool operator !=(const ClauseOfs& rhs) const
  { return clause_ofs_ != rhs.clause_ofs_; }

  bool operator <(const ClauseOfs& rhs) const
  { return clause_ofs_ < rhs.clause_ofs_; }

  bool operator >(const ClauseOfs& rhs) const
  { return clause_ofs_ > rhs.clause_ofs_; }

  bool operator <=(const ClauseOfs& rhs) const
  { return clause_ofs_ <= rhs.clause_ofs_; }

  bool operator >=(const ClauseOfs& rhs) const
  { return clause_ofs_ >= rhs.clause_ofs_; }

  ClauseOfs& operator ++() {
    ++clause_ofs_;
    return *this;
  }

  ClauseOfs operator ++(int) {
    auto before = *this;
    ++clause_ofs_;
    return before;
  }

  ClauseOfs& operator --() {
    --clause_ofs_;
    return *this;
  }

  ClauseOfs operator --(int) {
    auto before = *this;
    --clause_ofs_;
    return before;
  }
};

static const ClauseOfs NOT_A_CLAUSE(0);
static const ClauseOfs SENTINEL_CL(0);



typedef unsigned CacheEntryID;

enum class SOLVER_StateT {

  NO_STATE, SUCCESS, TIMEOUT, ABORTED
};
} // sharpSAT namespace
#endif /* PRIMITIVE_TYPES_H_ */
