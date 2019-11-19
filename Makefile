

CPPFLAGS = -Wall -Wextra -pedantic

exec : main.cpp
    g++ $(CPPARGS) $^ -o $@
