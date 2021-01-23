/** \file pxx/cxx/lookup.h
 *
 * Implements lookup of typenames.
 */

static std::regex ROOT_QUALIFIED_NAME = std::regex("(?<![a-zA-Z0-9_])((::)([a-zA-Z_][a-zA-Z0-9_]*))");
static std::regex QUALIFIED_NAME = std::regex("([a-zA-Z_][a-zA-Z0-9_]*)((::)([a-zA-Z_][a-zA-Z0-9_]*))");
static std::regex UNQUALIFIED_NAME = std::regex("(?<![:a-zA-Z0-9_])([a-zA-Z_][a-zA-Z0-9_]*)");
