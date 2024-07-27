#include "global.hpp"

LwipCtx* LwipCtx::instance = nullptr;
LwipCtx::level LwipCtx::init_lvl = LwipCtx::level::none;