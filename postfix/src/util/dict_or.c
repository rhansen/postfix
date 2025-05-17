/*++
/* NAME
/*	dict_or 3
/* SUMMARY
/*	dictionary manager interface for "or" tables
/* SYNOPSIS
/*	#include <dict_or.h>
/*
/*	DICT	*dict_or_open(name, open_flags, dict_flags)
/*	const char *name;
/*	int	open_flags;
/*	int	dict_flags;
/* DESCRIPTION
/*	dict_or_open() opens an "or" map wrapping zero or more underlying
/*	tables.  Example: "\fBor:{\fItype_1:name_1, ..., type_n:name_n\fR}".
/*
/*	A query to an "or:" table is passed to the first underlying table.  If
/*	no entry is found there, the query is passed to the second underlying
/*	table.  This continues until an entry is found, an underlying table
/*	returns an error, or there are no more underlying tables to query.
/*
/*	The first and last characters of the "or:" table name must be '{' and
/*	'}'.  Within these, individual maps are separated with comma or
/*	whitespace.
/*
/*	The open_flags and dict_flags arguments are passed on to the underlying
/*	dictionaries.
/* SEE ALSO
/*	dict(3) generic dictionary manager
/* LICENSE
/* .ad
/* .fi
/*	The Secure Mailer license must be distributed with this software.
/* AUTHOR(S)
/*	Wietse Venema
/*	IBM T.J. Watson Research
/*	P.O. Box 704
/*	Yorktown Heights, NY 10598, USA
/*
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

/* System library. */

#include <sys_defs.h>
#include <string.h>

/* Utility library. */

#include <msg.h>
#include "mymalloc.h"
#include "dict.h"
#include "dict_or.h"
#include "stringops.h"

/* Application-specific. */

typedef struct {
    DICT    dict;			/* generic members */
    ARGV   *map_or;			/* underlying tables */
} DICT_OR;

/* dict_or_lookup - search underlying tables */

static const char *dict_or_lookup(DICT *dict, const char *query)
{
    static const char myname[] = "dict_or_lookup";
    DICT_OR *dict_or = (DICT_OR *) dict;
    DICT   *map;
    char  **cpp;
    char   *dict_type_name;

    for (cpp = dict_or->map_or->argv; (dict_type_name = *cpp) != 0; cpp++) {
	if ((map = dict_handle(dict_type_name)) == 0)
	    msg_panic("%s: dictionary \"%s\" not found", myname, dict_type_name);
	const char *result = dict_get(map, query);
	if (result != 0 || map->error != DICT_ERR_NONE)
	    DICT_ERR_VAL_RETURN(dict, map->error, result);
    }
    DICT_ERR_VAL_RETURN(dict, DICT_ERR_NONE, 0);
}

/* dict_or_close - disassociate from underlying tables */

static void dict_or_close(DICT *dict)
{
    DICT_OR *dict_or = (DICT_OR *) dict;
    char  **cpp;
    char   *dict_type_name;

    for (cpp = dict_or->map_or->argv; (dict_type_name = *cpp) != 0; cpp++)
	dict_unregister(dict_type_name);
    argv_free(dict_or->map_or);
    dict_free(dict);
}

/* dict_or_open - open pipelined tables */

DICT   *dict_or_open(const char *name, int open_flags, int dict_flags)
{
    static const char myname[] = "dict_or_open";
    DICT_OR *dict_or;
    char   *saved_name = 0;
    char   *dict_type_name;
    ARGV   *argv = 0;
    char  **cpp;
    DICT   *dict;
    int     match_flags = 0;
    struct DICT_OWNER aggr_owner;
    size_t  len;

    /*
     * Clarity first. Let the optimizer worry about redundant code.
     */
#define DICT_OR_RETURN(x) do { \
	    if (saved_name != 0) \
		myfree(saved_name); \
	    if (argv != 0) \
		argv_free(argv); \
	    return (x); \
	} while (0)

    /*
     * Sanity checks.
     */
    if (open_flags != O_RDONLY)
	DICT_OR_RETURN(dict_surrogate(DICT_TYPE_OR, name,
				      open_flags, dict_flags,
				      "%s:%s map requires O_RDONLY access mode",
				      DICT_TYPE_OR, name));

    /*
     * Split the table name into its constituent parts.
     */
    if ((len = balpar(name, CHARS_BRACE)) == 0 || name[len] != 0)
	DICT_OR_RETURN(dict_surrogate(DICT_TYPE_OR, name,
				      open_flags, dict_flags,
				      "bad syntax: \"%s:%s\"; "
				      "need \"%s:{type:name...}\"",
				      DICT_TYPE_OR, name, DICT_TYPE_OR));
    saved_name = mystrndup(name + 1, len - 2);
    argv = argv_splitq(saved_name, CHARS_COMMA_SP, CHARS_BRACE);

    /*
     * The least-trusted table in the set determines the over-all trust
     * level. The first table determines the pattern-matching flags.
     */
    DICT_OWNER_AGGREGATE_INIT(aggr_owner);
    for (cpp = argv->argv; (dict_type_name = *cpp) != 0; cpp++) {
	if (msg_verbose)
	    msg_info("%s: %s", myname, dict_type_name);
	if (strchr(dict_type_name, ':') == 0)
	    DICT_OR_RETURN(dict_surrogate(DICT_TYPE_OR, name,
					  open_flags, dict_flags,
					  "bad syntax: \"%s:%s\"; "
					  "need \"%s:{type:name...}\"",
					  DICT_TYPE_OR, name, DICT_TYPE_OR));
	if ((dict = dict_handle(dict_type_name)) == 0)
	    dict = dict_open(dict_type_name, open_flags, dict_flags);
	dict_register(dict_type_name, dict);
	DICT_OWNER_AGGREGATE_UPDATE(aggr_owner, dict->owner);
	if (cpp == argv->argv)
	    match_flags = dict->flags & (DICT_FLAG_FIXED | DICT_FLAG_PATTERN);
    }

    /*
     * Bundle up the result.
     */
    dict_or = (DICT_OR *) dict_alloc(DICT_TYPE_OR, name, sizeof(*dict_or));
    dict_or->dict.lookup = dict_or_lookup;
    dict_or->dict.close = dict_or_close;
    dict_or->dict.flags = dict_flags | match_flags;
    dict_or->dict.owner = aggr_owner;
    dict_or->map_or = argv;
    argv = 0;
    DICT_OR_RETURN(DICT_DEBUG (&dict_or->dict));
}
