# coding: utf8
# cyrilchars.py
# 4/30/2015 jichi
from functools import partial
import unichars

"""
@param  unicode
@return  int
"""
findcyril = partial(unichars.findrange, start=unichars.ORD_CYRIL_FIRST, stop=unichars.ORD_CYRIL_LAST)

"""
@param  unicode
@return  bool
"""
anycyril = partial(unichars.ordany, start=unichars.ORD_CYRIL_FIRST, stop=unichars.ORD_CYRIL_LAST)

"""
@param  unicode
@return  bool
"""
allcyril = partial(unichars.ordall, start=unichars.ORD_CYRIL_FIRST, stop=unichars.ORD_CYRIL_LAST)

"""
@param  unicode
@return  bool
"""
iscyril = partial(unichars.charinrange, start=unichars.ORD_CYRIL_FIRST, stop=unichars.ORD_CYRIL_LAST)

# EOF
