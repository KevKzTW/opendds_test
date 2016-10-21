#ifndef __EXAMPLE_UTILITIES_H__
#define __EXAMPLE_UTILITIES_H__


/**
 * @file
 * This file defines some simple common utility functions for use in the Lite
 * C++ examples.
 */

#define NS_IN_ONE_US 1000
#define NS_IN_ONE_MS 1000000
#define US_IN_ONE_MS 1000
#define US_IN_ONE_SEC 1000000

/**
 * Sleep for the specified period of time.
 * @param milliseconds The period that should be slept for in milliseconds.
 */

#define TIME_STATS_SIZE_INCREMENT 50000

/**
 * Struct to keep a running average time as well as recording the minimum
 * and maximum times
 */
typedef struct ExampleTimeStats
{
  unsigned long* values;
  unsigned long valuesSize;
  unsigned long valuesMax;
  double average;
  unsigned long min;
  unsigned long max;
  unsigned long count;
} ExampleTimeStats;

/**
 * Returns an ExampleTimeStats struct with zero initialised variables
 * @return An ExampleTimeStats struct with zero initialised variables
 */
inline ExampleTimeStats exampleInitTimeStats (void)
{
  ExampleTimeStats stats;
  stats.values = (unsigned long*)malloc(TIME_STATS_SIZE_INCREMENT * sizeof(unsigned long));
  stats.valuesSize = 0;
  stats.valuesMax = TIME_STATS_SIZE_INCREMENT;
  stats.average = 0;
  stats.min = 0;
  stats.max = 0;
  stats.count = 0;

  return stats;
}

/**
 * Resets an ExampleTimeStats struct variables to zero
 * @param stats An ExampleTimeStats struct to reset
 */
inline void exampleResetTimeStats (ExampleTimeStats* stats)
{
  memset(stats->values, 0, stats->valuesMax * sizeof(unsigned long));
  stats->valuesSize = 0;
  stats->average = 0;
  stats->min = 0;
  stats->max = 0;
  stats->count = 0;
}

/**
 * Deletes the ExampleTimeStats values
 * @param stats An ExampleTimeStats struct delete the values from
 */
inline void exampleDeleteTimeStats (ExampleTimeStats* stats)
{
  free (stats->values);
}

/**
 * Updates an ExampleTimeStats struct with new time data, keeps a running average
 * as well as recording the minimum and maximum times
 * @param stats ExampleTimeStats struct to update
 * @param microseconds A time in microseconds to add to the stats
 * @return The updated ExampleTimeStats struct
 */
inline ExampleTimeStats* exampleAddMicrosecondsToTimeStats(ExampleTimeStats* stats, unsigned long microseconds)
{
  if(stats->valuesSize > stats->valuesMax)
  {
    unsigned long* temp = (unsigned long*)realloc(stats->values, (stats->valuesMax + TIME_STATS_SIZE_INCREMENT) * sizeof(unsigned long));
    if(temp)
    {
      stats->values = temp;
      stats->valuesMax += TIME_STATS_SIZE_INCREMENT;
    }
    else
    {
      printf("ERROR: Failed to expand values array");
    }
  }
  if(stats->valuesSize < stats->valuesMax)
  {
    stats->values[stats->valuesSize++] = microseconds;
  }
  stats->average = (stats->count * stats->average + microseconds)/(stats->count + 1);
  stats->min = (stats->count == 0 || microseconds < stats->min) ? microseconds : stats->min;
  stats->max = (stats->count == 0 || microseconds > stats->max) ? microseconds : stats->max;
  stats->count++;

  return stats;
}

/**
 * Compares two unsigned longs
 *
 * @param a an unsigned long
 * @param b an unsigned long
 * @param int -1 if a < b, 1 if a > b, 0 if equal
 */
inline int exampleCompareul (const void* a, const void* b)
{
  unsigned long ul_a = *((unsigned long*)a);
  unsigned long ul_b = *((unsigned long*)b);

  if(ul_a < ul_b) return -1;
  if(ul_a > ul_b) return 1;
  return 0;
}

/**
 * Calculates the median time from an ExampleTimeStats
 *
 * @param stats the ExampleTimeStats
 * @return the median time
 */
inline double exampleGetMedianFromTimeStats (ExampleTimeStats* stats)
{
  double median = 0;

  qsort(stats->values, stats->valuesSize, sizeof(unsigned long), exampleCompareul);

  if(stats->valuesSize % 2 == 0)
  {
    median = (double)(stats->values[stats->valuesSize / 2 - 1] + stats->values[stats->valuesSize / 2]) / 2;
  }
  else
  {
    median = (double)stats->values[stats->valuesSize / 2];
  }

  return median;
}

/**
 * Converts a timeval to microseconds
 * @param t The time to convert
 * @return The result of converting a timeval to microseconds
 */
inline unsigned long exampleTimevalToMicroseconds (ACE_Time_Value t)
{
  return ((unsigned long)t.sec() * US_IN_ONE_SEC) + t.usec();
}

/**
 * Converts microseconds to a timeval
 * @param microseconds the number of microseconds to convert
 * @return The result of converting a timeval to microseconds
 */
inline ACE_Time_Value exampleMicrosecondsToTimeval(const unsigned long microseconds)
{
  ACE_Time_Value tr;

  tr.sec((long)(microseconds / US_IN_ONE_SEC));
  tr.usec((long)(microseconds % US_IN_ONE_SEC));
  return tr;
}

inline void exampleResetTimeStats(ExampleTimeStats& stats)
{
  exampleResetTimeStats(&stats);
}

inline void exampleDeleteTimeStats(ExampleTimeStats& stats)
{
  exampleDeleteTimeStats(&stats);
}

inline ExampleTimeStats& operator+=(ExampleTimeStats& stats, unsigned long microseconds)
{
  return *exampleAddMicrosecondsToTimeStats(&stats, microseconds);
}

inline double exampleGetMedianFromTimeStats(ExampleTimeStats& stats)
{
  return exampleGetMedianFromTimeStats(&stats);
}


#endif
