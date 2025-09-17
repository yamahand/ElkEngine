#pragma once

// 使い方例:
// // ヘッダ/ソースで
// static const uint32_t TRANSFORM_TYPE_ID = ELK_REGISTER_TYPE("elk::ecs::Transform");
//
// // 別の翻訳単位でも同じ stable name を使えば同一の runtime id を取得できる。

#include "TypeRegistry.h"