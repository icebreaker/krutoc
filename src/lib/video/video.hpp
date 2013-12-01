#ifndef KRVIDEO_HPP
#define KRVIDEO_HPP

#include "gl.hpp"

namespace krutoc {

class Video
{
public:
	Video(void);
	~Video(void);

	void update(void);

	static Video *instance(void)
	{
		static Video staticVideoInstance;
		return &staticVideoInstance;
	}

private:
	FILE *mFile;
	FILE *mAudioFile;

	GLuint mWidth;
	GLuint mHeight;

	GLuint mBufferSize;
	GLubyte *mBuffer; // unsigned char *

	GLuint mIds[2];
	GLuint mActive;
	GLint  mBinding;

	bool mPBOAvailable;
	bool mInitialized;
};

}

#endif
