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

PRIMDEF("define", {
  checkTypes(
      s, Stack{new TypeElement(StackElement::DataType::Substack,
                               new TypeElement(StackElement::DataType::Type)),
               new TypeElement(
                   StackElement::DataType::Substack,
                   new TypeElement(StackElement::DataType::Identifier)),
               new TypeElement(StackElement::DataType::Substack),
               new TypeElement(StackElement::DataType::Identifier)});
  IdentifierPtr name(dynamic_cast<IdentifierElement*>(s.pop()));
  SubstackPtr body(dynamic_cast<SubstackElement*>(s.pop()));
  SubstackPtr params(dynamic_cast<SubstackElement*>(s.pop()));
  SubstackPtr sig(dynamic_cast<SubstackElement*>(s.pop()));
  auto iter = e->bindings.find(name->getName());
  if (iter != e->bindings.cend())
    throw RuntimeError("Cannot redefine " + name->getName() + ".");

  Environment* closure = new Environment(e);

  DefinedCommandElement* def = new DefinedCommandElement(
      params->getData(), sig->getData(), body->getData(), closure);
  e->bindings.insert(pair<string, StackElement*>(name->getName(), def));
})
PRIMDEF("undefine", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Identifier)});
  IdentifierPtr name(dynamic_cast<IdentifierElement*>(s.pop()));
  Environment* currEnv = e;
  while (currEnv != nullptr) {
    auto iter = currEnv->bindings.find(name->getName());
    if (iter != currEnv->bindings.cend()) {
      delete iter->second;
      currEnv->bindings.erase(iter);
      return;
    }
    currEnv = currEnv->parent;
  }

  throw RuntimeError("Identifier " + name->getName() +
                     " was not previously defined.");
})
PRIMDEF("include", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::String)});
  StringPtr given(dynamic_cast<StringElement*>(s.pop()));
  string path = given->getData();
  ifstream fin;

  // change this to be more portable.
  if (!fin.is_open()) fin.open(path);
  if (!fin.is_open()) fin.open(path + ".sta");
  if (path.find('/') ==
      string::npos) {  // filename - check file within libs folder
    if (!fin.is_open()) fin.open("libs/" + path);
    if (!fin.is_open()) fin.open("libs/" + path + ".sta");
  }
  if (!fin.is_open())
    throw RuntimeError("Could not open include file " + path + ".");

  string buffer;
  size_t lineNumber = 1;
  int substackLevel = 0;
  bool inString = false;
  bool inComment = false;
  bool setPrevChar = true;
  char prevChar = '\0';
  while (fin.peek() != EOF) {
    char c = fin.get();
    if (c == ';' && !inString) {
      inComment = true;
      continue;
    } else if (c == '"' && !inComment && !inString) {
      inString = true;
    } else if (c == '"' && !inComment && inString && prevChar != '\\') {
      inString = false;
    } else if (c == '\\' && inString && prevChar == '\\') {
      prevChar = '\0';
      setPrevChar = false;
    } else if (c == '<' && prevChar == '<' && !inString && !inComment) {
      prevChar = '\0';
      setPrevChar = false;
      substackLevel++;
    } else if (c == '>' && prevChar == '>' && !inString && !inComment) {
      prevChar = '\0';
      setPrevChar = false;
      if (--substackLevel < 0)
        throw ParserException(
            path + ":" + to_string(lineNumber) +
                ": Found unmatched closing substack delimiter.",
            "", 0);
    } else if (c == '\n' && substackLevel == 0 && trim(buffer) != "") {
      try {
        s.push(StackElement::parse(trim(buffer)));
      } catch (const StackOverflowError&) {
        throw StackOverflowError(s.getLimit());
      } catch (const ParserException& exn) {
        throw ParserException(
            path + ":" + to_string(lineNumber) + ": " + exn.getMessage(),
            exn.getContext(), exn.getLocation());
      }
      buffer.erase();
      inString = false;
      inComment = false;
      prevChar = '\0';
      execute(s, e);
      lineNumber++;
      continue;  // skip end block
    } else if (c == '\n' && substackLevel == 0 && trim(buffer) == "") {
      buffer.erase();
      inString = false;
      inComment = false;
      prevChar = '\0';
      lineNumber++;
      continue;  // skip end block
    } else if (c == '\n' && substackLevel > 0) {
      if (buffer.back() != ',' && !inString && !ends_with(buffer, "<<"))
        buffer += ',';
      prevChar = '\0';
      inString = false;
      inComment = false;
      lineNumber++;
      continue;
    }

    if (!inComment) {
      buffer += c;
      if (setPrevChar) prevChar = c;
      setPrevChar = true;
    }
  }

  if (substackLevel == 0 && trim(buffer) != "") {
    try {
      s.push(StackElement::parse(trim(buffer)));
    } catch (const StackOverflowError&) {
      throw StackOverflowError(s.getLimit());
    } catch (const ParserException& exn) {
      throw ParserException(
          path + ":" + to_string(lineNumber) + ": " + exn.getMessage(),
          exn.getContext(), exn.getLocation());
    }
    execute(s, e);
  } else if (substackLevel > 0) {
    throw ParserException(path + ":" + to_string(lineNumber) + ": Missing " +
                              to_string(substackLevel) +
                              " closing substack delimiter" +
                              (substackLevel > 1 ? "s" : "") + ".",
                          "", 0);
  }

  fin.close();
})
PRIMDEF("drop", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Any)});
  delete s.pop();
})
PRIMDEF("drop*", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Number)});
  NumberPtr num(dynamic_cast<NumberElement*>(s.pop()));
  long double remainder = num->getData();
  long double whole;
  remainder = modf(remainder, &whole);
  if (remainder != 0 || whole <= 0) {
    throw RuntimeError("Expected a positive integer, but got " +
                       static_cast<string>(*num) + " instead.");
  }
  while (whole-- > 0) delete s.pop();
})
PRIMDEF("clear", { s.clear(); })
PRIMDEF("rotate", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Number)});
  NumberPtr num(dynamic_cast<NumberElement*>(s.pop()));
  long double remainder = num->getData();
  long double whole;
  remainder = modf(remainder, &whole);
  if (remainder != 0 || whole <= 0) {
    throw RuntimeError("Expected a positive integer, but got " +
                       static_cast<string>(*num) + " instead.");
  }
  StackElement* grabbed;
  stack<StackElement*> rest;
  while (whole-- > 1) rest.push(s.pop());
  grabbed = s.pop();
  while (!rest.empty()) {
    s.push(rest.top());
    rest.pop();
  }
  s.push(grabbed);
})
PRIMDEF("rotate*", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Number),
                      new TypeElement(StackElement::DataType::Number)});
  NumberPtr second(dynamic_cast<NumberElement*>(s.pop()));
  NumberPtr first(dynamic_cast<NumberElement*>(s.pop()));
  long double reachRemainder = first->getData();
  long double reachWhole;
  reachRemainder = modf(reachRemainder, &reachWhole);
  if (reachRemainder != 0 || reachWhole <= 0) {
    throw RuntimeError("Expected a positive integer, but got " +
                       static_cast<string>(*second) + " instead.");
  }
  long double countRemainder = second->getData();
  long double countWhole;
  countRemainder = modf(countRemainder, &countWhole);
  if (countRemainder != 0 || countWhole <= 0)
    throw RuntimeError("Expected a positive integer, but got " +
                       static_cast<string>(*first) + " instead.");

  while (countWhole-- > 0) {
    long double reach = reachWhole;
    StackElement* grabbed;
    stack<StackElement*> rest;
    while (reach-- > 1) rest.push(s.pop());
    grabbed = s.pop();
    while (!rest.empty()) {
      s.push(rest.top());
      rest.pop();
    }
    s.push(grabbed);
  }
})
PRIMDEF("duplicate", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Number)});
  s.push(s.top()->clone());
})
PRIMDEF("error", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::String)});
  StringPtr str(dynamic_cast<StringElement*>(s.pop()));
  throw RuntimeError(str->getData());
})
PRIMDEF("error*", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::String),
                      new TypeElement(StackElement::DataType::String),
                      new TypeElement(StackElement::DataType::Number)});
  NumberPtr ctxIndex(dynamic_cast<NumberElement*>(s.pop()));
  StringPtr ctx(dynamic_cast<StringElement*>(s.pop()));
  StringPtr msg(dynamic_cast<StringElement*>(s.pop()));
  long double remainder = ctxIndex->getData();
  long double whole;
  remainder = modf(remainder, &whole);
  if (remainder != 0 || whole <= 0) {
    throw RuntimeError("Expected a positive integer, but got " +
                       static_cast<string>(*ctxIndex) + "instead.");
  }
  throw RuntimeError(msg->getData(), ctx->getData(), whole);
})
PRIMDEF("null", { return; })
PRIMDEF("identity", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Any)});
  return;
})
PRIMDEF("export", {
  checkTypes(s, Stack{new TypeElement(StackElement::DataType::Identifier)});
  IdentifierPtr name(dynamic_cast<IdentifierElement*>(s.pop()));

  if (e->parent == nullptr)
    throw RuntimeError("Cannot export beyond global environment.");
  auto iter = e->bindings.find(name->getName());
  if (iter == e->bindings.cend())
    throw RuntimeError("Identifier " + name->getName() + " is not defined.");
  e->parent->bindings.insert(*iter);
  e->bindings.erase(iter);
})