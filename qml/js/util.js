// util.js
// 10/20/2012 jichi

.pragma library // stateless

// - i18n -

var LANGUAGES = [
  'ja'
  , 'en'
  , 'zht', 'zhs'
  , 'ko'
  , 'th', 'vi', 'ms', 'id'
  , 'de', 'fr', 'es', 'it', 'nl', 'pl', 'pt', 'ru'
]

function isKnownLanguage(lang) { return -1 !== LANGUAGES.indexOf(lang) }

/**
 *  @param  lang  string
 *  @return  string
 **/

var LANGUAGE_NAME = {
 ja: "Japanese"
 , en: "English"
 , zh: "Chinese"
 , zht: "Chinese"
 , zhs: "Simplified Chinese"
 , ko: "Korean"
 , th: "Thai"
 , vi: "Vietnamese"
 , ms: "Melayu"
 , id: "Indonesian"
 , de: "German"
 , es: "Spanish"
 , fr: "French"
 , it: "Italian"
 , nl: "Dutch"
 , pl: "Polish"
 , pt: "Portuguese"
 , ru: "Russian"
}
function languageName(lang) { return LANGUAGE_NAME[lang] }

var LATIN_LANGUAGES = [
  'en'
  , 'de', 'es', 'fr', 'it', 'nl', 'pl', 'pt', 'ru'
  , 'th', 'vi', 'ms', 'id'
]
/**
 *  @param  lang  string
 *  @return  bool  whether the language is based on latin characters
 */

function isLatinLanguage(lang) { return -1 !== LATIN_LANGUAGES.indexOf(lang) }

var ASIAN_LANGUAGES = [
  'ja'
  , 'zht', 'zhs'
  , 'ko'
]
/**
 *  @param  lang  string
 *  @return  bool  whether it is eastern asian language
 */
function isAsianLanguage(lang) { return -1 !== ASIAN_LANGUAGES.indexOf(lang) }

/**
 *  @param  lang  string
 *  @return  bool  whether it is based on kanji characters
 */
function isKanjiLanguage(lang) {
  switch (lang) {
    case 'zht': case 'zhs': case 'ja': return true
    default: return false
  }
}

/**
 *  @param  lang  string
 *  @return  bool
 */
function isChineseLanguage(lang) {
  switch (lang) {
    case 'zht': case 'zhs': return true
    default: return false
  }
}

/**
 *  @param  lang  string
 *  @return  bool
 */
function spellSupportsLanguage(lang) {
  switch (lang) {
    case 'en': case 'de': case 'fr': return true
    default: return false
  }
}

/**
 *  @param  text  string
 *  @return  bool
 */
function containsLatin(text) {
  return text && text.match(/[a-zA-Z]/)
}

var LANGUAGE_FONT = {
  //ja: "MS Mincho"
  ja: "MS Gothic"
  , en: "Helvetica"
  , zh: "YouYuan"
  , zhs: "YouYuan"
  , zht: "YouYuan"
  , ko: "Batang"
  , vi: "Tahoma"
  , th: "Tahoma"
  , ms: "Helvetica" //"Aria"
  , id: "Helvetica" //"Aria"
  , fr: "Helvetica"
  , es: "Helvetica"
  , de: "Helvetica"
  , it: "Helvetica"
  , nl: "Helvetica"
  , pl: "Helvetica"
  , pt: "Helvetica"
  , ru: "Helvetica"
}
/**
 *  @param  lang  string
 *  @return  string
 *
 *  http://en.wikipedia.org/wiki/List_of_CJK_fonts
 */
function fontFamilyForLanguage(lang) { return LANGUAGE_FONT[lang] || '' }

var TRANSLATOR_NAME = {
  infoseek: "Infoseek"
  , excite: "Excite"
  , bing: "Bing"
  , google: "Google"
  , baidu: "百度"
  //, youdao: "有道"
  , jbeijing: "J北京"
  , dreye: "Dr.eye"
  , eztrans: "ezTrans XP"
  , atlas: "ATLAS"
  , lec: "LEC"
  , lecol: "LEC Online"
  , transru: "Translate.Ru"
  , lou: "ルー語"
}
function translatorName(tr) { return TRANSLATOR_NAME[tr] }

// - Cast -

/**
 *  @param  val  var
 *  @return  bool
 */
function toBool(val) {
  //return !!val
  return val ? true : false
}

// - String -

// See: http://blog.enjoitech.jp/article/180
// See: http://stackoverflow.com/questions/2429146/javascript-regular-expression-single-space-character
// \s: [\t\n\v\f\r \u00a0\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200a\u200b\u2028\u2029\u3000]
/**
 *  @param  str  string
 *  @return  string
 */
function utrim(str) { // unicode trim
  return str ? str.replace(/^\s+|\s+$/g, "") : ""
}

/**
 *  @param  str  string
 *  @return  string
 */
function ultrim(str) {
  return str ? str.replace(/^\s+/, "") : ""
}

/**
 *  @param  str  string
 *  @return  string
 */
function urtrim(str) {
  return str ? str.replace(/\s+$/, "") : ""
}

/**
 *  @param  str  string
 *  @return  string
 */
function trim(str) { // unicode trim
  return str ? str.replace(/^[\t\n\v\f\r ]+|[\t\n\v\f\r ]+$/g, "") : ""
}

/**
 *  @param  str  string
 *  @return  string
 */
function ltrim(str) {
  return str ? str.replace(/^[\t\n\v\f\r ]+/, "") : ""
}

/**
 *  @param  str  string
 *  @return  string
 */
function rtrim(str) {
  return str ? str.replace(/[\t\n\v\f\r ]+$/, "") : ""
}


// See: http://www.openjs.com/scripts/strings/setcharat_function.php
/**
 *  @param  str  string
 *  @param  index  int
 *  @param  ch  string[1]
 *  @return  string
 */
function setCharAt(str, index, ch) {
  return index < 0 || !str || index >= str.length ? str :
	       str.substr(0, index) + ch + str.substr(index + 1)
}

// See: http://dev.ariel-networks.com/Members/uchida/javascript7684startswith/
/**
 *  @param  str  string
 *  @param  prefix  string
 *  @return  bool
 */
function startsWith(str, prefix) {
  return !!str && !str.indexOf(prefix) //=== 0
}

// See: http://stackoverflow.com/questions/280634/endswith-in-javascript
/**
 *  @param  str  string
 *  @param  suffix  string
 *  @return  bool
 */
function endsWith(str, suffix) {
  return !!(str && suffix && ~str.indexOf(suffix, str.length - suffix.length)) // !== -1
}

// See: http://www.electrictoolbox.com/pad-number-zeroes-javascript/
/**
 *  @param  d  Date
 *  @return  int
 */
function padZero(number, length) {
  var ret = '' + number
  while (ret.length < length)
     ret = '0' + ret
  return ret
}

// - Datetime -

// See: http://www.electrictoolbox.com/unix-timestamp-javascript/
/**
 *  @param  d  Date
 *  @return  int
 */
function currentUnixTime() {
  return Math.floor(new Date().getTime() / 1000)
}

/**
 *  @param  d  Date
 *  @return  string
 */

var WEEK_NAME = [
  "日"
  , "月"
  , "火"
  , "水"
  , "木"
  , "金"
  , "土"
]

function dateTimeToString(d) {
  return (d.getMonth()+1) + "/" + d.getDate() + "/" + d.getFullYear() + " " + WEEK_NAME[d.getDay()] + " " +
         d.getHours() + ":" + padZero(d.getMinutes(), 2) + ":" + padZero(d.getSeconds(), 2)
}

function dateToString(d) {
  return (d.getMonth()+1) + "/" + d.getDate() + "/" + d.getFullYear() + " " + WEEK_NAME[d.getDay()]
}

function timeToString(d) {
  return d.getHours() + ":" + padZero(d.getMinutes(), 2) + ":" + padZero(d.getSeconds(), 2)
}

/**
 *  @param  seconds  int
 *  @return  string
 */
function datestampToString(seconds) {
  return dateToString(new Date(seconds * 1000))
}

function timestampToString(seconds) {
  return dateTimeToString(new Date(seconds * 1000))
}

/**
 *  @param  h  int
 *  @param  m  int
 *  @param* s  int or undefined
 *  @return  string
 */
function formatTime(h, m, s) {
  var r = h + ":" + padZero(m, 2)
  if (s !== undefined)
    r += ":" + padZero(s, 2)
  return r
}

/**
 *  @param  m  int  month
 *  @param  d  int  day
 *  @param* y  int  year
 *  @param* w  int  week
 *  @return  string
 */
function formatDate(m, d, y, w) {
  var r = m + "/" + d
  if (y !== undefined)
    r += "/" + y
  if (w !== undefined)
    r += " " + WEEK_NAME[w]
  return r
}

/**
 *  @param w  int  week
 *  @return  string
 */
function formatWeek(w) { return WEEK_NAME[w] }

// - QML -

/**
 *  @param  item  QtQuick.Item
 *  @return  Qt.point
 */
function itemGlobalPos(item) {
  var x = 0
  var y = 0
  while (item) {
    if (item.x) // test first as most x,y are zeros
      x += item.x
    if (item.y)
      y += item.y
    item = item.parent
  }
  return Qt.point(x, y)
}

// EOF
