# coding: utf8
# 9/27/2014 jichi
import config

def itercookies():
  """
  @return [{unicode k:unicode v}, [unicode url]]
  """
  return (
    # Must do for proxies

    ({'getchu_adalt_flag':'getchu.com'}, (config.PROXY_GETCHU_URL, "http://www.getchu.com")), # Getchu

    ({'afg':'0'}, ( # Tora no Ana
      config.PROXY_TORANOANA_URL, "http://www.toranoana.jp",
      config.PROXY_TORANOANA_IMG_URL, "http://img.toranoana.jp",
    )),

    # See: dmm.com/js/localized_welcome.js
    # $("div#welcome").click ($) ->
    #   expDayCklg.setTime expDayCklg.getTime() + (365 * 1000 * 60 * 60 * 24)
    #   document.cookie = "cklg=welcome;domain=" + localizeDomain + ";path=/;expires=" + expDayCklg.toGMTString()
    #   return
    #({'cklg':'welcome', 'ckcy':'2'}, (

    # This will get dmm blocked
    ({'cklg':'ja', 'ckcy':'1'}, (
      config.PROXY_DMM_JP_URL, "http://www.dmm.co.jp",
      config.PROXY_DMM_COM_URL, "http://www.dmm.com",
      #config.PROXY_DMM_COM_P_URL, "http://p.dmm.com",
    )),

    # Optional

    ({'adultchecked':'1'}, (config.PROXY_DLSITE_URL, "http://www.dlsite.com")), # DLsite
    ({'adult_check':'1'}, ("http://www.digiket.com",)), # DiGiket
    ({'adult_check_flag':'1'}, ("http://www.gyutto.com",)), # Gyutto
  )

# EOF
