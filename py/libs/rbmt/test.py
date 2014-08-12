# coding: utf8
# 8/10/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

# Interface

class MachineTokenizer(object):
  def tokenize(self, text): pass # unicode -> list  cascaded lists of tokens

class MachineParser(object):
  def parse(self, tokens): pass # list -> tree

class MachineTranslator(object):
  def translate(self, tree): pass # tree -> tree

class MachineDriver(object):
  def learn(self, *args, **kwargs): pass
  def translate(self, text): pass # unicode -> unicode

# Japanese tokenizer

import token
# The same as token.py
TOKEN_TYPE_NULL = ''
TOKEN_TYPE_KANJI = 'kanji'
TOKEN_TYPE_RUBY = 'ruby'
TOKEN_TYPE_PUNCT = 'punct'
TOKEN_TYPE_LATIN = 'latin'

TOKEN_PUNCT_NULL = 0
TOKEN_PUNCT_STOP = 1 << 1 # period
TOKEN_PUNCT_PAUSE = 1 << 2 # comma

# Three-dot is a stop, two-dot is a pause
_PAUSE_PUNCT = u"、‥『』"
_STOP_PUNCT = u"。？！「」♡…"
def token_punct_flags(text):
  ret = 0
  if text in _PAUSE_PUNCT:
    ret |= TOKEN_PUNCT_PAUSE
  elif text in _STOP_PUNCT:
    ret |= TOKEN_PUNCT_STOP
  return ret

class Token(object):
  def __init__(self, text, type=TOKEN_TYPE_NULL, flags=0, feature=''):
    self.text = text # unicode
    self.feature = feature # unicode
    self.type = type # int
    self.flags = flags

class JapaneseToken(Token):
  def __init__(self, text, **kwargs):
    type = token.text_type(text)
    flags = 0
    if type == TOKEN_TYPE_PUNCT:
      flags = token_punct_flags(text)
    super(JapaneseToken, self).__init__(text=text, type=type, flags=flags, **kwargs)

class ChineseToken(Token):
  def __init__(self, text):
    super(ChineseToken, self).__init__(text=text)

class CaboChaTokenizer(MachineTokenizer):
  def __init__(self):
    super(CaboChaTokenizer, self).__init__()
    self.encoding = 'utf8' # cabocha dictionary encoding

    import CaboCha
    self.parser = CaboCha.Parser()

  def tokenize(self, text):
    """@reimp
    @param  text  unicode
    @return  [[JapaneseToken]]
    """
    ret = []
    encoding = self.encoding
    tree = self.parser.parse(text.encode(encoding))

    # Link is not accurate and ignored
    newgroup = False
    #link = 0
    group = []
    for i in xrange(tree.token_size()):
      token = tree.token(i)

      surface = token.surface.decode(encoding, errors='ignore')
      feature = token.feature.decode(encoding, errors='ignore')

      #if not i and token.chunk: # first element
      #  link = token.chunk.link

      if newgroup and token.chunk:
        ret.append(group)
        group = [JapaneseToken(text=surface, feature=feature)]
        #link = token.chunk.link
        newgroup = False
      else:
        group.append(JapaneseToken(text=surface, feature=feature))
      newgroup = True

    if group:
      #ret.append((group, link))
      ret.append(group) # link is ignored
    return ret

# Japanese parser

AST_PHRASE_NULL = "null"
AST_PHRASE_NOUN = "noun"
AST_PHRASE_VERB = "verb"
AST_PHRASE_ADJ = "adj" # adjective
AST_PHRASE_ADV = "adv" # adverb
AST_PHRASE_SUB = "sub" # suject
AST_PHRASE_OBJ = "obj" # object
AST_PHRASE_PRED = "pred" # predicate

AST_CLASS_NULL = "null"
AST_CLASS_NODE = "node"
AST_CLASS_CONTAINER = "container"
AST_CLASS_PARAGRAPH = "paragraph"
AST_CLASS_SENTENCE = "sentence"
AST_CLASS_CLAUSE = "clause"
AST_CLASS_PHRASE = "phrase"
AST_CLASS_WORD = "word"

class ASTNode(object): # abstract
  classType = AST_CLASS_NODE
  def __init__(self):
    self.parent = None # ASTNode  parent
    self.previous = None # ASTNode  sibling
    self.next = None # ASTNode  sibling

  def unparse(self): assert False; return "" # unicode

class ASTNull(ASTNode): # abstract
  classType = AST_CLASS_NULL
  def __init__(self):
    super(ASTNull, self).__init__()

  def unparse(self):
    """@reimp"""
    return ""

class ASTContainer(ASTNode): # abstract
  classType = AST_CLASS_CONTAINER
  def __init__(self, children=[]):
    super(ASTContainer, self).__init__()
    self.children = children # [ASTNode]
    for i,it in enumerate(children):
      it.parent = self
      if i > 0:
        it.previous = children[i-1]
      if i < len(children) - 1:
        it.next = children[i+1]

  def unparse(self):
    """@reimp"""
    return "".join((it.unparse()for it in self.children))

class ASTParagraph(ASTContainer): # container of sentences
  classType = AST_CLASS_PARAGRAPH
  def __init__(self, *args, **kwargs):
    super(ASTParagraph, self).__init__(*args, **kwargs)

class ASTSentence(ASTContainer): # container of phrases
  classType = AST_CLASS_SENTENCE
  def __init__(self, children=[], hasPunct=False):
    super(ASTSentence, self).__init__(children=children)
    self.hasPunct = hasPunct # bool

class ASTClause(ASTContainer): # container of phrases
  classType = AST_CLASS_CLAUSE
  def __init__(self, children=[], hasPunct=False):
    super(ASTClause, self).__init__(children=children)
    self.hasPunct = hasPunct # bool

class ASTPhrase(ASTContainer):
  classType = AST_CLASS_PHRASE
  def __init__(self, children=[], type=AST_PHRASE_NULL):
    super(ASTPhrase, self).__init__(children=children)
    self.type = type # str

class ASTWord(ASTNode):
  classType = AST_CLASS_WORD
  def __init__(self, token=None):
    super(ASTWord, self).__init__()
    self.token = token # Token

  def unparse(self):
    """@reimp"""
    return self.token.text if self.token else ''

class JapaneseParser(MachineParser):
  def __init__(self):
    super(JapaneseParser, self).__init__()

  def parse(self, tokens):
    """@reimp
    @param  tokens  [[Token]]
    @return  ASTNode
    """
    paragraph_sentences = []
    sentence_clauses = []
    clause_phrases = []
    for group in tokens:
      words = [ASTWord(token=token) for token in group]
      phrase = ASTPhrase(children=words)

      clause_phrases.append(phrase)

      last = group[-1]
      if last.type == TOKEN_TYPE_PUNCT:
        clause = ASTClause(children=clause_phrases, hasPunct=True)
        clause_phrases = []
        sentence_clauses.append(clause)
        if last.flags & TOKEN_PUNCT_STOP:
          sentence = ASTSentence(children=sentence_clauses, hasPunct=True)
          paragraph_sentences.append(sentence)
          paragraph_sentences = []

    if clause_phrases:
      clause = ASTClause(children=clause_phrases, hasPunct=False)
      sentence_clauses.append(clause)

    if sentence_clauses:
      sentence = ASTSentence(children=sentence_clauses, hasPunct=False)
      paragraph_sentences.append(sentence)

    ret = ASTParagraph(children=paragraph_sentences)
    return ret

# Japanese-Chinese translator

J2C_PUNCT = {
  u"、": u"，",
}

J2C_WORDS = {
  u"多く": u"许多",
}

class JapaneseChineseTranslator(MachineTranslator):
  def __init__(self):
    super(JapaneseChineseTranslator, self).__init__()

  def translate(self, tree):
    """@reimp"""
    if not tree:
      return ASTNull()
    ret = self.translateNode(tree)
    self.updateToken(ret)
    return ret

  # Context insensitive translation

  def updateToken(self, node):
    assert node
    if node.classType == AST_CLASS_WORD:
      token = node.token
      if token.type == TOKEN_TYPE_PUNCT:
        token.text = self.translatePunctuation(token.text)
    else:
      for it in node.children:
        self.updateToken(it)

  def translatePunctuation(self, text):
    return J2C_PUNCT.get(text) or text

  # Context sensitive translation

  def translateNode(self, node):
    assert node
    if node.classType == AST_CLASS_PARAGRAPH:
      return self.translateParagraph(node)
    else:
      return ASTNull()

  def translateParagraph(self, node):
    cls = ASTParagraph
    assert node and node.classType == cls.classType
    return cls(children=map(self.translateSentence, node.children))

  def translateSentence(self, node):
    cls = ASTSentence
    assert node and node.classType == cls.classType
    return cls(children=map(self.translateClause, node.children), hasPunct=node.hasPunct)

  def translateClause(self, node):
    cls = ASTClause
    assert node and node.classType == cls.classType
    return cls(children=map(self.translatePhrase, node.children), hasPunct=node.hasPunct)

  def translatePhraseOrWord(self, node):
    assert node
    if node.classType == AST_CLASS_PHRASE:
      return self.translatePhrase(node)
    if node.classType == AST_CLASS_WORD:
      return self.translateWord(node)
    assert False

  def translatePhrase(self, node):
    cls = ASTPhrase
    assert node and node.classType == cls.classType
    return cls(children=self.translatePhrasesOrWords(node.children))

  def translatePhrasesOrWords(self, nodes): # [ASTNode]
    if len(nodes) == 1:
      return [self.translatePhraseOrWord(nodes[0])]
    else:
      last = nodes[-1]
      if last.classType == AST_CLASS_WORD:
        token = last.token
        if token.text == u"で":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.insert(0, ASTWord(token=ChineseToken(u"在")))
          return l
        elif token.text == u"の":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.append(ASTWord(token=ChineseToken(u"的")))
          return l
        elif token.text == u"が":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.append(ASTWord(token=ChineseToken(u"在")))
          return l
      return map(self.translatePhraseOrWord, nodes)

  def translateWord(self, node):
    cls = ASTWord
    assert node and node.classType == cls.classType

    text = node.token.text

    next = node.next
    if text == u"大小" and next and next.classType == AST_CLASS_WORD and next.token.text == u"の":
      text = u"大大小小"

    text = J2C_WORDS.get(text) or text
    return cls(token=ChineseToken(text))

## Japanese-Chinese translator
#
#class JapaneseChineseDriver(MachineDriver):
#  def __init__(self):
#    super(JapaneseChineseDriver, self).__init__()
#
#    self.tokenizer = CaboChaTokenizer()
#    self.parser = JapaneseParser()
#    self.translator = JapaneseChineseTranslator()
#
#  def translate(self, text):
#    """@reimp"""
#    tokens = self.tokenizer.tokenize(text)
#    tree = self.parser.parse(tokens)
#    tree = self.translator.translate(tree)
#    return tree.unparse()

if __name__ == '__main__':
  #text = u"近未来の日本、多くの都市で大小の犯罪が蔓延。警察を主とした治安機関は機能を失いつつあったが、暁東市は様々な犯罪対策を行なう事で国内でもトップクラスの治安を保っていた。そのため、安全を求める人々が集り暁東市の物価は高騰、その結果資産家が多く住む街となった。そのような状況のため、資産家の令嬢と彼女ら（プリンシパル、護衛対象者）を守るボディーガードを同時に育成する教育機関が存在している。"

  # baidu: 未来日本在许多城市，大大小小的犯罪蔓延。
  # manual: 近未来的日本，大大小小的犯罪在许多城市蔓延。
  text = u"近未来の近未来の日本、多くの多くの都市で大小の大小の多くの犯罪が蔓延。"

  print "-- test tokenizer --"
  _t = CaboChaTokenizer()
  tokens = _t.tokenize(text)
  for i,it in enumerate(tokens):
    print i, ','.join(t.text for t in it)

  print "-- test parser --"

  _p = JapaneseParser()
  tree = _p.parse(tokens)
  print tree.unparse()

  print "-- test translator --"
  _t = JapaneseChineseTranslator()
  tree = _t.translate(tree)
  print tree.unparse()

  #print "-- test driver --"
  #_d = JapaneseChineseDriver()
  #trans = _d.translate(text)
  #print trans


# EOF
