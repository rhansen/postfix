/*++
/* NAME
/*	test_support 3
/* SUMMARY
/*	utilities to simplify writing and running tests
/* SYNOPSIS
/*	#include <test_support.h>
/*
/*	typedef enum {
/* .in +4
/*	PASS = 0,
/*	FAIL,
/*	ERROR,
/* .in -4
/*	} TEST_RESULT;
/*
/*	typedef struct TEST_CASE {
/* .in +4
/*	const char *label;
/*	TEST_RESULT (*action) (void *arg);
/*	void	*arg;
/* .in -4
/*	} TEST_CASE;
/*
/*	TEST_RESULT run_tests(
/*	const TEST_CASE *test_cases)
/* DESCRIPTION
/*	TEST_CASE.label is a name that is logged when the test case
/*	runs.  TEST_CASE.arg is optional; it is passed to
/*	TEST_CASE.action, enabling function reuse across multiple test
/*	cases.
/*
/*	run_tests() executes the given list of test cases and returns
/*	PASS if none failed (including if there are no test cases).  A
/*	failed test case does not stop the execution of the subsequent
/*	test cases, but an error result does.  The final test case to
/*	run must be followed by a zero TEST_CASE (label and action are
/*	null).  The start and end of each test case is logged, and a
/*	summary of the results is logged at the end.
/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	porcupine.org
/*
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

 /*
  * System library.
  */
#include <sys_defs.h>

 /*
  * Utility library.
  */
#include <msg.h>

 /*
  * Testing library.
  */
#include <test_support.h>

/* run_tests - execute a zero-terminated suite of test cases */

TEST_RESULT run_tests(const TEST_CASE *test_cases)
{
    int tests_passed = 0;
    int tests_failed = 0;
    int tests_errored = 0;
    const TEST_CASE *tp;

    for (tp = test_cases; tp->label; tp++) {
	msg_info("RUN  %s", tp->label);
	TEST_RESULT result = tp->action(tp->arg);
	if (result == PASS) {
	    msg_info("PASS %s", tp->label);
	    tests_passed += 1;
	} else if (result == FAIL) {
	    msg_info("FAIL %s", tp->label);
	    tests_failed += 1;
	} else {
	    msg_info("ERROR %s", tp->label);
	    tests_errored += 1;
	    break;
	}
    }

    msg_info("PASS=%d FAIL=%d ERROR=%d",
	     tests_passed, tests_failed, tests_errored);
    return (tests_errored ? ERROR
	    : tests_failed == 0 ? PASS
	    : FAIL);
}
