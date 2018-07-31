// Copyright 2018 Justin Hu
//
// This file is part of the StackLang interpreter.
//
// The StackLang interpreter is free software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// The StackLang interpreter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// the StackLang interpreter.  If not, see <https://www.gnu.org/licenses/>.

// Implementation of various string utility functions

#include "util/stringUtils.h"

namespace util {
bool starts_with(const string& s, const string& s1) noexcept {
  return 0 == s.find(s1);
}

bool ends_with(const string& s, const string& s1) noexcept {
  return s.length() >= s1.length() &&
         s.length() - s1.length() == s.find(s1, s.length() - s1.length());
}

string escape(string s) noexcept {
  for (unsigned i = 0; i < s.length(); i++) {
    if (s[i] == '"')  // escape quotes
    {
      s.erase(i, 1);
      s.insert(i, "\\\"");
      i++;
    } else if (s[i] == '\n')  // newlines
    {
      s.erase(i, 1);
      s.insert(i, "\\n");
      i++;
    } else if (s[i] == '\\')  // escaped backslashes
    {
      s.erase(i, 1);
      s.insert(i, "\\\\");
      i++;
    }
  }

  return s;
}

string unescape(string s) noexcept {
  bool prevBackslash = false;
  bool prevPrevBackslash = false;

  for (unsigned i = 0; i < s.length(); i++) {
    if (s[i] == '"' && prevBackslash && !prevPrevBackslash)  // escaped quote
    {
      i--;
      s.erase(i, 2);
      s.insert(i, "\"");
      prevPrevBackslash = false;
      prevBackslash = false;
    } else if (s[i] == 'n' && prevBackslash && !prevPrevBackslash)  // escaped n
    {
      i--;
      s.erase(i, 2);
      s.insert(i, "\n");
      prevPrevBackslash = false;
      prevBackslash = false;
    } else if (s[i] == '\\' && prevBackslash &&
               !prevPrevBackslash)  // escaped backslash
    {
      i--;
      s.erase(i, 2);
      s.insert(i, "\\");
      prevPrevBackslash = false;
      prevBackslash = false;
    } else {
      prevPrevBackslash = prevBackslash;
      prevBackslash = s[i] == '\\';
    }
  }

  return s;
}

size_t findImproperEscape(const string& str) noexcept {
  for (unsigned i = 0; i < str.length(); i++) {
    if ((str[i] == '\\' &&
         i + 1 >= str.length()) ||  // if the last char's a backslash
        (str[i] == '\\' && str[i + 1] != 'n' && str[i + 1] != '"' &&
         str[i + 1] != '\\') ||  // make sure that we only escape \, n, and "
        (str[i] == '"'))         // it's an unescaped quote!
    {
      return i;
    } else if (str[i] == '\\')  // it's a valid escape - skip next char
    {
      i++;
    }
  }

  return string::npos;
}

string spaces(unsigned n) noexcept {
  string acc = "";
  acc.resize(n, ' ');
  return acc;
}

string removeChar(string s, char c) noexcept {
  for (unsigned i = 0; i < s.length(); i++) {
    if (s[i] == c) {
      s.erase(i, 1);
      i--;
    }
  }

  return s;
}

string trim(string s) noexcept {
  s.erase(0, s.find_first_not_of(WHITESPACE));
  s.erase(s.find_last_not_of(WHITESPACE) + 1);
  return s;
}
}  // namespace util