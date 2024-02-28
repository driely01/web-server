C = c++
flag = -Wall -Wextra -Werror -std=c++98
src = main.cpp mimeType.cpp request.cpp Server.cpp tools.cpp
obj = $(src:.cpp=.o)
header = Server.hpp
Name = a.out
all : $(Name)

$(Name) : $(obj)
	$(C) $(obj) -o $@
%.o : %.cpp $(header)
		$(C) $(flag) -c $< -o $@
clean:
		rm -r $(obj)
fclean: clean
		rm -rf $(Name)
re: fclean all

.PHONY: re fclean all clean