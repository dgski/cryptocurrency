#pragma once

#include "Utils.h"

str rsa_signData(const void* data, size_t dataLen, const str& privateKeyFilename, const str& publicKeyFilename);
bool rsa_isSignatureValid(const void* data, size_t dataLen, const str& publicKeyFilename, const str& signature);