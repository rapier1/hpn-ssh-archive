/* $OpenBSD: compat.h,v 1.44 2013/12/30 23:52:27 djm Exp $ */

/*
 * Copyright (c) 1999, 2000, 2001 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef COMPAT_H
#define COMPAT_H

#define	SSH_PROTO_UNKNOWN	0x00
#define	SSH_PROTO_1		0x01
#define	SSH_PROTO_1_PREFERRED	0x02
#define	SSH_PROTO_2		0x04

#define SSH_BUG_SIGBLOB		0x000000001
#define SSH_BUG_PKSERVICE	0x000000002
#define SSH_BUG_HMAC		0x000000004
#define SSH_BUG_X11FWD		0x000000008
#define SSH_OLD_SESSIONID	0x000000010
#define SSH_BUG_PKAUTH		0x000000020
#define SSH_BUG_DEBUG		0x000000040
#define SSH_BUG_BANNER		0x000000080
#define SSH_BUG_IGNOREMSG	0x000000100
#define SSH_BUG_PKOK		0x000000200
#define SSH_BUG_PASSWORDPAD	0x000000400
#define SSH_BUG_SCANNER		0x000000800
#define SSH_BUG_BIGENDIANAES	0x000001000
#define SSH_BUG_RSASIGMD5	0x000002000
#define SSH_OLD_DHGEX		0x000004000
#define SSH_BUG_NOREKEY		0x000008000
#define SSH_BUG_HBSERVICE	0x000010000
#define SSH_BUG_OPENFAILURE	0x000020000
#define SSH_BUG_DERIVEKEY	0x000040000
#define SSH_BUG_DUMMYCHAN	0x000100000
#define SSH_BUG_EXTEOF		0x000200000
#define SSH_BUG_PROBE		0x000400000
#define SSH_BUG_FIRSTKEX	0x000800000
#define SSH_OLD_FORWARD_ADDR	0x001000000
#define SSH_BUG_RFWD_ADDR	0x002000000
#define SSH_NEW_OPENSSH		0x004000000
#define SSH_BUG_DYNAMIC_RPORT	0x008000000
#define SSH_BUG_LARGEWINDOW     0x800000000

void     enable_compat13(void);
void     enable_compat20(void);
void     compat_datafellows(const char *);
int	 proto_spec(const char *);
char	*compat_cipher_proposal(char *);
char	*compat_pkalg_proposal(char *);

extern int compat13;
extern int compat20;
extern int datafellows;
#endif
