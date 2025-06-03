
all: raycast

raycast: raycast.cpp
	g++ raycast.cpp -o raycast -lSDL2

.PHONY: clean
clean:
	rm -f *.o raycast