# coding: utf8
# jlpman.py
# 3/28/2014 jichi

from mecabjlp import mecabdef
from sakurakit.skclass import memoized

@memoized
def manager(): return JlpManager()

class JlpManager:

  def __init__(self):
    self.ruby = mecabdef.RB_HIRA # str


  def setRuby(self, v): self.ruby = v # str

  def parseToRuby(self, text): #
    return [
      [
        ("助平","すけべい", "feature1", 'word1'),
        ("助平1","すけべい2", "feature2", 'word2'),
      ],
    ]

# EOF
