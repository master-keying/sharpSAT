/*
 * stopwatch.h
 *
 *  Created on: Jun 12, 2018
 *      Author: Radomir Cernoch
 */

#ifndef SHARP_SAT_STOPWATCH_H_
#define SHARP_SAT_STOPWATCH_H_

#include <cstdint>

namespace sharpSAT {

class StopWatch {

public:

    StopWatch();

    void start();
    void stop();

    /**
     * Checks if the StopWatch has been running for too long
     *
     * The amount of time this checks against can be set using
     * `setTimeBound`.
     */
    bool timeBoundBroken();
    double getElapsedSeconds();
    bool interval_tick();
    /**
     * Sets the amount of time `timeBoundBroken` checks against
     *
     * Setting it to 0 seconds deactivates the check.
     */
    void setTimeBound(int64_t seconds);

private:
    //! Time the StopWatch started measuring in us
    uint64_t m_start_time;
    //! Time the StopWatch stopped measuring in us
    uint64_t m_stop_time;
    //! Interval length in us. Defaults to 60s
    uint64_t m_interval_length = 60 * 1000 * 1000;
    //! Time the last interval started in us
    uint64_t m_last_interval_start;

    //! How long after start should timeBoundBroken report true
    uint64_t m_time_bound = 0;

    // if we have started and then stopped the watch, this returns
    // the elapsed time, otherwise, time elapsed from m_start_time till
    // now is returned
    uint64_t getElapsedTime();

}; // StopWatch
} // sharpSAT
#endif // SHARP_SAT_STOPWATCH_H_
