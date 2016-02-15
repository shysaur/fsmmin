

OBJDIR	= obj

DEPS	= fsm.h \
	  equivgraph.h \
	  minimize.h

CXX_SRC_MIN	= fsmmin.cpp \
	  fsm.cpp \
	  equivgraph.cpp \
	  minimize.cpp

CXX_SRC_GEN	= fsmgen.cpp

OBJ_MIN	= $(patsubst %, $(OBJDIR)/%, $(CXX_SRC_MIN:.cpp=.o))
OBJ_GEN	= $(patsubst %, $(OBJDIR)/%, $(CXX_SRC_GEN:.cpp=.o))

.PHONY:	all
.PHONY:	clean

all:	$(OBJDIR) fsmmin fsmgen

$(OBJDIR):	
	mkdir -p $(OBJDIR)

fsmmin:	$(OBJ_MIN)
	$(CXX) $(CXXFLAGS) -o fsmmin $(OBJ_MIN)

fsmgen:	$(OBJ_GEN)
	$(CXX) $(CXXFLAGS) -o fsmgen $(OBJ_GEN)

$(OBJDIR)/%.o:	%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:	
	rm -f $(OBJDIR)/*.o fsmmin fsmgen

