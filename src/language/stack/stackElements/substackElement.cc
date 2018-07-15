#include "language/stack/stackElements/substackElement.h"

#include "language/exceptions/parserException.h"
#include "language/stack/stackIterator.h"
#include "util/stringUtils.h"

namespace StackLang {
namespace StackElements {
using StackLang::StackElement;
using StackLang::Exceptions::ParserException;
using std::to_string;
using Util::trim;

const char* SubstackElement::SUBSTACK_BEGIN = "<< ";
const char* SubstackElement::SUBSTACK_END = " >>";
const char* SubstackElement::SUBSTACK_SEPARATOR = ", ";
const char* SubstackElement::SUBSTACK_EMPTY = "<< (empty) >>";

SubstackElement* SubstackElement::parse(const string& s) {
  int parseLevel = 0;
  bool inString = false;
  string accumulator = "";
  char lastChar = '\0';
  Stack buffer;

  for (auto iter = s.begin() + 2; iter != s.end() - 2;
       iter++)  // loop between the << and >>'s
  {
    accumulator += *iter;

    if (!inString && *iter == ',' &&
        parseLevel == 0)  // found a comma at top level - our responsibility
    {
      accumulator.erase(accumulator.end() - 1);  // remove the comma
      buffer.push(parse(trim(accumulator)));
      accumulator = "";
    } else if (*iter == '"' && lastChar != '\\')  // found an unescaped quote
    {
      inString = !inString;
    } else if (!inString && *iter == '<' &&
               lastChar == '<')  // start of substack
    {
      parseLevel++;
    } else if (!inString && *iter == '>' && lastChar == '>')  // end of substack
    {
      parseLevel--;
      if (parseLevel < 0) {
        throw ParserException(
            "Missing at least one matching opening substack delimiter.", s,
            iter - s.begin() - 1);
      }
    }

    lastChar = *iter;
  }

  if (parseLevel != 0) {
    throw ParserException("Missing " + to_string(parseLevel) +
                              " closing substack delimiter" +
                              (parseLevel == 1 ? "" : "s") + ".",
                          s, s.length() - 1);
  } else if (trim(accumulator) != "")  // allow empty lists and terminal commas
  {
    buffer.push(parse(trim(accumulator)));
  }

  buffer.reverse();
  return new SubstackElement(buffer);
}

SubstackElement::SubstackElement(const Stack& s)
    : StackElement(StackElement::DataType::Substack), data(s) {}

SubstackElement* SubstackElement::clone() const {
  return new SubstackElement(data);
}

SubstackElement::operator const string() const {
  if (data.size() == 0) {
    return SUBSTACK_EMPTY;
  }
  string buffer = SUBSTACK_BEGIN;

  for (auto elm : data) {
    buffer += string(*elm);
    buffer += SUBSTACK_SEPARATOR;
  }

  buffer.erase(buffer.length() - 2);

  buffer += SUBSTACK_END;

  return buffer;  // return value optimization
}

Stack& SubstackElement::getData() { return data; }
}  // namespace StackElements
}  // namespace StackLang