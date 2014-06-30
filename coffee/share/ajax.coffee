###
ajax.coffee
5/1//2013 jichi
###

#define = @define ? require 'amdefine', @module
define ['jquery'], ($) ->
  postJSON: (data:data, url:url, success:success, error:error) ->
    $.ajax
      type: 'POST'
      contentType: 'application/json;charset=utf-8'
      dataType: 'json'
      data: JSON.stringify data
      url: url
      success: success
      error: error

  # encodeURI VS encodeURIComponent:
  # See: http://stackoverflow.com/questions/75980/best-practice-escape-or-encodeuri-encodeuricomponent
  encodeObject: (obj) -> # object -> string
    ("#{k}=#{encodeURIComponent v}" for k,v of obj).join '&'

  #get: (type, url, fn) =>
  #  ts  = util.datestamp()
  #  url += (if '?' in url then '&' else '?') + ts
  #  if type is 'json'
  #    $.getJSON url, fn
  #  else
  #    $.get url, fn, type

# EOF
