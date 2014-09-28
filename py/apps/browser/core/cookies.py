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

    # FIXME: Not working for proxy

    ({'cklg':'ja', 'ckcy':'2'}, (config.PROXY_DMM_URL, "http://www.dmm.co.jp")), # DMM

    # Optional

    ({'adultchecked':'1'}, (config.PROXY_DLSITE_URL, "http://www.dlsite.com")), # DLsite
    ({'adult_check':'1'}, ("http://www.digiket.com",)), # DiGiket
    ({'adult_check_flag':'1'}, ("http://www.gyutto.com",)), # Gyutto
  )

# EOF
