CXX      := g++
CXXFLAGS := -std=c++17 -I.
LDFLAGS  := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu \
             -lsfml-graphics -lsfml-window -lsfml-system

SRCS     := DecisionNode.cpp \
            ConditionNode.cpp \
            ActionNode.cpp \
            BehaviorTreeFactory.cpp \
            BehaviorController.cpp \
            part1.cpp

OBJS     := $(SRCS:.cpp=.o)

all: part1

part1: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) part1

.PHONY: all clean
