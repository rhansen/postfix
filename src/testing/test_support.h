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
#define PASS	1
#define FAIL	0

struct TEST_CASE {
    const char *label;
    int     (*action) (void);
};

extern int run_tests(const struct TEST_CASE *test_cases);

/* LICENSE
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	porcupine.org
/*
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

#endif
