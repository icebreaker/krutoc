#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "video.hpp"
#include "audio/audio.hpp"

namespace krutoc {

Video::Video(void) : 
	mFile(NULL),
	mWidth(0),
	mHeight(0),
	mBufferSize(0),
	mBuffer(NULL),
	mActive(2),
	mBinding(0),
	mPBOAvailable(true),
	mInitialized(false)
{
	const char *cmd = getenv("KR_VIDEO");
	if(cmd == NULL)
	{
		fprintf(stderr, "KR_VIDEO has not been defined.\n");
		return;
	}
	else
	{
		fprintf(stdout, "KR_VIDEO=%s\n", cmd);
	}

	const char *width = getenv("KR_VIDEO_WIDTH");
	if(width == NULL || ((mWidth = atoi(width)) <= 0))
	{
		fprintf(stderr, "KR_VIDEO_WIDTH has not been defined or is invalid.\n");
		return;
	}

	const char *height = getenv("KR_VIDEO_HEIGHT");
	if(height == NULL || ((mHeight = atoi(height)) <= 0))
	{
		fprintf(stderr, "KR_VIDEO_HEIGHT has not been defined or is invalid.\n");
		return;
	}

	const char *pbo = getenv("KR_VIDEO_PBO");
	if(pbo != NULL)
	{
		mPBOAvailable = static_cast<bool>(atoi(pbo));
	}

	mFile = popen(cmd, "w"); // FIXME: "wb" fails
	if(mFile == NULL)
	{
		// FIXME: add better error reporting
		fprintf(stderr, "Unable to execute KR_VIDEO as a command.\n");
		return;
	}

	// TODO: add configurable BPP and format i.e RGBA vs BGRA
	mBufferSize = mWidth * mHeight * 4 * sizeof(GLubyte);
	mBuffer = new GLubyte[mBufferSize];

	mInitialized = true;
}

Video::~Video(void)
{
	// FIXME: clean up, we don't have the GL context when this is happening ...
	//
	// We would face the same issue if we registered a callback via atexit().
	// 
	// Solution:
	//
	// Hook into xglMakeCurrent() and do the PBO initialization / destruction
	// in there based on the arguments that were passed in.
	//
	// Caveats: Watch out for multiple threads calling xglMakeCurrent().
	//
	if(mPBOAvailable && mActive < 2)
	{
		// glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
		// glDeleteBuffersARB(2, mIds);
	}

	if(mFile != NULL)
	{
		fflush(mFile);
		pclose(mFile);
	}

	if(mBuffer != NULL)
		delete [] mBuffer;
}

void Video::update(void)
{
	if(!mInitialized)
		return;

	if(!mPBOAvailable)
	{
		glReadPixels(0, 0, mWidth, mHeight, GL_BGRA, GL_UNSIGNED_BYTE, mBuffer);
		fwrite(mBuffer, mBufferSize, 1, mFile);
		return;
	}

	// http://http.download.nvidia.com/developer/Papers/2005/Fast_Texture_Transfers/Fast_Texture_Transfers.pdf
	if(mActive == 2)
	{
		Audio::instance();

		const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
		if(extensions == NULL || strstr(extensions, "GL_ARB_pixel_buffer_object") == NULL)
		{
			mPBOAvailable = false;
			update();
			return;
		}

		glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &mBinding);
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glGenBuffersARB(2, mIds);

		for(int i=0; i<2; i++)
		{
			glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mIds[i]);
			glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, mBufferSize, 0, GL_STREAM_READ_ARB);
		}

		glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
		glPixelStorei(GL_PACK_ALIGNMENT, 8);
		glReadBuffer(GL_BACK);
		glReadPixels(0, 0, mWidth, mHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
		glPopClientAttrib();

		glPopAttrib();
		glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mBinding);
		mActive = 1;

		return;
	}

	glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB, &mBinding);

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mIds[1-mActive]);
	glPushAttrib(GL_PIXEL_MODE_BIT);
	glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);
    glPixelStorei(GL_PACK_ALIGNMENT, 8);
	glReadBuffer(GL_BACK);
	glReadPixels(0, 0, mWidth, mHeight, GL_BGRA, GL_UNSIGNED_BYTE, 0);
	glPopClientAttrib();
	glPopAttrib();

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mIds[mActive]);

	GLubyte *ptr = reinterpret_cast<GLubyte *>(glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB));
	if(ptr != NULL)
	{
		fwrite(ptr, mBufferSize, 1, mFile);
		glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
	}
#ifdef KR_DEBUG
	else
	{
		fprintf(stderr, "glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB) failed with %d\n", glGetError());
	}
#endif

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, mBinding);
	mActive = 1 - mActive;
}

}
