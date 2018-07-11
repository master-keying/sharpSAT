/*
 * primitive_types.h
 *
 *  Created on: Feb 5, 2013
 *      Author: mthurley
 */

#ifndef SHARP_SAT_PRIMITIVE_TYPES_H_
#define SHARP_SAT_PRIMITIVE_TYPES_H_

namespace sharpSAT {

typedef unsigned VariableIndex;
typedef unsigned ClauseIndex;
typedef unsigned ClauseOfs;

typedef unsigned CacheEntryID;

static const ClauseIndex NOT_A_CLAUSE(0);
static const auto SENTINEL_CL = NOT_A_CLAUSE;
static const auto clsSENTINEL = NOT_A_CLAUSE;
static const VariableIndex varsSENTINEL = 0;

enum class SOLVER_StateT {

  NO_STATE, SUCCESS, TIMEOUT, ABORTED
};
} // sharpSAT namespace
#endif /* PRIMITIVE_TYPES_H_ */
