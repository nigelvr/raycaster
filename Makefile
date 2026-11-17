
all: raycast

raycast: raycast.cpp
	g++ -g raycast.cpp -o raycast -lSDL2 -lSDL2_image

.PHONY: clean
clean:
	rm -f *.o raycast