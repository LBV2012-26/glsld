#pragma once

#ifdef _WIN64
#include "xstdafx.hpp"
#else
#include <bits/stdc++.h>
#endif

#include <ankerl/unordered_dense.h>
#include <nlohmann/json.hpp>

#ifdef _WIN64
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <Windows.h>
#  endif
#endif
