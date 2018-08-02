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

// tests for the StackElement parse method

#include "language/exceptions/interpreterExceptions.h"
#include "language/stack/stackElements.h"

#include "catch.hpp"

namespace {
using stacklang::StackElement;
using stacklang::exceptions::ParserException;
using stacklang::stackelements::BooleanElement;
using stacklang::stackelements::CommandElement;
using stacklang::stackelements::NumberElement;
using stacklang::stackelements::StringElement;
using stacklang::stackelements::SubstackElement;
using stacklang::stackelements::TypeElement;
}  // namespace

TEST_CASE("empty parse throws", "[parse][StackElement]") {
  REQUIRE_THROWS_AS(StackElement::parse(""), ParserException);
}

TEST_CASE("initial number parses as number", "[parse][StackElement][number]") {
  StackElement* elm = StackElement::parse("22/7");
  NumberElement* num = dynamic_cast<NumberElement*>(elm);
  REQUIRE(num != nullptr);
  REQUIRE(num->getData() == mpq_class("22/7"));
  delete num;
}

TEST_CASE("initial sign parses as number", "[parse][StackElement][number]") {
  StackElement* elm = StackElement::parse("-22/7");
  NumberElement* num = dynamic_cast<NumberElement*>(elm);
  REQUIRE(num != nullptr);
  REQUIRE(num->getData() == mpq_class("-22/7"));
  delete num;
}

TEST_CASE("initial inexact sign parses as number",
          "[parse][StackElement][number]") {
  StackElement* elm = StackElement::parse("~22/7");
  NumberElement* num = dynamic_cast<NumberElement*>(elm);
  REQUIRE(num != nullptr);
  REQUIRE(num->getData() == mpq_class("22/7"));
  delete num;
}

TEST_CASE("initial quote parses as string", "[parse][StackElement][string]") {
  StackElement* elm = StackElement::parse("\"string\"");
  StringElement* str = dynamic_cast<StringElement*>(elm);
  REQUIRE(str != nullptr);
  REQUIRE(str->getData() == "string");
  delete str;
}

TEST_CASE("substack delimiters recognized", "[parse][StackElement][substack]") {
  StackElement* elm = StackElement::parse("<< 5 >>");
  SubstackElement* s = dynamic_cast<SubstackElement*>(elm);
  REQUIRE(s != nullptr);
  REQUIRE(s->getType() == StackElement::DataType::Substack);
  delete s;
}

TEST_CASE("boolean true parses correctly", "[parse][StackElement][boolean]") {
  StackElement* elm = StackElement::parse("true");
  BooleanElement* b = dynamic_cast<BooleanElement*>(elm);
  REQUIRE(b != nullptr);
  REQUIRE(b->getData());
  delete b;
}

TEST_CASE("boolean false parses correctly", "[parse][StackElement][boolean]") {
  StackElement* elm = StackElement::parse("false");
  BooleanElement* b = dynamic_cast<BooleanElement*>(elm);
  REQUIRE(b != nullptr);
  REQUIRE_FALSE(b->getData());
  delete b;
}

TEST_CASE("parens parse to type", "[parse][StackElement][type]") {
  StackElement* elm = StackElement::parse("Substack(Number)");
  TypeElement* t = dynamic_cast<TypeElement*>(elm);
  REQUIRE(t != nullptr);
  REQUIRE(t->getData() == StackElement::DataType::Substack);
  delete t;
}

TEST_CASE("primitve type parses to type", "[parse][StackElement][type]") {
  StackElement* elm = StackElement::parse("Number");
  TypeElement* t = dynamic_cast<TypeElement*>(elm);
  REQUIRE(t != nullptr);
  REQUIRE(t->getData() == StackElement::DataType::Number);
  delete t;
}

TEST_CASE("word parses to command", "[parse][StackElement][command]") {
  StackElement* elm = StackElement::parse("cmd1*");
  CommandElement* cmd = dynamic_cast<CommandElement*>(elm);
  REQUIRE(cmd != nullptr);
  REQUIRE(cmd->getName() == "cmd1*");
  delete cmd;
}

TEST_CASE("command quote recognized", "[parse][StackElement][command]") {
  StackElement* elm = StackElement::parse("`Number");
  CommandElement* cmd = dynamic_cast<CommandElement*>(elm);
  REQUIRE(cmd != nullptr);
  REQUIRE(cmd->getName() == "Number");
  delete cmd;
}

TEST_CASE("unquoted command properly parsed", "[parse][CommandElement]") {
  CommandElement* cmd = CommandElement::parse("foldr");
  REQUIRE(cmd->getName() == "foldr");
  REQUIRE_FALSE(cmd->isQuoted());
  delete cmd;
}

TEST_CASE("quoted command properly parsed", "[parse][CommandElement]") {
  CommandElement* cmd = CommandElement::parse("`foldr");
  REQUIRE(cmd->getName() == "foldr");
  REQUIRE(cmd->isQuoted());
  delete cmd;
}

TEST_CASE("quote in middle of command", "[parse][CommandElement]") {
  REQUIRE_THROWS_AS(CommandElement::parse("`fold`r"), ParserException);
}

TEST_CASE("space in middle of command", "[parse][CommandElement]") {
  REQUIRE_THROWS_AS(CommandElement::parse("stuff todo"), ParserException);
}

TEST_CASE("symbol in middle of command", "[parse][CommandElement]") {
  REQUIRE_THROWS_AS(CommandElement::parse("things&others"), ParserException);
}

TEST_CASE("number with disallowed chars", "[parse][NumberElement]") {
  REQUIRE_THROWS_AS(NumberElement::parse("0x10c"), ParserException);
}

TEST_CASE("number with more than one symbol", "[parse][NumberElement]") {
  REQUIRE_THROWS_AS(NumberElement::parse("1..2"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("1/2/3"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("1.2/5"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("~~2"), ParserException);
}

TEST_CASE("number with symbol in the middle", "[parse][NumberElement]") {
  REQUIRE_THROWS_AS(NumberElement::parse("1~2"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("1+2"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("1-2"), ParserException);
  REQUIRE_THROWS_AS(NumberElement::parse("+~12"), ParserException);
}

TEST_CASE("number with quote chars between signs", "[parse][NumberElement]") {
  NumberElement* num = NumberElement::parse("~'+'1/2'");
  REQUIRE(num->getData() == mpq_class("1/2"));
  delete num;
}

TEST_CASE("number with blank denominator", "[parse][NumberElement]") {
  REQUIRE_THROWS_AS(NumberElement::parse("12/''''"), ParserException);
}

TEST_CASE("number with zero denominator", "[parse][NumberElement]") {
  REQUIRE_THROWS_AS(NumberElement::parse("12/'0'''"), ParserException);
}

TEST_CASE("exact fraction with sign", "[parse][NumberElement]") {
  NumberElement* num = NumberElement::parse("+'2'/'4'");
  REQUIRE(num->getData() == mpq_class("1/2"));
  REQUIRE(num->isExact());
  delete num;
}

TEST_CASE("inexact fraction with sign", "[parse][NumberElement]") {
  NumberElement* num = NumberElement::parse("~'-'1'/'2'");
  REQUIRE(num->getData() == mpq_class("-1/2"));
  REQUIRE_FALSE(num->isExact());
  delete num;
}

TEST_CASE("exact decimal with sign", "[parse][NumberElement]") {
  NumberElement* num = NumberElement::parse("-2.5");
  REQUIRE(num->getData() == mpq_class("-5/2"));
  REQUIRE(num->isExact());
  delete num;
}

TEST_CASE("inexact decimal with sign", "[parse][NumberElement]") {
  NumberElement* num = NumberElement::parse("~+2.5");
  REQUIRE(num->getData() == mpq_class("5/2"));
  REQUIRE_FALSE(num->isExact());
  delete num;
}

TEST_CASE("string with missing closing quote", "[parse][StringElement]") {
  REQUIRE_THROWS_AS(StringElement::parse("\"bad string"), ParserException);
}

TEST_CASE("string with bad escape", "[parse][StringElement]") {
  REQUIRE_THROWS_AS(StringElement::parse("\"bad\\string\""), ParserException);
}

TEST_CASE("valid string", "[parse][StringElement]") {
  StringElement* str = StringElement::parse("\"good\\nstring\"");
  StringElement* blank = StringElement::parse("\"\"");
  REQUIRE(str->getData() == "good\nstring");
  REQUIRE(blank->getData() == "");
  delete str;
}

TEST_CASE("substack with no closing delimiter", "[parse][SubstackElement]") {
  REQUIRE_THROWS_AS(SubstackElement::parse("<< 1, << 3 >>"), ParserException);
  REQUIRE_THROWS_AS(SubstackElement::parse("<< 1, << 3 >>,"), ParserException);
}

TEST_CASE("substack with extra closing delimiter", "[parse][SubstackElement]") {
  REQUIRE_THROWS_AS(SubstackElement::parse("<< 1 >>, >>"), ParserException);
}

TEST_CASE("regular substack parse", "[parse][SubstackElement]") {
  SubstackElement* s = SubstackElement::parse("<< 1, \">>\\\"string\\\"<<\">>");
  auto iter = s->getData().begin();
  const NumberElement* num = dynamic_cast<const NumberElement*>(*iter);
  REQUIRE(num != nullptr);
  REQUIRE(num->getData() == mpq_class("1"));
  ++iter;
  const StringElement* str = dynamic_cast<const StringElement*>(*iter);
  REQUIRE(str != nullptr);
  REQUIRE(str->getData() == ">>\"string\"<<");
  delete s;
}

TEST_CASE("type base case good", "[parse][TypeElement]") {
  TypeElement* t = TypeElement::parse("Number");
  REQUIRE(t->getData() == StackElement::DataType::Number);
  REQUIRE(t->getSpecialization() == nullptr);
  delete t;
}

TEST_CASE("type base case good specialization", "[parse][TypeElement]") {
  TypeElement* t = TypeElement::parse("Quoted");
  REQUIRE(t->getData() == StackElement::DataType::Quoted);
  REQUIRE(t->getSpecialization() == nullptr);
  delete t;
}

TEST_CASE("type base case bad", "[parse][TypeElement]") {
  REQUIRE_THROWS_AS(TypeElement::parse("Junk"), ParserException);
}

TEST_CASE("substack type specialized good", "[parse][TypeElement]") {
  TypeElement* t = TypeElement::parse("Substack(Any)");
  REQUIRE(t->getData() == StackElement::DataType::Substack);
  REQUIRE(t->getSpecialization()->getData() == StackElement::DataType::Any);
  REQUIRE(t->getSpecialization()->getSpecialization() == nullptr);
  delete t;
}

TEST_CASE("substack type specialized bad", "[parse][TypeElement]") {
  REQUIRE_THROWS_AS(TypeElement::parse("Substack(Exact)"), ParserException);
}

TEST_CASE("number type specialized good", "[parse][TypeElement]") {
  TypeElement* t = TypeElement::parse("Number(Inexact)");
  REQUIRE(t->getData() == StackElement::DataType::Number);
  REQUIRE(t->getSpecialization()->getData() == StackElement::DataType::Inexact);
  REQUIRE(t->getSpecialization()->getSpecialization() == nullptr);
  delete t;
}

TEST_CASE("number type specialized bad", "[parse][TypeElement]") {
  REQUIRE_THROWS_AS(TypeElement::parse("Number(Quoted)"), ParserException);
}

TEST_CASE("command type specialized good", "[parse][TypeElement]") {
  TypeElement* t = TypeElement::parse("Command(Quoted)");
  REQUIRE(t->getData() == StackElement::DataType::Command);
  REQUIRE(t->getSpecialization()->getData() == StackElement::DataType::Quoted);
  REQUIRE(t->getSpecialization()->getSpecialization() == nullptr);
  delete t;
}

TEST_CASE("command type specialized bad", "[parse][TypeElement]") {
  REQUIRE_THROWS_AS(TypeElement::parse("Command(Exact)"), ParserException);
}

TEST_CASE("unspecializable type specialzied", "[parse][TypeElement]") {
  REQUIRE_THROWS_AS(TypeElement::parse("String(Number)"), ParserException);
}