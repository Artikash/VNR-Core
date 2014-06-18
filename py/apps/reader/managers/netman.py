# coding: utf8
# netman.py
# 10/17/2012 jichi
#
# About requests:
# - Gzip are automatically decoded by requests according to:
# http://docs.python-requests.org/en/latest/user/quickstart/
# - requests uses urllib3, which is thread-safe
# http://docs.python-requests.org/en/latest/

import operator
import requests
from datetime import datetime
from functools import partial
from cStringIO import StringIO
#import xml.etree.cElementTree as etree
from lxml import etree
from PySide.QtCore import Signal, QObject
from PySide.QtNetwork import QNetworkConfigurationManager
from sakurakit import skthreads
from sakurakit.skclass import Q_Q, memoized, memoizedproperty, memoizedmethod_filter
from sakurakit.skdebug import dprint, dwarn, derror
from sakurakit.sknetio import GZIP_HEADERS
from mytr import my
from sysinfo import timestamp2jst
import config, dataman, defs, features, growl

API = config.API_REST
#API = "http://localhost:5000/api/1"
#API = "http://localhost:5000/api/1"

# Return true when context-type is xml or is None
def _response_is_xml(r):
  try: return "/xml" in r.headers['content-type'].lower()
  except (KeyError, TypeError, AttributeError): return True

POST_HEADERS = {
  'accept-encoding': 'gzip',
  'content-type': 'application/x-www-form-urlencoded',
}

@Q_Q
class _NetworkManager(object):
  def __init__(self):
    self._online = None # cached online status from qncm
    self.version = 0    # long timestamp

    self.cachedGamesById = {} # {long id:dataman.Game}
    self.cachedGamesByMd5 = {} # {str id:dataman.Game}
    self.blockedLanguages = set() # set(str) not None

    # Track online status when network is down
    #self._onlineTimer = QTimer(self.q)
    #self._onlineTimer.setInterval(5000) # update every 5 seconds
    #self._onlineTimer.timeout.connect(self._updateOnline)

    # Update online on startup
    self.online

  def _addBlockedLanguages(self, params):
    """
    @param  params  dict
    """
    if self.blockedLanguages:
      params['nolang'] = ','.join(self.blockedLanguages).replace('zh', 'zhs,zht')

  ## Filters ##

  #@staticmethod
  #def filterCommentText(text):
  #  """
  #  @param  unicode or None
  #  @return  unicode
  #  """
  #  return text.replace('\\sub', '').lstrip()

  ## Online status ##

  @memoizedproperty
  def qncm(self):
    ret = QNetworkConfigurationManager(self.q)
    ret.onlineStateChanged.connect(self._setOnline)
    return ret

  @property
  def online(self):
    if self._online is None:
      self.updateOnline()
    return self._online

  def updateOnline(self):
    self._setOnline(self.qncm.isOnline())

  def _setOnline(self, online):
    if features.INTERNET_CONNECTION == defs.INTERNET_CONNECTION_ON:
      online = True
      dwarn("force enabling Internet CONNECTION")
      growl.notify(my.tr("Always enable Internet access as you wish"))
    elif features.INTERNET_CONNECTION == defs.INTERNET_CONNECTION_OFF:
      online = False
      dwarn("force disabling Internet access")
      growl.notify(my.tr("Always disable Internet access as you wish"))
    if self._online != online:
      self._online = online
      dprint("online = %s" % online)

      if online:
        growl.msg(my.tr("Connected to the Internet"))
      else:
        growl.notify(my.tr("Disconnected from the Internet"))
      self.q.onlineChanged.emit(online)

  ## App ##

  @property
  @memoizedmethod_filter(bool)
  def cachedVersion(self):
     return skthreads.runsync(self.queryVersion, parent=self.q)

  def queryVersion(self):
    params = {'ver': self.version}
    try:
      r = requests.get(API + '/app/version', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        #app = root.find('./apps/app[@id="%i"]' % config.VERSION_ID)
        app = root.find('./apps/app[@name="vnr"]')
        timestamp = int(app.find('./timestamp').text)
        dprint("timestamp = %i" % timestamp)
        return timestamp

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url)
    except: pass
    return 0

  def getUpdateMessage(self, lang):
    """
    @param  lang  str
    @return  unicode or None
    """
    params = {'ver': self.version}
    if lang not in ('en', 'zhs', 'zht'):
      lang = 'en'
    try:
      r = requests.get(API + '/app/msg', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        #app = root.find('./apps/app[@id="%i"]' % config.VERSION_ID)
        for msg in root.iterfind('./messages/message[@app="vnr"][@language="%s"]' % lang):
          timestamp = int(msg.find('./timestamp').text)
          if timestamp > self.version:
            text = msg.find('./text').text
            return text

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url)
    except: pass

  ## User #

  def queryUser(self, userName, password):
    assert userName and password, "missing user name or password "
    params = {'ver':self.version, 'login':userName, 'password':password}
    try:
      r = requests.get(API + '/user/query', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        user = root.find('./users/user')

        ret = dataman.User()
        ret.id = int(user.get('id'))
        for e in user:
          tag = e.tag
          if tag in ('name', 'language', 'gender', 'color', 'avatar', 'homepage'):
            setattr(ret, tag, e.text or '')
          elif tag == 'levels':
            for it in e:
              if it.tag in ('term', 'comment'):
                setattr(ret, it.tag + 'Level', int(it.text))

        ret.password = password

        dprint("user name = %s" % ret.name)
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def updateUser(self, userName, password, language=None, gender=None, avatar=None, color=None, homepage=None):
    """Return if succeeded"""
    assert userName and password, "missing user name or password "
    params = {'ver':self.version, 'login':userName, 'password':password}
    if language:
      params['lang'] = language
    if gender:
      params['gender'] = gender
    if avatar:
      params['avatar'] = avatar
    if homepage:
      params['homepage'] = homepage
    if color is not None:
      if color:
        params['color'] = color
      else:
        params['delcolor'] = True

    try:
      r = requests.post(API + '/user/update', data=params, headers=POST_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        user = root.find('./users/user')
        userId = int(user.get('id'))

        dprint("user id = %i" % userId)
        return userId != 0

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return False

  ## Game ##

  #def getGameFiles(self):
  #  """
  #  @return [str md5] or None
  #  """
  #  params = {'ver': self.version}
  #  try:
  #    r = requests.get(API + '/game/md5', params=params, headers=GZIP_HEADERS)
  #    if r.ok:
  #      c = r.content
  #      if c:
  #        ret = c.split('\n')
  #        if len(ret) > 100:
  #          dprint("count = %i" % len(ret))
  #          return ret

  #  except socket.error, e:
  #    dwarn("socket error", e.args)
  #  except requests.ConnectionError, e:
  #    dwarn("connection error", e.args)
  #  except requests.HTTPError, e:
  #    dwarn("http error", e.args)

  #  dwarn("failed URL follows")
  #  try: dwarn(r.url)
  #  except: pass

  def getGameItems(self):
    """
    @return {long itemId:dataman.GameItem} or None
    """
    params = {'ver': self.version}
    try:
      r = requests.get(API + '/item/list', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        TYPES = dataman.Reference.TYPES
        ret = {}
        items = root.find('./items')
        if items is not None:
          for item in items:
            e = dataman.GameItem(id=int(item.get('id')))
            for el in item:
              tag = el.tag
              text = el.text
              if tag in ('title', 'romajiTitle', 'brand', 'series', 'image', 'tags', 'artists', 'sdartists', 'writers', 'musicians'):
                setattr(e, tag, text)
              elif tag in ('otome', 'okazu'):
                setattr(e, tag, text == 'true')
              elif tag == 'timestamp':
                setattr(e, tag, long(text))
              elif tag == 'date':
                e.date = datetime.strptime(text, '%Y%m%d')
              elif tag == 'scape':
                for it in el:
                  if it.tag in ('count', 'median'):
                    setattr(e, 'scape' + it.tag.capitalize(), int(it.text))
            ret[e.id] = e
        dprint("game item count = %i" % len(ret))
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  #def getReferenceDigests(self):
  #  """
  #  @return {long itemId:[dataman.ReferenceDigest]} or None
  #  """
  #  params = {'ver': self.version}
  #  try:
  #    r = requests.get(API + '/ref/list', params=params, headers=GZIP_HEADERS)
  #    if r.ok and _response_is_xml(r):
  #      root = etree.fromstring(r.content)
  #      TYPES = dataman.Reference.TYPES
  #      ret = {}
  #      refs = root.find('./references')
  #      if refs is not None:
  #        TYPES = dataman.Reference.TYPES
  #        for ref in refs:
  #          r = dataman.ReferenceDigest(
  #              id = int(ref.get('id')),
  #              type = ref.get('type'))
  #          for el in ref:
  #            tag = el.tag
  #            text = el.text
  #            if tag in ('itemId', 'gameId', 'timestamp'):
  #              setattr(r, tag, int(text))
  #            elif tag in ('key', 'title', 'brand', 'image'):
  #              setattr(r, tag, text or '')
  #            elif tag == 'date':
  #              r.date = timestamp2jst(int(text))
  #          if r.type in TYPES:
  #            try: ret[r.itemId].append(r)
  #            except KeyError: ret[r.itemId] = [r]
  #      dprint("game item count = %i" % len(ret))
  #      return ret

  #  except socket.error, e:
  #    dwarn("socket error", e.args)
  #  except requests.ConnectionError, e:
  #    dwarn("connection error", e.args)
  #  except requests.HTTPError, e:
  #    dwarn("http error", e.args)
  #  except etree.ParseError, e:
  #    dwarn("xml parse error", e.args)
  #  except KeyError, e:
  #    dwarn("invalid response header", e.args)
  #  except (TypeError, ValueError, AttributeError), e:
  #    dwarn("xml malformat", e.args)
  #  except Exception, e:
  #    derror(e)

  #  dwarn("failed URL follows")
  #  try: dwarn(r.url, params)
  #  except: pass

  def getGameFiles(self):
    """
    @return {long id:dataman.GameFile} or None
    """
    params = {'ver': self.version}
    try:
      r = requests.get(API + '/game/list', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        ret = {}
        games = root.find('./games')
        if games is not None:
          for game in games:
            g = dataman.GameFile(id=int(game.get('id')))
            for el in game:
              tag = el.tag
              if tag in ('itemId', 'visitCount', 'commentCount'):
                setattr(g, tag, int(el.text))
              elif tag == 'md5':
                g.md5 = el.text
              elif tag == 'names':
                e = el.find('./name[@type="file"]')
                if e is not None:
                  g.name = e.text
            ret[g.id] = g
        dprint("game count = %i" % len(ret))
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def getUsers(self):
    """
    @return {long id:dataman.UserDigest} or None
    """
    params = {'ver': self.version}
    try:
      r = requests.get(API + '/user/list', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        context = etree.iterparse(StringIO(r.content), events=('start','end'))

        ret = {}
        path = 0
        for event, elem in context:
          if event == 'start':
            path += 1
            if path == 3: # grimoire/users/user
              u = dataman.UserDigest(id=int(elem.get('id')))
          else:
            path -= 1
            if path == 3: # grimoire/users/user
              tag = elem.tag
              if tag in ('name', 'language', 'gender', 'avatar', 'color', 'homepage'):
                setattr(u, tag, elem.text or '')
              elif tag == 'levels':
                for it in elem:
                  if it.tag in ('term', 'comment'):
                    setattr(u, it.tag + 'Level', int(it.text))
            elif path == 2:
              ret[u.id] = u

        #dprint("user count = %i" % len(ret))
        dprint("succeed")
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def queryGame(self, id=None, md5=None):
    assert id or md5, "missing id and digest"
    params = {'ver': self.version}
    if id:
      params['id'] = id
    else:
      params['md5'] = md5

    try:
      r = requests.get(API + '/game/query', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        game = root.find('./games/game')

        ret = dataman.Game()
        ret.id = int(game.get('id'))

        for e in game:
          tag = e.tag
          if tag in ('md5', 'encoding', 'language'):
            setattr(ret, tag, e.text or '')
          elif tag in ('itemId', 'commentCount'):
            setattr(ret, tag, int(e.text))
          elif tag in ('removesRepeat', 'ignoresRepeat', 'keepsSpace', 'threadKept'):
            setattr(ret, tag, e.text == 'true')
          elif tag == 'hook':
            if e.get('deleted') == 'true':
              ret.deletedHook = e.text
            else:
              ret.hook = e.text
              #if e.get('kept') == 'true':
              #  ret.hookKept = True
          elif tag == 'threads':
            thread = e.find('./thread[@type="scene"]')
            if thread is not None and thread.get('deleted') != 'true':
              ret.threadName = thread.find('./name').text
              ret.threadSignature = long(thread.find('./signature').text)
            thread = e.find('./thread[@type="name"]')
            if thread is not None and thread.get('deleted') != 'true':
              ret.nameThreadName = thread.find('./name').text
              ret.nameThreadSignature = long(thread.find('./signature').text)
          elif tag == 'names':
            ret.names = {
              t:[it.text for it in e.iterfind('./name[@type="%s"]' % t)]
              for t in ret.NAME_TYPES
            }

        if not ret.language: ret.language = 'ja'

        dprint("game id = %i" % ret.id)
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def updateGame(self, game, userName, password, deleteHook=False):
    """Return if succeeded"""
    assert userName and password, "missing user name or password"
    assert game and (game.id or game.md5), "missing id and digest"

    params = {'ver':self.version, 'login':userName, 'password':password}

    #if game.md5 and len(game.md5) != defs.MD5_HEX_LENGTH:
    #  assert False, "invalid md5 digest"

    if game.id:               params['id']          = game.id
    if game.md5:              params['md5']         = game.md5
    if game.encoding:         params['encoding']    = game.encoding
    if game.language:         params['lang']        = game.language
    if game.hook:             params['hook']        = game.hook
    if game.threadName:       params['threadname']  = game.threadName
    if game.threadSignature:  params['threadsig']   = game.threadSignature
    if game.nameThreadName:      params['namethreadname'] = game.nameThreadName
    if game.nameThreadSignature: params['namethreadsig']  = game.nameThreadSignature
    if deleteHook:            params['delhook'] = True
    #if game.hookKept is not None: params['keephook'] = game.hookKept
    if game.threadKept is not None: params['keepthread'] = game.threadKept
    if game.keepsSpace is not None:  params['keepspace'] = game.keepsSpace
    if game.removesRepeat is not None:  params['removerepeat'] = game.removesRepeat
    if game.ignoresRepeat is not None:  params['ignorerepeat'] = game.ignoresRepeat
    if game.nameThreadDisabled is not None: params['delnamethread'] = game.nameThreadDisabled

    for t in game.names:
      if game.names[t]:       params['%sname' % t]      = game.names[t]

    try:
      r = requests.post(API + '/game/update', data=params, headers=POST_HEADERS)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        e_game = root.find('./games/game')
        game.id = int(e_game.get('id'))
        game.md5 = e_game.find('./md5').text

        dprint("game id = %i" % game.id)
        return game.id != 0

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return False

  ## References ##

  def queryReferences(self, gameId=None, md5=None, init=True):
    """
    @param  init  bool whether init comment object
    @param  hash  bool
    @return  [Reference] or None
    """
    assert gameId or md5, "missing id and digest"
    params = {'ver':self.version}
    if gameId:
      params['gameid'] = gameId
    else:
      params['md5'] = md5

    try:
      r = requests.get(API + '/ref/query', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        #root = etree.fromstring(r.content)
        GUEST_ID = dataman.GUEST.id

        context = etree.iterparse(StringIO(r.content), events=('start','end'))

        ret = []
        path = 0
        TYPES = dataman.Reference.TYPES
        for event, elem in context:
          if event == 'start':
            path += 1
            if path == 3: # grimoire/references/reference
              kw = {
                #'comment': "",
                #'updateComment': "",
                #'updateTimestamp':  0,
                #'updateUserId': 0,
                #'disabled': False,
                'id': int(elem.get('id')),
                'type': elem.get('type'),
              }
          else:
            path -= 1
            if path == 3: # grimoire/references/reference
              tag = elem.tag
              text = elem.text
              if tag in ('key', 'title', 'brand', 'url', 'comment', 'updateComment'): #'image'
                kw[tag] = text or ''
              elif tag in ('itemId', 'gameId', 'userId', 'userHash', 'timestamp', 'updateUserId', 'updateTimestamp'):
                kw[tag] = int(text)
              elif tag == 'disabled':
                kw[tag] = text == 'true'
              elif tag == 'date':
                kw[tag] = timestamp2jst(int(text))
              #else:
              #  kw[tag] = text or ''

            elif path == 2 and kw['type'] in TYPES: # grimoire/references
              #if not kw.get('userHash'):
              #  kw['userHash'] = kw['userId']
              ret.append(dataman.Reference(init=init, **kw))

        dprint("reference count = %i" % len(ret))
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def submitReference(self, ref, userName, password, md5=None, async=False):
    """Update reference if succeeded
    @return  (gameId, itemId) or None
    """
    assert userName and password, "missing user name or password"
    assert ref and (ref.gameId or md5), "missing game id and digest"

    params = {
      'ver': self.version,
      'login': userName,
      'password': password,
      'type': ref.type,
      'key': ref.key,
      'url': ref.url,
      'title': ref.title
          if len(ref.title) <= defs.MAX_TEXT_LENGTH
          else ref.title[:defs.MAX_TEXT_LENGTH],
    }
    if ref.brand:
      params['brand'] = ref.brand
    if ref.date:
      params['date'] = ref.date
    if ref.image:
      params['image'] = ref.image
    #if ref.itemId:
    #  params['itemId'] = ref.itemId
    if ref.gameId:
      params['gameid'] = ref.gameId
    else:
      params['md5'] = md5 or ref.gameMd5

    if ref.comment:
      params['comment'] = (ref.comment
          if len(ref.comment) <= defs.MAX_TEXT_LENGTH
          else ref.comment[:defs.MAX_TEXT_LENGTH])

    if ref.updateComment:
      params['updatecomment'] = (ref.updateComment
          if len(ref.updateComment) <= defs.MAX_TEXT_LENGTH
          else ref.updateComment[:defs.MAX_TEXT_LENGTH])

    if ref.deleted:
      # Should never happen. I mean, deleted subs should have been skipped in dataman
      params['del'] = True
    if ref.disabled:
      params['disable'] = True

    try:
      if not async:
        r = requests.post(API + '/ref/submit', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/ref/submit', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./references/reference')

        # Be careful about async here
        ref.id = int(el.get('id'))
        ref.itemId = int(el.find('itemId').text)

        gameId = itemId = 0
        el = root.find('./games/game')
        if el:
          gameId = int(el.get('id'))
          itemId = int(el.find('itemId').text)

        dprint("ref id = %i" % ref.id)
        return gameId, itemId

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def updateReference(self, ref, userName, password, async=False):
    """
    @return  (int gameId, int itemId) or None
    """
    assert userName and password, "missing user name or password"
    assert ref and ref.id, "missing reference id"

    params = {}
    pty = ref.dirtyProperties()
    if not pty:
      dwarn("warning: reference to update is not dirty")
      return ref.itemId

    if 'deleted' in pty:        params['del'] = ref.deleted
    if not ref.deleted:
      if 'disabled' in pty:     params['disable'] = ref.disabled

      for k in 'comment', 'updateComment':
        if k in pty:
          v = getattr(ref, k)
          if v:
            params[k.lower()] = (v
                if len(v) <= defs.MAX_TEXT_LENGTH
                else v[:defs.MAX_TEXT_LENGTH])
          else:
            params['del' + k.lower()] = True

    if not params:
      dwarn("warning: nothing change")
      return ref.itemId

    params['login'] = userName
    params['password'] = password
    params['id'] = ref.id
    params['ver'] = self.version

    try:
      if not async:
        r = requests.post(API + '/ref/update', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/ref/update', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./references/reference')
        refId = int(el.get('id'))
        if refId:
          gameId = itemId = 0
          el = root.find('./games/game')
          if el:
            gameId = int(el.get('id'))
            itemId = int(el.find('itemId').text)
          dprint("ref id = %i" % refId)
          return gameId, itemId

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  ## Comment ##

  def queryComments(self, gameId=None, md5=None, init=True, hash=True):
    """
    @param  init  bool whether init comment object
    @param  hash  bool
    @return  {long:Comment} or [Comment] or None
    """
    assert gameId or md5, "missing id and digest"
    params = {'ver':self.version}
    if gameId:
      params['gameid'] = gameId
    else:
      params['md5'] = md5

    self._addBlockedLanguages(params)

    try:
      r = requests.get(API + '/comment/query', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        #root = etree.fromstring(r.content)

        ret = {} if hash else []

        context = etree.iterparse(StringIO(r.content), events=('start','end'))
        path = 0
        TYPES = dataman.Comment.TYPES
        for event, elem in context:
          if event == 'start':
            path += 1
            if path == 3: # grimoire/comments/comment
              kw = {
                #'comment': "",
                #'updateComment': "",
                #'updateTimestamp':  0,
                #'updateUserId': 0,
                #'disabled': False,
                #'locked': False,
                'id': int(elem.get('id')),
                'type': elem.get('type'),
              }
          else:
            path -= 1
            if path == 3: # grimoire/comments/comment
              tag = elem.tag
              text = elem.text
              if tag in ('gameId', 'userId', 'userHash', 'timestamp', 'updateUserId', 'updateTimestamp'):
                kw[tag] = int(text)
              elif tag in ('disabled', 'locked'):
                kw[tag] = text == 'true'
              else:
                kw[tag] = text or ''

              if tag == 'context':
                kw['hash'] = long(elem.get('hash'))
                kw['contextSize'] = int(elem.get('size'))

            elif path == 2 and kw['type'] in TYPES: # grimoire/comments
              #c = dataman.Comment(init=init,
              #  id=p['id'], type=p['type'], gameId=p['gameId'], userId=p['userId'],
              #  language=p['language'], timestamp=p['timestamp'], disabled=p['disabled'], locked=p['locked'],
              #  updateTimestamp=p['updateTimestamp'], updateUserId=p['updateUserId'],
              #  text=p['text'], context=p['context'], hash=p['hash'], contextSize=p['contextSize'],
              #  comment=p['comment'], updateComment=p['updateComment'])
              #if not kw.get('userHash'):
              #  kw['userHash'] = kw['userId']
              c = dataman.Comment(init=init, **kw)
              if not hash:
                ret.append(c)
              else:
                h = kw['hash']
                if h in ret:
                  ret[h].append(c)
                else:
                  ret[h] = [c]

        dprint("comment count = %i" % len(ret))
        return ret

        #comments = root.find('./comments')
        #if comments is not None:
        #  for comment in comments.iterfind('./comment'):
        #    c_type = comment.get('type')
        #    if not c_type in dataman.Comment.TYPES:
        #      continue
        #    c_id = long(comment.get('id'))
        #    c_gameId = long(comment.find('./gameId').text)
        #    c_userName = comment.find('./userName').text
        #    c_language = comment.find('./language').text
        #    c_timestamp = long(comment.find('./timestamp').text)

        #    e = comment.find('./updateTimestamp')
        #    c_updateTimestamp = long(e.text) if e is not None else 0

        #    e = comment.find('./updateUserName')
        #    c_updateUserName = e.text if e is not None else ''

        #    e = comment.find('./disabled')
        #    c_disabled = e is not None and e.text == 'true'

        #    e = comment.find('./locked')
        #    c_locked = e is not None and e.text == 'true'

        #    #e = comment.find('./popup')
        #    #c_popup = e is not None and e.text == 'true'

        #    c_text = comment.find('./text').text
        #    #if '\\' in c_text:
        #    #  c_text = _NetworkManager.filterCommentText(c_text)

        #    ctx = comment.find('./context')
        #    c_context = ctx.text
        #    c_hash = long(ctx.get('hash'))
        #    c_contextSize = int(ctx.get('size'))

        #    e = comment.find('./comment')
        #    c_comment = e.text if e is not None else ""
        #    e = comment.find('./updateComment')
        #    c_updateComment = e.text if e is not None else ""

        #    if c_hash:
        #      #if c_context:
        #      #  contexts[c_hash] = c_context
        #      if c_text:
        #        c = dataman.Comment(init=init,
        #          id=c_id, type=c_type, gameId=c_gameId, userName=c_userName,
        #          language=c_language, timestamp=c_timestamp, disabled=c_disabled, locked=c_locked,
        #          updateTimestamp=c_updateTimestamp, updateUserName=c_updateUserName,
        #          text=c_text, context=c_context, hash=c_hash, contextSize=c_contextSize,
        #          comment=c_comment, updateComment=c_updateComment)

        #        if not hash:
        #          ret.append(c)
        #        else:
        #          if c_hash in ret:
        #            ret[c_hash].append(c)
        #          else:
        #            ret[c_hash] = [c]

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass

  def submitComment(self, comment, userName, password, md5=None, async=False):
    """Return comment and update comment if succeeded"""
    assert userName and password, "missing user name or password"
    assert comment and (comment.gameId or md5), "missing game id and digest"

    params = {
      'ver': self.version,
      'login': userName,
      'password': password,
      'lang': comment.language,
      'type': comment.type,
      'ctxhash': comment.hash,
      'ctxsize': comment.contextSize,
      'text': comment.text
          if len(comment.text) <= defs.MAX_TEXT_LENGTH
          else comment.text[:defs.MAX_TEXT_LENGTH],
    }
    if comment.gameId:
      params['gameid'] = comment.gameId
    else:
      params['md5'] = md5 or comment.gameMd5

    if comment.comment:
      params['comment'] = (comment.comment
          if len(comment.comment) <= defs.MAX_TEXT_LENGTH
          else comment.comment[:defs.MAX_TEXT_LENGTH])

    if comment.updateComment:
      params['updatecomment'] = (comment.updateComment
          if len(comment.updateComment) <= defs.MAX_TEXT_LENGTH
          else comment.updateComment[:defs.MAX_TEXT_LENGTH])

    if comment.context:
      params['ctx'] = (comment.context
          if len(comment.context) <= defs.MAX_TEXT_LENGTH
          else comment.context[:defs.MAX_TEXT_LENGTH])
    if comment.deleted:
      # Should never happen. I mean, deleted subs should have been skipped in dataman
      params['del'] = True
    if comment.disabled:
      params['disable'] = True
    if comment.locked:
      params['lock'] = True
    #if comment.popup:
    #  params['popup'] = True

    try:
      if not async:
        r = requests.post(API + '/comment/submit', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/comment/submit', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./comments/comment')

        # Be careful about async here
        comment.id = int(el.get('id'))

        dprint("comment id = %i" % comment.id)
        return comment.id

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return 0

  def updateComment(self, comment, userName, password, async=False):
    """Return if succeeded"""
    assert userName and password, "missing user name or password"
    assert comment and comment.id, "missing comment id"

    params = {}
    pty = comment.dirtyProperties()
    if not pty:
      dwarn("warning: comment to update is not dirty")
      return True

    if 'deleted' in pty:        params['del'] = comment.deleted
    if not comment.deleted:
      for k,v in (
          ('type', 'type'),
          ('language', 'lang'),
          ('hash', 'ctxhash'),
          ('contextSize', 'ctxsize'),
          ('disabled', 'disable'),
          ('locked', 'lock'),
        ):
        if k in pty:
          params[v] = getattr(comment, k)

      if 'text' in pty:
        params['text'] = (comment.text
            if len(comment.text) <= defs.MAX_TEXT_LENGTH
            else comment.text[:defs.MAX_TEXT_LENGTH])
      if 'context' in pty and comment.context:
        params['ctx'] = (comment.context
            if len(comment.context) <= defs.MAX_TEXT_LENGTH
            else comment.context[:defs.MAX_TEXT_LENGTH])

      for k in 'comment', 'updateComment':
        if k in pty:
          v = getattr(comment, k)
          if v:
            params[k.lower()] = (v
                if len(v) <= defs.MAX_TEXT_LENGTH
                else v[:defs.MAX_TEXT_LENGTH])
          else:
            params['del' + k.lower()] = True

    if not params:
      dwarn("warning: nothing change")
      return True

    params['login'] = userName
    params['password'] = password
    params['id'] = comment.id
    params['ver'] = self.version

    try:
      if not async:
        r = requests.post(API + '/comment/update', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/comment/update', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./comments/comment')
        commentId = int(el.get('id'))
        ok = commentId != 0
        #if ok:
        #  comment.clearDirtyProperties()
        dprint("comment id = %i" % commentId)
        return ok

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return False

  ## Terminology ##

  def getTerms(self, userName, password, init=True):
    """
    @param  userName  str
    @param  password  str
    @param* init  bool  whether init term object
    """
    params = {
      'ver': self.version,
      'login': userName,
      'password': password,
    }
    self._addBlockedLanguages(params)
    try:
      r = requests.get(API + '/term/list', params=params, headers=GZIP_HEADERS)
      if r.ok and _response_is_xml(r):
        context = etree.iterparse(StringIO(r.content), events=('start','end'))

        ret = []
        path = 0
        TYPES = dataman.Term.TYPES
        for event, elem in context:
          if event == 'start':
            path += 1
            if path == 3: # grimoire/terms/term
              kw = {
                #'gameId': 0,
                #'text': "",
                #'pattern': "",
                #'disabled': False,
                #'special': False,
                #'regex': False,
                ##'ignoresCase': False,
                ##'bbcode': False,
                #'comment': "",
                #'updateComment': "",
                #'updateTimestamp':  0,
                #'updateUserId': 0,
                'id': int(elem.get('id')),
                'type': elem.get('type'),
              }
          else:
            path -= 1
            if path == 3: # grimoire/terms/term
              tag = elem.tag
              text = elem.text
              if tag in ('gameId', 'userId', 'userHash', 'timestamp', 'updateUserId', 'updateTimestamp'):
                kw[tag] = int(text)
              elif tag in ('special', 'private', 'hentai', 'regex', 'disabled'):
                kw[tag] = text == 'true'
              else:
                kw[tag] = text or ''

            elif path == 2 and kw['type'] in TYPES:
              #if not kw.get('userHash'):
              #  kw['userHash'] = kw['userId']
              ret.append(dataman.Term(init=init, **kw))

        dprint("term count = %i" % len(ret))
        return ret

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url)
    except: pass

  def submitTerm(self, term, userName, password, async=False):
    """Return term and update term if succeeded"""
    assert userName and password, "missing user name or password"
    assert term, "term"

    params = {
      'ver': self.version,
      'login': userName,
      'password': password,
      'lang': term.language,
      'type': term.type,
    }
    if term.gameId:
      params['gameid'] = term.gameId
    elif term.gameMd5:
      params['md5'] = term.gameMd5

    if term.pattern:
      params['pattern'] = (term.pattern
          if len(term.pattern) <= defs.MAX_TEXT_LENGTH
          else term.pattern[:defs.MAX_TEXT_LENGTH])

    if term.text:
      params['text'] = (term.text
          if len(term.text) <= defs.MAX_TEXT_LENGTH
          else term.text[:defs.MAX_TEXT_LENGTH])

    if term.comment:
      params['comment'] = (term.comment
          if len(term.comment) <= defs.MAX_TEXT_LENGTH
          else term.comment[:defs.MAX_TEXT_LENGTH])

    if term.updateComment:
      params['updatecomment'] = (term.updateComment
          if len(term.updateComment) <= defs.MAX_TEXT_LENGTH
          else term.updateComment[:defs.MAX_TEXT_LENGTH])

    if term.special: params['special'] = True
    if term.private: params['private'] = True
    if term.hentai: params['hentai'] = True
    if term.regex: params['regex'] = True
    #if term.bbcode: params['bbcode'] = True
    #if term.ignoresCase: params['ignoreCase'] = True

    if term.deleted:
      # Should never happen. I mean, deleted subs should have been skipped in dataman
      params['del'] = True
    if term.disabled:
      params['disable'] = True

    try:
      if not async:
        r = requests.post(API + '/term/submit', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/term/submit', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./terms/term')

        # Be careful about async here
        term.id = int(el.get('id'))

        dprint("term id = %i" % term.id)
        return term.id

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return 0

  def updateTerm(self, term, userName, password, async=False):
    """Return if succeeded"""
    assert userName and password, "missing user name or password"
    assert term and term.id, "missing term id"

    params = {}
    pty = term.dirtyProperties()
    if not pty:
      dwarn("warning: term to update is not dirty")
      return True

    if 'deleted' in pty:        params['del'] = term.deleted
    if not term.deleted:
      if 'language' in pty:     params['lang'] = term.language
      if 'disabled' in pty:     params['disable'] = term.disabled

      for k in 'gameId', 'type', 'special', 'private', 'hentai', 'regex':
        if k in pty:
          params[k.lower()] = getattr(term, k)

      # Note: actually, there is no 'delpattern'
      for k in 'pattern', 'text', 'comment', 'updateComment':
        if k in pty:
          v = getattr(term, k)
          if v:
            params[k.lower()] = (v
                if len(v) <= defs.MAX_TEXT_LENGTH
                else v[:defs.MAX_TEXT_LENGTH])
          else:
            params['del' + k.lower()] = True

    if not params:
      dwarn("warning: nothing change")
      return True

    params['login'] = userName
    params['password'] = password
    params['id'] = term.id
    params['ver'] = self.version

    try:
      if not async:
        r = requests.post(API + '/term/update', data=params, headers=POST_HEADERS)
      else:
        r = skthreads.runsync(partial(
            requests.post,
            API + '/term/update', data=params, headers=POST_HEADERS),
            parent=self.q)

      if r.ok and _response_is_xml(r):
        root = etree.fromstring(r.content)
        el = root.find('./terms/term')
        termId = int(el.get('id'))
        ok = termId != 0
        #if ok:
        #  term.clearDirtyProperties()
        dprint("term id = %i" % termId)
        return ok

    #except socket.error, e:
    #  dwarn("socket error", e.args)
    except requests.ConnectionError, e:
      dwarn("connection error", e.args)
    except requests.HTTPError, e:
      dwarn("http error", e.args)
    except etree.ParseError, e:
      dwarn("xml parse error", e.args)
    except KeyError, e:
      dwarn("invalid response header", e.args)
    except (TypeError, ValueError, AttributeError), e:
      dwarn("xml malformat", e.args)
    except Exception, e:
      derror(e)

    dwarn("failed URL follows")
    try: dwarn(r.url, params)
    except: pass
    return False

class NetworkManager(QObject):
  """All methods are stateless and thread-safe"""

  def __init__(self, parent=None):
    super(NetworkManager, self).__init__(parent)
    self.__d = _NetworkManager(self)

  ## Properties ##

  onlineChanged = Signal(bool)
  def isOnline(self): return self.__d.online
  def updateOnline(self): self.__d.updateOnline()

  #def blockedLanguages(self, v): return self.__d.blockedLanguages
  def setBlockedLanguages(self, v): self.__d.blockedLanguages = v

  ## Update ##

  def setVersion(self, timestamp):
    """
    @param  timestamp  long
    """
    self.__d.version = timestamp

  def queryVersion(self):
    """Return the version timestamp. Result is cached.
    @return  int or 0

    Thread-safe.
    """
    return self.__d.cachedVersion if self.isOnline() else 0

  def getUpdateMessage(self, lang):
    """
    @param  lang  str
    @return  unicode or None

    Thread-safe.
    """
    if self.isOnline() and lang:
      return skthreads.runsync(partial(
          self.__d.getUpdateMessage, lang),
          parent=self)

  ## User ##

  def queryUser(self, userName, password):
    """
    @return  dataman.User or None

    Thread-safe.
    """
    if self.isOnline() and userName and password:
      return skthreads.runsync(partial(
          self.__d.queryUser, userName, password),
          parent=self)

  def updateUser(self, userName, password, language=None, gender=None, avatar=None, color=None, homepage=None):
    """
    @return  bool  if succeed

    Thread-safe.
    """
    if self.isOnline() and userName and password:
      return skthreads.runsync(partial(
          self.__d.updateUser, userName, password,
          language=language, gender=gender, avatar=avatar, color=color, homepage=homepage),
          parent=self)
    return False

  def getUsers(self):
    """
    @return {long id:dataman.UserDigest} or None
    """
    if self.isOnline():
      return skthreads.runsync(self.__d.getUsers, parent=self)

  ## Games ##

  def getGameFiles(self):
    """
    @return {long id:dataman.GameFile} or None
    """
    if self.isOnline():
      return skthreads.runsync(self.__d.getGameFiles, parent=self)

  def queryGame(self, id=0, md5=None, cached=True):
    """Either id or digest should be specified
    @param  cached  bool
    @return  dataman.Game or None

    Thread-safe.
    """
    d = self.__d
    if not id and not md5:
      return
    if cached or not self.isOnline():
      if id:
        try: return d.cachedGamesById[id]
        except KeyError: pass
      if md5:
        try: return d.cachedGamesByMd5[md5]
        except KeyError: pass

    if self.isOnline():
      ret = skthreads.runsync(partial(
          d.queryGame, id, md5),
          parent=self)
      if ret:
        d.cachedGamesById[ret.id] = ret
        d.cachedGamesById[ret.md5] = ret
      return ret

  def updateGame(self, game, userName, password, deleteHook=False):
    """Either id or digest should be specified.
    @param[inout]  game  dataman.Game  Update gameid If succeed,.
    @param  deleteHook  bool
    @return  bool  if succeed

    Thread-safe.
    """
    if self.isOnline() and (game.id or game.md5) and userName and password:
      return skthreads.runsync(partial(
          self.__d.updateGame, game, userName, password, deleteHook=deleteHook),
          parent=self)
    return False

  ## Items ##

  def getGameItems(self):
    """
    @return {long itemId:dataman.GameItem} or None
    """
    if self.isOnline():
      return skthreads.runsync(self.__d.getGameItems, parent=self)

  ## References ##

  #def getReferenceDigests(self):
  #  """
  #  @return {long itemId:[dataman.ReferenceDigest]} or None
  #  """
  #  if self.isOnline():
  #    return skthreads.runsync(self.__d.getReferenceDigests, parent=self)

  def queryReferences(self, gameId=0, md5=None, init=True):
    """Either gameid or digest should be specified
    @param  gameId  long or 0
    @param  md5  str or None
    @param* init  bool
    @return  [dataman.Reference] or None
    """
    if self.isOnline() and (gameId or md5):
      ret = skthreads.runsync(partial(
          self.__d.queryReferences, gameId, md5, init=False),
          parent=self)
      if ret and init:
        #map(dataman.Reference.init, ret)   # for loop is faster
        for it in ret: it.init()
      return ret

  def submitReference(self, ref, userName, password, md5=None, async=False):
    """Either id or digest should be specified.
    @param[inout]  ref  dataman.References  Update ref id If succeed,.
    @return  (int gameId, int itemId) or None

    Thread-safe.
    """
    if self.isOnline() and (ref.gameId or md5) and userName and password:
      return self.__d.submitReference(ref, userName, password, md5, async=async)

  def updateReference(self, ref, userName, password, async=False):
    """Reference id must be specified
    @param[in]  ref  dataman.Reference  Do NOT modify the ref
    @return  (int gameId, int itemId) or None

    Thread-safe.
    """
    if self.isOnline() and ref.id and userName and password:
      return self.__d.updateReference(ref, userName, password, async=async)

  ## Comments ##

  def queryComments(self, gameId=0, md5=None, hash=True):
    """Either gameid or digest should be specified
    @param  gameId  long or 0
    @param  md5  str or None
    @param* hash  bool
    @return  ({long:[dataman.Comment] if hash else [dataman.Comment]) or None
    """
    if self.isOnline() and (gameId or md5):
      ret = skthreads.runsync(partial(
          self.__d.queryComments, gameId, md5, init=False, hash=hash),
          parent=self)
      if ret:
        if hash:
          for l in ret.itervalues():
            #map(dataman.Comment.init, l) # for loop is faster
            for it in l: it.init()
        else:
          #map(dataman.Comment.init, ret) # for loop is faster
          for it in ret: it.init()
      return ret

  def submitComment(self, comment, userName, password, md5=None, async=False):
    """Either id or digest should be specified.
    @param[inout]  comment  dataman.Comment  Update comment id If succeed,.
    @return  long or 0   comment id if succeed

    Thread-safe.
    """
    #return (skthreads.runsync(partial(
    #    self.__d.submitComment, comment, userName, password, md5),
    #    parent=self)
    #    if self.isOnline() and (comment.gameId or md5) and userName and password
    #    else 0)
    if self.isOnline() and (comment.gameId or md5) and userName and password:
      return self.__d.submitComment(comment, userName, password, md5, async=async)
    return 0

  def updateComment(self, comment, userName, password, async=False):
    """Comment id must be specified
    @param[in]  comment  dataman.Comment  Do NOT modify the comment
    @return  bool  If succeed

    Thread-safe.
    """
    #return (skthreads.runsync(partial(
    #    self.__d.updateComment, comment, userName, password),
    #    parent=self)
    #    if self.isOnline() and comment.id and userName and password
    #    else False)
    if self.isOnline() and comment.id and userName and password:
      return self.__d.updateComment(comment, userName, password, async=async)
    return False

  ## Terms ##

  def getTerms(self, userName='', password=''):
    """
    @param* userName  str
    @param* password  str
    @return  [dataman.Term] or None
    """
    if self.isOnline():
      ret = skthreads.runsync(partial(
          self.__d.getTerms, userName, password, init=False),
          parent=self)
      if ret:
        #map(dataman.Term.init, ret) # for is faster
        for it in ret: it.init()
        ret.sort(key=operator.attrgetter('modifiedTimestamp'))
      return ret

  def submitTerm(self, term, userName, password, async=False):
    """Either id or digest should be specified.
    @param[inout]  term  dataman.Term  Update term id If succeed,.
    @return  long or 0   term id if succeed

    Thread-safe.
    """
    if self.isOnline() and userName and password:
      return self.__d.submitTerm(term, userName, password, async=async)
    return 0

  def updateTerm(self, term, userName, password, async=False):
    """Term id must be specified
    @param[in]  term  dataman.Term  Do NOT modify the term
    @return  bool  If succeed

    Thread-safe.
    """
    if self.isOnline() and term.id and userName and password:
      return self.__d.updateTerm(term, userName, password, async=async)
    return False

@memoized
def manager(): return NetworkManager()

# EOF
