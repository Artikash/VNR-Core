# coding: utf8
# rb.py
# 9/29/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

# Rules
# The expression is mostly borrowed from PERL.
#
# - Regular expression requires back-slashed quotes, such as /This is a regex/.
# - String with spaces require quotes, such as "a string with space".
# - $x: single tree node
# - @x: a list of sibling tree nodes
#
# Examples:
# (@x の)   => (@x 的)
# (@x で)   => (在 @x)
#
# @x の   => @x 的
# @x で   => 在 @x
#
# (@x=/.+/ の)  => (@x 的)
# (@x=/.+/ で)  => (在 @x)
#
# Parse tree:
# 私のことを好きですか
# (((私 の) (こと を)) (好きですか?))

if __name__ == '__main__':
  pass

# EOF
