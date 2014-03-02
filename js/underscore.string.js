// underscore.string.js
// 2/21/2014 jichi

//.pragma library // stateless

var defaultToWhiteSpace = function(characters) {
  if (characters == null)
    return '\\s';
  else if (characters.source)
    return characters.source;
  else
    return '[' + _s.escapeRegExp(characters) + ']';
};

function ltrim(str, characters){
  if (str == null) return '';
  //if (!characters && nativeTrimLeft) return nativeTrimLeft.call(str);
  characters = defaultToWhiteSpace(characters);
  return String(str).replace(new RegExp('^' + characters + '+'), '');
}

// EOF
