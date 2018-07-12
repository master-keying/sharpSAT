/*
 * stopwatch.cpp
 *
 *  Created on: Jun 12, 2018
 *      Author: Radomir Cernoch
 */

#include "sharpSAT/stopwatch.h"

#include <chrono>

namespace {
    uint64_t get_us_since_epoch() {
        return std::chrono::duration_cast<std::chrono::microseconds>( std::chrono::high_resolution_clock::now().time_since_epoch() ).count();
    }
}


namespace sharpSAT {

StopWatch::StopWatch(){
    start();
}

uint64_t StopWatch::getElapsedTime() {
    // If we haven't stopped yet, just return the running total
    if (m_stop_time == m_start_time) {
        return get_us_since_epoch() - m_start_time;
    }
    return m_stop_time - m_start_time;
}

void StopWatch::start() {
    m_start_time = m_stop_time = m_last_interval_start = get_us_since_epoch();
}

void StopWatch::stop() {
    m_stop_time = get_us_since_epoch();
}

double StopWatch::getElapsedSeconds() {
    return getElapsedTime() / 1000000.;
}

bool StopWatch::timeBoundBroken() {
    return m_time_bound != 0 && getElapsedSeconds() > m_time_bound;
}

void StopWatch::setTimeBound(int64_t seconds) {
    m_time_bound = seconds;
}

bool StopWatch::interval_tick() {
    if (get_us_since_epoch() > m_last_interval_start + m_interval_length) {
        m_last_interval_start = get_us_since_epoch();
        return true;
    }
    return false;
}

} // sharpSAT
