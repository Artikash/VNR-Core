#ifndef UNICHAR_H
#define UNICHAR_H

// unichar.h
// 1/6/2015 jichi

#ifdef __clang__
# pragma clang diagnostic ignored "-Wlogical-op-parentheses"
#endif // __clang__

namespace unistr {

// Orders of wide and thin characters

enum { thin_wide_dist = 65248 };
enum : char { thin_min = 33, thin_max = 126 };
enum : wchar_t { wide_min = thin_min + thin_wide_dist, wide_max = thin_max + thin_wide_dist };

inline bool isthin(int ch) { return thin_min <= ch && ch <= thin_max; }
inline bool iswide(int ch) { return wide_min <= ch && ch <= wide_max; }

inline int wide2thin(int ch) { return isthin(ch) ? ch + thin_wide_dist : ch; }
inline int thin2wide(int ch) { return iswide(ch) ? ch - thin_wide_dist : ch; }

// Japanese

enum { hira_kata_dist = 96 };
enum : wchar_t { hira_min = 12353, hira_max = 12438 }; // ぁ-ゖ
enum : wchar_t { kata_min = 12449, kata_max = 12538 }; // ァ-ヺ

inline bool ishira(int ch) { return hira_min <= ch && ch <= hira_max; }
inline bool iskata(int ch) { return kata_min <= ch && ch <= kata_max; }

inline bool iskana(int ch) { return ishira(ch) || iskata(ch); }

inline int hira2kata(int ch) { return ishira(ch) ? ch + hira_kata_dist : ch; }

inline int kata2hira(int ch) // because katagana has a larger range
{ return kata_min <= ch && ch <= hira_max + hira_kata_dist ? ch - hira_kata_dist : ch; }

// Chinese

enum : wchar_t { kanji_min = 19968, kanji_max = 40869 }; // 一-龥
inline bool iskanji(int ch) { return kanji_min <= ch && ch <= kanji_max; }

// Korean

inline bool ishangul(int ch)
{
  return 0xac00 <= ch && ch <= 0xd7a3   // Hangul Syllables (AC00-D7A3) which corresponds to (가-힣)
      || 0x1100 <= ch && ch <= 0x11ff   // Hangul Jamo (1100–11FF)
      || 0x3130 <= ch && ch <= 0x318f   // Hangul Compatibility Jamo (3130-318F)
      || 0xa960 <= ch && ch <= 0xa97f   // Hangul Jamo Extended-A (A960-A97F)
      || 0xd7b0 <= ch && ch <= 0xd7ff;  // Hangul Jamo Extended-B (D7B0-D7FF)
}

// Chinese

} // namespace unistr

#endif // UNICHAR_H
