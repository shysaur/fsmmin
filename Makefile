

OBJDIR	= obj

DEPS	= fsm.h \
	  equivgraph.h

CXX_SOURCE	= fsmmin.cpp \
	  fsm.cpp \
	  equivgraph.cpp

OBJ	= $(patsubst %, $(OBJDIR)/%, $(CXX_SOURCE:.cpp=.o))

.PHONY:	all
.PHONY:	clean

all:	$(OBJDIR) fsmmin

$(OBJDIR):	
	mkdir -p $(OBJDIR)

fsmmin:	$(OBJ)
	$(CXX) $(CXXFLAGS) -o fsmmin $(OBJ)

$(OBJDIR)/%.o:	%.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:	
	rm -f $(OBJDIR)/*.o fsmmin

