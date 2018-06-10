/* Copyright © 2018, Mykos Hudson-Crisp <micklionheart@gmail.com>
* All rights reserved. */
#pragma once

#include "NumericTypes.h"

void StreamCipher(void* data, u64 length, const void* key, u64 keylength);
void BlockCipher(void* data, u64 length, const void* key, u64 keylength);
void BlockDecipher(void* data, u64 length, const void* key, u64 keylength);
