#pragma once

#ifdef _WIN64
#include "xstdafx.hpp"
#else
#include <bits/stdc++.h>
#endif

#include <ankerl/unordered_dense.h>
#include <magic_enum/magic_enum_all.hpp>
#include <mimalloc.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#ifdef _WIN64
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <Windows.h>
#  endif
#endif
