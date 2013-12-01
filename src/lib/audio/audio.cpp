#include "audio.hpp"

namespace krutoc {

Audio::Audio(void) : mFile(NULL)
{
	const char *cmd = getenv("KR_AUDIO");
	if(cmd == NULL)
	{
		fprintf(stderr, "KR_AUDIO has not been defined. Disabling Audio.\n");
		return;
	}
	else
	{
		fprintf(stdout, "KR_AUDIO=%s\n", cmd);
	}

	mFile = popen(cmd, "w"); // FIXME: "wb" fails
	if(mFile == NULL)
	{
		// FIXME: add better error reporting
		fprintf(stderr, "Unable to execute KR_AUDIO as a command. Audio won't be captured.\n");
		return;
	}
}

Audio::~Audio(void)
{
	if(mFile != NULL)
	{
		fprintf(mFile, "q\n");
		fflush(mFile);
		pclose(mFile);
	}	
}

}
