/*
 * instance.cpp
 *
 *  Created on: Aug 23, 2012
 *      Author: Marc Thurley
 */

#include <sharpSAT/instance.h>

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sys/stat.h>

using namespace std;

namespace sharpSAT {

void Instance::cleanClause(ClauseOfs cl_ofs) {
  bool satisfied = false;
  for (auto it = beginOf(cl_ofs); *it != SENTINEL_LIT; it++)
    if (isSatisfied(*it)) {
      satisfied = true;
      break;
    }
  // mark the clause as empty if satisfied
  if (satisfied) {
    *beginOf(cl_ofs) = SENTINEL_LIT;
    return;
  }
  auto jt = beginOf(cl_ofs);
  auto it = beginOf(cl_ofs);
  // from now, all inactive literals are resolved
  for (; *it != SENTINEL_LIT; it++, jt++) {
    while (*jt != SENTINEL_LIT && !isActive(*jt))
      jt++;
    *it = *jt;
    if (*jt == SENTINEL_LIT)
      break;
  }
  unsigned length = it - beginOf(cl_ofs);
  // if it has become a unit clause, it should have already been asserted
  if (length == 1) {
    *beginOf(cl_ofs) = SENTINEL_LIT;
    // if it has become binary, transform it to binary and delete it
  } else if (length == 2) {
    addBinaryClause(*beginOf(cl_ofs), *(beginOf(cl_ofs) + 1));
    *beginOf(cl_ofs) = SENTINEL_LIT;
  }
}

void Instance::compactClauses() {
  vector<ClauseOfs> clause_ofs;
  clause_ofs.reserve(statistics_.num_long_clauses_);

  // clear watch links and occurrence lists
  for (auto it_lit = literal_pool_.begin(); it_lit != literal_pool_.end();
      it_lit++) {
    if (*it_lit == SENTINEL_LIT) {
      if (it_lit + 1 == literal_pool_.end())
        break;
      it_lit += ClauseHeader::overheadInLits();
      clause_ofs.push_back(ClauseOfs(1 + it_lit - literal_pool_.begin()));
    }
  }

  for (auto ofs : clause_ofs)
    cleanClause(ofs);

  for (auto &l : literals_)
    l.resetWatchList();

  occurrence_lists_.clear();
  occurrence_lists_.resize(variables_.size());

  vector<LiteralID> tmp_pool = literal_pool_;
  literal_pool_.clear();
  literal_pool_.push_back(SENTINEL_LIT);
  ClauseOfs new_ofs;
  unsigned num_clauses = 0;
  for (auto ofs : clause_ofs) {
    auto it = (tmp_pool.begin() + static_cast<unsigned>(ofs));
    if (*it != SENTINEL_LIT) {
      for (unsigned i = 0; i < ClauseHeader::overheadInLits(); i++)
        literal_pool_.push_back(LiteralID());
      new_ofs = ClauseOfs(literal_pool_.size());
      literal(*it).addWatchLinkTo(new_ofs);
      literal(*(it + 1)).addWatchLinkTo(new_ofs);
      num_clauses++;
      for (; *it != SENTINEL_LIT; it++) {
        literal_pool_.push_back(*it);
        occurrence_lists_[*it].push_back(new_ofs);
      }
      literal_pool_.push_back(SENTINEL_LIT);
    }
  }

  vector<LiteralID> tmp_bin;
  unsigned bin_links = 0;
  for (auto &l : literals_) {
    tmp_bin.clear();
    for (auto it = l.binary_links_.begin(); *it != SENTINEL_LIT; it++)
      if (isActive(*it))
        tmp_bin.push_back(*it);
    bin_links += tmp_bin.size();
    tmp_bin.push_back(SENTINEL_LIT);
    l.binary_links_ = tmp_bin;
  }
  statistics_.num_long_clauses_ = num_clauses;
  statistics_.num_binary_clauses_ = bin_links >> 1;
}

void Instance::compactVariables() {
  VariableIndexedVector<VariableIndex> var_map(variables_.size(), VariableIndex(0));
  VariableIndex last_ofs(0);
  unsigned num_isolated = 0;
  LiteralIndexedVector<vector<LiteralID> > _tmp_bin_links(1);
  LiteralIndexedVector<TriValue> _tmp_values = literal_values_;

  for (auto l : literals_)
    _tmp_bin_links.push_back(l.binary_links_);

  assert(_tmp_bin_links.size() == literals_.size());
  for (VariableIndex v(1); v < VariableIndex(variables_.size()); ++v)
    if (isActive(LiteralID(v, true))) {
      if (isolated(v)) {
        num_isolated++;
        continue;
      }
      ++last_ofs;
      var_map[v] = last_ofs;
    }

  variables_.clear();
  variables_.resize(static_cast<unsigned>(last_ofs) + 1);
  occurrence_lists_.clear();
  occurrence_lists_.resize(variables_.size());
  literals_.clear();
  literals_.resize(variables_.size());
  viewed_lits_.clear();
  viewed_lits_.resize(variables_.size(),0);
  literal_values_.clear();
  literal_values_.resize(variables_.size(), TriValue::X_TRI);

  unsigned bin_links = 0;
  LiteralID newlit;
  for (auto l = LiteralID(VariableIndex(0), false); l != _tmp_bin_links.end_lit(); l.inc()) {
    if (var_map[l.var()] != VariableIndex(0)) {
      newlit = LiteralID(var_map[l.var()], l.sign());
      for (auto it = _tmp_bin_links[l].begin(); *it != SENTINEL_LIT; it++) {
        assert(var_map[it->var()] != VariableIndex(0));
        literals_[newlit].addBinLinkTo(
            LiteralID(var_map[it->var()], it->sign()));
      }
      bin_links += literals_[newlit].binary_links_.size() - 1;
    }
  }

  vector<ClauseOfs> clause_ofs;
  clause_ofs.reserve(statistics_.num_long_clauses_);
  // clear watch links and occurrence lists
  for (auto it_lit = literal_pool_.begin(); it_lit != literal_pool_.end();
      it_lit++) {
    if (*it_lit == SENTINEL_LIT) {
      if (it_lit + 1 == literal_pool_.end())
        break;
      it_lit += ClauseHeader::overheadInLits();
      clause_ofs.push_back(ClauseOfs(1 + it_lit - literal_pool_.begin()));
    }
  }

  for (auto ofs : clause_ofs) {
    literal(LiteralID(var_map[beginOf(ofs)->var()], beginOf(ofs)->sign())).addWatchLinkTo(
        ofs);
    literal(LiteralID(var_map[(beginOf(ofs) + 1)->var()],
            (beginOf(ofs) + 1)->sign())).addWatchLinkTo(ofs);
    for (auto it_lit = beginOf(ofs); *it_lit != SENTINEL_LIT; it_lit++) {
      *it_lit = LiteralID(var_map[it_lit->var()], it_lit->sign());
      occurrence_lists_[*it_lit].push_back(ofs);
    }
  }

  literal_values_.clear();
  literal_values_.resize(variables_.size(), TriValue::X_TRI);
  unit_clauses_.clear();

  statistics_.num_variables_ = variables_.size() - 1 + num_isolated;

  statistics_.num_used_variables_ = num_variables();
  statistics_.num_free_variables_ = num_isolated;
}

void Instance::compactConflictLiteralPool(){
  auto write_pos = conflict_clauses_begin();
  vector<ClauseOfs> tmp_conflict_clauses = conflict_clauses_;
  conflict_clauses_.clear();
  for(auto clause_ofs: tmp_conflict_clauses){
    auto read_pos = beginOf(clause_ofs) - ClauseHeader::overheadInLits();
    for(unsigned i = 0; i < ClauseHeader::overheadInLits(); i++)
      *(write_pos++) = *(read_pos++);
    ClauseOfs new_ofs = ClauseOfs(write_pos - literal_pool_.begin());
    conflict_clauses_.push_back(new_ofs);
    // first substitute antecedent if clause_ofs implied something
    if(isAntecedentOf(clause_ofs, *beginOf(clause_ofs)))
      var(*beginOf(clause_ofs)).ante = Antecedent(new_ofs);

    // now redo the watches
    literal(*beginOf(clause_ofs)).replaceWatchLinkTo(clause_ofs,new_ofs);
    literal(*(beginOf(clause_ofs)+1)).replaceWatchLinkTo(clause_ofs,new_ofs);
    // next, copy clause data
    assert(read_pos == beginOf(clause_ofs));
    while(*read_pos != SENTINEL_LIT)
      *(write_pos++) = *(read_pos++);
    *(write_pos++) = SENTINEL_LIT;
  }
  literal_pool_.erase(write_pos,literal_pool_.end());
}


//bool Instance::deleteConflictClauses() {
//  statistics_.times_conflict_clauses_cleaned_++;
//  vector<ClauseOfs> tmp_conflict_clauses = conflict_clauses_;
//  conflict_clauses_.clear();
//  vector<double> tmp_ratios;
//  double score, lifetime;
//  for(auto clause_ofs: tmp_conflict_clauses){
//    score = getHeaderOf(clause_ofs).score();
//    lifetime = statistics_.num_conflicts_ - getHeaderOf(clause_ofs).creation_time();
//    tmp_ratios.push_back(score/lifetime/(getHeaderOf(clause_ofs).length()));
//  }
//  vector<double> tmp_ratiosB = tmp_ratios;
//
//  sort(tmp_ratiosB.begin(), tmp_ratiosB.end());
//
//  double cutoff = tmp_ratiosB[tmp_ratiosB.size()/2];
//
//  for(unsigned i = 0; i < tmp_conflict_clauses.size(); i++){
//    if(tmp_ratios[i] < cutoff){
//      if(!markClauseDeleted(tmp_conflict_clauses[i]))
//        conflict_clauses_.push_back(tmp_conflict_clauses[i]);
//    } else
//      conflict_clauses_.push_back(tmp_conflict_clauses[i]);
//  }
//  return true;
//}

bool Instance::deleteConflictClauses() {
  statistics_.times_conflict_clauses_cleaned_++;
  vector<ClauseOfs> tmp_conflict_clauses = conflict_clauses_;
  conflict_clauses_.clear();
  vector<double> tmp_ratios;
  double score;
  for(auto clause_ofs: tmp_conflict_clauses){
    score = getHeaderOf(clause_ofs).score();
    tmp_ratios.push_back(score);

  }
  vector<double> tmp_ratiosB = tmp_ratios;

  sort(tmp_ratiosB.begin(), tmp_ratiosB.end());

  double cutoff = tmp_ratiosB[tmp_ratiosB.size()/2];

  for(unsigned i = 0; i < tmp_conflict_clauses.size(); i++){
    if(tmp_ratios[i] < cutoff){
      if(!markClauseDeleted(tmp_conflict_clauses[i]))
        conflict_clauses_.push_back(tmp_conflict_clauses[i]);
    } else
      conflict_clauses_.push_back(tmp_conflict_clauses[i]);
  }
  return true;
}


bool Instance::markClauseDeleted(ClauseOfs cl_ofs){
  // only first literal may possibly have cl_ofs as antecedent
  if(isAntecedentOf(cl_ofs, *beginOf(cl_ofs)))
    return false;

  literal(*beginOf(cl_ofs)).removeWatchLinkTo(cl_ofs);
  literal(*(beginOf(cl_ofs)+1)).removeWatchLinkTo(cl_ofs);
  return true;
}


void Instance::initialize(
    unsigned int nVars,
    unsigned int nCls,
    unsigned int poolSize) {

  // 1) Clear everything
  literal_pool_.clear();
  literal_pool_.push_back(SENTINEL_LIT);

  variables_.clear();
  variables_.push_back(Variable()); //initializing the Sentinel
  literal_values_.clear();
  unit_clauses_.clear();

  // 2) Allocate correct sizes
  variables_.resize(nVars + 1);
  literal_values_.resize(nVars + 1, TriValue::X_TRI);
  literal_pool_.reserve(poolSize);
  conflict_clauses_.reserve(2*nCls);
  occurrence_lists_.clear();
  occurrence_lists_.resize(nVars + 1);

  literals_.clear();
  literals_.resize(nVars + 1);

  viewed_lits_.clear();
  viewed_lits_.resize(nVars + 1,0);
}


void Instance::add_clause(std::vector<LiteralID>& literals) {
  assert(!literals.empty());
  statistics_.incorporateClauseData(literals);
  ClauseOfs cl_ofs = addClause(literals);
  if (literals.size() >= 3) {
    assert(static_cast<unsigned>(cl_ofs) > 0);
    for (auto l : literals)
      occurrence_lists_[l].push_back(cl_ofs);
  }
}


void Instance::finalize(unsigned int nVars, unsigned int nCls) {
  statistics_.num_variables_ = statistics_.num_original_variables_ = nVars;
  statistics_.num_used_variables_ = num_variables();
  statistics_.num_free_variables_ = nVars - num_variables();

  statistics_.num_original_clauses_ = nCls;

  statistics_.num_original_binary_clauses_ = statistics_.num_binary_clauses_;
  statistics_.num_original_unit_clauses_ = statistics_.num_unit_clauses_ =
      unit_clauses_.size();

  original_lit_pool_size_ = literal_pool_.size();
}


bool Instance::createfromFile(const string &file_name) {
  unsigned int nVars; // #variables in the file
  unsigned int nCls; // #clauses in the file
  unsigned clauses_added = 0; // #clauses added to the instance

  // Start reading the file
  ifstream input_file(file_name);
  if (!input_file) {
    cerr << "Cannot open file: " << file_name << endl;
    exit(0);
  }

  char c;
  // fast-forward to the 'p [variables] [clauses]' line
  while (input_file >> c && c != 'p')
    input_file.ignore(numeric_limits<streamsize>::max(), '\n');

  string idstring;
  if (!(input_file >> idstring && idstring == "cnf" && input_file >> nVars
      && input_file >> nCls)) {
    cerr << "Invalid CNF file" << endl;
    exit(1);
  }

  // estimate literal count
  struct stat filestatus;
  stat(file_name.c_str(), &filestatus);
  // prepare to be filled
  initialize(nVars, nCls, filestatus.st_size);


  vector<LiteralID> literals;
  literals.reserve(10000);

  while ((input_file >> c) && clauses_added < nCls) {
    input_file.unget(); //extracted a nonspace character to determine if we have a clause, so put it back
    if ((c == '-') || isdigit(c)) {
      literals.clear();
      bool skip_clause = false;
      int lit; // current literal
      while ((input_file >> lit) && lit != 0) {
        bool duplicate_literal = false;
        for (auto i : literals) {
          if (i.toInt() == lit) {
            duplicate_literal = true;
            break;
          }
          if (i.toInt() == -lit) {
            skip_clause = true;
            break;
          }
        }
        if (!duplicate_literal) {
          literals.push_back(LiteralID(lit));
        }
      }
      if (!skip_clause) {
        clauses_added++;
        add_clause(literals);
      }
    }
    input_file.ignore(numeric_limits<streamsize>::max(), '\n');
  }

  // all done
  finalize(nVars, nCls);
  return true;
}

} // sharpSAT namespace
