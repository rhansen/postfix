/*++
/* AUTHOR(S)
/*	Richard Hansen <rhansen@rhansen.org>
/*--*/

 /*
  * System library.
  */
#include <sys_defs.h>
#include <stdlib.h>
#include <string.h>

 /*
  * Utility library.
  */
#include <dict.h>
#include <msg.h>
#include <msg_vstream.h>
#include <stringops.h>
#include <vstring.h>

 /*
  * Testing library.
  */
#include <test_support.h>

#define LEN(x)	VSTRING_LEN(x)
#define STR(x)	vstring_str(x)

static TEST_RESULT test_multiple_registrations(void *arg)
{
    const char *variant = arg;
    VSTRING *reg_origname = vstring_alloc(100);
    VSTRING *reg_alias = vstring_alloc(100);
    int     backup = dict_allow_multiple_dict_register_names;
    static const char dict_type[] = "static";
    static const char dict_name[] = "value";
    int     open_flags = O_RDONLY;
    int     dict_flags = DICT_FLAG_LOCK;
    DICT   *dict = 0;
    int     needs_close = 0;
    int     needs_unregister = 0;

#undef RETURN
#define RETURN(x) \
    do { \
	if (needs_close) dict_close(dict); \
	if (needs_unregister) dict_unregister(STR(reg_alias)); \
	if (reg_origname) vstring_free(reg_origname); \
	if (reg_alias) vstring_free(reg_alias); \
	dict_allow_multiple_dict_register_names = backup; \
	return (x); \
    } while (0)

    dict_allow_multiple_dict_register_names = 1;
    dict = dict_open3(dict_type, dict_name, open_flags, dict_flags);
    needs_close = 1;
    dict_make_registered_name4(
	reg_origname, dict_type, dict_name, open_flags, dict_flags);
    if (dict->reg_name == 0) {
	msg_warn("table %s:%s reg_name unexpectedly null, want '%s'",
		 dict_type, dict_name, STR(reg_origname));
	RETURN(FAIL);
    }
    if (strcmp(dict->reg_name, STR(reg_origname)) != 0) {
	msg_warn("table %s:%s unexpected reg_name '%s', want '%s'",
		 dict_type, dict_name, dict->reg_name, STR(reg_origname));
	RETURN(FAIL);
    }
    if (dict_handle(STR(reg_origname)) != dict) {
	msg_warn("table %s:%s not registered under original name '%s'",
		 dict_type, dict_name, STR(reg_origname));
	RETURN(FAIL);
    }
    vstring_sprintf(reg_alias, "%s foo", STR(reg_origname));
    dict_register(STR(reg_alias), dict);
    needs_unregister = 1;
    if (dict_handle(STR(reg_alias)) != dict) {
	msg_warn("table %s:%s not registered under alias '%s'",
		 dict_type, dict_name, STR(reg_alias));
	RETURN(FAIL);
    }

    /* Unregister one of the names. */
    if (strcmp(variant, "unregister alias then close") == 0) {
	dict_unregister(STR(reg_alias));
	needs_unregister = 0;
	if (dict_handle(STR(reg_alias)) != 0) {
	    msg_warn("table %s:%s is still registered under alias '%s'",
		     dict_type, dict_name, STR(reg_alias));
	    RETURN(FAIL);
	}
    } else if (strcmp(variant, "close then unregister alias") == 0) {
	dict_close(dict);
	needs_close = 0;
	/*
	 * The alias holds a reference via the original name so the table should
	 * still be registered under the original name.
	 */
	if (dict_handle(STR(reg_origname)) != dict) {
	    msg_warn("table %s:%s should still be registered under original "
		     "name '%s'", dict_type, dict_name, STR(reg_origname));
	    RETURN(FAIL);
	}
    } else { /* Typo check. */
	msg_warn("bad variant: %s", variant);
	RETURN(ERROR);
    }

    /*
     * Exercise the table in case it was erroneously destroyed when either the
     * object was closed or the alias was unregistered (Valgrind should easily
     * detect the use-after-free error).
     */
    const char *got = dict_get(dict, "key");
    static const char want[] = "value";
    if (strcmp(got, want) != 0) {
	msg_warn("table %s:%s lookup returned unexpected value '%s', want '%s'",
		 dict_type, dict_name, got, want);
	RETURN(FAIL);
    }

    /* Unregister the other name. */
    if (strcmp(variant, "unregister alias then close") == 0) {
	dict_close(dict);
	needs_close = 0;
	if (dict_handle(STR(reg_origname)) != 0) {
	    msg_warn("table %s:%s is still registered under original name '%s'",
		     dict_type, dict_name, STR(reg_origname));
	    RETURN(FAIL);
	}
    } else if (strcmp(variant, "close then unregister alias") == 0) {
	dict_unregister(STR(reg_alias));
	needs_unregister = 0;
	if (dict_handle(STR(reg_alias)) != 0) {
	    msg_warn("table %s:%s is still registered under alias '%s'",
		     dict_type, dict_name, STR(reg_alias));
	    RETURN(FAIL);
	}
	if (dict_handle(STR(reg_origname)) != 0) {
	    msg_warn("table %s:%s is still registered under original name '%s'",
		     dict_type, dict_name, STR(reg_origname));
	    RETURN(FAIL);
	}
    } else { /* Typo check. */
	msg_warn("bad variant: %s", variant);
	RETURN(ERROR);
    }

    RETURN(PASS);
}

static const TEST_CASE test_cases[] = {
    {"test_multiple_registrations (unregister alias then close)",
     test_multiple_registrations, "unregister alias then close"},
    {"test_multiple_registrations (close then unregister alias)",
     test_multiple_registrations, "close then unregister alias"},
    {0},
};

int     main(int argc, char **argv)
{
    msg_vstream_init(sane_basename((VSTRING *) 0, argv[0]), VSTREAM_ERR);
    exit(run_tests(test_cases));
}
