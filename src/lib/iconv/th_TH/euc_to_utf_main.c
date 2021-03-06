/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright (c) 1996 by Sun Microsystems, Inc.
 */

#pragma	ident	"@(#)euc2utf_main.c 1.2     98/04/16 SMI"

#include <errno.h>
#include <widec.h>
#include "common_def.h"
#include "common_thai.h"
#include "euc_utf_api.h"


/****  _ I C V _ O P E N  ****/

void* _icv_open()
{
        return((void*)MAGIC_NUMBER);
}  /* end of int _icv_open(). */


/****  _ I C V _ C L O S E  ****/

void _icv_close(int* cd)
{
        if (!cd || cd != (int*)MAGIC_NUMBER)
                errno = EBADF;
}  /* end of void _icv_close(int*). */


/****  _ I C V _ I C O N V  ****/

size_t _icv_iconv(int* cd, char** inbuf, size_t* inbufleft,
			char** outbuf, size_t* outbufleft)
{
	size_t		ret_val = 0;
	unsigned char*	ib;
	unsigned char*	ob;
	unsigned char*	ibtail;
	unsigned char*	obtail;

	if (!cd || cd != (int*)MAGIC_NUMBER)
	{
		errno = EBADF;
		return((size_t)-1);
	}

	if (!inbuf || !(*inbuf))
		return((size_t)0);

	ib = (unsigned char*)*inbuf;
	ob = (unsigned char*)*outbuf;
	ibtail = ib + *inbufleft;
	obtail = ob + *outbufleft;

	while (ib < ibtail)
	{
		if (*ib & 0x80)  /* Thai EUC doesn't have CS2 or CS3. */
		{
			hcode_type euc_code, utf_code;

			if ((ibtail - ib) < 1)
			{
				errno = EINVAL;
				ret_val = (size_t)-1;
				break;
			}

			if ((*ib < 0xA1) ||
			    (*ib > 0xFB) ||
			    ((*ib > 0xDC) && (*ib < 0xDE)))
			{
				errno = EILSEQ;
				ret_val = (size_t)-1;
				break;
			}

			euc_code.code = 0;
			euc_code.byte.byte4 = *ib;

			utf_code = _eucTH_to_utf8(euc_code);

			if (utf_code.code != 0)
			{
				if ((obtail - ob) < 3)
				{
					errno = E2BIG;
					ret_val = (size_t)-1;
					break;
				}
				/* UTF8 code from 2 bytes is always 3 bytes */
				*ob++ = (char)utf_code.byte.byte2;
				*ob++ = (char)utf_code.byte.byte3;
				*ob++ = (char)utf_code.byte.byte4;
			}
			else  /* FAILED - this means input char isn't belong to
			       *	  input codeset. */
			{
				errno = EILSEQ;
				ret_val = (size_t)-1;
				break;
			}
			ib += 1;

		}
		else  /* CS0 */
		{
			if (ob >= obtail)
			{
				errno = E2BIG;
				ret_val = (size_t)-1;
				break;
			}
			*ob++ = *ib++;
		}
	}

	*inbuf = (char*)ib;
	*inbufleft = ibtail - ib;
	*outbuf = (char*)ob;
	*outbufleft = obtail - ob;

	return(ret_val);
}  /* end of size_t _icv_iconv(int*, char**, size_t*, char**, size_t*). */
