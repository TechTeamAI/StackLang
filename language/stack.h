#ifndef STACK_H
#define STACK_H

#include "language/stackElement.h"

namespace StackLang
{
    class Stack
    {
        struct Node;

        public:

        class StackIterator;

        Stack ();
        Stack (const Stack& other);

        ~Stack ();

        void push (StackElement* ptr);
        void pop ();
        StackElement* top ();

        StackIterator begin () const;
        StackIterator end () const;

        unsigned size () const;

        private:

        struct Node
        {
            StackElement* elm;
            Node* next;
        };

        Node* head;
        int dataSize;
    };
}

#endif /* STACK_H */