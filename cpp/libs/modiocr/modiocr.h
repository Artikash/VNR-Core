#pragma once

// modiocr.h
// 8/11/2014 jichi

#include <boost/function.hpp>

bool modiocr_available();

typedef boost::function<void (const wchar_t *)> modiocr_collect_fun_t;
bool modiocr_from_file(const wchar_t *path, const modiocr_collect_fun_t &fun);

// EOF
