
#include <stdio.h>
#include <string.h>
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

int main(int argc, char *argv[]) {
  pcre2_code *re = NULL;

  PCRE2_SPTR pattern; /* PCRE2_SPTR is a pointer to unsigned code units of */
  PCRE2_SPTR subject; /* the appropriate width (8, 16, or 32 bits). */
  PCRE2_SPTR name_table;

  int crlf_is_newline;
  int errornumber;
  int find_all;
  int i;
  int namecount;
  int name_entry_size;
  int rc;
  int utf8;

  uint32_t option_bits;
  uint32_t newline;

  PCRE2_SIZE erroroffset;
  PCRE2_SIZE *ovector;

  size_t subject_length;
  pcre2_match_data *match_data;

  /**************************************************************************
   * First, sort out the command line. There is only one possible option at  *
   * the moment, "-g" to request repeated matching to find all occurrences,  *
   * like Perl's /g option. We set the variable find_all to a non-zero value *
   * if the -g option is present. Apart from that, there must be exactly two *
   * arguments.                                                              *
   **************************************************************************/

  find_all = 0;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-g") == 0)
      find_all = 1;
    else
      break;
  }

  /* After the options, we require exactly two arguments, which are the pattern,
  and the subject string. */

  if (argc - i != 2) {
    printf("Two arguments required: a regex and a subject string\n");
    return 1;
  }

  /* As pattern and subject are char arguments, they can be straightforwardly
  cast to PCRE2_SPTR as we are working in 8-bit code units. */

  pattern = (PCRE2_SPTR)argv[i];
  subject = (PCRE2_SPTR)argv[i + 1];
  subject_length = strlen((char *)subject);

  /*************************************************************************
   * Now we are going to compile the regular expression pattern, and handle *
   * any errors that are detected.                                          *
   *************************************************************************/

  re = pcre2_compile(
      pattern,               /* the pattern */
      PCRE2_ZERO_TERMINATED, /* indicates pattern is zero-terminated */
      0,                     /* default options */
      &errornumber,          /* for error number */
      &erroroffset,          /* for error offset */
      NULL);                 /* use default compile context */
  if (re == NULL) {
    PCRE2_UCHAR buffer[256];
    pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
    printf("PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset,
           buffer);
    return 1;
  }
  pcre2_match(NULL, NULL, 0, 0, 0, NULL, NULL);
  return 0;
}
