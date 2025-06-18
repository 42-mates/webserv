# -------------------
# Project Name
# -------------------
NAME	= webserv

# -------------------
# Compiler & Flags
# -------------------
CXX			= g++
CXXFLAGS	= -Wall -Wextra -Werror -std=c++98 -MMD -g3
# -MMD generates .d files for header dependencies while compiling .o files

# -------------------
# Source files
# -------------------
SRC			=	main.cpp				utils.cpp
# Parser
SRC			+=	parser.cpp				lexer.cpp		json.cpp \
				config.cpp
# Multiplexer
SRC			+=	EventHandler.cpp		Server.cpp		ClientConnection.cpp \
				EventHandler_cgi.cpp	CGIManager.cpp	EventHandler_chunks.cpp
# HTTP
SRC			+=	AHttpData.cpp			Request.cpp		Response.cpp \
				DirectoryListing.cpp	CgiContent.cpp
# Cluster
SRC			+=	cluster.cpp
# Define source file search paths with vpath
vpath %.cpp src/\
			src/parser\
			src/multiplexer\
			src/http\
			src/cluster

# -------------------
# Headers & Dependencies
# -------------------
INC			=	-I include \
				-I include/parser \
				-I include/multiplexer \
				-I include/http \
				-I include/cluster
DEP			=	$(OBJ:%.o=%.d)
-include $(DEP)

# Object files
OBJ_FILES	=	$(SRC:%.cpp=%.o)
OBJ_PATH	=	obj/
OBJ			=	$(addprefix $(OBJ_PATH), $(OBJ_FILES))

RM			=	rm -rf
#######################################################
# Colors
DEFAULT		= \e[0;39m
BLACK		= \e[0;30m
WHITE		= \e[0;97m
WHITE_BOLD	= \e[1;97m
GREY		= \e[0;37m
GREY_DARK	= \e[0;90m
GREY_ITALIC	= \e[3;90m
RED			= \e[0;31m
RED_BOLD	= \e[1;31m
RED_LIGHT	= \e[0;31;110m
RED_FLASH	= \e[0;31;5;110m
PINK		= \e[0;38;5;198m
PINK_BOLD	= \e[1;95m
PINK_LIGHT	= \e[0;38;5;200m
ORANGE_ITALIC= \e[3;91m
YELLOW		= \e[0;33m
YELLOW_BOLD	= \e[1;33m
GREEN		= \e[1;32;111m
GREEN_BOLD	= \e[1;32m
GREEN_LIGHT	= \e[0;38;5;121m
BLUE_BOLD	= \e[1;34m
BLUE_LIGHT	= \e[0;38;5;123m
BLUE_GREY	= \e[1;38;5;110m
SKY			= \e[0;36m
SKY_BOLD	= \e[1;36m
CYAN_LIGHT	= \e[0;96m
NOCOLOR		= \e[0m
RESET_LINE	= \e[2K\r
#######################################################
# -------------------
# Rules
# -------------------
all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(INC) $^ -o $@
	@printf "	$(RESET_LINE)$(BLUE_LIGHT)webserv\t: $(GREEN_LIGHT)Updated\n$(NOCOLOR)"

$(OBJ_PATH):
	@mkdir -p $@

$(OBJ_PATH)%.o: %.cpp | $(OBJ_PATH)
	@mkdir -p $(dir $@)
	@printf "$(RESET_LINE)$(RED_FLASH)Compiling...	$(GREY)$<$(SKY)$(NOCOLOR)"
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/.	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/..	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/...	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/.	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/..	$(GREY)"
	@sleep 0.1
	@printf "$(RESET_LINE)$(RED_FLASH)Deleting webserv obj/...	$(GREY)"
	@sleep 0.1
	@$(RM) $(OBJ_PATH)
	@printf "$(RESET_LINE)$(RED_FLASH)Deleted successfully!\n$(NOCOLOR)"

fclean: clean
	@$(RM) $(NAME)

re:	fclean all

test:	all
	bash test.sh all

.PHONY: all clean fclean re test

# Notes:
# 1. @ suppresses the command being printed to the terminal when executed.
# 2. $< is the source file.
# 3. $@ specifies that the output should have the same name as the target file.
# 4. -o means output.