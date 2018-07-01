#include "language/stack.h"

#include "language/exceptions/stackOverflowError.h"
#include "language/stack/stackIterator.h"

#include <memory>

namespace StackLang
{
using StackLang::StackElement;
using StackLang::Exceptions::StackOverflowError;

Stack::Stack () :
    head (nullptr), dataSize (0), limit (__LONG_MAX__)
{}

Stack::Stack (unsigned long lim) :
    head (nullptr), dataSize (0), limit (lim)
{}

Stack::~Stack ()
{
    clear ();
}

void Stack::push (StackElement* ptr)
{
    if (dataSize >= limit)
    {
        throw StackOverflowError (limit);
    }

    Node* temp = new Node;
    temp->elm = unique_ptr< StackElement > (ptr);
    temp->next = head;
    head = temp;
    dataSize++;
}

StackElement* Stack::pop ()
{
    if (head != nullptr)
    {
        StackElement* retval = head->elm.release ();
        Node* temp = head->next;
        delete head;
        head = temp;
        dataSize--;
    }
}

StackElement*
        Stack::top ()
{
    if (head != nullptr)
    {
        return head->elm.get ();
    }
}

unsigned long
        Stack::size () const
{
    return dataSize;
}

unsigned long
        Stack::getLimit () const
{
    return limit;
}

void Stack::setLimit (unsigned long newLimit)
{
    if (dataSize > newLimit)
    {
        throw StackOverflowError (newLimit);
    }

    limit = newLimit;
}

bool Stack::empty () const
{
    return dataSize == 0;
}

Stack::StackIterator
        Stack::begin () const
{
    return StackIterator (head);
}

Stack::StackIterator
        Stack::end () const
{
    return StackIterator (nullptr);
}

void Stack::clear ()
{

    while (head != nullptr)
    {
        pop ();
    }

    dataSize = 0;
}
} // namespace StackLang