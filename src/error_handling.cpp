#include "context.h"

void setErrCode(sglEErrorCode c)
{
  if (sceneManager->errorCode == SGL_NO_ERROR)
      sceneManager->errorCode = c;
}

//---------------------------------------------------------------------------
// sglGetError()
//---------------------------------------------------------------------------
sglEErrorCode sglGetError(void)
{
  sglEErrorCode ret = sceneManager->errorCode;
  sceneManager->errorCode = SGL_NO_ERROR;
  return ret;
}

//---------------------------------------------------------------------------
// sglGetErrorString()
//---------------------------------------------------------------------------
const char *sglGetErrorString(sglEErrorCode error)
{
	static const char *errStrigTable[] =
		{
			"Operation succeeded",
			"Invalid argument(s) to a call",
			"Invalid enumeration argument(s) to a call",
			"Invalid call",
			"Quota of internal resources exceeded",
			"Internal library error",
			"Matrix stack overflow",
			"Matrix stack underflow",
			"Insufficient memory to finish the requested operation"};

	if ((int)error < (int)SGL_NO_ERROR || (int)error > (int)SGL_OUT_OF_MEMORY)
		return "Invalid value passed to sglGetErrorString()";

	return errStrigTable[(int)error];
}
