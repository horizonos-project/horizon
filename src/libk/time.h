/**
 * @file time.h
 * @brief Time and Delay Functions
 * 
 * Provides timing utilities for the kernel. Currently implements
 * busy-wait delays for situations where precise timing is needed
 * (e.g., hardware initialization sequences).
 * 
 * @note These are blocking operations - the CPU spins during the delay
 * @warning Not recommended for use
 */

#ifndef TIME_H
#define TIME_H

#include <stdint.h>

/**
 * @brief Busy-wait delay
 * 
 * Blocks execution for approximately the specified number of milliseconds
 * by spinning the CPU in a tight loop. The actual delay depends on CPU
 * frequency and may not be precise.
 * 
 * @param ms Number of milliseconds to delay
 * 
 * @note This is a BLOCKING operation - use sparingly!
 * @warning Accuracy depends on CPU speed and is not guaranteed
 * 
 * @todo Avoid excessive use, or use in general
 */
void sleep(volatile uint32_t ms);

#endif
