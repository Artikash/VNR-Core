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
# - $x:n Represent a noun, or use $x:v to represent a verb
#
# Examples:
# (@x の)   => (@x 的)
# (@x で)   => (在 @x)
#
# $x=/regex/:n   the :n means $x is a noun, similarly, $vt, $v
# $x=/regex/:n:v  either .noun or .verb
# It is just like CSS class selector, but the relation is or instead of and.
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
#
#
# Non-node group:   (?:A B C)  the "(?:" means it is not groupped??

if __name__ == '__main__':
  pass

# EOF
