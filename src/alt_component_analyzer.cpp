/*
 * alt_component_analyzer.cpp
 *
 *  Created on: Mar 5, 2013
 *      Author: mthurley
 */


#include <sharpSAT/alt_component_analyzer.h>

using namespace std;

namespace sharpSAT {

void AltComponentAnalyzer::initialize(LiteralIndexedVector<Literal> & literals,
    vector<LiteralID> &lit_pool) {

  // untyped local copy of max_variable_id_
  unsigned max_variable_id = static_cast<unsigned>(literals.end_lit().var()) - 1;
  max_variable_id_ = VariableIndex(max_variable_id);

  search_stack_.reserve(max_variable_id + 1);
  var_frequency_scores_.resize(max_variable_id + 1, 0);
  variable_occurrence_lists_pool_.clear();
  variable_link_list_offsets_.clear();
  variable_link_list_offsets_.resize(max_variable_id + 1, 0);

  VariableIndexedVector<vector<ClauseIndex>> occs(max_variable_id + 1);
  VariableIndexedVector<vector<ClauseOrLiteral>> occ_long_clauses(max_variable_id + 1);
  VariableIndexedVector<vector<ClauseOrLiteral>> occ_ternary_clauses(max_variable_id + 1);

  vector<LiteralID> tmp;
  max_clause_id_ = ClauseIndex(0);
  unsigned curr_clause_length = 0;
  auto it_curr_cl_st = lit_pool.begin();

  for (auto it_lit = lit_pool.begin(); it_lit < lit_pool.end(); it_lit++) {
    if (*it_lit == SENTINEL_LIT) {

      if (it_lit + 1 == lit_pool.end())
        break;

      max_clause_id_++;
      it_lit += ClauseHeader::overheadInLits();
      it_curr_cl_st = it_lit + 1;
      curr_clause_length = 0;

    } else {
      assert(it_lit->var() <= max_variable_id_);
      curr_clause_length++;

      getClause(tmp, it_curr_cl_st, it_lit->var());

      assert(tmp.size() > 1);
      if(tmp.size() == 2) {

        auto& target = occ_ternary_clauses[it_lit->var()];
        target.reserve(target.size() + 1 + tmp.size());

        target.push_back(max_clause_id_);
        for (LiteralID lit : tmp) {
          target.push_back(lit);
        }
      } else {
        assert(tmp.size() >= 3);
        occs[it_lit->var()].push_back(max_clause_id_);
        occs[it_lit->var()].push_back(ClauseIndex(occ_long_clauses[it_lit->var()].size()));

        auto& target = occ_long_clauses[it_lit->var()];
        target.reserve(target.size() + 1 + tmp.size());

        for (LiteralID lit : tmp) {
          target.push_back(lit);
        }
        target.push_back(clsSENTINEL);
      }
    }
  }

  ComponentArchetype::initArrays(max_variable_id_, max_clause_id_);
  // the unified link list
  unified_variable_links_lists_pool_.clear();
  unified_variable_links_lists_pool_.push_back(0); // never accessed
  unified_variable_links_lists_pool_.push_back(0); // never accessed

  for (VariableIndex v(1); v < VariableIndex(occs.size()); v++) {
    // BEGIN data for binary clauses
    variable_link_list_offsets_[v] = unified_variable_links_lists_pool_.size();
    for (auto l : literals[LiteralID(v, false)].binary_links_)
      if (l != SENTINEL_LIT)
        unified_variable_links_lists_pool_.push_back(l.var());

    for (auto l : literals[LiteralID(v, true)].binary_links_)
      if (l != SENTINEL_LIT)
        unified_variable_links_lists_pool_.push_back(l.var());

    unified_variable_links_lists_pool_.push_back(varsSENTINEL);

    // BEGIN data for ternary clauses
    unified_variable_links_lists_pool_.insert(
        unified_variable_links_lists_pool_.end(),
        occ_ternary_clauses[v].begin(),
        occ_ternary_clauses[v].end()
    );
    // This can't be typed using ClauseOrVariableOrLiteral,
    // because the previous items are either Clause or a Literal
    // (not 1 concrete type).
    unified_variable_links_lists_pool_.push_back(0);

    // BEGIN data for long clauses
    for(auto it = occs[v].begin(); it != occs[v].end(); it+=2){
      unified_variable_links_lists_pool_.push_back(*it);
      unified_variable_links_lists_pool_.push_back(*(it + 1) + ClauseIndex(occs[v].end() - it));
    }
    unified_variable_links_lists_pool_.push_back(clsSENTINEL);

    unified_variable_links_lists_pool_.insert(
        unified_variable_links_lists_pool_.end(),
        occ_long_clauses[v].begin(),
        occ_long_clauses[v].end()
    );
  }
}


//void AltComponentAnalyzer::recordComponentOf(const VariableIndex var) {
//
//  search_stack_.clear();
//  setSeenAndStoreInSearchStack(var);
//
//  for (auto vt = search_stack_.begin(); vt != search_stack_.end(); vt++) {
//    //BEGIN traverse binary clauses
//    assert(isActive(*vt));
//    unsigned *p = beginOfLinkList(*vt);
//    for (; *p; p++) {
//      if(isUnseenAndActive(*p)){
//        setSeenAndStoreInSearchStack(*p);
//        var_frequency_scores_[*p]++;
//        var_frequency_scores_[*vt]++;
//      }
//    }
//    //END traverse binary clauses
//    auto s = p;
//    for ( p++; *p ; p+=3) {
////      if(archetype_.clause_unseen_in_sup_comp(*p)){
////        LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(p + 1);
////        searchThreeClause(*vt,*p, pstart_cls);
////      }
//    }
//    //END traverse ternary clauses
//
//    for (p++; *p ; p +=2) {
//      if(archetype_.clause_unseen_in_sup_comp(*p)){
//        LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(p + 1 + *(p+1));
//        searchClause(*vt,*p, pstart_cls);
//      }
//    }
//
//    for ( s++; *s ; s+=3) {
//          if(archetype_.clause_unseen_in_sup_comp(*s)){
//            LiteralID * pstart_cls = reinterpret_cast<LiteralID *>(s + 1);
//            searchThreeClause(*vt,*s, pstart_cls);
//          }
//        }
//  }
//}

void AltComponentAnalyzer::recordComponentOf(const VariableIndex var) {

  search_stack_.clear();
  setSeenAndStoreInSearchStack(var);

  for (auto vt = search_stack_.begin(); vt != search_stack_.end(); vt++) {
    //BEGIN traverse binary clauses
    assert(isActive(*vt));
    auto p = beginOfLinkList(*vt);
    for (; p->var() != varsSENTINEL; p++) {
      if(manageSearchOccurrenceOf(LiteralID(p->var(),true))){
        var_frequency_scores_[p->var()]++;
        var_frequency_scores_[*vt]++;
      }
    }
    //END traverse binary clauses

    for ( p++; static_cast<unsigned>(*p) ; p+=3) {
      if(archetype_.clause_unseen_in_sup_comp(p->cls())) {
        LiteralID litA = (p + 1)->lit();
        LiteralID litB = (p + 2)->lit();
        if(isSatisfied(litA)|| isSatisfied(litB))
          archetype_.setClause_nil(p->cls());
        else {
          var_frequency_scores_[*vt]++;
          manageSearchOccurrenceAndScoreOf(litA);
          manageSearchOccurrenceAndScoreOf(litB);
          archetype_.setClause_seen(p->cls(),
              isActive(litA) & isActive(litB));
        }
      }
    }
    //END traverse ternary clauses

    for (p++; p->cls() != clsSENTINEL; p +=2)
      if(archetype_.clause_unseen_in_sup_comp(p->cls()))
        searchClause(*vt, p->cls(), p + 1 + static_cast<unsigned>((p+1)->cls()));
  }
}

} // sharpSAT namespace
