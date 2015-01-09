# coding: utf8
# _dictman.py
# 10/10/2012 jichi

import re
from sakurakit import skstr

RIGHT_ARROW = u"→" # みぎ

def render_edict(text):
  """
  @param  text  unicode
  @return  unicode  html

  Example: /(n,adj-no) (1) center/centre/middle/heart/core/focus/pivot/emphasis/balance/(suf) (2) -centered/-centred/-focussed/-oriented/centered on/focussed on/(P)/
  """
  if text.startswith('/'):
    text = text[1:]
  if text.endswith('/'):
    text = text[:-1]
  role = ''
  if text.startswith('('): # extract leading word role
    i = text.find(')')
    if i != -1:
      role = text[1:i]
      text = text[i+1:]
  popular = text.endswith('(P)') # deal with trailing (popular) mark
  if popular:
    text = text[:-3]
  if text.endswith('/'): # replace (1) (2) ... by html list
    text = text[:-1]
  # Render list
  t = re.sub(r'\s?\(\d+\)\s?', '<li>', text)
  if t == text: # ul
    text = "<ul><li>%s</ul>" % text
  else: # ol
    if not t.startswith('<li>'):
      i = t.find('<li>')
      if i != -1:
        if role:
          role += ' ' + RIGHT_ARROW
        role += t[:i]
        t = t[i:]
    t = t.replace('/<', '<')
    text = "<ol>%s</ol>" % t
  # Render heading
  head = ''
  if role:
    head = '<span class="role">[%s]</span>' % role
  if popular:
    if head:
      head += ' '
    head += '<span class="flag">(common usage)</div>'
  if head:
    head = '<div class="head">%s</div>' % head
    text = head + text
  return text

def _render_lingoes(text, dic):
  """Render lingoes ja-zh dictionary.
  @param  text  unicode
  @param* dic  str  'ovdp', 'naver', 'vicon'
  @return  unicode  html

  Example: <C><F><H /><I><N><P><U>否定助动</U></P><Q>[接动词及动词型活用的助动词 「れる·られる，せる·させる」的未然形， 「する」 后接 「ない」 时要变成 「し」。</Q><Q>「ない」 的活用形式与形容词相同，但未然形 「なかろ」+「う」 的用法不常见，一般表示否定的推测时用 「ないだろう」]</Q><Q>[表示对前面(动词所表示的)动作或作用(的状态)的否定]不，没。<T><W>押して開かなければ引いてごらん</W><X>如果推不开，拉拉看。</X></T></Q><Q>→「…てはいけない」、「…てはならない」。</Q><Q>→「なければならない」、「なくてはならない」、「ないといけない」。</Q><Q>→ 「なくてもよい」。</Q><Q>以 「…ないか」 的形式，或向对方询问，确认，或征求对方同意，或劝诱对方，或表示说话人的愿望(省略 「か」 时句尾读升调)。<T><W>早く雪が降らないかなあ</W><X>怎幺还不快点儿下雪啊!</X></T></Q><Q>[以 「…ないで」 的形式向对方表示说话人否定的愿望或委婉的禁止。</Q><Q>这种用法类似终助词，可以认为后面省略了 「くれ」 或 「ください」] 别。<T><W>約束の時間に遅れないでよ</W><X>约好的时间可别晚了啊!</X></T></Q></N></I></F></C><br class="xml"/><C><F><H /><I><N><Y>ある</Y><Q /></N></I></F></C>
  """
  text = re.sub(r'(\[.*?\])', r'<span class="hl">\1</span> ', text) # highlight text in []
  text = text.replace('<T>', '<div>').replace('</T>', '</div>') # example sentence
  text = text.replace('</W><X>', '</W>%s<X>' % RIGHT_ARROW)
  #text = text.replace('<W>', '<div>').replace('</W>', '</div>') # example sentence text
  #text = text.replace('<X>', '<div>').replace('</X>', '</div>') # example sentence translation
  # Reading
  text = text.replace('<M>', ' <span class="reading">(')
  text = text.replace('</M>', ')</span> ')
  text = text.replace('<g>', ' <span class="reading">(')
  text = text.replace('</g>', ')</span> ')
  qc = text.count('<Q>')
  if qc > 1: # skip only 1 quote case
    text = text.replace('<Q>','<li>').replace('</Q>', '</li>') # quote => list
    #if qc == 1: # ul
    #  text = text.replace('<li>', '<ul><li>', 1)
    #  text = skstr.rreplace(text, '</li>', '</li></ul>', 1)
    #else: # ol
    text = text.replace('<li>', '<ol><li>', 1)
    text = skstr.rreplace(text, '</li>', '</li></ol>', 1)
  #return text
  if dic == 'naver':
    text = text.replace('<n />', '<br/>') # new line
    # Disabled as the original colors are ugly
    #text = text.replace('</x>', '</font>')
    #text = text.replace('<x>', '<font>')
    #text = text.replace('<x K=', '<font color=')
    text = text.replace('<E>', '<p class="entry">')
    text = text.replace('</E> ', '</p>')
  elif dic == 'vicon':
    text = text.replace('<N>', '<br/>') # new line
  # Recovered kanji
  return text
  #return skstr.escapehtml(text) + '<br/>' + text

# Example Vietnamese dictionary:
# ちょっと一杯
# <C><F><H /><K><![CDATA[<ul><li><font color='#cc0000'><b> {ちょっといっぱい}</b></font></li></ul><ul><li><font color='#cc0000'><b> {let's have quick drink}</b></font></li></ul>]] > </K></F></C>
def _simplify_ovdp_xml(text): # unicode -> unicode
  """
  @param  text  unicode
  @return  unicode
  """
  return (text
      .replace('<![CDATA[', '').replace(']] >', '').replace(']]>', '')
      .replace("<font color='#cc0000'>", '').replace('</font>', '')
      .replace('<ul>', '').replace('</ul>', '')
      .replace('<li>', '<br/>').replace('</li>', '<br/>')
      .replace('{', '').replace('}', ''))

def render_lingoes(text, dic=None):
  """Render lingoes ja-zh dictionary.
  @param  text  unicode
  @param* dic  str
  @return  unicode  html
  """
  if dic == 'ovdp':
    return _simplify_ovdp_xml(text)
  else:
    return _render_lingoes(text, dic)

# EOF
