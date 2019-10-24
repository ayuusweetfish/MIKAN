#include "uspios.h"

void *malloc (unsigned nSize)
{
}

void free (void *pBlock)
{
}

void MsDelay (unsigned nMilliSeconds)
{
}	
void usDelay (unsigned nMicroSeconds)
{
}

unsigned StartKernelTimer (unsigned	        nHzDelay,
			   TKernelTimerHandler *pHandler,
			   void *pParam, void *pContext)
{
}

void CancelKernelTimer (unsigned hTimer)
{
}

void ConnectInterrupt (unsigned nIRQ, TInterruptHandler *pHandler, void *pParam)
{
}

int SetPowerStateOn (unsigned nDeviceId)
{
}

int GetMACAddress (unsigned char Buffer[6])
{
}

void LogWrite (const char *pSource,
	       unsigned	   Severity,
	       const char *pMessage, ...)
{
}

void uspi_assertion_failed (const char *pExpr, const char *pFile, unsigned nLine)
{
}

void DebugHexdump (const void *pBuffer, unsigned nBufLen, const char *pSource)
{
}
