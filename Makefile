

CPPFLAGS = -Wall -Wextra -pedantic

exec: main.cpp
	g++ $(CPPFLAGS) $< -o $@
