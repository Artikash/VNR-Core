###
forum.coffee
1/5/2014 jichi
###

#define = @define ? require 'amdefine', @module
define [
  'main/ajax'
  'main/defs'
  'main/growl'
  'main/tr'
], (ajax, defs, growl, tr) ->
  list: (type, data:data, success:success, error:error) ->
    ajax.postJSON
      url: "/json/#{type}/list"
      data: data
      success: (res) ->
        if res.status is 0 and res.data
          list = res.data
          console.log type, 'list: count =', list.length
          #growl.showEmptyList type unless list.length
          success? list
          return
        growl.showInternetError tr type
        error?()
      error: (xhr) ->
        console.warn type, 'error:', JSON.stringify xhr
        growl.showInternetError tr type
        error?()

  query: (type, data:data, success:success, error:error) ->
    ajax.postJSON
      url: "/json/#{type}/query"
      data: data
      success: (res) ->
        if res.status is 0 and res.data?.id
          obj = res.data
          console.log type, 'query: id =', obj.id
          success? obj
          return
        growl.showInternetError tr type
        error?()
      error: (xhr) ->
        console.warn type, 'error:', JSON.stringify xhr
        growl.showInternetError tr type
        error?()

  create: (type, data:data, success:success, error:error) ->
    ajax.postJSON
      url: "/json/#{type}/create"
      data: data
      success: (res) ->
        if res.status is 0 and res.data?.id
          obj = res.data
          console.log type, 'create: id =', obj.id
          success? obj
          return
        switch res.status
          when defs.STATUS_USER_ERR then growl.showSignInError()
          when defs.STATUS_DUP_ERR then growl.showDupError tr type
          else growl.showInternetError tr type
        error?()
      error: (xhr) ->
        console.warn type, 'error:', JSON.stringify xhr
        growl.showInternetError tr type
        error?()

  update: (type, data:data, success:success, error:error) ->
    ajax.postJSON
      url: "/json/#{type}/update"
      data: data
      success: (res) ->
        if res.status is 0 and res.data?.id
          obj = res.data
          console.log type, 'update: id =', obj.id
          success? obj
          return
        if res.status is defs.STATUS_USER_ERR
          growl.showSignInError()
        else
          growl.showInternetError tr type
        error?()
      error: (xhr) ->
        console.warn type, 'error:', JSON.stringify xhr
        growl.showInternetError tr type
        error?()

# EOF
