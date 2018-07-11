/*
 * basic_types.h
 *
 *  Created on: Jun 24, 2012
 *      Author: Marc Thurley
 */

#ifndef SHARP_SAT_SOLVER_CONFIG_H_
#define SHARP_SAT_SOLVER_CONFIG_H_

namespace sharpSAT {

struct SolverConfiguration {

  bool perform_non_chron_back_track = true;

  // TODO component caching cannot be deactivated for now!
  bool perform_component_caching = true;
  bool perform_failed_lit_test = true;
  bool perform_pre_processing = true;

  unsigned long time_bound_seconds = 100000;

  bool verbose = false;

  // quiet = true will override verbose;
  bool quiet = false;
}; // SolverConfiguration
} // sharpSAT namespace
#endif /* SOLVER_CONFIG_H_ */
