#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define C4 60 /* Tabs support a range C3..B6, C4 is a middle C reference */

static char *RST = "\x1b[0m";    /* Normal  style */
static char *TXT = "\x1b[37m";   /* Text:   white */
static char *DIM = "\x1b[35m";   /* Dimmed: magenta */
static char *ACC = "\x1b[1;33m"; /* Accent: bold yellow */
static char *ERR = "\x1b[31m";   /* Error:  red */

static char *INDENT = "    ";
static char *SHARP = "♯";
static char *VLINE = "│";
static char *HLINE = "─";
static char *CLINE = "┊";

static char *FE = "○";
static char *FF = "●";
static char *FL = "◐";
static char *FR = "◑";
static char *FT = "◕";
static char *FQ = "◔";
static char *FB = "◒";
static char *FU = "◓";
static char *FO = "▼";
static char *FP = "+";

#define NLINES 10               /* Max height of a multi-line buffer */
#define LINESZ 1024             /* Max width of a multi-line buffer */
static char ln[NLINES][LINESZ]; /* Multiline buffer, global for all renderers */

struct instr {
  void (*reset)(void *);
  void (*sym)(void *, int);
  void (*note)(void *, int);
  void *ctx;
};

/* Like snprintf, but to append a formatted string */
static void strcatf(const char *ln, const char *fmt, ...) {
  va_list va;
  int n = strlen(ln);
  va_start(va, fmt);
  vsnprintf((char *)ln + n, LINESZ - n - 1, fmt, va);
  va_end(va);
}

int isempty(const char *s) {
  for (; *s; s++)
    if (*s != ' ' && *s != '\n') return 0;
  return 1;
}

/* ------------------- String fretted instruments ------------------------- */
struct frets {
  int n;             /* Number of strings */
  char *tuning;      /* One letter per string */
  int roots[NLINES]; /* Note numbers for each open string */

  int hasnotes;
};

static void frets_reset(void *ctx) {
  int i;
  struct frets *f = (struct frets *)ctx;
  f->hasnotes = 0;
  for (i = 0; i < f->n; i++) {
    snprintf(ln[i], LINESZ - 1, "%s%c%s-%s", DIM, f->tuning[i], VLINE, RST);
  }
}

static void frets_sym(void *ctx, int c) {
  int i;
  struct frets *f = (struct frets *)ctx;
  if (c == '\n') {
    if (f->hasnotes) {
      for (i = 0; i < f->n; i++) { printf("%s%s\n", INDENT, ln[i]); }
      frets_reset(f);
    }
  } else if (c == ' ') {
    for (i = 0; i < f->n; i++) { strcatf(ln[i], "%s--%s", DIM, RST); }
  } else if (c == '|') {
    for (i = 0; i < f->n; i++) { strcatf(ln[i], "%s%s-%s", DIM, VLINE, RST); }
  }
}

static void frets_note(void *ctx, int n) {
  int i;
  struct frets *f = (struct frets *)ctx;
  int index = -1;
  int fret = -1;
  f->hasnotes = 1;
  for (i = 0; i < f->n; i++) {
    int j = n - f->roots[i];
    if (j >= 0 && (fret == -1 || j <= fret)) {
      index = i;
      fret = j;
    }
  }
  if (index == -1) {
    for (i = 0; i < f->n; i++) { strcatf(ln[i], "%sx%s-%s", ERR, DIM, RST); }
  } else {
    for (i = 0; i < f->n; i++) {
      if (index != i) {
        strcatf(ln[i], "%s%s%s", DIM, fret < 10 ? "--" : "---", RST);
      } else {
        strcatf(ln[i], "%s%d%s-%s", ACC, fret, DIM, RST);
      }
    }
  }
}

/* TODO: support diatonic instruments: canjo, Seagull Guitar */
/* TODO: 5-string banjo */
/* TODO: Balalaika */
/* TODO: Mandoline */

struct frets frets_diddley = {1, "C", {C4}, 0};
struct frets frets_gd = {2, "gD", {C4 + 7, C4 + 2}, 0};
struct frets frets_gc = {2, "gD", {C4 + 7, C4}, 0};
struct frets frets_cbg = {3, "gDG", {C4 + 7, C4 + 2, C4 - 5}, 0};
struct frets frets_uke = {4, "AECg", {C4 + 9, C4 + 4, C4, C4 + 7}, 0};
struct frets frets_guitar = {6, "eBGDAE", {C4 + 16, C4 + 11, C4 + 7, C4 + 2, C4 - 3, C4 - 8}, 0};

static struct instr diddley = {frets_reset, frets_sym, frets_note, &frets_diddley};
static struct instr gd = {frets_reset, frets_sym, frets_note, &frets_gd};
static struct instr gc = {frets_reset, frets_sym, frets_note, &frets_gc};
static struct instr cbg = {frets_reset, frets_sym, frets_note, &frets_cbg};
static struct instr uke = {frets_reset, frets_sym, frets_note, &frets_uke};
static struct instr guitar = {frets_reset, frets_sym, frets_note, &frets_guitar};

/* -------------- Flutes, Brass, Woodwinds ------------------- */

struct flute {
  int n;                  /* number of rows in a tab */
  int w;                  /* width of a single tab */
  int k;                  /* key of the instrument */
  int r;                  /* range of the instrument in semitones */
  const char *charts[64]; /* All possible fingering charts, each NxW chars */
};

static void flute_reset(void *ctx) {
  int i;
  struct flute *flute = (struct flute *)ctx;
  for (i = 0; i < flute->n; i++) { ln[i][0] = 0; }
}

static void flute_sym(void *ctx, int c) {
  int i;
  struct flute *flute = (struct flute *)ctx;
  switch (c) {
    case ' ':
      for (i = 0; i < flute->n; i++) strcatf(ln[i], "  ");
      break;
    case '|':
      for (i = 0; i < flute->n; i++) strcatf(ln[i], "%s ", VLINE);
      break;
    case '\n':
      for (i = 0; i < flute->n; i++) printf("%s%s\n", INDENT, ln[i]);
      flute_reset(ctx);
      break;
  }
}

static void flute_note(void *ctx, int c) {
  int i;
  struct flute *flute = (struct flute *)ctx;
  const char *fingering;
  if (c < flute->k || c >= flute->k + flute->r) {
    for (i = 0; i < flute->n; i++) { strcatf(ln[i], "%s%s%s", ERR, "x ", RST); }
    return;
  }

  fingering = flute->charts[c - flute->k];
  for (i = 0; i < flute->n * flute->w; i++) {
    char *s = ln[i / flute->w];
    switch (fingering[i]) {
      case 'x': strcatf(s, "%s%s%s", ACC, FF, RST); break;
      case 'o': strcatf(s, "%s%s%s", ACC, FE, RST); break;
      case 'l': strcatf(s, "%s%s%s", ACC, FL, RST); break;
      case 'r': strcatf(s, "%s%s%s", ACC, FR, RST); break;
      case 'u': strcatf(s, "%s%s%s", ACC, FU, RST); break;
      case 'b': strcatf(s, "%s%s%s", ACC, FB, RST); break;
      case 'q': strcatf(s, "%s%s%s", ACC, FQ, RST); break;
      case 'Q': strcatf(s, "%s%s%s", ACC, FT, RST); break;
      case 'k': strcatf(s, "%s%s%s", DIM, FO, RST); break;
      case '+': strcatf(s, "%s%s%s", DIM, FP, RST); break;
      default:  strcatf(s, "%s%c%s", DIM, fingering[i], RST); break;
    }
    if (i % flute->w == flute->w - 1) { strcatf(s, " "); }
  }
}

/*
TODO: recorder (two fingerings)
TODO: naf4 (pentatonic)
TODO: xaphoon
TODO: more ocarina types
TODO: Traverse flute
TODO: Clarinet, Oboe, Duduk?
TODO: Bansuri? Quena? Shakuhachi?
*/

/* Irish Tin Whistle in D*/
struct flute flute_tinwhistle = {
    7,      /* 4 rows: 6 holes + overblow */
    1,      /* 1 column*/
    C4 + 2, /* D-4*/
    25,     /* Octaves + 1 higher notes*/
    {
        "xxxxxx ", /* D-4*/
        "xxxxxl ", /* D#4*/
        "xxxxxo ", /* E-4*/
        "xxxxlo ", /* F-4*/
        "xxxxoo ", /* F#4*/
        "xxxooo ", /* G-4*/
        "xxlooo ", /* G#4*/
        "xxoooo ", /* A-4*/
        "xoxxxx ", /* A#4*/
        "xooooo ", /* B-4*/
        "oxxooo ", /* C-5*/
        "oooooo ", /* C#5*/
        "oxxxxx ", /* D-5*/
        "xxxxxl+", /* D#5*/
        "xxxxxo+", /* E-5*/
        "xxxxlo+", /* F-5*/
        "xxxxoo+", /* F-5*/
        "xxxooo+", /* G-5*/
        "xxlooo+", /* G-5*/
        "xxoooo+", /* A-5*/
        "xoxxxx+", /* A-5*/
        "xooooo+", /* B-5*/
        "oxxooo+", /* C-5*/
        "oooooo+", /* C-5*/
        "oxxxxx+", /* D-6*/
    },
};

/* Pendant Ocarina in C (4 holes + 2 optional octave holes)*/
struct flute flute_pendant = {
    3,  /* 3 rows: 2x2 holes + 1 octave row*/
    4,  /* 4 cols: octave keys are drawn somewhat apart*/
    C4, /* Key of C*/
    17, /* One octave + 4 higher notes*/
    {
        " xx  xx     ", /* C-4*/
        " xr  xx     ", /* C#4*/
        " xo  xx     ", /* D-4*/
        " xx  xr     ", /* D#4*/
        " xx  xo     ", /* E-4*/
        " xo  xo     ", /* F-4*/
        " ox  xx     ", /* F#4*/
        " oo  xx     ", /* G-4*/
        " ox  xo     ", /* G#4*/
        " oo  xo     ", /* A-4*/
        " oo  ox     ", /* A#4*/
        " ox  oo     ", /* B-4*/
        " oo  oo     ", /* C-5*/
        " oo  ox x  o", /* C#5*/
        " oo  oo x  o", /* D-5*/
        " oo  ox o  o", /* D#5*/
        " oo  oo o  o", /* E-5*/
    },
};

/* Alto Saxophone in Bb*/
struct flute flute_sax = {
    8,      /* 7 rows: 3+3+1 buttons and a delimiter */
    3,      /* 3 columns: octave key, main keys, additional keys */
    C4 - 2, /* Bb */
    32,     /* 2 octaves + 6 lower notes + 1 higher*/
    {
        " x  x  x  -b x  x  x b  ", /* A#3 */
        " x  x  x  -l x  x  x b  ", /* B-3 */
        " x  x  x  -  x  x  x b  ", /* C-4 */
        " x  x  x  -r x  x  x b  ", /* C#4 */
        " x  x  x  -  x  x  x    ", /* D-4 */
        " x  x  x  -  x  x  x u  ", /* D#4 */
        " x  x  x  -  x  x  o    ", /* E-4 */
        " x  x  x  -  x  o  o    ", /* F-4 */
        " x  x  x  -  o  x  o    ", /* F#4 */
        " x  x  x  -  o  o  o    ", /* G-4 */
        " x  x  x  -u o  o  o    ", /* G#4 */
        " x  x  o  -  o  o  o    ", /* A-4 */
        " x  o  o  -  x  o  o    ", /* A#4 */
        " x  o  o  -  o  o  o    ", /* B-4 */
        " o  x  o  -  o  o  o    ", /* C-5 */
        " o  o  o  -  o  o  o    ", /* C#5 */
        "kx  x  x  -  x  x  x    ", /* D-5 */
        "kx  x  x  -  x  x  x u  ", /* D#5 */
        "kx  x  x  -  x  x  o    ", /* E-5 */
        "kx  x  x  -  x  o  o    ", /* F-5 */
        "kx  x  x  -  o  x  o    ", /* F#5 */
        "kx  x  x  -  o  o  o    ", /* G-5 */
        "kx  x  x  -u o  o  o    ", /* G#5 */
        "kx  x  o  -  o  o  o    ", /* A-5 */
        "kx  o  o  -  x  o  o    ", /* A#5 */
        "kx  o  o  -  o  o  o    ", /* B-5 */
        "ko  x  o  -  o  o  o    ", /* C-6 */
        "ko  o  o  -  o  o  o    ", /* C#6 */
        "kxr x  x  -  x  x  x b  ", /* D-6 */
        "kxQ x  x  -  x  x  x b  ", /* D#6 */
        "kxQ x  x  - lx  x  x b  ", /* E-6 */
        "kxx x  x  - lx  x  x b  ", /* F-6 */
    },
};

/* Trumpet in Bb*/
struct flute flute_trumpet = {
    4,      /* 4 rows: Partial note + 3 buttons*/
    1,      /* 1 column*/
    C4 - 6, /* F#3*/
    31,     /* 2 octaves + 6 lower notes + 1 higher*/
    {
        "Cxxx", /* F#3 - 1st partial*/
        "Cxox", /* G-3*/
        "Coxx", /* G#3*/
        "Cxxo", /* A-3*/
        "Cxoo", /* A#3*/
        "Coxo", /* B-3*/
        "Cooo", /* C-4*/
        "Gxxx", /* C#4 - 2nd partial*/
        "Gxox", /* D-4*/
        "Goxx", /* D#4*/
        "Gxxo", /* E-4*/
        "Gxoo", /* F-4*/
        "Goxo", /* F#4*/
        "Gooo", /* G-4*/
        "coxx", /* G#4 - 3rd partial*/
        "cxxo", /* A-4*/
        "cxoo", /* A#4*/
        "coxo", /* B-4*/
        "cooo", /* C-5*/
        "exxo", /* C#5 - 4th partial*/
        "exoo", /* D-5*/
        "eoxo", /* D#5*/
        "eooo", /* E-5*/
        "gxoo", /* F-5 - 5th partial*/
        "goxo", /* F#5*/
        "gooo", /* G-5*/
        "+oxx", /* G#5 - 7th partial (6th is too flat)*/
        "+xxo", /* A-5*/
        "+xoo", /* A#5*/
        "+oxo", /* B-5*/
        "+ooo", /* C-6*/
    },
};

/* Native American Flute in A (6 holes)*/
struct flute flute_naf6 = {
    6,      /* 6 rows, no overblow*/
    1,      /* 1 column*/
    C4 - 3, /* A-4*/
    18,     /* One octave + 6 higher notes*/
    {
        "xxxxxx", /* A-3*/
        "xxxxxQ", /* A#3*/
        "xxxxxl", /* B-3*/
        "xxxxxo", /* C-4*/
        "xxxxox", /* C#4*/
        "xxxxoo", /* D-4*/
        "xxxoxo", /* D#4*/
        "xxxooo", /* E-4*/
        "xxoxoo", /* F-4*/
        "xxoooo", /* F#4*/
        "xoxooo", /* G-5*/
        "oxxooo", /* G#5*/
        "ooxooo", /* A-5*/
        "oxxxxx", /* A#5*/
        "rxxxxl", /* B-5*/
        "rxxxxo", /* C-6*/
        "rxxxlo", /* C#5*/
        "rxxxoo", /* D-6*/
    },
};

/* Native American Flute in A (5 holes)*/
struct flute flute_naf5 = {
    5,      /* 6 rows, no overblow*/
    1,      /* 1 column*/
    C4 - 3, /* A-4*/
    18,     /* One octave + 6 higher notes*/
    {
        "xxxxx", /* A-3*/
        "xxxxQ", /* A#3*/
        "xxxxl", /* B-3*/
        "xxxxo", /* C-4*/
        "xxxox", /* C#4*/
        "xxxoo", /* D-4*/
        "xxoxo", /* D#4*/
        "xxooo", /* E-4*/
        "xoxox", /* F-4*/
        "xoxoo", /* F#4*/
        "xoooo", /* G-5*/
        "ooxoo", /* G#5*/
        "ooooo", /* A-5*/
        "oxxxx", /* A#5*/
        "rxxxl", /* B-5*/
        "rxxxo", /* C-6*/
        "rxxlo", /* C#5*/
        "rxxoo", /* D-6*/
    },
};

struct instr tinwhistle = {flute_reset, flute_sym, flute_note, &flute_tinwhistle};
struct instr pendant = {flute_reset, flute_sym, flute_note, &flute_pendant};
struct instr trumpet = {flute_reset, flute_sym, flute_note, &flute_trumpet};
struct instr sax = {flute_reset, flute_sym, flute_note, &flute_sax};
struct instr naf = {flute_reset, flute_sym, flute_note, &flute_naf6};
struct instr naf5 = {flute_reset, flute_sym, flute_note, &flute_naf5};

/* --------------------- Harmonica ----------------------- */

/* TODO */

/* ---------------------- Jianpu ------------------------- */
struct jianpu {
  int hasln[3];
};
static void jianpu_reset(void *ctx) {
  int i;
  struct jianpu *jianpu = (struct jianpu *)ctx;
  for (i = 0; i < 3; i++) {
    jianpu->hasln[i] = 0;
    ln[i][0] = 0;
  }
}

static void jianpu_sym(void *ctx, int c) {
  int i;
  struct jianpu *jianpu = (struct jianpu *)ctx;
  switch (c) {
    case '\n':
      for (i = 0; i < 3; i++) {
        if (jianpu->hasln[i]) printf("%s%s\n", INDENT, ln[i]);
      }
      jianpu_reset(ctx);
      break;
    case ' ':
      for (i = 0; i < 3; i++) strcatf(ln[i], " ");
      break;
    case '|':
      strcatf(ln[0], "  ");
      strcatf(ln[1], "%s| %s", DIM, RST);
      strcatf(ln[2], "  ");
      break;
  }
}

static void jianpu_note(void *ctx, int c) {
  struct jianpu *jianpu = (struct jianpu *)ctx;
  int n = c % 12;
  int o = c / 12;
  char *hoct = "      .:>>>>";
  char *loct = "<<<<*       ";
  char *acc = " # #  # # # ";
  char *note = "112234455667";
  int isacc = acc[n] == '#';
  jianpu->hasln[1] = 1;
  if (hoct[o] != ' ') jianpu->hasln[0] = 1;
  if (loct[o] != ' ') jianpu->hasln[2] = 1;
  strcatf(ln[0], "%s%s%c%s ", ACC, isacc ? " " : "", hoct[o], RST);
  strcatf(ln[1], "%s%s%c%s ", ACC, isacc ? SHARP : "", note[n], RST);
  strcatf(ln[2], "%s%s%c%s ", ACC, isacc ? " " : "", loct[o], RST);
}

struct jianpu jnpu = {0};
struct instr jianpu = {jianpu_reset, jianpu_sym, jianpu_note, &jnpu};

/* ----------------------- Klavarscribo -------------------------- */

struct klavar {
  int n;
  int root;
};
static int isacc[] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0};
static void klavar_reset(void *ctx) { (void)ctx; }
static void klavar_sym(void *ctx, int c) {
  struct klavar *klavar = (struct klavar *)ctx;
  int i;
  char *fill = " ";
  if (c == ' ') fill = " ";
  if (c == '|') fill = HLINE;
  if (c == '\n') return;

  printf("%s", INDENT);
  for (i = 0; i < klavar->n; i++) {
    printf("%s%s%s", (i % 12 == 0 ? ACC : DIM),
           (isacc[i % 12] ? VLINE
            : i % 12 == 0 ? CLINE
                          : fill),
           RST);
  }
  printf("\n");
}

static void klavar_note(void *ctx, int c) {
  int i;
  struct klavar *klavar = (struct klavar *)ctx;
  printf("%s", INDENT);
  for (i = 0; i < klavar->n; i++) {
    char *fill = " ";
    char *color = i % 12 == 0 ? ACC : DIM;
    if (c == i + klavar->root) {
      fill = isacc[i % 12] ? FE : FF;
      color = ACC;
    } else {
      fill = isacc[i % 12] ? VLINE : i % 12 == 0 ? CLINE : " ";
    }
    printf("%s%s%s", color, fill, RST);
  }
  printf("\n");
}

struct klavar pianofull = {48, C4 - 12};
struct klavar pianotoy = {25, C4};
struct instr piano = {klavar_reset, klavar_sym, klavar_note, &pianofull};
struct instr toy = {klavar_reset, klavar_sym, klavar_note, &pianotoy};

/* ---------------- TODO: Kalimba -------------------- */

/* ---------------- TODO: Piano tabs like guiar -------------------- */

static int note(char c) {
  const int N[] = {9, 11, 0, 2, 4, 5, 7};
  if ((c >= 'A' && c <= 'G') || (c >= 'a' && c <= 'g')) {
    return C4 + N[tolower(c) - 'a'] + (12 * (c >= 'a'));
  }
  return 0;
}

static void tabs_abc(FILE *f, struct instr *instr, int xpose) {
  char line[LINESZ];
  while (fgets(line, sizeof(line), f)) {
    char *p;
    int acc = 0;
    int isabc = 1, q = 0;
    /* Tell text/meta/lyrics from music notation lines */
    for (p = line; *p; p++) {
      if (*p == '"') {
        q = !q;
      } else if (!q && !isspace(*p) && !ispunct(*p) && !note(*p) && !isdigit(*p) && *p != 'z') {
        isabc = 0;
        break;
      }
    }
    if (!isabc) {
      printf("%s%s%s%s", INDENT, TXT, line, RST);
      continue;
    }
    if (isempty(line)) {
      printf("\n");
      continue;
    }

    /* Render a note*/
    q = 0;

    instr->reset(instr->ctx);
    for (p = line; *p; p++) {
      int n;
      char c = *p;
      if (c == '"') {
        q = !q;
      } else if (c == '\n')
        instr->sym(instr->ctx, '\n');
      else if (isspace(c))
        instr->sym(instr->ctx, ' ');
      else if (c == '|')
        instr->sym(instr->ctx, '|');
      else if (c == '_')
        acc--;
      else if (c == '^')
        acc++;
      n = note(c);
      if (!q && n) {
        while (*p) {
          int nc = *++p;
          switch (nc) {
            case '#':  acc = acc + 1; break;
            case '\'': acc = acc + 12; break;
            case ',':  acc = acc - 12; break;
            default:   p--; goto out;
          }
        }
      out:
        instr->note(instr->ctx, n + xpose + acc);
        acc = 0;
      }
    }
  }
  /* Final row may be without a newline, flush it*/
  instr->sym(instr->ctx, '\n');
}

static struct {
  const char *name;
  const char *descr;
  struct instr *instr;
} TABS[] = {
    /* String */
    {"guitar", "6-string Guitar Tabs", &guitar},
    {"uke", "Ukulele Tabs", &uke},
    {"cbg", "Cigar Box Guitar (GDg tuning)", &cbg},
    {"diddley", "Diddley Bow (Unitar)", &diddley},
    {"2gd", "Two-string Diddley Bow (G+D)", &gd},
    {"2gc", "Two-string Diddley Bow (G+C)", &gc},
    /* Woodwind+Brass */
    {"whistle", "Irish Tin Whistle in D", &tinwhistle},
    {"pendant", "Pendant Ocarina (4-hole)", &pendant},
    {"naf", "Native American Flute in A (6-hole)", &naf},
    {"naf6", "Native American Flute in A (6-hole)", &naf},
    {"naf5", "Native American Flute in A (5-hole)", &naf5},
    {"trumpet", "Trumbet", &trumpet},
    {"sax", "Alto Saxophone (Eb)", &sax},
    /* Keys */
    {"piano", "Klavarscribo for 48-key piano", &piano},
    {"toy", "Toy 25-key piano", &toy},
    /* Other */
    {"jianpu", "Chinese Numeric Notation", &jianpu},
    {"123", "Chinese Numeric Notation", &jianpu},
};

static void usage(const char *argv0) {
  unsigned int i;
  fprintf(stderr, "USAGE: %s [-t tab] [-x num] [file ...]\n", argv0);
  fprintf(stderr, "\nOptions:\n\n");
  fprintf(stderr, "  -t TAB\tUse TAB tabulature format (see below)\n");
  fprintf(stderr, "  -x NUM\tTranspose by NUM semitones\n");
  fprintf(stderr, "\nTabulature Formats:\n\n");
  for (i = 0; i < sizeof(TABS) / sizeof(TABS[0]); i++) {
    fprintf(stderr, "  * %-10s\t%s\n", TABS[i].name, TABS[i].descr);
  }
  fprintf(stderr, "\n");
}

int main(int argc, char *argv[]) {
  int c;
  int xpose = 0;
  unsigned int t;
  struct instr *instr = &guitar;

  /* TODO: respect isatty, NO_COLOR and -d for dumb ascii output */
  /* TODO: custom indent */
  while ((c = getopt(argc, argv, "ht:x:")) != -1) {
    switch (c) {
      case 't':
        for (t = 0; t < sizeof(TABS) / sizeof(TABS[0]); t++) {
          if (strcmp(TABS[t].name, optarg) == 0) {
            instr = TABS[t].instr;
            break;
          }
        }
        break;
      case 'x': xpose = atoi(optarg); break; /* TODO: invalid transpose */
      default:  usage(argv[0]); return 1;
    }
  }

  if (optind == argc) {
    tabs_abc(stdin, instr, xpose);
  } else {
    int i;
    for (i = optind; i < argc; i++) {
      FILE *f = fopen(argv[i], "r");
      if (f == NULL) {
        perror("fopen");
        return 1;
      }
      tabs_abc(f, instr, xpose);
      fclose(f);
    }
  }
  return 0;
}
