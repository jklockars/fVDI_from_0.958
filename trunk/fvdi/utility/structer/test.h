typedef struct _rect {
   int x1;
   int y1;
   int x2;
   int y2;
} Rect;

typedef struct _vdi {
   struct _line {
//      Colour pen;
      short pen;
      short type;
   } line;
//   Rect clip;
   struct _irect {
      short x1;
      short y1;
      short x2;
      short y2;
   } clip;
   struct _div {
      char *d;
      char a;
      char b;
      char c;
   } div;
   struct _e {
      char t;
      char w;
   } w;
} vdi;

vdi a;

//void func(void)
//{
//   a.div.a = 1;
//   a.div.c = 3;
//   a.w.t = 4;
//}
