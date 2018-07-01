#ifndef PARSERERROR_H
#define PARSERERROR_H

#include "language/exceptions/languageException.h"

namespace StackLang
{
namespace Exceptions
{
class ParserError : public LanguageException
{
  public:
    ParserError (const string&, const string&, unsigned);

    const string getKind () const override;
};
} // namespace Exceptions
} // namespace StackLang

#endif /* PARSERERROR_H */