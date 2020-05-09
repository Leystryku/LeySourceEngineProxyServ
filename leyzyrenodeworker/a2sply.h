#include <stdint.h>
#include <string>

struct a2sply
{
	uint8_t players;
	char names[0xFF+1][0xFF];
	long frags[0xFF+1];
	float connectiontimes[0xFF+1];
};

/*
struct a2sply_fakes
{
	int lastgenerated = 0;
	char names[0xFF][0xFF];
	long frags[0xFF];
	float connectiontimes[0xFF];
};*/