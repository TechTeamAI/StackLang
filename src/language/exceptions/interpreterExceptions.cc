// Copyright 2018 Justin Hu, Bronwyn Damm, Jacques Marais, Ramon Rakow, and Jude
// Sidloski
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

// Implementation of interpreter-level errors and exceptions

#include "language/exceptions/interpreterExceptions.h"

namespace stacklang::exceptions {
namespace {
using std::string;
using std::to_string;
using std::vector;
}  // namespace

ArgumentError::ArgumentError(const string& msg) noexcept
    : LanguageException(msg) {}

ArgumentError::ArgumentError(const string& msg, const string& actual,
                             size_t posDif) noexcept
    : LanguageException(msg, actual, posDif) {}

string ArgumentError::getKind() const noexcept {
  return "Command line arguments invalid:";
}

ParserException::ParserException(const string& msg, const string& ctx,
                                 size_t loc, vector<string> trace) noexcept
    : LanguageException(msg, ctx, loc, trace) {}

string ParserException::getKind() const noexcept { return "Could not parse:"; }
}  // namespace stacklang::exceptions
