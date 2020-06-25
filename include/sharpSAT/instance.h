/*
 * instance.h
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#ifndef SHARP_SAT_INSTANCE_H_
#define SHARP_SAT_INSTANCE_H_

#include <sharpSAT/statistics.h>
#include <sharpSAT/structures.h>
#include <sharpSAT/containers.h>

#include <assert.h>

namespace sharpSAT {

class Instance {
public:

  /**
   * Make this instance empty.
   *
   * Make sure to call \ref finalize() after filling this instance.
   *
   * @param[in] nVars number of used variables
   * @param[in] nCls number of added clauses
   * @param[in] poolSize expected number of literals
   */
  void initialize(unsigned int nVars, unsigned int nCls,
                  unsigned int poolSize = 1000);

  /**
   * Adds a single clause into the instance.
   *
   * Precondition:
   * - clause is not empty
   * - no literal is listed twice
   * - no variable is listed both as a positive and negative literal
   */
  void add_clause(std::vector<LiteralID>& clause);

  /**
   * Must be called after having added all clauses.
   *
   * @param[in] nVars number of used variables
   * @param[in] nCls number of added clauses
   */
  void finalize(unsigned int nVars, unsigned int nCls);

protected:

  void unSet(LiteralID lit) {
    var(lit).ante = Antecedent(NOT_A_CLAUSE);
    var(lit).decision_level = INVALID_DL;
    literal_values_[lit] = TriValue::X_TRI;
    literal_values_[lit.neg()] = TriValue::X_TRI;
  }

  Antecedent & getAntecedent(LiteralID lit) {
    return variables_[lit.var()].ante;
  }

  bool hasAntecedent(LiteralID lit) {
    return variables_[lit.var()].ante.isAnt();
  }

  bool isAntecedentOf(ClauseOfs ante_cl, LiteralID lit) {
    return var(lit).ante.isAClause() && (var(lit).ante.asCl() == ante_cl);
  }

  bool isolated(VariableIndex v) {
    LiteralID lit(v, false);
    return (literal(lit).binary_links_.size() <= 1)
        & occurrence_lists_[lit].empty()
        & (literal(lit.neg()).binary_links_.size() <= 1)
        & occurrence_lists_[lit.neg()].empty();
  }

  bool free(VariableIndex v) {
    auto true_lit = LiteralID(v, true);
    return isolated(v) & isActive(true_lit);
  }

  bool deleteConflictClauses();
  bool markClauseDeleted(ClauseOfs cl_ofs);

  // Compact the literal pool erasing all the clause
  // information from deleted clauses
  void compactConflictLiteralPool();

  // we assert that the formula is consistent
  // and has not been found UNSAT yet
  // hard wires all assertions in the literal stack into the formula
  // removes all set variables and essentially reinitiallizes all
  // further data
  void compactClauses();
  void compactVariables();
  void cleanClause(ClauseOfs cl_ofs);

  /////////////////////////////////////////////////////////
  // END access to variables and literals
  /////////////////////////////////////////////////////////


  size_t num_conflict_clauses() const {
    return conflict_clauses_.size();
  }

  size_t num_variables() {
    return variables_.size() - 1;
  }

  bool createfromFile(const std::string &file_name);

  DataAndStatistics statistics_;

  /**
   * All clauses are stored here.
   *
   * _Format:_
   * - First literal is \ref SENTINEL_LIT.
   * - Every clause begins with a \ref ClauseHeader structure
   *   followed by the literals terminated by SENTINEL_LIT.
   */
  std::vector<LiteralID> literal_pool_;

  // this is to determine the starting offset of
  // conflict clauses
  unsigned original_lit_pool_size_;

  //! Literal-related data, indexed by LiteralID.
  LiteralIndexedVector<Literal> literals_;

  //! Used in implicitBCP...
  LiteralIndexedVector<unsigned char> viewed_lits_;

  /*!
   * Non-unit non-binary clauses, in which a literal appears.
   *
   * Outer index is the \ref LiteralID. Inner vectors are sorted initially
   * (order may change during \ref compactClauses() - to be verified).
   * Values are clause offset within \ref literal_pool_.
   *
   * _Note:_ Only the non-learnt clauses are stored here.
   */
  LiteralIndexedVector<std::vector<ClauseOfs>> occurrence_lists_;

  //! Offsets of clauses learnt on conflict analysis.
  std::vector<ClauseOfs> conflict_clauses_;

  //! Clauses with only 1 literal.
  std::vector<LiteralID> unit_clauses_;

  VariableIndexedVector<Variable> variables_;

  /*!
   * Assignment of truth values to literals.
   *
   * This data-structure is redundant.
   * If a literal is assigned a value,
   * the vector also contains value of
   * its negation.
   */
  LiteralIndexedVector<TriValue> literal_values_;

  void decayActivities() {
    for (auto l_it = literals_.begin(); l_it != literals_.end(); l_it++)
      l_it->activity_score_ *= 0.5;

    for(auto clause_ofs: conflict_clauses_)
        getHeaderOf(clause_ofs).decayScore();

  }
//  void decayActivities();

  void updateActivities(ClauseOfs clause_ofs) {
    getHeaderOf(clause_ofs).increaseScore();
    for (auto it = beginOf(clause_ofs); *it != SENTINEL_LIT; it++) {
      literal(*it).increaseActivity();
    }
  }

  bool isUnitClause(const LiteralID lit) {
    for (auto l : unit_clauses_)
      if (l == lit)
        return true;
    return false;
  }

  bool existsUnitClauseOf(VariableIndex v) {
    for (auto l : unit_clauses_)
      if (l.var() == v)
        return true;
    return false;
  }

  // addUnitClause checks whether lit or lit.neg() is already a
  // unit clause
  // a negative return value implied that the Instance is UNSAT
  bool addUnitClause(const LiteralID lit) {
    for (auto l : unit_clauses_) {
      if (l == lit)
        return true;
      if (l == lit.neg())
        return false;
    }
    unit_clauses_.push_back(lit);
    return true;
  }

  inline ClauseOfs addClause(std::vector<LiteralID> &literals);

  // adds a UIP Conflict Clause
  // and returns it as an Antecedent to the first
  // literal stored in literals
  inline Antecedent addUIPConflictClause(std::vector<LiteralID> &literals);

  inline bool addBinaryClause(LiteralID litA, LiteralID litB);

  /////////////////////////////////////////////////////////
  // BEGIN access to variables, literals, clauses
  /////////////////////////////////////////////////////////

  inline Variable &var(const LiteralID lit) {
    return variables_[lit.var()];
  }

  Literal & literal(LiteralID lit) {
    return literals_[lit];
  }

  //! Determine if the literal is assigned the `true` value.
  inline bool isSatisfied(const LiteralID &lit) const {
    return literal_values_[lit] == TriValue::T_TRI;
  }

  //! Determine if the literal is assigned the `false` value.
  bool isResolved(LiteralID lit) const {
    return literal_values_[lit] == TriValue::F_TRI;
  }

  //! Determine if the literal has not been assigned yet.
  bool isActive(LiteralID lit) const {
    return literal_values_[lit] == TriValue::X_TRI;
  }

  std::vector<LiteralID>::const_iterator beginOf(ClauseOfs cl_ofs) const {
    return literal_pool_.begin() + static_cast<unsigned>(cl_ofs);
  }
  std::vector<LiteralID>::iterator beginOf(ClauseOfs cl_ofs) {
    return literal_pool_.begin() + static_cast<unsigned>(cl_ofs);
  }

  decltype(literal_pool_.begin()) conflict_clauses_begin() {
     return literal_pool_.begin() + original_lit_pool_size_;
   }

  ClauseHeader &getHeaderOf(ClauseOfs cl_ofs) {
    return *reinterpret_cast<ClauseHeader *>(&literal_pool_[
      static_cast<unsigned>(cl_ofs) - ClauseHeader::overheadInLits() ]);
  }

  bool isSatisfied(ClauseOfs cl_ofs) {
    for (auto lt = beginOf(cl_ofs); *lt != SENTINEL_LIT; lt++)
      if (isSatisfied(*lt))
        return true;
    return false;
  }
}; // Instance

/*!
 * Internal method to add a new clause.
 *
 * @returns ID of its first literal within \ref literal_pool_
 *          or \ref NOT_A_CLAUSE if the clause was neither unit, nor binary
 */
ClauseOfs Instance::addClause(std::vector<LiteralID> &literals) {
  if (literals.size() == 1) {
    // TODO Deal properly with the situation that opposing unit clauses are learned.
    //      This should probably call addUnitClause(literals[0]) and inspect retval.
    assert(!isUnitClause(literals[0].neg()));
    unit_clauses_.push_back(literals[0]);
    return NOT_A_CLAUSE;
  }
  if (literals.size() == 2) {
    addBinaryClause(literals[0], literals[1]);
    return NOT_A_CLAUSE;
  }

  // make room for ClauseHeader in literal_pool_
  for (unsigned i = 0; i < ClauseHeader::overheadInLits(); i++)
    literal_pool_.push_back(LiteralID());

  // where the literals will start
  ClauseOfs cl_ofs = ClauseOfs(literal_pool_.size());
  for (auto l : literals) {
    literal_pool_.push_back(l);
    literal(l).increaseActivity(1);
  }

  // make an end: SENTINEL_LIT
  literal_pool_.push_back(SENTINEL_LIT);

  // register for clause updates
  literal(literals[0]).addWatchLinkTo(cl_ofs);
  literal(literals[1]).addWatchLinkTo(cl_ofs);

  // Initialize the ClauseHeader
  new(&getHeaderOf(cl_ofs)) ClauseHeader();
  getHeaderOf(cl_ofs).set_creation_time(statistics_.num_conflicts_);
  // TODO destroy ClauseHeader when destroying this
  return cl_ofs;
}

/*!
 * Adds a clause learnt due to a conflict.
 *
 * _Note:_ For whatever reason, \ref occurrence_lists_ is not updated!
 * See \ref Instance::add_clause for an explanation.
 *
 * @returns a non-Antecedent if the clause is a unit clause,
 * a Literal-Antecedent if the clause is binary or
 * a Clause-Antecedent if the clause has 3 or more literals
 */
Antecedent Instance::addUIPConflictClause(std::vector<LiteralID> &literals) {
    Antecedent ante(NOT_A_CLAUSE);
    statistics_.num_clauses_learned_++;
    ClauseOfs cl_ofs = addClause(literals);
    if (cl_ofs != ClauseOfs(0)) {
      conflict_clauses_.push_back(cl_ofs);
      getHeaderOf(cl_ofs).set_length(literals.size());
      ante = Antecedent(cl_ofs);
    } else if (literals.size() == 2){
      ante = Antecedent(literals.back());
      statistics_.num_binary_conflict_clauses_++;
    } else if (literals.size() == 1)
      statistics_.num_unit_clauses_++;
    return ante;
  }

bool Instance::addBinaryClause(LiteralID litA, LiteralID litB) {
   if (literal(litA).hasBinaryLinkTo(litB))
     return false;
   literal(litA).addBinLinkTo(litB);
   literal(litB).addBinLinkTo(litA);
   literal(litA).increaseActivity();
   literal(litB).increaseActivity();
   return true;
 }
} // sharpSAT namespace
#endif /* INSTANCE_H_ */
