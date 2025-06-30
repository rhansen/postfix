#ifndef TEST_SUPPORT_H_INCLUDED
#define TEST_SUPPORT_H_INCLUDED

/*++
/* NAME
/*	test_support 3h
/* SUMMARY
/*	utilities to simplify writing and running tests
/* SYNOPSIS
/*	#include <test_support.h>
/* DESCRIPTION
/* .nf

 /*
  * External interface.
  */
typedef enum {
    FAIL,
    PASS,
} TEST_RESULT;

typedef struct TEST_CASE {
    const char *label;
    TEST_RESULT (*action) (void);
} TEST_CASE;

extern TEST_RESULT run_tests(const TEST_CASE *test_cases);

/* LICENSE
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	porcupine.org
/*
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

#endif
