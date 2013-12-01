#ifndef KRAUDIO_HPP
#define KRAUDIO_HPP

#include <stdio.h>
#include <stdlib.h>

namespace krutoc {

class Audio
{
public:
	Audio(void);
	~Audio(void);

	static Audio *instance(void)
	{
		static Audio staticAudioInstance;
		return &staticAudioInstance;
	}

private:
	FILE *mFile;
};

}

#endif
