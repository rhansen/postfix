#ifndef _DICT_OR_H_INCLUDED_
#define _DICT_OR_H_INCLUDED_

/*++
/* NAME
/*	dict_or 3h
/* SUMMARY
/*	dictionary manager interface for "or" tables
/* SYNOPSIS
/*	#include <dict_or.h>
/* DESCRIPTION
/* .nf

 /*
  * Utility library.
  */
#include <dict.h>

 /*
  * External interface.
  */
#define DICT_TYPE_OR "or"

extern DICT *dict_or_open(const char *, int, int);

/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

#endif
