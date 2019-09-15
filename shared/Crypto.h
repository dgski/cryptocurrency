#pragma once

#include "Utils.h"

str rsa_signData(const void* data, size_t dataLen, const str& privateKey);
bool rsa_isSignatureValid(const void* data, size_t dataLen, const str& publicKey, const str& signature);