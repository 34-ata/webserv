include colors.mk
include settings.mk

TEST_FILE = <test_file>

all: $(NAME)

$(NAME): $(OBJS) | $B
	$(CXX) $(CXXFLAGS) $< -o $B/$(NAME)

$O/%.o: $S/%.cpp | $O $D
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
	mv $O/$(notdir $*).d $D/

$B:
	mkdir -p $B

$O:
	mkdir -p $O

$D:
	mkdir -p $D

-include $(DEPS)

run: $(NAME)
	./$B/$(NAME) $(TEST_FILE)

clean:
	$(RM) $O $D

fclean: clean
	$(RM) $B

re: fclean all

credit:
	@awk 'BEGIN{ \
		printf "$(BOLD_RED)%.*s\n$(RESET)", 60, $(BAR); \
		printf "$(BOLD_RED)#%.*s#\n$(RESET)", 48, $(SPACES); \
		printf "$(BOLD_RED)#%.*s$(NAME)%.*s#\n$(RESET)", (49 - length("$(NAME)")) / 2, $(SPACES), (48 - length("$(NAME)")) / 2, $(SPACES); \
		printf "$(BOLD_RED)#%.*s#\n$(RESET)", 48, $(SPACES); \
		printf "$(BOLD_RED)#%.*sby: $(AUTHOR)%.*s#\n$(RESET)", (48 - length("by: $(AUTHOR)")) / 4 * 3, $(SPACES), (52 - length("by: $(AUTHOR)")) / 4, $(SPACES); \
		printf "$(BOLD_RED)#%.*sby: $(AUTHOR2)%.*s#\n$(RESET)", (48 - length("by: $(AUTHOR)")) / 4 * 3, $(SPACES), (60 - length("by: $(AUTHOR2)")) / 4, $(SPACES); \
		printf "$(BOLD_RED)#%.*s#\n$(RESET)", 48, $(SPACES); \
		printf "$(BOLD_RED)%.*s\n$(RESET)", 60, $(BAR); \
		}'

help:
	@awk 'BEGIN{ \
		printf "$(BOLD_GREEN)How to use this Makefile:\n"; \
		printf "\t$(BOLD_CYAN)make: $(BOLD_WHITE)Builds the project.\n"; \
		printf "\t$(BOLD_CYAN)make re: $(BOLD_WHITE)Rebuilds the project.\n"; \
		printf "\t$(BOLD_CYAN)make clean: $(BOLD_WHITE)Removes object files.\n"; \
		printf "\t$(BOLD_CYAN)make fclean: $(BOLD_WHITE)Removes object files and executable.\n"; \
		printf "\t$(BOLD_CYAN)make run: $(BOLD_WHITE)Runs the project with $(TEST_FILE) file.\n"; \
		printf "\t$(BOLD_CYAN)make leak: $(BOLD_WHITE)Runs the project with $(TEST_FILE) file and checks for memory leaks.\n"; \
		printf "\t$(BOLD_CYAN)make credit: $(BOLD_WHITE)Shows the project credits.\n"; \
		printf "\t$(BOLD_CYAN)make help: $(BOLD_WHITE)Shows this help message.\n"; \
	}'

.PHONY: all run clean fclean re help credit

