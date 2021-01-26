program = mydict
build_dir = build
sources = $(wildcard *.cpp)
objects = $(addprefix $(build_dir)/,$(sources:cpp=o))
deps = $(addprefix $(build_dir)/,$(sources:cpp=d))
CPPFLAGS = -MMD -MP
CXXFLAGS = -std=c++17 -Wall -Wextra

PQXX_LINK := $(shell pkg-config --libs libpqxx)
PQXX_INCLUDE := $(shell pkg-config --cflags libpqxx)

.PHONY : all
all : $(program)

.PHONY : clean
clean:
	$(RM) $(objects) $(deps) $(program)

$(program) : $(objects)
	$(CXX) $(PQXX_LINK) $^ -o $@

$(objects) : | $(build_dir)

$(build_dir) :
	mkdir $(build_dir)

$(build_dir)/%.o : %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(PQXX_INCLUDE) -c $< -o $@

-include $(deps)
