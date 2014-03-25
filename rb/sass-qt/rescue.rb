# compass-qt/rescue.rb
# 3/25/2014 jichi
# Disable error checking
require_relative 'config'

# https://groups.google.com/forum/#!topic/haml/fg9Pwzb20h4
# Enable percentage used as the alpha parameter in Qt CSS
# See: sass/util.rb
#module Sass::Util
#  alias _check_range check_range
#  def check_range(name, range, value, unit='') # @override
#    begin
#      return _check_range(name, range, value, unit)
#    rescue ArgumentError => msg
#      puts "jichi: ignore range check exception: #{msg}" if DEBUG
#      value = "#{value}%" if value.is_a? Numeric
#      return value
#    end
#  end
#end

# EOF
