// underscore.js
// 2/21/2014 jichi

.pragma library // stateless

// Copied from coffeescript instead of underscore.js
function bind(fn, me){
  return function() {
    return fn.apply(me, arguments);
  };
}

// Slight different from underscore.js
// It can only take one parameter
function extend(obj, source) {
  if (source)
    for (var prop in source)
      obj[prop] = source[prop];
  return obj;
};


// EOF
