#ifndef HANJACHARS_H
#define HANJACHARS_H

// hanjachars.h
// 1/6/2015 jichi

# pragma clang diagnostic ignored "-Wlogical-op-parentheses"

inline bool is_hangul(int ch)
{
  return 0xac00 <= ch && ch <= 0xd7a3   // Hangul Syllables (AC00-D7A3) which corresponds to (가-힣)
      || 0x1100 <= ch && ch <= 0x11ff   // Hangul Jamo (1100–11FF)
      || 0x3130 <= ch && ch <= 0x318f   // Hangul Compatibility Jamo (3130-318F)
      || 0xa960 <= ch && ch <= 0xa97f   // Hangul Jamo Extended-A (A960-A97F)
      || 0xd7b0 <= ch && ch <= 0xd7ff;  // Hangul Jamo Extended-B (D7B0-D7FF)
}

#endif // HANJACHARS_H
