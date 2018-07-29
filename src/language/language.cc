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

// Implementation of the core language evaluation system.

#include "language/language.h"

#include <algorithm>
#include <iterator>

#include <gmp.h>
#include <gmpxx.h>

#include "language/exceptions/stopError.h"
#include "language/exceptions/syntaxError.h"
#include "language/exceptions/typeError.h"
#include "language/stack.h"
#include "language/stack/stackElements/booleanElement.h"
#include "language/stack/stackElements/commandElement.h"
#include "language/stack/stackElements/numberElement.h"
#include "language/stack/stackElements/stringElement.h"
#include "language/stack/stackElements/substackElement.h"
#include "language/stack/stackElements/typeElement.h"

namespace stacklang {
using stacklang::exceptions::StopError;
using stacklang::exceptions::SyntaxError;
using stacklang::exceptions::TypeError;
using stacklang::stackelements::BooleanElement;
using stacklang::stackelements::CommandElement;
using stacklang::stackelements::NumberElement;
using stacklang::stackelements::StringElement;
using stacklang::stackelements::SubstackElement;
using stacklang::stackelements::TypeElement;
using std::all_of;
using std::begin;
using std::end;
using std::find_if;

bool stopFlag = false;

const map<string, PrimitiveFunction>& PRIMITIVES() noexcept {
  static map<string, PrimitiveFunction>* prims =
      new map<string, PrimitiveFunction>{
          // Special include files to group definition of primitives.
#include "language/primitives/boolean.inc"
#include "language/primitives/command.inc"
#include "language/primitives/number.inc"
#include "language/primitives/special.inc"
#include "language/primitives/stdlib.inc"
#include "language/primitives/string.inc"
#include "language/primitives/substack.inc"
#include "language/primitives/type.inc"
      };
  return *prims;
}

bool checkType(const StackElement* elm, const TypeElement type) noexcept {
  if (type.getSpecialization() == nullptr ||
      type.getSpecialization()->getData() ==
          StackElement::DataType::Any)  // has no specialization or is an Any
                                        // specialized substack.
  {
    return elm->getType() == type.getData() &&
           (elm->getType() != StackElement::DataType::Command ||
            !static_cast<const CommandElement*>(elm)
                 ->isQuoted());  // type matches plainly and any commands are
                                 // unquoted
  } else if (elm->getType() == type.getData() &&
             type.getData() ==
                 StackElement::DataType::Number) {  // is a specialized number
    return type.getSpecialization()->getData() ==
           (static_cast<const NumberElement*>(elm)->isExact()
                ? StackElement::DataType::Exact
                : StackElement::DataType::Inexact);
  } else if (elm->getType() == type.getData() &&
             type.getData() ==
                 StackElement::DataType::Command) {  // is a specialized command
    // - has to be quoted.
    return static_cast<const CommandElement*>(elm)->isQuoted();
  } else if (elm->getType() == type.getData() &&
             type.getData() ==
                 StackElement::DataType::Substack) {  // is a specialized
                                                      // substack
    const Stack& s = static_cast<const SubstackElement*>(elm)->getData();
    const TypeElement* spec = type.getSpecialization();

    return all_of(s.begin(), s.end(), [&spec](const StackElement* e) {
      return checkType(e, *spec);
    });
  } else {         // is a specialized non-substack, non-number
    return false;  // can't happen!
  }
}

void checkTypes(const Stack& s, const list<TypeElement>& types) {
  auto typeIter = types.begin();
  auto stackIter = s.begin();

  for (; typeIter != types.end() && stackIter != s.end();
       typeIter++, stackIter++) {
    if (!checkType(*stackIter, *typeIter)) {
      throw TypeError(*typeIter, *stackIter);
    }
  }
  if (typeIter != types.end()) {
    throw TypeError(*typeIter);
  }
}

void execute(Stack& s, map<string, DefinedFunction>& defines,
             list<CommandElement*> context) {
  if (stopFlag) {
    stopFlag = false;
    throw StopError();
  }

  const auto& PRIMS = PRIMITIVES();

  if (s.top()->getType() == StackElement::DataType::Command &&
      !dynamic_cast<CommandElement*>(s.top())->isQuoted()) {
    CommandElement* command = dynamic_cast<CommandElement*>(s.pop());
    const auto& primResult =
        find_if(PRIMS.begin(), PRIMS.end(),
                [&command](const pair<string, PrimitiveFunction>& entry) {
                  return entry.first == command->getData();
                });

    if (primResult != PRIMS.end()) {
      const auto& types = primResult->second.first;

      auto typeIter = types.begin();
      auto stackIter = s.begin();

      checkTypes(s, types);

      primResult->second.second(s, defines);
    } else {
      // find from non-primitives
      const auto& defResult =
          find_if(defines.begin(), defines.end(),
                  [&command](const pair<string, DefinedFunction>& entry) {
                    return entry.first == command->getData();
                  });

      if (defResult != defines.end()) {
        const auto& types = defResult->second.signature;
        const auto& commands = defResult->second.body;

        checkTypes(s, types);

        context.push_back(command);
        for (auto c : commands) {
          s.push(c->clone());
          execute(s, defines, context);
        }
      } else {
        throw SyntaxError("Given command is not recognized.",
                          command->getData(), 0);
      }
    }

    delete command;
  }
}
}  // namespace stacklang