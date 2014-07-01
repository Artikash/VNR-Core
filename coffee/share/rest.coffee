###
rest.coffee
1/5/2014 jichi
###

#define 'jquery'

# libs/ajax.coffee

postJSON = (data:data, url:url, success:success, error:error) ->
  $.ajax
    type: 'POST'
    contentType: 'application/json;charset=utf-8'
    dataType: 'json'
    data: JSON.stringify data
    url: url
    success: success
    error: error

growl =
  showInternetError: ->
  showSignInError: ->
  showDupError: ->

#HOST = 'http://sakuradite.com'
HOST = 'http://153.121.54.194'

@rest = # export
  forum: # ajax/forum.coffee
    list: (type, data:data, success:success, error:error) ->
      postJSON
        url: "#{HOST}/json/#{type}/list"
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
      postJSON
        url: "#{HOST}/json/#{type}/query"
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
      postJSON
        url: "#{HOST}/json/#{type}/create"
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
      postJSON
        url: "#{HOST}/json/#{type}/update"
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
