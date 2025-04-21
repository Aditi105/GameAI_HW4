CXX      := g++
CXXFLAGS := -std=c++17 -I.
LDFLAGS  := -L/usr/lib/aarch64-linux-gnu -L/usr/lib/x86_64-linux-gnu \
             -lsfml-graphics -lsfml-window -lsfml-system

# core library sources
SRCS_LIB := DecisionNode.cpp \
            ConditionNode.cpp \
            ActionNode.cpp \
            BehaviorTreeFactory.cpp \
            BehaviorController.cpp \
            SequenceNode.cpp \
            SelectorNode.cpp \
            RandomSelectorNode.cpp \
            MonsterTasks.cpp \
            MonsterBehaviorFactory.cpp \
            MonsterController.cpp \
            Environment.cpp \
            Node.cpp \
            DataRecorder.cpp \
			Environment.cpp \
            Node.cpp \
            DataRecorder.cpp

# part1, part2 and part3 each provide their own main()
PART_SRCS := part1.cpp \
             part2.cpp \
             part3.cpp

# object files
OBJS_LIB   := $(SRCS_LIB:.cpp=.o)
PART_OBJS  := $(PART_SRCS:.cpp=.o)

# executables to build
PARTS := part1 part2 part3

all: $(PARTS)

# link each part executable out of the common objs + its main obj
$(PARTS): %: $(OBJS_LIB) %.o
	$(CXX) $^ $(LDFLAGS) -o $@

# compilation rule for all .cpp → .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_LIB) $(PART_OBJS) $(PARTS)

.PHONY: all clean
