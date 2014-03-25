# compass-qt/color.rb
# 3/25/2014 jichi
# Disable error checking
require_relative 'config'

# https://groups.google.com/forum/#!topic/haml/fg9Pwzb20h4
# Enable percentage used as the alpha parameter in Qt CSS
# See: sass/script/color.rb
module Sass::Script
  class Color < Literal
    def rgba_str # @override  render alpha as percentage
      #delim = options[:style] == :compressed ? ',' : ', '
      delim = ','
      "rgba(#{rgb.join(delim)}#{delim}#{Number.round(alpha*100)}%)"
    end

    # Enable this if alpha could be a percentage string
    #def alpha? # override
    #  if alpha.is_a? Number
    #    alpha < 1
    #  elsif alpha.is_a? String
    #    alpha != '100%'
    #  else
    #    false
    #  end
    #end

  end
end

# EOF
