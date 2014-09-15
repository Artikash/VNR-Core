###
util.coffee
8/31/2014 jichi

beans:
- cacheBean: cacheman.CacheCoffeeBean
- i18nBean: coffeebean.I18nBean

require:
- bbcode
- moment
- underscore
- libs/linkify
###

#HOST = 'http://sakuradite.com'
HOST = 'http://153.121.54.194'

@tr = (text) -> i18nBean.tr text # string -> string

@cacheimg = (url) -> cacheBean.cacheImage url
@cacheurl = (url) -> cacheBean.cacheUrl url # string -> string

# Utilities

@util =
  fillObject: (dst, src) -> # object, object
    for k,v of dst
      delete dst[k]
    #_.extend pty, src
    for k,v of src
      dst[k] = src[k]

  getLangName: (lang) -> i18nBean.getLangShortName lang # string -> string

  getImageUrl: (data) -> # object -> string
    cacheimg "#{HOST}/upload/image/#{data.id}.#{data.suffix}"

  getAvatarUrl: (id, size=0) -> # string, int -> string
    unless id
      ''
    else unless size
      cacheimg "http://media.getchute.com/media/#{id}"
    else
      cacheimg "http://media.getchute.com/media/#{id}/#{size}x#{size}"

  renderContent: (t) -> # string -> string
    return '' unless t?
    bbcode.parse linkify _.escape t.replace /]\n/g, ']'
      .replace /<li><\/li>/g, '<li>'

  formatDate: (t, fmt='H:mm M/D/YY ddd') -> # long, string -> string
    try
      t *= 1000 if typeof(t) in ['number', 'string']
      if t then moment(t).format fmt else ''
      #date = @dateFromUnixTime date if typeof(date) in ['number', 'string']
      #dateformat date, fmt
    catch
      ''

# EOF
