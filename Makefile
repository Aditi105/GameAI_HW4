CXX      := g++
CXXFLAGS := -std=c++17 -I.
LDFLAGS  := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu \
             -lsfml-graphics -lsfml-window -lsfml-system

SRCS     := DecisionNode.cpp \
            ConditionNode.cpp \
            ActionNode.cpp \
            BehaviorTreeFactory.cpp \
            BehaviorController.cpp \
            part4.cpp

OBJS     := $(SRCS:.cpp=.o)

all: part4

part4: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) part4

.PHONY: all clean
