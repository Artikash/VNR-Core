#pragma once

// aitalkutil.h
// 10/11/2014 jichi
#include "aitalk/aitalkapi.h"

namespace AITalk {

class AITalkUtil
{
  AITalkAPI api_;
  bool valid_;
public:
  /**
   *  @param  h  aitalked.dll module
   *
   *  Create Config and invoke AITalkAPI::Init.
   */
  explicit AITalkUtil(HMODULE h);

  /**
   *  Invoke AITalk::End.
   */
  ~AITalkUtil();

  bool IsValid() const { return valid_; }
};

} // namespace AITalk
