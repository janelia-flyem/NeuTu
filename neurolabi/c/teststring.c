/* teststring.c
 *
 * 17-Jun-2008 Initial write: Ting Zhao
 */

#include <stdio.h>
#include <regex.h>
#include <string.h>
#include "tz_string.h"
#include "tz_utilities.h"
#include "tz_iarray.h"
#include "tz_darray.h"
#include "tz_stack_lib.h"
#include "tz_math.h"
#include "tz_error.h"

static int test_strrmspc()
{
  char *str = strdup("  test  \n\r");
  strrmspc(str);
  if (!eqstr(str, "test")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  str = strdup("  t e s t  \n\r");
  strrmspc(str);
  if (!eqstr(str, "test")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  str = strdup("  \n\r");
  strrmspc(str);
  if (strlen(str) != 0) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  str = strdup("  t e s\n t  ");
  strrmspc(str);
  if (!eqstr(str, "test")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  return 0;
}

static int test_strsplit()
{
  char *str = strdup("test here");
  char *str2 = strsplit(str, ' ', 1);
  if (!eqstr(str, "test") || !eqstr(str2, "here")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  str = strdup("test here");
  str2 = strsplit(str, ' ', -1);
  if (!eqstr(str, "test") || !eqstr(str2, "here")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  str = strdup("test here too");
  str2 = strsplit(str, ' ', -1);
  if (!eqstr(str, "test here") || !eqstr(str2, "too")) {
    PRINT_EXCEPTION("Bug?", "unexpected string value.");
    return 1;
  }
  free(str);

  return 0;
}

static int test_string_match()
{
  if (String_Ends_With("", "") == FALSE) {
    PRINT_EXCEPTION("Bug?", "unexpected value.");
    return 1;
  }

  if (String_Starts_With("", "") == FALSE) {
    PRINT_EXCEPTION("Bug?", "unexpected value.");
    return 1;
  }

  if (String_Ends_With("test.here", "ere") == FALSE) {
    PRINT_EXCEPTION("Bug?", "unexpected value.");
    return 1;
  }

  if (String_Starts_With("test.here", "test") == FALSE) {
    PRINT_EXCEPTION("Bug?", "unexpected value.");
    return 1;
  }


  return 0;
}

static int test_read_word()
{
  char word[500];
  const char *test_file = "../data/benchmark/strtest.txt";
  FILE *fp = fopen(test_file, "r");
  if (fp != NULL) {
    int n = Read_Word(fp, word, 100);
    if (n != 3) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "#if")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word(fp, word, 100);
    if (n != 1) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "0")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word(fp, word, 100);
    if (n != 3) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "int")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word(fp, word, 100);
    n = Read_Word(fp, word, 3);
    if (n != 3) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "dou")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word(fp, word, 100);
    if (n != 3) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "ble")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word(fp, word, 0);
    if (n != 6) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "*array")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }
    
    n = Read_Word_D(fp, word, 0, tz_islinebreak);
    if (n != 2) {
      printf("%d\n", n);
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    if (!eqstr(word, "= ")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Read_Word_D(fp, word, 0, tz_islinebreak);
    if (!eqstr(word, "  String_To_Double_Array(\" -345.4, -.23, --2.324, - nubmer, hello - 3.0, .10 ,\", NULL, &n);")) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    fclose(fp);

    n = Count_Word_D("test here", tz_isspace);
    if (n != 2) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Count_Word_D(" ", tz_isspace);
    if (n != 0) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Count_Word_D(" ", tz_isspace);
    if (n != 0) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Count_Word_D("test\nhere ", tz_islinebreak);
    if (n != 2) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Count_Word_D("test\nhere\n", tz_islinebreak);
    if (n != 2) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }

    n = Count_Word_P("22 faf", tz_isspace, Is_Integer);
    if (n != 1) {
      PRINT_EXCEPTION("Bug?", "unexpected value.");
      return 1;
    }
  } else {
    printf("%s does not exist.", test_file);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  static char *Spec[] = {"[-t]", NULL};

  Process_Arguments(argc, argv, Spec, 1);
 
  if (Is_Arg_Matched("-t")) {
    if (tz_isletter('a') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('A') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('z') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('Z') == 0) {
      PRINT_EXCEPTION("Bug?", "should be a letter.");
      return 1;
    }

    if (tz_isletter('9') == 1) {
      PRINT_EXCEPTION("Bug?", "should not be a letter.");
      return 1;
    }

    if (tz_isletter('&') == 1) {
      PRINT_EXCEPTION("Bug?", "should not be a letter.");
      return 1;
    }

    char *str = strdup("  test  \n\r");
    strtrim(str);
    if (!eqstr(str, "test")) {
      PRINT_EXCEPTION("Bug?", "unexpected string value.");
      return 1;
    }
    free(str);

    str = strdup("  t e s t  \n\r");
    strtrim(str);
    if (!eqstr(str, "t e s t")) {
      PRINT_EXCEPTION("Bug?", "unexpected string value.");
      return 1;
    }
    free(str);

    str = strdup("  \n\r");
    strtrim(str);
    if (strlen(str) != 0) {
      PRINT_EXCEPTION("Bug?", "unexpected string value.");
      return 1;
    }
    free(str);

    str = strdup("  t e s\n t  ");
    strtrim(str);
    if (!eqstr(str, "t e s\n t")) {
      PRINT_EXCEPTION("Bug?", "unexpected string value.");
      return 1;
    }
    free(str);

    int n;
    int *array = String_To_Integer_Array("100 - - -101 frea-w2 13", NULL, &n);

    if (n != 4) {
      PRINT_EXCEPTION("Bug?", "unexpected number.");
      return 1;
    }

    if (array[0] != 100 || array[1] != -101 || array[2] != 2 || array[3] != 13) {
      PRINT_EXCEPTION("Bug?", "unexpected number.");
      return 1;
    }

    if (test_strrmspc() != 0) {
      return 1;
    }

    if (test_strsplit() != 0) {
      return 1;
    }

    if (test_string_match() != 0) {
      return 1;
    }

    if (test_read_word() != 0) {
      return 1;
    }

    printf(":) Testing passed.\n");

    return 0;
  }

#if 0 /* test regular expression */
  regex_t preg;
  const char *pattern = ".*\\.tif";
  regcomp(&preg, pattern, REG_BASIC);
  int result = regexec(&preg, "hello.tif", 0, NULL, REG_BASIC);
  printf("%d\n", result);
  regfree(&preg);
#endif

#if 0 /* test Read_Word */
  FILE *fp = fopen("teststring.c", "r");
  char str[100];
  int n = 0;
  while ((n = Read_Word_D(fp, str, 0, tz_issemicolon)) > 0) {
    printf("%s\n", str);
  }
  fclose(fp);
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("tz_darray.c", "r");
  char *line;
  int i = 1;
  while((line = Read_Line(fp, sw)) != NULL) {
    printf("%d: %s\n", i++, line);
  };
  fclose(fp);
  
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("../data/stringtest.txt", "r");
  printf("%s\n", Read_Param(fp, "var3", sw));
  fclose(fp);
  
#endif

#if 0
  if (Is_Integer("23")) {
    printf("An integer\n");
  } else {
    printf("Not an integer\n");
  }
#endif

#if 0
  printf("%s\n", fullpath("../data/fly_neuron_n11", "mask.tif", NULL));
#endif

#if 0
  printf("%d\n", Count_Word_D(",", tz_isdlm));
  printf("%d\n", Count_Number_D("345, 23, 2324, nubmer, hello -30, 10 ,", tz_iscoma));
#endif

#if 0
  int n;
  int *array = String_To_Integer_Array(" 345,-23,-2324, nubmer, hello -30 10 ,",
				       NULL, &n);
  iarray_print2(array, n, 1);
#endif

#if 0
  int n;
  double *array = 
    String_To_Double_Array(" -345.4, -.23, --2.324, - nubmer, hello - 3.0, .10 ,", NULL, &n);
  darray_print2(array, n, 1);
#endif

#if 0 /* test strsplit */
  char str[] = {"/This/is/a/test/"};

  char *tail = strsplit(str, '/', -6);

  printf("%s\n", str);
  if (tail != NULL) {
    printf("tail: %s\n", tail);
  }
#endif

#if 0
  printf("%d\n", File_Line_Number("../data/test.xml", FALSE));
  printf("%d\n", File_Line_Number("../data/test.xml", TRUE));
#endif

#if 0
  printf("%d\n", String_Ends_With("This is a test", "test2"));
#endif

#if 0
  String_Workspace *sw = New_String_Workspace();
  FILE *fp = fopen("../data/score.txt", "r");
  char *line;
  int i = 0;
  double score_array[300];
  while((line = Read_Line(fp, sw)) != NULL) {
    //printf("%d: %s\n", i++, line);
    int n;
    String_To_Double_Array(line, score_array + i, &n);
    i++;
  }
  
  fclose(fp);

  //int neuron_id[] = {209,285743,211940,189938,181639,196,285714,215,446263,29565,194027,24070,170689,5809,106054,1172,1513,277709,386464,280303,2341,278848,545716,3453,210};

  fp = fopen("../data/test.csv", "w");

  int j;
  int index = 0;

  double score_matrix[25][25];

  double max_score = 0.0;
  double min_score = 10000.0;

  double row_max[25];

  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      if (i < j) {
        score_matrix[i][j] = score_array[index++];
        score_matrix[j][i] = score_matrix[i][j];
        if (max_score < score_matrix[i][j]) {
          max_score = score_matrix[i][j];
        }

        if (min_score > score_matrix[i][j]) {
          min_score = score_matrix[i][j];
        }
      }
      if (i == j) {
        score_matrix[i][i] = 0.0;
      }
    }
  }


  for (i = 0; i < 25; i++) {
    row_max[i] = 0.0;
    for (j = 0; j < 25; j++) {
      if (row_max[i] < score_matrix[i][j]) {
        row_max[i] = score_matrix[i][j];
      }
    }
  }

  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      if (i < j) {
        score_matrix[i][j] /= row_max[i];
      } else{
        score_matrix[i][j] = score_matrix[j][i];
      }
    }
  }

  Stack *stack = Make_Stack(GREY, 25, 25, 1);

  int offset = 0;
  for (i = 0; i < 25; i++) {
    for (j = 0; j < 25; j++) {
      fprintf(fp, "%g, ", score_matrix[i][j]);
      if (i == j) {
        stack->array[offset++] = 255;
      } else {
        /*
        int value = iround(255.0 / (max_score - min_score) * 
            (score_matrix[i][j] - min_score));
            */
        int value = iround(score_matrix[i][j] * 255.0);

        if (value > 255) {
          value = 255;
        }
        stack->array[offset++] = value;
      }
    }
    fprintf(fp, "\n");
  }

  Write_Stack("../data/test.tif", stack);

  fclose(fp);
#endif

#if 1
  //printf("double number: %d\n", count_double(".9 3.4 e +3.3e-3 4"));
  //printf("double number: %d\n", count_double(".9-.9.9.9.9"));

  char *str = ".9 3.4 e +3.3e-3 4";
  //char *str = ".9-.9.9.9.9E-41.23.4e0r2.1";
  int n;
  double *array = 
    //String_To_Double_Array(str, NULL, &n);
  String_To_Double_Array(" -345.4e-.23, --2.324e-05, - nubmer, hello - 3.0, .10 ,", NULL, &n);
  darray_printf(array, n, "%g");

#endif

  
  return 1;
}
