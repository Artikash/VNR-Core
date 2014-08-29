# chat.coffee
# 8/29/2014 jichi
# Required by chat.haml
#
# Beans:
# - i18nBean: coffeebean.I18nBean

dprint = -> console.log.apply console, arguments
#dprint = ->
timer = -> new choco.Timer arguments ...

HOST = 'http://sakuradite.com'

## Main ##

init = ->
  unless @gameBean?
    dprint 'init: wait'
    setTimeout init, 100 # Wait until bean is ready
  else
    dprint 'init: enter'

    #@GAME_ID = $('.game').data 'id' # global game item id

    #setTimeout gm.show, 200
    #setTimeout _.partial(quicksearch, styleClass, gf.refreshFilter),  2000
    dprint 'init: leave'

$ -> init()

# EOF
