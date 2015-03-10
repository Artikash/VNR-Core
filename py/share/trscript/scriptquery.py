# coding: utf8
# scriptquery.py
# 10/8/2012 jichi

__all__ = 'AbstractScriptQuery', 'ScriptQuery'

#from sakurakit.skprof import SkProfiler

class AbstractScriptQuery(object):
  def __init__(self, scriptData):
    """
    @param  scriptData  [_Term]
    """
    self.data = scriptData

  def iterData(self, *args, **kwargs):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @yield  _Term
    """
    data = self.data
    data = self.filterData(data, *args, **kwargs)
    data = self.sortData(data, *args, **kwargs)
    return data

  def filterData(self, data, type='', to='', fr=''):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @yield  _Term
    """
    for it in data:
      if (not type or type == it.type) and (not fr or fr == it.fr) and (not to or to == it.to):
        yield it

  def sortData(self, data, *args, **kwargs):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @yield  _Term
    """
    return data

class ScriptQuery(AbstractScriptQuery):
  def __init__(self, scriptData):
    """
    @param  scriptData  [_Term]
    """
    super(ScriptQuery, self).__init__(scriptData)

  def iterMacroData(self, to, fr):
    return self.iterSortedData(type='macro')

# EOF
