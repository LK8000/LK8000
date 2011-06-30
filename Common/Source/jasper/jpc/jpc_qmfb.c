/*
 * Copyright (c) 1999-2000 Image Power, Inc. and the University of
 *   British Columbia.
 * Copyright (c) 2001-2003 Michael David Adams.
 * All rights reserved.
 */

/* __START_OF_JASPER_LICENSE__
 * 
 * JasPer License Version 2.0
 * 
 * Copyright (c) 1999-2000 Image Power, Inc.
 * Copyright (c) 1999-2000 The University of British Columbia
 * Copyright (c) 2001-2003 Michael David Adams
 * 
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person (the
 * "User") obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 * 
 * 1.  The above copyright notices and this permission notice (which
 * includes the disclaimer below) shall be included in all copies or
 * substantial portions of the Software.
 * 
 * 2.  The name of a copyright holder shall not be used to endorse or
 * promote products derived from the Software without specific prior
 * written permission.
 * 
 * THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL PART OF THIS
 * LICENSE.  NO USE OF THE SOFTWARE IS AUTHORIZED HEREUNDER EXCEPT UNDER
 * THIS DISCLAIMER.  THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS
 * "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD PARTY RIGHTS.  IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL
 * INDIRECT OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING
 * FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION
 * WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.  NO ASSURANCES ARE
 * PROVIDED BY THE COPYRIGHT HOLDERS THAT THE SOFTWARE DOES NOT INFRINGE
 * THE PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS OF ANY OTHER ENTITY.
 * EACH COPYRIGHT HOLDER DISCLAIMS ANY LIABILITY TO THE USER FOR CLAIMS
 * BROUGHT BY ANY OTHER ENTITY BASED ON INFRINGEMENT OF INTELLECTUAL
 * PROPERTY RIGHTS OR OTHERWISE.  AS A CONDITION TO EXERCISING THE RIGHTS
 * GRANTED HEREUNDER, EACH USER HEREBY ASSUMES SOLE RESPONSIBILITY TO SECURE
 * ANY OTHER INTELLECTUAL PROPERTY RIGHTS NEEDED, IF ANY.  THE SOFTWARE
 * IS NOT FAULT-TOLERANT AND IS NOT INTENDED FOR USE IN MISSION-CRITICAL
 * SYSTEMS, SUCH AS THOSE USED IN THE OPERATION OF NUCLEAR FACILITIES,
 * AIRCRAFT NAVIGATION OR COMMUNICATION SYSTEMS, AIR TRAFFIC CONTROL
 * SYSTEMS, DIRECT LIFE SUPPORT MACHINES, OR WEAPONS SYSTEMS, IN WHICH
 * THE FAILURE OF THE SOFTWARE OR SYSTEM COULD LEAD DIRECTLY TO DEATH,
 * PERSONAL INJURY, OR SEVERE PHYSICAL OR ENVIRONMENTAL DAMAGE ("HIGH
 * RISK ACTIVITIES").  THE COPYRIGHT HOLDERS SPECIFICALLY DISCLAIM ANY
 * EXPRESS OR IMPLIED WARRANTY OF FITNESS FOR HIGH RISK ACTIVITIES.
 * 
 * __END_OF_JASPER_LICENSE__
 */

/*
 * Quadrature Mirror-Image Filter Bank (QMFB) Library
 *
 * $Id: jpc_qmfb.c,v 1.2 2007/09/14 17:11:17 jwharington Exp $
 */

/*
 * Original JasPer file modified 09/09/2005 by Kaspar Daugaard (kdaugaard@lionhead.com)
 * Improved performance by making the code process larger chunks of data
 * in a more cache efficient way. The speed-up depends on your platform
 * and compiler, but typically the modified library is several times
 * faster, while creating bit-for-bit identical files. 
 * The size of a chunk (in 4 byte elements) is defined by JPC_BATCH_SIZE.
 */
 
/*
 * Modified 12 Oct 2005 by Greg Coats (gregcoats@mac.com)
 * Reordered two statements so code would compile with gcc 4
 * Changed original JPC_BATCH_SIZE default from 8 to 128
 * Testing with Intel Pentium 4 and Apple G5 CPUs shows that 128 is a good
 * default value for JPC_BATCH_SIZE
 */

/******************************************************************************\
* Includes.
\******************************************************************************/

#include <assert.h>

#include "jasper/jas_fix.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

#include "jpc_qmfb.h"
#include "jpc_tsfb.h"
#include "jpc_math.h"

#include "utils/heapcheck.h"

/******************************************************************************\
*
\******************************************************************************/

#if ( defined(_MSC_VER) || defined(WIN32) || defined(__INTEL_COMPILER) || defined(__ICC) )
//  #pragma message("QMFB: BATCH_SIZE = 256")
  #define JPC_BATCH_SIZE 128
  //#define JPC_BATCH_SIZE 256
#else
//  #pragma message("QMFB: BATCH_SIZE = 128")
  #define JPC_BATCH_SIZE 128
#endif

static jpc_qmfb1d_t *jpc_qmfb1d_create(void);

static int jpc_ft_getnumchans(jpc_qmfb1d_t *qmfb);
static int jpc_ft_getanalfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters);
static int jpc_ft_getsynfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters);
static void jpc_ft_analyze(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x);
static void jpc_ft_synthesize(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x);

static int jpc_ns_getnumchans(jpc_qmfb1d_t *qmfb);
static int jpc_ns_getanalfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters);
static int jpc_ns_getsynfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters);
static void jpc_ns_analyze(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x);
static void jpc_ns_synthesize(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x);

/******************************************************************************\
*
\******************************************************************************/

jpc_qmfb1dops_t jpc_ft_ops = {
	jpc_ft_getnumchans,
	jpc_ft_getanalfilters,
	jpc_ft_getsynfilters,
	jpc_ft_analyze,
	jpc_ft_synthesize
};

jpc_qmfb1dops_t jpc_ns_ops = {
	jpc_ns_getnumchans,
	jpc_ns_getanalfilters,
	jpc_ns_getsynfilters,
	jpc_ns_analyze,
	jpc_ns_synthesize
};

/******************************************************************************\
*
\******************************************************************************/

static void jpc_qmfb1d_setup(jpc_fix_t *startptr, int startind, int endind,
  int intrastep, jpc_fix_t **lstartptr, int *lstartind, int *lendind,
  jpc_fix_t **hstartptr, int *hstartind, int *hendind)
{
	*lstartind = JPC_CEILDIVPOW2(startind, 1);
	*lendind = JPC_CEILDIVPOW2(endind, 1);
	*hstartind = JPC_FLOORDIVPOW2(startind, 1);
	*hendind = JPC_FLOORDIVPOW2(endind, 1);
	*lstartptr = startptr;
	*hstartptr = &startptr[(*lendind - *lstartind) * intrastep];
}

static void jpc_qmfb1d_split(jpc_fix_t *startptr, int startind, int endind,
  register int step, jpc_fix_t *lstartptr, int lstartind, int lendind,
  jpc_fix_t *hstartptr, int hstartind, int hendind)
{
	int bufsize = JPC_CEILDIVPOW2(endind - startind, 2);
#if !defined(HAVE_VLA)
#define QMFB_SPLITBUFSIZE 4096
	jpc_fix_t splitbuf[QMFB_SPLITBUFSIZE];
#else
	jpc_fix_t splitbuf[bufsize];
#endif
	jpc_fix_t *buf = splitbuf;
	int llen;
	int hlen;
	int twostep;
	jpc_fix_t *tmpptr;
	register jpc_fix_t *ptr;
	register jpc_fix_t *hptr;
	register jpc_fix_t *lptr;
	register int n;
	int state;

	twostep = step << 1;
	llen = lendind - lstartind;
	hlen = hendind - hstartind;

#if !defined(HAVE_VLA)
	/* Get a buffer. */
	if (bufsize > QMFB_SPLITBUFSIZE) {
		if (!(buf = jas_malloc(bufsize * sizeof(jpc_fix_t)))) {
			/* We have no choice but to commit suicide in this case. */
			abort();
		}
	}
#endif

	if (hstartind < lstartind) {
		/* The first sample in the input signal is to appear
		  in the highpass subband signal. */
		/* Copy the appropriate samples into the lowpass subband
		  signal, saving any samples destined for the highpass subband
		  signal as they are overwritten. */
		tmpptr = buf;
		ptr = &startptr[step];
		lptr = lstartptr;
		n = llen;
		state = 1;
		while (n-- > 0) {
			if (state) {
				*tmpptr = *lptr;
				++tmpptr;
			}
			*lptr = *ptr;
			ptr += twostep;
			lptr += step;
			state ^= 1;
		}
		/* Copy the appropriate samples into the highpass subband
		  signal. */
		/* Handle the nonoverwritten samples. */
		hptr = &hstartptr[(hlen - 1) * step];
		ptr = &startptr[(((llen + hlen - 1) >> 1) << 1) * step];
		n = hlen - (tmpptr - buf);
		while (n-- > 0) {
			*hptr = *ptr;
			hptr -= step;
			ptr -= twostep;
		}
		/* Handle the overwritten samples. */
		n = tmpptr - buf;
		while (n-- > 0) {
			--tmpptr;
			*hptr = *tmpptr;
			hptr -= step;
		}
	} else {
		/* The first sample in the input signal is to appear
		  in the lowpass subband signal. */
		/* Copy the appropriate samples into the lowpass subband
		  signal, saving any samples for the highpass subband
		  signal as they are overwritten. */
		state = 0;
		ptr = startptr;
		lptr = lstartptr;
		tmpptr = buf;
		n = llen;
		while (n-- > 0) {
			if (state) {
				*tmpptr = *lptr;
				++tmpptr;
			}
			*lptr = *ptr;
			ptr += twostep;
			lptr += step;
			state ^= 1;
		}
		/* Copy the appropriate samples into the highpass subband
		  signal. */
		/* Handle the nonoverwritten samples. */
		ptr = &startptr[((((llen + hlen) >> 1) << 1) - 1) * step];
		hptr = &hstartptr[(hlen - 1) * step];
		n = hlen - (tmpptr - buf);
		while (n-- > 0) {
			*hptr = *ptr;
			ptr -= twostep;
			hptr -= step;
		}
		/* Handle the overwritten samples. */
		n = tmpptr - buf;
		while (n-- > 0) {
			--tmpptr;
			*hptr = *tmpptr;
			hptr -= step;
		}
	}

#if !defined(HAVE_VLA)
	/* If the split buffer was allocated on the heap, free this memory. */
	if (buf != splitbuf) {
		jas_free(buf);
	}
#endif
}

#define QMFB_SPLITBUFSIZE_BATCH (8192/JPC_BATCH_SIZE)
#define	QMFB_JOINBUFSIZE_BATCH (8192/JPC_BATCH_SIZE)

typedef struct {
	jpc_fix_t vec[JPC_BATCH_SIZE];
} jpc_batch_t;

static void jpc_qmfb1d_split_batch(jpc_fix_t *startptr, int startind, int endind,
  register int step, jpc_fix_t *lstartptr, int lstartind, int lendind,
  jpc_fix_t *hstartptr, int hstartind, int hendind)
{
	int bufsize = JPC_CEILDIVPOW2(endind - startind, 2);
	jpc_batch_t splitbuf[QMFB_SPLITBUFSIZE_BATCH];
	jpc_batch_t *buf = splitbuf;
	int llen;
	int hlen;
	int twostep;
	jpc_batch_t *tmpptr;
	register jpc_fix_t *ptr;
	register jpc_fix_t *hptr;
	register jpc_fix_t *lptr;
	register int n;
	int state;

	twostep = step << 1;
	llen = lendind - lstartind;
	hlen = hendind - hstartind;

	/* Get a buffer. */
	if (bufsize > QMFB_SPLITBUFSIZE_BATCH) {
		if (!(buf = jas_malloc(bufsize * sizeof(jpc_batch_t)))) {
			/* We have no choice but to commit suicide in this case. */
			abort();
		}
	}

	if (hstartind < lstartind) {
		/* The first sample in the input signal is to appear
		  in the highpass subband signal. */
		/* Copy the appropriate samples into the lowpass subband
		  signal, saving any samples destined for the highpass subband
		  signal as they are overwritten. */
		tmpptr = buf;
		ptr = &startptr[step];
		lptr = lstartptr;
		n = llen;
		state = 1;
		while (n-- > 0) {
			if (state) {
				*tmpptr = *(jpc_batch_t*)lptr;
				++tmpptr;
			}
			*(jpc_batch_t*)lptr = *(jpc_batch_t*)ptr;
			ptr += twostep;
			lptr += step;
			state ^= 1;
		}
		/* Copy the appropriate samples into the highpass subband
		  signal. */
		/* Handle the nonoverwritten samples. */
		hptr = &hstartptr[(hlen - 1) * step];
		ptr = &startptr[(((llen + hlen - 1) >> 1) << 1) * step];
		n = hlen - (tmpptr - buf);
		while (n-- > 0) {
			*(jpc_batch_t*)hptr = *(jpc_batch_t*)ptr;
			hptr -= step;
			ptr -= twostep;
		}
		/* Handle the overwritten samples. */
		n = tmpptr - buf;
		while (n-- > 0) {
			--tmpptr;
			*(jpc_batch_t*)hptr = *tmpptr;
			hptr -= step;
		}
	} else {
		/* The first sample in the input signal is to appear
		  in the lowpass subband signal. */
		/* Copy the appropriate samples into the lowpass subband
		  signal, saving any samples for the highpass subband
		  signal as they are overwritten. */
		state = 0;
		ptr = startptr;
		lptr = lstartptr;
		tmpptr = buf;
		n = llen;
		while (n-- > 0) {
			if (state) {
				*tmpptr = *(jpc_batch_t*)lptr;
				++tmpptr;
			}
			*(jpc_batch_t*)lptr = *(jpc_batch_t*)ptr;
			ptr += twostep;
			lptr += step;
			state ^= 1;
		}
		/* Copy the appropriate samples into the highpass subband
		  signal. */
		/* Handle the nonoverwritten samples. */
		ptr = &startptr[((((llen + hlen) >> 1) << 1) - 1) * step];
		hptr = &hstartptr[(hlen - 1) * step];
		n = hlen - (tmpptr - buf);
		while (n-- > 0) {
			*(jpc_batch_t*)hptr = *(jpc_batch_t*)ptr;
			ptr -= twostep;
			hptr -= step;
		}
		/* Handle the overwritten samples. */
		n = tmpptr - buf;
		while (n-- > 0) {
			--tmpptr;
			*(jpc_batch_t*)hptr = *tmpptr;
			hptr -= step;
		}
	}

	/* If the split buffer was allocated on the heap, free this memory. */
	if (buf != splitbuf) {
		jas_free(buf);
	}
}

static void jpc_qmfb1d_join(jpc_fix_t *startptr, int startind, int endind,
  register int step, jpc_fix_t *lstartptr, int lstartind, int lendind,
  jpc_fix_t *hstartptr, int hstartind, int hendind)
{
	int bufsize = JPC_CEILDIVPOW2(endind - startind, 2);
#define	QMFB_JOINBUFSIZE	4096
#if !defined(HAVE_VLA)
	jpc_fix_t joinbuf[QMFB_JOINBUFSIZE];
#else
	jpc_fix_t joinbuf[bufsize];
#endif
	jpc_fix_t *buf = joinbuf;
	int llen;
	int hlen;
	int twostep;
	jpc_fix_t *tmpptr;
	register jpc_fix_t *ptr;
	register jpc_fix_t *hptr;
	register jpc_fix_t *lptr;
	register int n;
	int state;

#if !defined(HAVE_VLA)
	/* Allocate memory for the join buffer from the heap. */
	if (bufsize > QMFB_JOINBUFSIZE) {
		if (!(buf = jas_malloc(bufsize * sizeof(jpc_fix_t)))) {
			/* We have no choice but to commit suicide. */
			abort();
		}
	}
#endif

	twostep = step << 1;
	llen = lendind - lstartind;
	hlen = hendind - hstartind;

	if (hstartind < lstartind) {
		/* The first sample in the highpass subband signal is to
		  appear first in the output signal. */
		/* Copy the appropriate samples into the first phase of the
		  output signal. */
		tmpptr = buf;
		hptr = hstartptr;
		ptr = startptr;
		n = (llen + 1) >> 1;
		while (n-- > 0) {
			*tmpptr = *ptr;
			*ptr = *hptr;
			++tmpptr;
			ptr += twostep;
			hptr += step;
		}
		n = hlen - ((llen + 1) >> 1);
		while (n-- > 0) {
			*ptr = *hptr;
			ptr += twostep;
			hptr += step;
		}
		/* Copy the appropriate samples into the second phase of
		  the output signal. */
		ptr -= (lendind > hendind) ? (step) : (step + twostep);
		state = !((llen - 1) & 1);
		lptr = &lstartptr[(llen - 1) * step];
		n = llen;
		while (n-- > 0) {
			if (state) {
				--tmpptr;
				*ptr = *tmpptr;
			} else {
				*ptr = *lptr;
			}
			lptr -= step;
			ptr -= twostep;
			state ^= 1;
		}
	} else {
		/* The first sample in the lowpass subband signal is to
		  appear first in the output signal. */
		/* Copy the appropriate samples into the first phase of the
		  output signal (corresponding to even indexed samples). */
		lptr = &lstartptr[(llen - 1) * step];
		ptr = &startptr[((llen - 1) << 1) * step];
		n = llen >> 1;
		tmpptr = buf;
		while (n-- > 0) {
			*tmpptr = *ptr;
			*ptr = *lptr;
			++tmpptr;
			ptr -= twostep;
			lptr -= step;
		}
		n = llen - (llen >> 1);
		while (n-- > 0) {
			*ptr = *lptr;
			ptr -= twostep;
			lptr -= step;
		}
		/* Copy the appropriate samples into the second phase of
		  the output signal (corresponding to odd indexed
		  samples). */
		ptr = &startptr[step];
		hptr = hstartptr;
		state = !(llen & 1);
		n = hlen;
		while (n-- > 0) {
			if (state) {
				--tmpptr;
				*ptr = *tmpptr;
			} else {
				*ptr = *hptr;
			}
			hptr += step;
			ptr += twostep;
			state ^= 1;
		}
	}

#if !defined(HAVE_VLA)
	/* If the join buffer was allocated on the heap, free this memory. */
	if (buf != joinbuf) {
		jas_free(buf);
	}
#endif
}

static void jpc_qmfb1d_join_batch(jpc_fix_t *startptr, int startind, int endind,
  register int step, jpc_fix_t *lstartptr, int lstartind, int lendind,
  jpc_fix_t *hstartptr, int hstartind, int hendind)
{
	int bufsize = JPC_CEILDIVPOW2(endind - startind, 2);
	jpc_batch_t joinbuf[QMFB_JOINBUFSIZE_BATCH];
	jpc_batch_t *buf = joinbuf;
	int llen;
	int hlen;
	int twostep;
	jpc_batch_t *tmpptr;
	register jpc_fix_t *ptr;
	register jpc_fix_t *hptr;
	register jpc_fix_t *lptr;
	register int n;
	int state;

	/* Allocate memory for the join buffer from the heap. */
if (bufsize > QMFB_JOINBUFSIZE_BATCH) {
		if (!(buf = jas_malloc(bufsize * sizeof(jpc_batch_t)))) {
			/* We have no choice but to commit suicide. */
			abort();
		}
	}

	twostep = step << 1;
	llen = lendind - lstartind;
	hlen = hendind - hstartind;

	if (hstartind < lstartind) {
		/* The first sample in the highpass subband signal is to
		  appear first in the output signal. */
		/* Copy the appropriate samples into the first phase of the
		  output signal. */
		tmpptr = buf;
		hptr = hstartptr;
		ptr = startptr;
		n = (llen + 1) >> 1;
		while (n-- > 0) {
			*tmpptr = *(jpc_batch_t*)ptr;
			*(jpc_batch_t*)ptr = *(jpc_batch_t*)hptr;
			++tmpptr;
			ptr += twostep;
			hptr += step;
		}
		n = hlen - ((llen + 1) >> 1);
		while (n-- > 0) {
			*(jpc_batch_t*)ptr = *(jpc_batch_t*)hptr;
			ptr += twostep;
			hptr += step;
		}
		/* Copy the appropriate samples into the second phase of
		  the output signal. */
		ptr -= (lendind > hendind) ? (step) : (step + twostep);
		state = !((llen - 1) & 1);
		lptr = &lstartptr[(llen - 1) * step];
		n = llen;
		while (n-- > 0) {
			if (state) {
				--tmpptr;
				*(jpc_batch_t*)ptr = *tmpptr;
			} else {
				*(jpc_batch_t*)ptr = *(jpc_batch_t*)lptr;
			}
			lptr -= step;
			ptr -= twostep;
			state ^= 1;
		}
	} else {
		/* The first sample in the lowpass subband signal is to
		  appear first in the output signal. */
		/* Copy the appropriate samples into the first phase of the
		  output signal (corresponding to even indexed samples). */
		lptr = &lstartptr[(llen - 1) * step];
		ptr = &startptr[((llen - 1) << 1) * step];
		n = llen >> 1;
		tmpptr = buf;
		while (n-- > 0) {
			*tmpptr = *(jpc_batch_t*)ptr;
			*(jpc_batch_t*)ptr = *(jpc_batch_t*)lptr;
			++tmpptr;
			ptr -= twostep;
			lptr -= step;
		}
		n = llen - (llen >> 1);
		while (n-- > 0) {
		  if (ptr != lptr) {
		    *(jpc_batch_t*)ptr = *(jpc_batch_t*)lptr;
		  }
		  ptr -= twostep;
		  lptr -= step;
		}
		/* Copy the appropriate samples into the second phase of
		  the output signal (corresponding to odd indexed
		  samples). */
		ptr = &startptr[step];
		hptr = hstartptr;
		state = !(llen & 1);
		n = hlen;
		while (n-- > 0) {
			if (state) {
				--tmpptr;
				*(jpc_batch_t*)ptr = *tmpptr;
			} else {
			  if (ptr != hptr) {
			    *(jpc_batch_t*)ptr = *(jpc_batch_t*)hptr;
			  }
			}
			hptr += step;
			ptr += twostep;
			state ^= 1;
		}
	}

	/* If the join buffer was allocated on the heap, free this memory. */
	if (buf != joinbuf) {
		jas_free(buf);
	}
}

/******************************************************************************\
* Code for 5/3 transform.
\******************************************************************************/

static int jpc_ft_getnumchans(jpc_qmfb1d_t *qmfb)
{
	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	return 2;
}

static int jpc_ft_getanalfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;
	len = 0;
	filters = 0;
	abort();
	return -1;
}

static int jpc_ft_getsynfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	jas_seq_t *lf;
	jas_seq_t *hf;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	lf = 0;
	hf = 0;

	if (len > 1 || (!len)) {
		if (!(lf = jas_seq_create(-1, 2))) {
			goto error;
		}
		jas_seq_set(lf, -1, jpc_dbltofix(0.5));
		jas_seq_set(lf, 0, jpc_dbltofix(1.0));
		jas_seq_set(lf, 1, jpc_dbltofix(0.5));
		if (!(hf = jas_seq_create(-1, 4))) {
			goto error;
		}
		jas_seq_set(hf, -1, jpc_dbltofix(-0.125));
		jas_seq_set(hf, 0, jpc_dbltofix(-0.25));
		jas_seq_set(hf, 1, jpc_dbltofix(0.75));
		jas_seq_set(hf, 2, jpc_dbltofix(-0.25));
		jas_seq_set(hf, 3, jpc_dbltofix(-0.125));
	} else if (len == 1) {
		if (!(lf = jas_seq_create(0, 1))) {
			goto error;
		}
		jas_seq_set(lf, 0, jpc_dbltofix(1.0));
		if (!(hf = jas_seq_create(0, 1))) {
			goto error;
		}
		jas_seq_set(hf, 0, jpc_dbltofix(2.0));
	} else {
		abort();
	}

	filters[0] = lf;
	filters[1] = hf;

	return 0;

error:
	if (lf) {
		jas_seq_destroy(lf);
	}
	if (hf) {
		jas_seq_destroy(hf);
	}
	return -1;
}

#define	NFT_LIFT0(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pluseq) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (hendind) - (hstartind); \
	if ((hstartind) < (lstartind)) { \
		pluseq(*hptr, *lptr); \
		hptr += (step); \
		--n; \
	} \
	if ((hendind) >= (lendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		pluseq(*hptr, jpc_fix_asr(jpc_fix_add(*lptr, lptr[(step)]), 1)); \
		hptr += (step); \
		lptr += (step); \
	} \
	if ((hendind) >= (lendind)) { \
		pluseq(*hptr, *lptr); \
	} \
}

#define	NFT_LIFT1(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pluseq) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (lendind) - (lstartind); \
	if ((hstartind) >= (lstartind)) { \
		pluseq(*lptr, *hptr); \
		lptr += (step); \
		--n; \
	} \
	if ((lendind) > (hendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		pluseq(*lptr, jpc_fix_asr(jpc_fix_add(*hptr, hptr[(step)]), 2)); \
		lptr += (step); \
		hptr += (step); \
	} \
	if ((lendind) > (hendind)) { \
		pluseq(*lptr, *hptr); \
	} \
}

#define	NFT_LIFT0_BATCH(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pluseq) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (hendind) - (hstartind); \
	register int v; \
	if ((hstartind) < (lstartind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(hptr[v], lptr[v]); \
		} \
		hptr += (step); \
		--n; \
	} \
	if ((hendind) >= (lendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(hptr[v], jpc_fix_asr(jpc_fix_add(lptr[v], lptr[(step)+v]), 1)); \
		} \
		hptr += (step); \
		lptr += (step); \
	} \
	if ((hendind) >= (lendind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(hptr[v], lptr[v]); \
		} \
	} \
}

#define	NFT_LIFT1_BATCH(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pluseq) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (lendind) - (lstartind); \
	register int v; \
	if ((hstartind) >= (lstartind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(lptr[v], hptr[v]); \
		} \
		lptr += (step); \
		--n; \
	} \
	if ((lendind) > (hendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(lptr[v], jpc_fix_asr(jpc_fix_add(hptr[v], hptr[(step)+v]), 2)); \
		} \
		lptr += (step); \
		hptr += (step); \
	} \
	if ((lendind) > (hendind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			pluseq(lptr[v], hptr[v]); \
		} \
	} \
}

#define	RFT_LIFT0(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pmeqop) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (hendind) - (hstartind); \
	if ((hstartind) < (lstartind)) { \
		*hptr pmeqop *lptr; \
		hptr += (step); \
		--n; \
	} \
	if ((hendind) >= (lendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		*hptr pmeqop (*lptr + lptr[(step)]) >> 1; \
		hptr += (step); \
		lptr += (step); \
	} \
	if ((hendind) >= (lendind)) { \
		*hptr pmeqop *lptr; \
	} \
}

#define	RFT_LIFT1(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pmeqop) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (lendind) - (lstartind); \
	if ((hstartind) >= (lstartind)) { \
		*lptr pmeqop ((*hptr << 1) + 2) >> 2; \
		lptr += (step); \
		--n; \
	} \
	if ((lendind) > (hendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		*lptr pmeqop ((*hptr + hptr[(step)]) + 2) >> 2; \
		lptr += (step); \
		hptr += (step); \
	} \
	if ((lendind) > (hendind)) { \
		*lptr pmeqop ((*hptr << 1) + 2) >> 2; \
	} \
}

#define	RFT_LIFT0_BATCH(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pmeqop) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (hendind) - (hstartind); \
	register int v; \
	if ((hstartind) < (lstartind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			hptr[v] pmeqop lptr[v]; \
		} \
		hptr += (step); \
		--n; \
	} \
	if ((hendind) >= (lendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			hptr[v] pmeqop (lptr[v] + lptr[(step)+v]) >> 1; \
		} \
		hptr += (step); \
		lptr += (step); \
	} \
	if ((hendind) >= (lendind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			hptr[v] pmeqop lptr[v]; \
		} \
	} \
}

#define	RFT_LIFT1_BATCH(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, pmeqop) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (lendind) - (lstartind); \
	register int v; \
	if ((hstartind) >= (lstartind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			lptr[v] pmeqop ((hptr[v] << 1) + 2) >> 2; \
		} \
		lptr += (step); \
		--n; \
	} \
	if ((lendind) > (hendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			lptr[v] pmeqop ((hptr[v] + hptr[(step)+v]) + 2) >> 2; \
		} \
		lptr += (step); \
		hptr += (step); \
	} \
	if ((lendind) > (hendind)) { \
		for (v=0; v<JPC_BATCH_SIZE; v++) { \
			lptr[v] pmeqop ((hptr[v] << 1) + 2) >> 2; \
		} \
	} \
}

#define JPC_BATCH_OP(ptr, op) \
{ \
	int v; \
	for (v=0; v<JPC_BATCH_SIZE; v++) { \
		(ptr)[v] op; \
	} \
}

#define JPC_BATCH_FIX_AS1(ptr) \
{ \
	int v; \
	for (v=0; v<JPC_BATCH_SIZE; v++) { \
		(ptr)[v] = jpc_fix_asl((ptr)[v], 1); \
	} \
}

#define JPC_BATCH_FIX_ASR(ptr, n) \
{ \
	int v; \
	for (v=0; v<JPC_BATCH_SIZE; v++) { \
		(ptr)[v] = jpc_fix_asr((ptr)[v], (n)); \
	} \
}

static void jpc_ft_analyze(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	jpc_fix_t *startptr;
	int startind;
	int endind;
	jpc_fix_t *  lstartptr;
	int   lstartind;
	int   lendind;
	jpc_fix_t *  hstartptr;
	int   hstartind;
	int   hendind;
	int interstep;
	int intrastep;
	int numseq;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	if (flags & JPC_QMFB1D_VERT) {
		interstep = 1;
		intrastep = jas_seq2d_rowstep(x);
		numseq = jas_seq2d_width(x);
		startind = jas_seq2d_ystart(x);
		endind = jas_seq2d_yend(x);
	} else {
		interstep = jas_seq2d_rowstep(x);
		intrastep = 1;
		numseq = jas_seq2d_height(x);
		startind = jas_seq2d_xstart(x);
		endind = jas_seq2d_xend(x);
	}

	assert(startind < endind);

	startptr = jas_seq2d_getref(x, jas_seq2d_xstart(x), jas_seq2d_ystart(x));
	if (flags & JPC_QMFB1D_RITIMODE) {
		if (interstep==1) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			while (numseq >= JPC_BATCH_SIZE) {
				if (endind - startind > 1) {
					jpc_qmfb1d_split_batch(startptr, startind, endind,
					  intrastep, lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind);
					RFT_LIFT0_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep, -=);
					RFT_LIFT1_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep, +=);
				} else {
					if (lstartind == lendind) {
						JPC_BATCH_OP(startptr, <<= 1);
					}
				}
				startptr += JPC_BATCH_SIZE;
				lstartptr += JPC_BATCH_SIZE;
				hstartptr += JPC_BATCH_SIZE;
				numseq -= JPC_BATCH_SIZE;
			}
		}
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				jpc_qmfb1d_split(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
				RFT_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep, -=);
				RFT_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep, +=);
			} else {
				if (lstartind == lendind) {
					*startptr <<= 1;
				}
			}
			startptr += interstep;
		}
	} else {
		if (interstep==1) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			while (numseq >= JPC_BATCH_SIZE) {
				if (endind - startind > 1) {
					jpc_qmfb1d_split_batch(startptr, startind, endind,
					  intrastep, lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind);
					NFT_LIFT0_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep,
					  jpc_fix_minuseq);
					NFT_LIFT1_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep,
					  jpc_fix_pluseq);
				} else {
					if (lstartind == lendind) {
						JPC_BATCH_FIX_AS1(startptr);
					}
				}
				startptr += JPC_BATCH_SIZE;
				lstartptr += JPC_BATCH_SIZE;
				hstartptr += JPC_BATCH_SIZE;
				numseq -= JPC_BATCH_SIZE;
			}
		}
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				jpc_qmfb1d_split(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
				NFT_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_fix_minuseq);
				NFT_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_fix_pluseq);
			} else {
				if (lstartind == lendind) {
					*startptr = jpc_fix_asl(*startptr, 1);
				}
			}
			startptr += interstep;
		}
	}
}

static void jpc_ft_synthesize(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	jpc_fix_t *startptr;
	int startind;
	int endind;
	jpc_fix_t *lstartptr;
	int lstartind;
	int lendind;
	jpc_fix_t *hstartptr;
	int hstartind;
	int hendind;
	int interstep;
	int intrastep;
	int numseq;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	if (flags & JPC_QMFB1D_VERT) {
		interstep = 1;
		intrastep = jas_seq2d_rowstep(x);
		numseq = jas_seq2d_width(x);
		startind = jas_seq2d_ystart(x);
		endind = jas_seq2d_yend(x);
	} else {
		interstep = jas_seq2d_rowstep(x);
		intrastep = 1;
		numseq = jas_seq2d_height(x);
		startind = jas_seq2d_xstart(x);
		endind = jas_seq2d_xend(x);
	}

	assert(startind < endind);

	startptr = jas_seq2d_getref(x, jas_seq2d_xstart(x), jas_seq2d_ystart(x));
	if (flags & JPC_QMFB1D_RITIMODE) {
		if (interstep==1) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			while (numseq >= JPC_BATCH_SIZE) {
				if (endind - startind > 1) {
					RFT_LIFT1_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep, -=);
					RFT_LIFT0_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep, +=);
					jpc_qmfb1d_join_batch(startptr, startind, endind,
					  intrastep, lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind);
				} else {
					if (lstartind == lendind) {
						JPC_BATCH_OP(startptr, >>= 1);
					}
				}
				startptr += JPC_BATCH_SIZE;
				lstartptr += JPC_BATCH_SIZE;
				hstartptr += JPC_BATCH_SIZE;
				numseq -= JPC_BATCH_SIZE;
			}
		}
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				RFT_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep, -=);
				RFT_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep, +=);
				jpc_qmfb1d_join(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
			} else {
				if (lstartind == lendind) {
					*startptr >>= 1;
				}
			}
			startptr += interstep;
		}
	} else {
		if (interstep==1) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			while (numseq >= JPC_BATCH_SIZE) {
				if (endind - startind > 1) {
					NFT_LIFT1_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep,
					  jpc_fix_minuseq);
					NFT_LIFT0_BATCH(lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind, intrastep,
					  jpc_fix_pluseq);
					jpc_qmfb1d_join_batch(startptr, startind, endind,
					  intrastep, lstartptr, lstartind, lendind,
					  hstartptr, hstartind, hendind);
				} else {
					if (lstartind == lendind) {
						JPC_BATCH_FIX_ASR(startptr, 1);
					}
				}
				startptr += JPC_BATCH_SIZE;
				lstartptr += JPC_BATCH_SIZE;
				hstartptr += JPC_BATCH_SIZE;
				numseq -= JPC_BATCH_SIZE;
			}
		}
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				NFT_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_fix_minuseq);
				NFT_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_fix_pluseq);
				jpc_qmfb1d_join(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
			} else {
				if (lstartind == lendind) {
					*startptr = jpc_fix_asr(*startptr, 1);
				}
			}
			startptr += interstep;
		}
	}
}

/******************************************************************************\
* Code for 9/7 transform.
\******************************************************************************/

static int jpc_ns_getnumchans(jpc_qmfb1d_t *qmfb)
{
	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	return 2;
}

static int jpc_ns_getanalfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;
	len = 0;
	filters = 0;

	abort();
	return -1;
}

static int jpc_ns_getsynfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	jas_seq_t *lf;
	jas_seq_t *hf;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	lf = 0;
	hf = 0;

	if (len > 1 || (!len)) {
		if (!(lf = jas_seq_create(-3, 4))) {
			goto error;
		}
		jas_seq_set(lf, -3, jpc_dbltofix(-0.09127176311424948));
		jas_seq_set(lf, -2, jpc_dbltofix(-0.05754352622849957));
		jas_seq_set(lf, -1, jpc_dbltofix(0.5912717631142470));
		jas_seq_set(lf, 0, jpc_dbltofix(1.115087052456994));
		jas_seq_set(lf, 1, jpc_dbltofix(0.5912717631142470));
		jas_seq_set(lf, 2, jpc_dbltofix(-0.05754352622849957));
		jas_seq_set(lf, 3, jpc_dbltofix(-0.09127176311424948));
		if (!(hf = jas_seq_create(-3, 6))) {
			goto error;
		}
		jas_seq_set(hf, -3, jpc_dbltofix(-0.02674875741080976 * 2.0));
		jas_seq_set(hf, -2, jpc_dbltofix(-0.01686411844287495 * 2.0));
		jas_seq_set(hf, -1, jpc_dbltofix(0.07822326652898785 * 2.0));
		jas_seq_set(hf, 0, jpc_dbltofix(0.2668641184428723 * 2.0));
		jas_seq_set(hf, 1, jpc_dbltofix(-0.6029490182363579 * 2.0));
		jas_seq_set(hf, 2, jpc_dbltofix(0.2668641184428723 * 2.0));
		jas_seq_set(hf, 3, jpc_dbltofix(0.07822326652898785 * 2.0));
		jas_seq_set(hf, 4, jpc_dbltofix(-0.01686411844287495 * 2.0));
		jas_seq_set(hf, 5, jpc_dbltofix(-0.02674875741080976 * 2.0));
	} else if (len == 1) {
		if (!(lf = jas_seq_create(0, 1))) {
			goto error;
		}
		jas_seq_set(lf, 0, jpc_dbltofix(1.0));
		if (!(hf = jas_seq_create(0, 1))) {
			goto error;
		}
		jas_seq_set(hf, 0, jpc_dbltofix(2.0));
	} else {
		abort();
	}

	filters[0] = lf;
	filters[1] = hf;

	return 0;

error:
	if (lf) {
		jas_seq_destroy(lf);
	}
	if (hf) {
		jas_seq_destroy(hf);
	}
	return -1;
}

#define	NNS_LIFT0(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, alpha) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (hendind) - (hstartind); \
	jpc_fix_t twoalpha = jpc_fix_mulbyint(alpha, 2); \
	if ((hstartind) < (lstartind)) { \
		jpc_fix_pluseq(*hptr, jpc_fix_mul(*lptr, (twoalpha))); \
		hptr += (step); \
		--n; \
	} \
	if ((hendind) >= (lendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		jpc_fix_pluseq(*hptr, jpc_fix_mul(jpc_fix_add(*lptr, lptr[(step)]), (alpha))); \
		hptr += (step); \
		lptr += (step); \
	} \
	if ((hendind) >= (lendind)) { \
		jpc_fix_pluseq(*hptr, jpc_fix_mul(*lptr, (twoalpha))); \
	} \
}

#define	NNS_LIFT1(lstartptr, lstartind, lendind, hstartptr, hstartind, hendind, step, alpha) \
{ \
	register jpc_fix_t *lptr = (lstartptr); \
	register jpc_fix_t *hptr = (hstartptr); \
	register int n = (lendind) - (lstartind); \
	int twoalpha = jpc_fix_mulbyint(alpha, 2); \
	if ((hstartind) >= (lstartind)) { \
		jpc_fix_pluseq(*lptr, jpc_fix_mul(*hptr, (twoalpha))); \
		lptr += (step); \
		--n; \
	} \
	if ((lendind) > (hendind)) { \
		--n; \
	} \
	while (n-- > 0) { \
		jpc_fix_pluseq(*lptr, jpc_fix_mul(jpc_fix_add(*hptr, hptr[(step)]), (alpha))); \
		lptr += (step); \
		hptr += (step); \
	} \
	if ((lendind) > (hendind)) { \
		jpc_fix_pluseq(*lptr, jpc_fix_mul(*hptr, (twoalpha))); \
	} \
}

#define	NNS_SCALE(startptr, startind, endind, step, alpha) \
{ \
	register jpc_fix_t *ptr = (startptr); \
	register int n = (endind) - (startind); \
	while (n-- > 0) { \
		jpc_fix_muleq(*ptr, alpha); \
		ptr += (step); \
	} \
}

static void jpc_ns_analyze(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	jpc_fix_t *startptr;
	int startind;
	int endind;
	jpc_fix_t *lstartptr;
	int lstartind;
	int lendind;
	jpc_fix_t *hstartptr;
	int hstartind;
	int hendind;
	int interstep;
	int intrastep;
	int numseq;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	if (flags & JPC_QMFB1D_VERT) {
		interstep = 1;
		intrastep = jas_seq2d_rowstep(x);
		numseq = jas_seq2d_width(x);
		startind = jas_seq2d_ystart(x);
		endind = jas_seq2d_yend(x);
	} else {
		interstep = jas_seq2d_rowstep(x);
		intrastep = 1;
		numseq = jas_seq2d_height(x);
		startind = jas_seq2d_xstart(x);
		endind = jas_seq2d_xend(x);
	}

	assert(startind < endind);

	startptr = jas_seq2d_getref(x, jas_seq2d_xstart(x), jas_seq2d_ystart(x));
	if (!(flags & JPC_QMFB1D_RITIMODE)) {
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				jpc_qmfb1d_split(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
				NNS_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(-1.586134342));
				NNS_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(-0.052980118));
				NNS_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(0.882911075));
				NNS_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(0.443506852));
				NNS_SCALE(lstartptr, lstartind, lendind,
				  intrastep, jpc_dbltofix(1.0/1.23017410558578));
				NNS_SCALE(hstartptr, hstartind, hendind,
				  intrastep, jpc_dbltofix(1.0/1.62578613134411));
			} else {
#if 0
				if (lstartind == lendind) {
					*startptr = jpc_fix_asl(*startptr, 1);
				}
#endif
			}
			startptr += interstep;
		}
	} else {
		/* The reversible integer-to-integer mode is not supported
		  for this transform. */
		abort();
	}
}

static void jpc_ns_synthesize(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	jpc_fix_t *startptr;
	int startind;
	int endind;
	jpc_fix_t *lstartptr;
	int lstartind;
	int lendind;
	jpc_fix_t *hstartptr;
	int hstartind;
	int hendind;
	int interstep;
	int intrastep;
	int numseq;

	/* Avoid compiler warnings about unused parameters. */
	qmfb = 0;

	if (flags & JPC_QMFB1D_VERT) {
		interstep = 1;
		intrastep = jas_seq2d_rowstep(x);
		numseq = jas_seq2d_width(x);
		startind = jas_seq2d_ystart(x);
		endind = jas_seq2d_yend(x);
	} else {
		interstep = jas_seq2d_rowstep(x);
		intrastep = 1;
		numseq = jas_seq2d_height(x);
		startind = jas_seq2d_xstart(x);
		endind = jas_seq2d_xend(x);
	}

	assert(startind < endind);

	startptr = jas_seq2d_getref(x, jas_seq2d_xstart(x), jas_seq2d_ystart(x));
	if (!(flags & JPC_QMFB1D_RITIMODE)) {
		while (numseq-- > 0) {
			jpc_qmfb1d_setup(startptr, startind, endind, intrastep,
			  &lstartptr, &lstartind, &lendind, &hstartptr,
			  &hstartind, &hendind);
			if (endind - startind > 1) {
				NNS_SCALE(lstartptr, lstartind, lendind,
				  intrastep, jpc_dbltofix(1.23017410558578));
				NNS_SCALE(hstartptr, hstartind, hendind,
				  intrastep, jpc_dbltofix(1.62578613134411));
				NNS_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(-0.443506852));
				NNS_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(-0.882911075));
				NNS_LIFT1(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(0.052980118));
				NNS_LIFT0(lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind, intrastep,
				  jpc_dbltofix(1.586134342));
				jpc_qmfb1d_join(startptr, startind, endind,
				  intrastep, lstartptr, lstartind, lendind,
				  hstartptr, hstartind, hendind);
			} else {
#if 0
				if (lstartind == lendind) {
					*startptr = jpc_fix_asr(*startptr, 1);
				}
#endif
			}
			startptr += interstep;
		}
	} else {
		/* The reversible integer-to-integer mode is not supported
		  for this transform. */
		abort();
	}
}

/******************************************************************************\
*
\******************************************************************************/

jpc_qmfb1d_t *jpc_qmfb1d_make(int qmfbid)
{
	jpc_qmfb1d_t *qmfb;
	if (!(qmfb = jpc_qmfb1d_create())) {
		return 0;
	}
	switch (qmfbid) {
	case JPC_QMFB1D_FT:
		qmfb->ops = &jpc_ft_ops;
		break;
	case JPC_QMFB1D_NS:
		qmfb->ops = &jpc_ns_ops;
		break;
	default:
		jpc_qmfb1d_destroy(qmfb);
		return 0;
		break;
	}
	return qmfb;
}

static jpc_qmfb1d_t *jpc_qmfb1d_create()
{
	jpc_qmfb1d_t *qmfb;
	if (!(qmfb = jas_malloc(sizeof(jpc_qmfb1d_t)))) {
		return 0;
	}
	qmfb->ops = 0;
	return qmfb;
}

jpc_qmfb1d_t *jpc_qmfb1d_copy(jpc_qmfb1d_t *qmfb)
{
	jpc_qmfb1d_t *newqmfb;

	if (!(newqmfb = jpc_qmfb1d_create())) {
		return 0;
	}
	newqmfb->ops = qmfb->ops;
	return newqmfb;
}

void jpc_qmfb1d_destroy(jpc_qmfb1d_t *qmfb)
{
	jas_free(qmfb);
}

/******************************************************************************\
*
\******************************************************************************/

void jpc_qmfb1d_getbands(jpc_qmfb1d_t *qmfb, int flags, uint_fast32_t xstart,
  uint_fast32_t ystart, uint_fast32_t xend, uint_fast32_t yend, int maxbands,
  int *numbandsptr, jpc_qmfb1dband_t *bands)
{
	int start;
	int end;

	assert(maxbands >= 2);

	if (flags & JPC_QMFB1D_VERT) {
		start = ystart;
		end = yend;
	} else {
		start = xstart;
		end = xend;
	}
	assert(jpc_qmfb1d_getnumchans(qmfb) == 2);
	assert(start <= end);
	bands[0].start = JPC_CEILDIVPOW2(start, 1);
	bands[0].end = JPC_CEILDIVPOW2(end, 1);
	bands[0].locstart = start;
	bands[0].locend = start + bands[0].end - bands[0].start;
	bands[1].start = JPC_FLOORDIVPOW2(start, 1);
	bands[1].end = JPC_FLOORDIVPOW2(end, 1);
	bands[1].locstart = bands[0].locend;
	bands[1].locend = bands[1].locstart + bands[1].end - bands[1].start;
	assert(bands[1].locend == end);
	*numbandsptr = 2;
}

/******************************************************************************\
*
\******************************************************************************/

int jpc_qmfb1d_getnumchans(jpc_qmfb1d_t *qmfb)
{
	return (*qmfb->ops->getnumchans)(qmfb);
}

int jpc_qmfb1d_getanalfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	return (*qmfb->ops->getanalfilters)(qmfb, len, filters);
}

int jpc_qmfb1d_getsynfilters(jpc_qmfb1d_t *qmfb, int len, jas_seq2d_t **filters)
{
	return (*qmfb->ops->getsynfilters)(qmfb, len, filters);
}

void jpc_qmfb1d_analyze(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	(*qmfb->ops->analyze)(qmfb, flags, x);
}

void jpc_qmfb1d_synthesize(jpc_qmfb1d_t *qmfb, int flags, jas_seq2d_t *x)
{
	(*qmfb->ops->synthesize)(qmfb, flags, x);
}
