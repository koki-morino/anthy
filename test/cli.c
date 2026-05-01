#include <anthy/anthy.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define ANTHY_ROOT "./build"

int
main(void)
{
#ifdef _WIN32
  SetConsoleCP(CP_UTF8);
  SetConsoleOutputCP(CP_UTF8);
#endif

  int ret             = 0;
  anthy_context_t ctx = NULL;

  // initialization
  {
    anthy_conf_override("CONFFILE",
                        ANTHY_ROOT "/anthy-unicode.conf");
    anthy_conf_override("DIC_FILE",
                        ANTHY_ROOT "/mkanthydic/anthy.dic");
    if (anthy_init() != 0) {
      fprintf(stderr, "error: couldn't initialize anthy\n");
      ret = 1;
      goto out;
    }

    ctx = anthy_create_context();
    if (!ctx) {
      fprintf(stderr, "error: couldn't create anthy context\n");
      ret = 1;
      goto out;
    }
    anthy_context_set_encoding(ctx, ANTHY_UTF8_ENCODING);
  }

  // read input
  char input[BUFSIZ];
  {
    if (!fgets(input, BUFSIZ, stdin)) {
      fprintf(stderr, "error: couldn't read input\n");
      ret = 1;
      goto out;
    }
    input[strlen(input) - 1] = '\0';
    printf("#input: %s\n", input);
  }

  // conversion
  {
    anthy_set_string(ctx, input);

    struct anthy_conv_stat stat;
    anthy_get_stat(ctx, &stat);

    // resize segment so the input becomes a single segment
    {
      int limit = 10;
      if (stat.nr_segment > limit) {
        fprintf(stderr, "error: segments should be less than %d\n", limit);
        ret = 1;
        goto out;
      }

      // grow segment 0 until there is only one segment.
      while (stat.nr_segment > 1) {
        anthy_resize_segment(ctx, 0, 1);
        anthy_get_stat(ctx, &stat);
      }
    }

    printf("#segments: %d\n", stat.nr_segment);

    struct anthy_segment_stat seg_stat;
    anthy_get_segment_stat(ctx, 0, &seg_stat);
    printf("#candidates: %d\n", seg_stat.nr_candidate);
    for (int i = 0; i < seg_stat.nr_candidate; ++i) {
      char buf[256];
      anthy_get_segment(ctx, 0, i, buf, sizeof(buf));
      printf("%d: %s\n", i, buf);
      if (i != 0) {
        // TODO: commit all candidates. currently, only the first candidate is
        // committed, which is not ideal for testing.
        anthy_commit_segment(ctx, 0, i);
      }
    }
  }

  // extract a few characters from the input
  char prediction_input[BUFSIZ];
  {
    int copy_len     = 0;
    int char_count   = 0;
    int target_chars = 3;
    while (input[copy_len] && char_count < target_chars) {
      unsigned char c = (unsigned char)input[copy_len];
      int len         = 1;
      if ((c & 0x80) == 0)
        len = 1;
      else if ((c & 0xE0) == 0xC0)
        len = 2;
      else if ((c & 0xF0) == 0xE0)
        len = 3;
      else if ((c & 0xF8) == 0xF0)
        len = 4;
      copy_len += len;
      char_count++;
    }
    strncpy(prediction_input, input, copy_len);
    prediction_input[copy_len] = '\0';
    printf("#prediction input: %s\n", prediction_input);
  }

  // prediction
  {
    anthy_set_prediction_string(ctx, prediction_input);
    struct anthy_prediction_stat stat;
    anthy_get_prediction_stat(ctx, &stat);
    for (int i = 0; i < stat.nr_prediction; ++i) {
      char buf[256];
      anthy_get_prediction(ctx, i, buf, sizeof(buf));
      printf("%d: %s\n", i, buf);
    }
  }

out:
  if (ctx) {
    anthy_release_context(ctx);
  }
  anthy_quit();

  return ret;
}
