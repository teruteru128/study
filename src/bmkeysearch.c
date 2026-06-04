/*
 * Unified build entry point for the historical bmkeysearch variants.
 *
 * Each CMake target defines BMKEYSEARCH_VARIANT_SOURCE to select the original
 * implementation that used to live directly under src/ as bmkeysearchN.c.
 * Keeping a single top-level bmkeysearch*.c source reduces directory clutter
 * while preserving the existing bmkeysearch1..16 executables and behavior.
 */
#ifndef BMKEYSEARCH_VARIANT_SOURCE
#error "BMKEYSEARCH_VARIANT_SOURCE must name a bmkeysearch variant implementation"
#endif

#include BMKEYSEARCH_VARIANT_SOURCE
