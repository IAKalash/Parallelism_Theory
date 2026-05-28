CXX = pgc++
SRC = lab.cpp
TARGET_HOST = therm_host
TARGET_MULTICORE = therm_multicore
TARGET_GPU = therm_gpu
BOOST_LIBS = -lboost_program_options
CXXFLAGS = -O3 -std=c++11 -Minfo=all

all: host multicore gpu

host: $(SRC)
	$(CXX) -acc=host $(CXXFLAGS) -o $(TARGET_HOST) $(SRC) $(BOOST_LIBS)

multicore: $(SRC)
	$(CXX) -acc=multicore $(CXXFLAGS) -o $(TARGET_MULTICORE) $(SRC) $(BOOST_LIBS)

gpu: $(SRC)
	$(CXX) -acc=gpu $(CXXFLAGS) -o $(TARGET_GPU) $(SRC) $(BOOST_LIBS)

clean:
	rm -f $(TARGET_HOST) $(TARGET_MULTICORE) $(TARGET_GPU) result.dat out10.txt out13.txt
