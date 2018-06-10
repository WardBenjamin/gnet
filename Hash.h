/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
 * All rights reserved. */
#pragma once

#include "NumericTypes.h"

void Hash(void* out, u64 outlen, const void* data, u64 len, const void* key = 0, u64 keylen = 0, u64 domain = 0, u64 counter = 0);

