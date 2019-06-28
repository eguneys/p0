
#pragma once

#include <string>
#include <vector>

namespace pzero {

// Joins strings using @delim as delimiter.
std::string StrJoin(const std::vector<std::string>& strings,
                    const std::string& delim = " ");

// Splits strings at whitespace.
std::vector<std::string> StrSplitAtWhitespace(const std::string& str);

// Split string by delimiter.
std::vector<std::string> StrSplit(const std::string& str,
                                  const std::string& delim);

// Parses comma-separated list of integers.
std::vector<int> ParseIntList(const std::string& str);

// Trims a string of whitespace from the start.
std::string LeftTrim(std::string str);

// Trims a string of whitespace from the end.
std::string RightTrim(std::string str);

// Trims a string of whitespace from both ends.
std::string Trim(std::string str);

// Returns whether strings are equal, ignoring case.
bool StringsEqualIgnoreCase(const std::string& a, const std::string& b);

// Flow text into lines of width up to @width.
std::vector<std::string> FlowText(const std::string& src, size_t width);

}  // namespace pzero
