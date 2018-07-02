#include "language/exceptions/languageException.h"
#include "language/exceptions/stackOverflowError.h"
#include "language/language.h"
#include "language/stack.h"
#include "language/stack/stackElement.h"
#include "language/stack/stackElements/commandElement.h"
#include "language/stack/stackElements/stringElement.h"
#include "ui/lineEditor.h"
#include "ui/ui.h"

#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <ncurses.h>
#include <stdexcept>
#include <string>

namespace TermUI
{
namespace SigHandler
{
using StackLang::Stack;
using Util::LineEditor;

const Stack* UpdateStack; // used for resize handlers - set once, then ignored
const LineEditor* UpdateBuffer;
} // namespace SigHandler
} // namespace TermUI

namespace KeyInfo
{
const char KEY_CTRL_D = 4; // self defined constants
}

int main (int argc, char* argv[])
{
    using StackLang::Stack;
    using StackLang::StackElement;
    using StackLang::Exceptions::LanguageException;
    using StackLang::Exceptions::StackOverflowError;
    using StackLang::StackElements::CommandElement;
    using StackLang::StackElements::StringElement;
    using std::cerr;
    using std::endl;
    using std::invalid_argument;
    using std::map;
    using std::numeric_limits;
    using std::stoi;
    using std::stoul;
    using std::string;
    using TermUI::addstring;
    using TermUI::displayInfo;
    using TermUI::drawError;
    using TermUI::drawPrompt;
    using TermUI::drawStack;
    using TermUI::init;
    using TermUI::LineEditor;
    using TermUI::printError;
    using TermUI::SigHandler::UpdateBuffer;
    using TermUI::SigHandler::UpdateStack;

    Stack s;
    // map<string, ???> defines;

    int key = 0;
    LineEditor buffer;

    int debugmode = 0;
    bool errorFlag = false;
    bool argsInclude = false;
    bool argsQuiet = false;

    UpdateStack = &s; // set const pointers for data access from event handlers
    UpdateBuffer = &buffer;

    for (int i = 1; i < argc; i++) // command line args evaluation
    {
        if (string (argv[i]) == "--") // end of specified files to include
        {
            argsInclude = false;
        }
        else if (argsInclude)
        {
            s.push (new StringElement (argv[i]));
            s.push (new CommandElement ("include"));

            try
            {
                execute (s);
            }
            catch (const LanguageException& e)
            {
                printError (e);
                cerr << "Aborting." << endl;
                exit (EXIT_FAILURE);
            }
        }
        else if (string (argv[i]) == "-d") // set debug mode
        {
            if (i + 1 >= argc)
            {
                cerr << "Expected number after `-d`, found nothing. Abort." << endl;
                exit (EXIT_FAILURE);
            }

            try
            {
                debugmode = stoi (argv[++i]);
            }
            catch (const invalid_argument& e)
            {
                cerr << "Expected number after `-d`, found `" << argv[i] << "`. Abort."
                     << endl;
                exit (EXIT_FAILURE);
            }
        }
        else if (string (argv[i]) == "-I") // start of specified files to include
        {
            argsInclude = true;
        }
        else if (string (argv[i]) == "-l") // set limit
        {
            if (i + 1 >= argc)
            {
                cerr << "Expected number after `-l`, found nothing. Abort" << endl;
            }

            try
            {
                s.setLimit (stoul (argv[++i]));
            }
            catch (const invalid_argument& e)
            {
                cerr << "Expected number after `-l`, found `" << argv[i] << "`. Abort."
                     << endl;
            }
            catch (const StackOverflowError& e)
            {
                cerr << "Specified limit is too low to contain " << s.size ()
                     << " elements, the current size of the stack. Abort." << endl;
            }
        }
    }

    init ();

    displayInfo (); // splash screen

    drawStack (s);
    drawPrompt (buffer); // draw the stack and prompt

    while (true)
    {
        key = getch ();

        if (key == KeyInfo::KEY_CTRL_D) // overriding keypresses
        {
            break;
        }
        else if (errorFlag) //anything on an error is ignored, but the error is cleared
        {
            drawStack (s);
            drawPrompt (buffer);
            errorFlag = false;
            continue;
        }

        if (key < numeric_limits< char > ().max () && isprint (key) && key != '\n' &&
            key != '\r' && key != KEY_ENTER) // normal characters added to buffer.
        {
            buffer += key;
            drawPrompt (buffer);
        }
        else if (key == '\n' || key == '\r' || key == KEY_ENTER) // enter - add and execute
        {
            try
            {
                s.push (StackElement::parse (buffer));
                buffer.enter ();
                drawStack (s);
                drawPrompt (buffer);
                execute (s);
            }
            catch (const LanguageException& e)
            {
                drawError (e);
                errorFlag = true;
                buffer.clear ();
            }
        }
        else if (key == KEY_BACKSPACE) // line editing.
        {
            buffer.backspace ();
            drawPrompt (buffer);
        }
        else if (key == KEY_DC)
        {
            buffer.del ();
            drawPrompt (buffer);
        }
        else if (key == KEY_UP)
        {
            buffer.up ();
            drawPrompt (buffer);
        }
        else if (key == KEY_DOWN)
        {
            buffer.down ();
            drawPrompt (buffer);
        }
        else if (key == KEY_RIGHT)
        {
            buffer.right ();
            drawPrompt (buffer);
        }
        else if (key == KEY_LEFT)
        {
            buffer.left ();
            drawPrompt (buffer);
        }
        else if (key == KEY_END)
        {
            buffer.toEnd ();
            drawPrompt (buffer);
        }
        else if (key == KEY_HOME)
        {
            buffer.toHome ();
            drawPrompt (buffer);
        }
        else // not recognized.
        {
            beep ();
        }

        if (debugmode == 1) // debug options - must not include 0
        {
            move (0, getmaxx (stdscr) - 3);
            clrtoeol ();
            addstring (std::to_string (key).c_str ());
            move (getmaxy (stdscr) - 1, buffer.cursorPosition () + 2);
        }
        else if (debugmode == 2)
        {
            move (0, 0);
            clrtoeol ();
            addstring ("|");
            addstring (buffer);
            addstring ("|");
            move (getmaxy (stdscr) - 1, buffer.cursorPosition () + 2);
        }
    }

    exit (EXIT_SUCCESS);
}