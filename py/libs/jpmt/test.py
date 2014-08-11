# coding: utf8
# 8/10/2014

# Interface

class MachineTokenizer(object):
  def tokenize(self, text): pass # unicode -> list

class MachineParser(object):
  def parse(self, tokens): pass # list -> tree

class MachineTranslator(object):
  def translate(self, tree): pass # tree -> tree

class MachineDriver(object):
  def learn(self, *args, **kwargs): pass
  def translate(self, text): pass # unicode -> unicode

# Japanese tokenizer

class JapaneseTokenizer(MachineTokenizer):
  def __init__(self):
    super(JapaneseTokenizer, self).__init__()

  def tokenize(self, text):
    """@reimp"""
    ret = [
      u"近未来",
      u"の",
      u"日本",
      u"、",
      u"多く",
      u"の",
      u"都市",
      u"で",
      u"大小",
      u"の",
      u"犯罪",
      u"が",
      u"蔓延",
      u"。"
    ]
    return ret

# Japanese parser

AST_PHRASE_NOUN = "noun"
AST_PHRASE_VERB = "verb"
AST_PHRASE_ADJ = "adj" # adjective
AST_PHRASE_ADV = "adv" # adverb
AST_PHRASE_SUB = "sub" # suject
AST_PHRASE_OBJ = "obj" # object
AST_PHRASE_PRED = "pred" # predicate

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

class ASTSentence(ASTContainer): # container of clauses
  classType = AST_CLASS_SENTENCE
  def __init__(self, *args, **kwargs):
    super(ASTSentence, self).__init__(*args, **kwargs)

class ASTClause(ASTContainer): # container of phrases
  classType = AST_CLASS_CLAUSE
  def __init__(self, children=[], punctuation=""):
    super(ASTClause, self).__init__(children=children)
    self.punctuation = punctuation # unicode

  def unparse(self):
    """@reimp"""
    return super(ASTClause, self).unparse() + self.punctuation

class ASTPhrase(ASTContainer):
  classType = AST_CLASS_PHRASE
  def __init__(self, children=[], role=""):
    super(ASTPhrase, self).__init__(children=children)
    self.role = role # str

class ASTWord(ASTNode):
  classType = AST_CLASS_WORD
  def __init__(self, text=""):
    super(ASTWord, self).__init__()
    self.text = text # unicode

  def unparse(self):
    """@reimp"""
    return self.text

class JapaneseParser(MachineParser):
  def __init__(self):
    super(JapaneseParser, self).__init__()

  def parse(self, tokens):
    """@reimp"""
    #  近未来の日本、
    future = ASTWord(u"近未来")
    no = ASTWord(u"の")
    adj = ASTPhrase(children=[future, no], role=AST_PHRASE_ADJ)
    jp = ASTWord(u"日本")
    time = ASTPhrase(children=[adj, jp], role=AST_PHRASE_NOUN)

    clause1 = ASTClause(children=[time], punctuation=u"、")

    # 多くの都市で大小の犯罪が蔓延。

    many = ASTWord(u"多く")
    no = ASTWord(u"の")
    adj = ASTPhrase(children=[many, no], role=AST_PHRASE_ADJ)
    city = ASTWord(u"都市")
    place = ASTPhrase(children=[adj, city], role=AST_PHRASE_NOUN)
    de = ASTWord(u"で")
    where = ASTPhrase(children=[place, de], role=AST_PHRASE_ADV)

    couple = ASTWord(u"大小")
    no = ASTWord(u"の")
    adj = ASTPhrase(children=[couple, no], role=AST_PHRASE_ADJ)
    crime = ASTWord(u"犯罪")
    noun = ASTPhrase(children=[adj, crime], role=AST_PHRASE_NOUN)
    ga = ASTWord(u"が")
    subject = ASTPhrase(children=[noun, ga], role=AST_PHRASE_SUB)

    spread = ASTWord(u"蔓延")
    predicate = ASTPhrase(children=[spread], role=AST_PHRASE_PRED)

    clause2 = ASTClause(children=[where, subject, predicate], punctuation=u"。")

    sentence = ASTSentence(children=[clause1, clause2])
    root = ASTParagraph(children=[sentence])
    return root

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
    if tree:
      return self.translateNode(tree)

  def translateNode(self, node):
    assert node
    if node.classType == AST_CLASS_PARAGRAPH:
      return self.translateParagraph(node)

  def translateParagraph(self, node):
    cls = ASTParagraph
    assert node and node.classType == cls.classType
    return cls(children=map(self.translateSentence, node.children))

  def translateSentence(self, node):
    cls = ASTSentence
    assert node and node.classType == cls.classType
    return cls(children=map(self.translateClause, node.children))

  def translateClause(self, node):
    cls = ASTClause
    assert node and node.classType == cls.classType
    return cls(
        children=map(self.translatePhrase, node.children),
        punctuation=self.translatePunctuation(node.punctuation))

  def translatePunctuation(self, text):
    return J2C_PUNCT.get(text) or text

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
        if last.text == u"で":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.insert(0, ASTWord(text=u"在"))
          return l
        elif last.text == u"の":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.append(ASTWord(text=u"的"))
          return l
        elif last.text == u"が":
          l = self.translatePhrasesOrWords(nodes[:-1])
          l.append(ASTWord(text=u"在"))
          return l
      return map(self.translatePhraseOrWord, nodes)

  def translateWord(self, node):
    cls = ASTWord
    assert node and node.classType == cls.classType

    text = node.text

    next = node.next
    if text == u"大小" and next and next.classType == AST_CLASS_WORD and next.text == u"の":
      text = u"大大小小"

    text = J2C_WORDS.get(text) or text
    return cls(text=text)

# Japanese-Chinese translator

class JapaneseChineseDriver(MachineDriver):
  def __init__(self):
    super(JapaneseChineseDriver, self).__init__()

    self.tokenizer = JapaneseTokenizer()
    self.parser = JapaneseParser()
    self.translator = JapaneseChineseTranslator()

  def translate(self, text):
    """@reimp"""
    tokens = self.tokenizer.tokenize(text)
    tree = self.parser.parse(tokens)
    tree = self.translator.translate(tree)
    return tree.unparse()

if __name__ == '__main__':
  # baidu: 未来日本在许多城市，大大小小的犯罪蔓延。
  # manual: 近未来的日本，大大小小的犯罪在许多城市蔓延。
  text = u"近未来の日本、多くの都市で大小の犯罪が蔓延。"

  m = JapaneseChineseDriver()

  trans = m.translate(text)
  print trans

# EOF
