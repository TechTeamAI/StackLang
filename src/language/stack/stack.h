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

// Central data stack (not using STL stack because we are working with a
// polymorphic pointer). Also includes iterator and base element declaration.

#ifndef STACKLANG_LANGUAGE_STACK_H_
#define STACKLANG_LANGUAGE_STACK_H_

#include <initializer_list>
#include <limits>
#include <memory>
#include <string>

namespace stacklang {

// Represents a element in the stack. Subclassed to make a specific element.
class StackElement {
 public:
  static const int NUM_PRIM_TYPES;
  enum class DataType {
    Number,
    String,
    Boolean,
    Substack,
    Type,
    Command,
    Identifier,
    Primitive,
    Defined,
    Any,
  };

  // Produces a StackElement (of some type) from a terminal input string
  static StackElement* parse(const std::string&);

  StackElement(const StackElement&) = default;
  StackElement(StackElement&&) = default;
  virtual StackElement* clone() const noexcept = 0;

  StackElement& operator=(const StackElement&) = default;
  StackElement& operator=(StackElement&&) = default;

  virtual ~StackElement() = 0;

  virtual bool operator==(const StackElement&) const noexcept = 0;

  // Produces a nicely formatted string of the element (for print to
  // console)
  explicit virtual operator std::string() const noexcept = 0;

  // Getter for the DataType
  DataType getType() const noexcept;

  // Gets the data from the element (returns different type depending on
  // dataType) should exist in all subclasses, where T is the closest
  // dataTYpe to the element's type T getData () const;

  // Constructs an element of some type (takes different param type
  // depending on dataType) should exist in all subclasses. Element
  // (dataType);

 protected:
  // Constructs a StackElement by setting the dataType
  explicit StackElement(DataType) noexcept;

  DataType dataType;
};

typedef std::unique_ptr<StackElement> ElementPtr;

// A literal stack of StackElements.
class Stack {
  struct Node;

 public:
  class StackIterator;

  // Creates a stack, optionally with a limit on the number of elements.
  explicit Stack(size_t = std::numeric_limits<size_t>().max()) noexcept;
  explicit Stack(std::initializer_list<StackElement*>) noexcept;
  Stack(const Stack&) noexcept;
  Stack(Stack&&) noexcept;

  ~Stack() noexcept;

  Stack& operator=(const Stack&) noexcept;
  Stack& operator=(Stack&&) noexcept;

  // Manipulates stack elements. Pop does not delete the element, but the stack
  // destructor does assume that when the stack is destroyed, it's managed
  // elements should be deallocated.
  void push(StackElement*);
  StackElement* pop();
  const StackElement* top();

  void clear() noexcept;

  StackIterator begin() const noexcept;
  StackIterator end() const noexcept;

  size_t size() const noexcept;
  size_t getLimit() const noexcept;

  // Sets a new limit. Throws StackOverflowError if too big.
  void setLimit(size_t);

  void reverse() noexcept;

  bool isEmpty() const noexcept;

 private:
  struct Node {
    std::unique_ptr<StackElement> elm;
    Node* next;
    Node(StackElement*, Node*) noexcept;
  };
  Node* copy(Node*) noexcept;

  Node* head;
  size_t dataSize;
  size_t limit;
};

// iterator for a Stack
class Stack::StackIterator {
 public:
  typedef size_t difference_type;
  typedef const StackElement* value_type;
  typedef const StackElement** pointer;
  typedef const StackElement*& reference;
  typedef size_t size_type;
  typedef std::input_iterator_tag iterator_category;

  explicit StackIterator(const Stack::Node*) noexcept;
  StackIterator(const StackIterator&) = default;

  StackIterator& operator=(const StackIterator&) = default;

  friend bool operator==(const Stack::StackIterator&,
                         const Stack::StackIterator&) noexcept;
  friend bool operator!=(const Stack::StackIterator&,
                         const Stack::StackIterator&) noexcept;

  const StackElement* operator*() noexcept;

  StackIterator& operator++() noexcept;
  StackIterator operator++(int) noexcept;

 private:
  const Stack::Node* curr;
};

bool operator==(const Stack::StackIterator&,
                const Stack::StackIterator&) noexcept;
bool operator!=(const Stack::StackIterator&,
                const Stack::StackIterator&) noexcept;
}  // namespace stacklang
#endif  // STACKLANG_LANGUAGE_STACK_H_
