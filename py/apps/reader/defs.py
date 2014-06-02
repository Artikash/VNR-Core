# coding: utf8
# defs.py
# 11/4/2012 jichi

# Exit codes

EXIT_SUCCEED = 0 # os.EX_OK
EXIT_FAILURE = 1
EXIT_RESTART = -1

# Features
INTERNET_CONNECTION_AUTO = 'auto'
INTERNET_CONNECTION_ON = 'on'
INTERNET_CONNECTION_OFF = 'off'

# Date time

# Time to protect anonymous entries
#PROTECTED_INTERVAL = 86400 * 15 # 15 days
PROTECTED_INTERVAL = 86400 * 7 # 7 days

# Game agent
# These values must be consistent with vnragent.dll
UNKNOWN_TEXT_ROLE = 0
SCENARIO_TEXT_ROLE = 1
NAME_TEXT_ROLE = 2
OTHER_TEXT_ROLE = 3

# Text constraints

MIRAGE_MAX_TEXT_LENGTH = 512

MAX_NAME_LENGTH = 64
MAX_NAME_LENGTH *= 3 # increased due to repetition removal

MAX_TEXT_LENGTH = 255
MD5_HEX_LENGTH = 32

#MAX_DATA_LENGTH = MAX_TEXT_LENGTH * 2
#MAX_DATA_LENGTH = int(MAX_DATA_LENGTH * 1.5) # Increase data size
#MAX_REPEAT_DATA_LENGTH = MAX_DATA_LENGTH * 2

USER_DEFINED_THREAD_NAME = 'User-defined'
CAONIMAGEBI_USERNAME = '@CaoNiMaGeBi'
CAONIMAGEBI_ENGINES = frozenset((
  'Artemis',
  'Gesen18',
  'NEXTON',
  'QLIE2',
  'Rejet',
  'SiglusEngine2',
))

SINGLE_ENGINE_SIGNATURE = 0x10000 # this value is consistent with FIXED_SPLIT_VALUE in ITH

OK123_USERNAME = '@ok123'
OK123_ENGINES = frozenset((
  'Malie3',
  'Mono',
  'Unity',
))

CONTEXT_SEP = "||"

# The game engines that require the game to have normal/slow text speed
SLOW_GAME_ENGINES = frozenset(('EmonEngine', 'RunrunEngine', 'RunrunEngine Old'))
FAST_GAME_ENGINES = 'MarineHeart',

# The game engines that require the game to be hooked after loaded
DELAY_ENGINES = 'Adobe AIR',

# Convert new game agent name to old ITH engine name
def to_ith_engine_name(name): # str -> str
  return (
      'MAJIRO' if name == 'Majiro' else
      'SiglusEngine2' if name == 'SiglusEngine' else
      name)

# VNR agent supported engines
VNRAGENT_ENGINES = (
  'BGI',
  'Eushully',
  'SiglusEngine2',
  'Majiro',
)

# Back up file suffix
BACKUP_FILE_SUFFIX = '.bak'

# User

USER_DEFAULT_COLOR = '#006400' # darkgreen

# Types

GENDERS = '', 'm', 'f'

# Furigana

FURI_KATA = 'katagana'
FURI_HIRA = 'hiragana'
FURI_ROMAJI = 'romaji'
FURI_KANJI = 'kanji'
FURI_HANGUL = 'hangul'
FURI_THAI = 'thai'
FURI_TR = 'tr'

# Translations

TERM_ESCAPE = "9%i.678" # ESCAPE of escaped terms. At least 3 digits so that youdao work well!
NAME_ESCAPE = "9%i.%i58" # ESCAPE of escaped character name

CHARA_ESCAPE = "9%i.%i48" # ESCAPE of character names from getchu

# Game info
OKAZU_TAGS = [
  u"催眠",
]

# Thread type
NULL_THREAD_TYPE = 0    # invalid
ENGINE_THREAD_TYPE = 1
CUI_THREAD_TYPE = 2
GUI_THREAD_TYPE = 3
HOOK_THREAD_TYPE = 4

def threadtype(name):
  """
  @param  name  str
  @return  int
  """
  import config
  return (
      NULL_THREAD_TYPE if not name else
      HOOK_THREAD_TYPE if name == USER_DEFINED_THREAD_NAME else
      GUI_THREAD_TYPE if name in config.GUI_TEXT_THREADS else
      CUI_THREAD_TYPE if name == 'KiriKiri2' else # force kirikiri2 to be CUI
      CUI_THREAD_TYPE if name in config.NON_GUI_TEXT_THREADS else
      ENGINE_THREAD_TYPE)

# EOF
