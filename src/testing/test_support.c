/*++
/* NAME
/*	test_support 3
/* SUMMARY
/*	utilities to simplify writing and running tests
/* SYNOPSIS
/*	#include <test_support.h>
/*
/*	#define PASS	1
/*	#define FAIL	0
/*
/*	typedef struct TEST_CASE {
/* .in +4
/*	const char *label;
/*	int	(*action) (void);
/* .in -4
/*	} TEST_CASE;
/*
/*	int	run_tests(
/*	const TEST_CASE *test_cases)
/* DESCRIPTION
/*	.B TEST_CASE.label
/*	is a name that is logged when the test case runs.
/*	.B TEST_CASE.action
/*	returns \fBPASS\fR or \fBFAIL\fR.
/*
/*	.BR run_tests ()
/*	executes the given list of test cases and returns:
/*	.RS 4
/*	.IP \fBPASS\fR 6
/*	if no test cases returned \fBFAIL\fR (including if there are
/*	no test cases).
/*	.IP \fBFAIL\fR
/*	if any test case returned \fBFAIL\fR.
/*	.RE
/*
/*	A test case that returns \fBFAIL\fR does not prevent the
/*	execution of the remaining test cases.
/*	The final test case to run must be followed by a zero
/*	\fBTEST_CASE\fR (\fB.label\fR and \fB.action\fR are null).
/*	The start and end of each test case is logged, and a summary
/*	of the results is logged at the end.
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

int     run_tests(const TEST_CASE *test_cases)
{
    int tests_passed = 0;
    int tests_failed = 0;
    const TEST_CASE *tp;

    for (tp = test_cases; tp->label; tp++) {
	msg_info("RUN  %s", tp->label);
	if (tp->action() == PASS) {
	    msg_info("PASS %s", tp->label);
	    tests_passed += 1;
	} else {
	    msg_info("FAIL %s", tp->label);
	    tests_failed += 1;
	}
    }

    msg_info("PASS=%d FAIL=%d", tests_passed, tests_failed);
    return (tests_failed == 0 ? PASS : FAIL);
}
