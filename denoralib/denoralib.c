/* Library of common denora code, built as a library
 *
 * (c) 2004-2013 Denora Team
 * Contact us at info@denorastats.org
 *
 * Please read COPYING and README for furhter details.
 *
 *
 */

char *DenoraLastError;
int DenoraLastErrorCode;


char *DenoraLib_GetLastError(void)
{
	if (DenoraLastError)
	{
		return DenoraLastError;
	}
	return NULL;
}

void DenoraLib_SetLastError(int code, char *msg)
{
	DenoraLastErrorCode = code;
	if (DenoraLastError)
	{
		free(DenoraLastError)
	}
	DenoraLastError = StringDup(msg);
	return;
}
