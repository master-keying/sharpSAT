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
static const ClauseIndex clsSENTINEL(0);

typedef unsigned ClauseOfs; //!< Offset of a clause
static const ClauseOfs NOT_A_CLAUSE(0);
static const ClauseOfs SENTINEL_CL(0);

typedef unsigned CacheEntryID;

static const VariableIndex varsSENTINEL = 0;

enum class SOLVER_StateT {

  NO_STATE, SUCCESS, TIMEOUT, ABORTED
};
} // sharpSAT namespace
#endif /* PRIMITIVE_TYPES_H_ */
