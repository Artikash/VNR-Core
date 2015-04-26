# coding: utf8
# stardict.py
# 4/26/2015  jichi
from sakurakit.skdebug import dwarn

class StarDict(object):
  ifo_compressed = False
  idx_compressed = False
  dict_compressed = False

  def __init__(self, path):
    """
    @param  path  unicode  base name
    """
    self.ifo_file = path + ".ifo" # unicode
    self.idx_file = path + ".idx" # unicode
    self.dict_file = path + ".dict" # unicode
    self.reader = None

  def init(self):
    import stardictlib
    try:
      ifo_reader = stardictlib.IfoFileReader(self.ifo_file, compressed=self.ifo_compressed)
      idx_reader = stardictlib.IdxFileReader(self.idx_file, compressed=self.idx_compressed)
      dict_reader = stardictlib.DictFileReader(self.dict_file, ifo_reader, idx_reader,compressed=self.ifo_compressed)

      self.reader = dict_reader
    except Exception, e: dwarn(e)

  def query(self, text):
    """
    @param  text  unicode
    @return  [unicode] or None
    """
    try:
      m = self.reader.get_dict_by_word(t)
      if m:
        ret = []
        for it in m:
          ret.extend(it.itervalues())
        return ret
    except Exception, e: dwarn(e)

# EOF
