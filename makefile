# Copyright 2018 Justin Hu, Bronwyn Damm, Jacques Marais, Ramon Rakow, and
# Jude Sidloski
#
# This file is part of the StackLang interpreter.
#
# The StackLang interpreter is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# The StackLang interpreter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# the StackLang interpreter.  If not, see <https:#www.gnu.org/licenses/>.

#command options
CC := g++
RM := rm -rf
MKDIR := mkdir -p


#File options
SRCDIR := src
SRCS := $(shell find -O3 $(SRCDIR)/ -type f -name '*.cc')

OBJDIR := bin
OBJS := $(patsubst $(SRCDIR)/%.cc,$(OBJDIR)/%.o,$(SRCS))

DEPDIR := dependencies
DEPS := $(patsubst $(SRCDIR)/%.cc,$(DEPDIR)/%.dep,$(SRCS))


#Test file options
TSRCDIR := tests
TSRCS := $(shell find -O3 $(TSRCDIR)/ -type f -name '*.cc')

TOBJDIR := tests/bin
TOBJS := $(patsubst $(TSRCDIR)/%.cc,$(TOBJDIR)/%.o,$(TSRCS))

TDEPDIR := tests/dependencies
TDEPS := $(patsubst $(TSRCDIR)/%.cc,$(TDEPDIR)/%.dep,$(TSRCS))


#compiler configuration
GPPWARNINGS := -Wlogical-op -Wuseless-cast -Wnoexcept -Wstrict-null-sentinel
WARNINGS := -pedantic -pedantic-errors -Wall -Wextra $(GPPWARNINGS) -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wmissing-declarations -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-overflow=5 -Wswitch-default -Wundef -Wzero-as-null-pointer-constant -Wno-unused
OPTIONS := -std=c++17 $(WARNINGS)

#build-specific compiler options
DEBUGOPTIONS := -Og -ggdb
RELEASEOPTIONS := -Os -Wunused

#libraries and included files
LIBS := $(shell pkg-config --libs ncurses)
INCLUDES := -I$(SRCDIR)
TINCLUDES := -I$(TSRCDIR)

#final executable name
EXENAME := stacklang
TEXENAME := stacklangTest


.PHONY: debug release clean diagnose
.SECONDEXPANSION:


debug: OPTIONS := $(OPTIONS) $(DEBUGOPTIONS)
debug: $(EXENAME)# $(TEXENAME)
	@echo "Done compiling debug."
#	@echo "Running tests."
#	@echo ""
#	@./$(TEXENAME)
	@echo "Debug build finished."

release: OPTIONS := $(OPTIONS) $(RELEASEOPTIONS)
release: $(EXENAME)# $(TEXENAME)
	@echo "Done with release."
#	@echo "Running tests."
#	@echo ""
#	@./$(TEXENAME)
	@echo "Release build finished."


clean:
	@echo "Removing $(DEPDIR)/, $(OBJDIR)/, and $(EXENAME)"
	@$(RM) $(OBJDIR) $(DEPDIR) $(EXENAME)
	@echo "Removing $(TDEPDIR)/, $(TOBJDIR)/, and $(TEXENAME)"
	@$(RM) $(TOBJDIR) $(TDEPDIR) $(TEXENAME)


$(EXENAME): $(OBJS)
	@echo "Linking..."
	@$(CC) -o $(EXENAME) $(OPTIONS) $(OBJS) $(LIBS)

$(OBJS): $$(patsubst $(OBJDIR)/%.o,$(SRCDIR)/%.cc,$$@) $$(patsubst $(OBJDIR)/%.o,$(DEPDIR)/%.dep,$$@) | $$(dir $$@)
	@echo "Compiling $@..."
	@clang-format -i $(filter-out %.dep,$^)
	@$(CC) $(OPTIONS) $(INCLUDES) -c $< -o $@

$(DEPS): $$(patsubst $(DEPDIR)/%.dep,$(SRCDIR)/%.cc,$$@) | $$(dir $$@)
	@set -e; $(RM) $@; \
	 $(CC) $(OPTIONS) $(INCLUDES) -MM -MT $(patsubst $(DEPDIR)/%.dep,$(OBJDIR)/%.o,$@) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$


$(TEXENAME): $(TOBJS) $(OBJS)
	@echo "Linking test..."
	@$(CC) -o $(TEXENAME) $(OPTIONS) $(filter-out %main.o,$(OBJS)) $(TOBJS) $(LIBS)

$(TOBJS): $$(patsubst $(TOBJDIR)/%.o,$(TSRCDIR)/%.cc,$$@) $$(patsubst $(TOBJDIR)/%.o,$(TDEPDIR)/%.dep,$$@) | $$(dir $$@)
	@echo "Compiling $@..."
	@clang-format -i $(filter-out %.dep,$^)
	@$(CC) $(OPTIONS) $(INCLUDES) $(TINCLUDES) -c $< -o $@

$(TDEPS): $$(patsubst $(TDEPDIR)/%.dep,$(TSRCDIR)/%.cc,$$@) | $$(dir $$@)
	@set -e; $(RM) $@; \
	 $(CC) $(OPTIONS) $(INCLUDES) $(TINCLUDES) -MM -MT $(patsubst $(TDEPDIR)/%.dep,$(TOBJDIR)/%.o,$@) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$


%/:
	@$(MKDIR) $@


-include $(DEPS) $(TDEPS)